#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdmmc_cmd.h"

#include "minmea.h"
#include "page/page.h"
#include "display_manager.h"

#include "status/battery_state.h"
#include "status/gnss_state.h"
#include "status/encoder_state.h"
#include "status/trace_state.h"

#define MAIN_TAG "Main"
#define MOUNT_POINT "/spiflash"

void monitorSystemResources(Context &context) {
    UBaseType_t stack_high_watermark = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(MAIN_TAG, "Task Stack High Water Mark: %d bytes (minimum remaining stack)", stack_high_watermark);

    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_8BIT);
    size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    ESP_LOGI(MAIN_TAG, "Heap Memory: Free=%d bytes, Total=%d bytes, Largest Block=%d bytes",
             free_heap, total_heap, largest_block);

    uint64_t total_bytes, free_bytes;
    esp_err_t err = esp_vfs_fat_info(MOUNT_POINT, &total_bytes, &free_bytes);
    if (err == ESP_OK) {
        ESP_LOGI("FATFS", "Total size: %llu bytes", total_bytes);
        ESP_LOGI("FATFS", "Free space: %llu bytes", free_bytes);
    } else {
        ESP_LOGE("Main", "Failed to get file system info: %s", esp_err_to_name(err));
    }

    while (xSemaphoreTake(context.storage_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {}
    context.ram_total = total_heap;
    context.ram_used = total_heap - free_heap;
    context.flash_total = total_bytes;
    context.flash_used = total_bytes - free_bytes;
    xSemaphoreGive(context.storage_mutex);
}

void mount_sdcard_spi() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_17);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;  // 启用内部上拉
    gpio_config(&io_conf);
    
    // 检测SD卡是否插入
    if (gpio_get_level(GPIO_NUM_17) == 0) {  // 低电平表示卡插入
        ESP_LOGI(MAIN_TAG, "SD card detected, proceeding to mount...");
    } else {
        ESP_LOGE(MAIN_TAG, "SD card not detected, please insert the card");
        return;
    }

	sdmmc_host_t host=SDSPI_HOST_DEFAULT();
	host.slot=SPI2_HOST;
	spi_bus_config_t bus_cnf={
		.mosi_io_num = GPIO_NUM_10,
        .miso_io_num = GPIO_NUM_18,
        .sclk_io_num = GPIO_NUM_11,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1,
	};
	spi_bus_initialize(SPI2_HOST, &bus_cnf, SPI_DMA_CH_AUTO);
	static sdspi_device_config_t slot_cnf={
        .host_id=SPI2_HOST,
		.gpio_cs=GPIO_NUM_8,
		.gpio_cd=SDSPI_SLOT_NO_CD,
		.gpio_wp=GPIO_NUM_NC,
		.gpio_int=GPIO_NUM_NC,
    };
	sdmmc_card_t *card;
	esp_vfs_fat_sdmmc_mount_config_t mount_cnf={
		.format_if_mount_failed=false,
		.max_files=5,
		.allocation_unit_size=16*1024,
	};

	esp_vfs_fat_sdspi_mount(MOUNT_POINT,&host,&slot_cnf,&mount_cnf,&card);
	sdmmc_card_print_info(stdout,card);
}

DisplayManager display(GPIO_NUM_2, GPIO_NUM_1);
GNSSState gnss_state(GPIO_NUM_21, GPIO_NUM_47);
Battery battery_state(GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_5);
Encoder encoder_state(GPIO_NUM_9, GPIO_NUM_3, GPIO_NUM_46);
Trace trace_state;

Context context(gnss_state, battery_state, encoder_state, trace_state);

extern "C" void app_main() {
    context.start_ts_ms = esp_timer_get_time() / 1000;
    mount_sdcard_spi();

    gpio_reset_pin(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 1);
    gpio_install_isr_service(0);
    encoder_state.init();
    gnss_state.init();

    xTaskCreate([](void* param) {
        while (true) {
            monitorSystemResources(context);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }, "SystemMonitorTask", 4096, NULL, 1, NULL);

    xTaskCreate([](void* param) {
        while (true) {
            gnss_state.parse();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }, "UpdateGNSSState", 4096, NULL, 1, NULL);

    if (!display.init()) {
        ESP_LOGE("OLED", "Failed to initialize OLED");
        return;
    }

    while (1) {
        encoder_state.encoder_press_handler();
        // gnss_state.print_debug_info();
        // battery_state.print_debug_info();
        display.updateDisplay(context);

        if (context.enable_track && context.fresh_cnt == 0 && context.gnss_state.valid) { // every second
            trace_state.add_waypoint(context.gnss_state);
        }
    }
}
