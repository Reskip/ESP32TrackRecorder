#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_heap_caps.h"
#include "esp_psram.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "sdmmc_cmd.h"

#include "minmea.h"
#include "page/page.h"
#include "display_manager.h"
#include "web_manager.h"

#include "status/battery_state.h"
#include "status/gnss_state.h"
#include "status/encoder_state.h"
#include "status/trace_state.h"

#include "utils/json.hpp"
#include "utils/utils.h"

#define MAIN_TAG "Main"
#define CONFIG_FILE "CONFIG.TXT"

void load_config(Context &context) {
    using json = nlohmann::json;
    const std::string conf_file_path = MOUNT_POINT "/" CONFIG_FILE;

    if (!std::filesystem::exists(conf_file_path)) {
        const json config = {
            {"timezone", 8},
            {"wifi_ssid", ""},
            {"wifi_passwd", ""}
        };
        
        std::ofstream(conf_file_path) << config.dump(4);
        return;
    }

    std::ifstream file(conf_file_path);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件!" << std::endl;
        return;
    }

    json config;
    file >> config;

    if (config.is_discarded()) {
        std::cerr << "JSON格式错误!" << std::endl;
        return;
    }

    context.timezone = config.value("timezone", 8);
    context.wifi_ssid = config.value("wifi_ssid", "");
    context.wifi_passwd = config.value("wifi_passwd", "");
}

void mount_sdcard_spi() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_17);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    if (gpio_get_level(GPIO_NUM_17) == 0) {
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

	esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_cnf, &mount_cnf, &card);
	sdmmc_card_print_info(stdout, card);
}

DisplayManager display_manager(GPIO_NUM_2, GPIO_NUM_1);
GNSSState gnss_state(GPIO_NUM_21, GPIO_NUM_47);
Battery battery_state(GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_6);
Encoder encoder_state(GPIO_NUM_9, GPIO_NUM_3, GPIO_NUM_46);
Trace trace_state;

Context context(gnss_state, battery_state, encoder_state, trace_state);
WebManager web_manager(&context);

extern "C" void app_main() {
    context.start_ts_ms = esp_timer_get_time() / 1000;
    srand((unsigned int) time(NULL));
    mount_sdcard_spi();
    load_config(context);

    web_manager.init();

    gpio_reset_pin(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 1);
    gpio_install_isr_service(0);
    encoder_state.init();
    gnss_state.init();

    xTaskCreate([](void* param) {
        while (true) {
            monitorSystemResources(context);
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }, "SystemMonitorTask", 4096, NULL, 1, NULL);

    xTaskCreate([](void* param) {
        while (true) {
            gnss_state.parse();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }, "UpdateGNSSState", 4096, NULL, 1, NULL);

    if (!display_manager.init()) {
        ESP_LOGE(MAIN_TAG, "Failed to initialize OLED");
        return;
    }

    while (1) {
        encoder_state.encoder_press_handler();
        // gnss_state.print_debug_info();
        // battery_state.print_debug_info();
        display_manager.updateDisplay(context);

        if (context.fresh_cnt && rand() % 10 == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        context.gnss_state.mutex.lock_read();
        if (context.enable_track && context.fresh_cnt == 0 && context.gnss_state.valid) { // every second
            trace_state.add_waypoint(context.gnss_state);
        }
        context.gnss_state.mutex.unlock_read();

        if (!context.enable_track) {
            trace_state.try_close_trace();
        }
    }
}
