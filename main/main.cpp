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
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "lwip/ip4_addr.h"
#include "sdmmc_cmd.h"

#include "minmea.h"
#include "page/page.h"
#include "display_manager.h"

#include "status/battery_state.h"
#include "status/gnss_state.h"
#include "status/encoder_state.h"
#include "status/trace_state.h"

#include "utils/json.hpp"

#define MAIN_TAG "Main"
#define MOUNT_POINT "/spiflash"
#define CONFIG_FILE "CONFIG.TXT"

void load_config(Context &context) {
    std::string conf_file_path = std::string(MOUNT_POINT) + "/" + CONFIG_FILE;

    if (!std::filesystem::exists(conf_file_path)) {
        std::cout << "Config file not found. Creating with default values..." << std::endl;

        std::ofstream file(conf_file_path);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot create config file!" << std::endl;
            return;
        }
        
        context.timezone = 8;
        context.wifi_ssid = "";
        context.wifi_passwd = "";
        file << "timezone=" << context.timezone << "\n";
        file << "wifi_ssid=" << context.wifi_ssid << "\n";
        file << "wifi_passwd=" << context.wifi_passwd << "\n";
        file.close();
        return;
    }

    std::ifstream file(conf_file_path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file!" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "timezone") {
            context.timezone = std::stoi(value);
        } else if (key == "wifi_ssid") {
            context.wifi_ssid = value;
        } else if (key == "wifi_passwd") {
            context.wifi_passwd = value;
        }
    }

    file.close();
    return;
}

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
        ESP_LOGI(MAIN_TAG, "Total size: %llu bytes", total_bytes);
        ESP_LOGI(MAIN_TAG, "Free space: %llu bytes", free_bytes);
    } else {
        ESP_LOGE(MAIN_TAG, "Failed to get file system info: %s", esp_err_to_name(err));
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

	esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_cnf, &mount_cnf, &card);
	sdmmc_card_print_info(stdout, card);
}

DisplayManager display(GPIO_NUM_2, GPIO_NUM_1);
GNSSState gnss_state(GPIO_NUM_21, GPIO_NUM_47);
Battery battery_state(GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_6);
Encoder encoder_state(GPIO_NUM_9, GPIO_NUM_3, GPIO_NUM_46);
Trace trace_state;

Context context(gnss_state, battery_state, encoder_state, trace_state);

static httpd_handle_t server = NULL;

static esp_err_t root_handler(httpd_req_t *req) {
    const char *html = R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Tracker - 高德地图版</title>
    <!-- 引入Leaflet CSS和JS -->
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" integrity="sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=" crossorigin=""/>
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js" integrity="sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=" crossorigin=""></script>
    <style>
        body, html {
            margin: 0;
            padding: 0;
            height: 100%;
            font-family: "Microsoft YaHei", Arial, sans-serif;
        }
        #map {
            width: 100%;
            height: calc(100vh - 80px);
        }
        .header {
            padding: 10px 20px;
            background-color: #f0f0f0;
            border-bottom: 1px solid #ddd;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .info-panel {
            background-color: white;
            padding: 10px;
            border-radius: 5px;
            box-shadow: 0 1px 5px rgba(0,0,0,0.4);
            margin: 10px;
        }
        .btn {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 8px 16px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 14px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        .btn-refresh {
            background-color: #008CBA;
        }
        .status {
            margin: 0 10px;
            font-weight: bold;
        }
        .status.connected {
            color: green;
        }
        .status.disconnected {
            color: red;
        }
        .tile-note {
            position: absolute;
            bottom: 10px;
            left: 10px;
            background-color: rgba(255,255,255,0.8);
            padding: 5px 10px;
            border-radius: 4px;
            font-size: 12px;
            z-index: 1000;
        }
        .coord-note {
            position: absolute;
            bottom: 35px;
            left: 10px;
            background-color: rgba(255,255,255,0.8);
            padding: 5px 10px;
            border-radius: 4px;
            font-size: 12px;
            z-index: 1000;
        }
    </style>
</head>
<body>
    <div class="header">
        <h3>ESP32 TRACKER</h3>
        <div>
            <span class="status disconnected">未连接</span>
            <button id="refreshBtn" class="btn btn-refresh">刷新轨迹</button>
        </div>
    </div>
    <div class="info-panel">
        <span id="distanceInfo">距离: -- km</span>
        <span id="statusInfo">状态: --</span>
    </div>
    <div id="map"></div>

    <script>
        // WGS84到GCJ-02坐标转换算法
        const transform = {
            // 定义转换所需常量
            X_PI: 3.14159265358979324 * 3000.0 / 180.0,
            PI: 3.1415926535897932384626,
            A: 6378245.0,
            EE: 0.00669342162296594323,
            
            // WGS84转GCJ-02
            wgs84ToGcj02(lng, lat) {
                let dlat = this.transformLat(lng - 105.0, lat - 35.0);
                let dlng = this.transformLon(lng - 105.0, lat - 35.0);
                let radlat = lat / 180.0 * this.PI;
                let magic = Math.sin(radlat);
                magic = 1 - this.EE * magic * magic;
                let sqrtmagic = Math.sqrt(magic);
                dlat = (dlat * 180.0) / ((this.A * (1 - this.EE)) / (magic * sqrtmagic) * this.PI);
                dlng = (dlng * 180.0) / (this.A / sqrtmagic * Math.cos(radlat) * this.PI);
                let mglat = lat + dlat;
                let mglng = lng + dlng;
                return [mglng, mglat];
            },
            
            // 辅助转换函数
            transformLat(x, y) {
                let ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * Math.sqrt(Math.abs(x));
                ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0;
                ret += (20.0 * Math.sin(y * this.PI) + 40.0 * Math.sin(y / 3.0 * this.PI)) * 2.0 / 3.0;
                ret += (160.0 * Math.sin(y / 12.0 * this.PI) + 320 * Math.sin(y * this.PI / 30.0)) * 2.0 / 3.0;
                return ret;
            },
            
            transformLon(x, y) {
                let ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * Math.sqrt(Math.abs(x));
                ret += (20.0 * Math.sin(6.0 * x * this.PI) + 20.0 * Math.sin(2.0 * x * this.PI)) * 2.0 / 3.0;
                ret += (20.0 * Math.sin(x * this.PI) + 40.0 * Math.sin(x / 3.0 * this.PI)) * 2.0 / 3.0;
                ret += (150.0 * Math.sin(x / 12.0 * this.PI) + 300.0 * Math.sin(x / 30.0 * this.PI)) * 2.0 / 3.0;
                return ret;
            }
        };
        
        // 初始化地图（北京默认中心点）
        const map = L.map('map').setView([39.9042, 116.4074], 12);
        
        // 替换为高德地图瓦片源（无需API密钥）
        const gaodeTileLayer = L.tileLayer('https://webrd0{s}.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x={x}&y={y}&z={z}', {
            subdomains: '1234',       // 多子域名加载优化
            attribution: '© 高德地图', // 版权声明
            maxZoom: 18,             // 最大缩放级别
            minZoom: 3               // 最小缩放级别
        }).addTo(map);
        
        // 轨迹图层组
        const trackLayer = L.layerGroup().addTo(map);
        
        // 当前轨迹线
        let currentTrack = null;
        // 轨迹点标记
        let trackMarkers = [];

        function convertWgs84ToGcj02(points) {
            return points.map(point => {
                const [lng, lat] = transform.wgs84ToGcj02(point.lon, point.lat);
                return {
                    lon: lng,
                    lat: lat
                };
            });
        }
        
        function loadTrackData() {
            fetch('/trace')  // 注意：实际使用时需确保该接口返回正确数据格式
                .then(response => {
                    if (!response.ok) {
                        throw new Error('网络请求失败');
                    }
                    return response.json();
                })
                .then(data => {
                    // 更新连接状态
                    document.querySelector('.status').textContent = '已连接';
                    document.querySelector('.status').className = 'status connected';
                    document.getElementById('distanceInfo').textContent = `距离: ${(data.distance * 1000).toFixed(2)} 米`;
                    document.getElementById('statusInfo').textContent = `状态: ${data.in_track ? '记录中' : '未在记录'}`;
                    
                    // 清除旧轨迹
                    clearTrack();
                    
                    const wgs84Points = data.trace;
                    if (wgs84Points.length > 0) {
                        // 调用转换函数
                        const gcj02Points = convertWgs84ToGcj02(wgs84Points);
                        
                        // 绘制轨迹线（使用转换后的坐标）
                        currentTrack = L.polyline(gcj02Points, {
                            color: '#ff4500',       // 橙色轨迹线
                            weight: 4,              // 线宽
                            opacity: 0.8,           // 透明度
                            lineJoin: 'round',      // 圆角连接
                            className: 'track-line' // 自定义类名
                        }).addTo(trackLayer);
                    }
                    if (points.length == 1) {
                        map.fitBounds(currentTrack.getBounds(), { padding: [50, 50] });
                    }
                })
                .catch(error => {
                    console.error('获取轨迹数据失败:', error);
                    document.querySelector('.status').textContent = '连接失败';
                    document.querySelector('.status').className = 'status disconnected';
                });
        }
        
        // 清除轨迹数据
        function clearTrack() {
            if (currentTrack) {
                trackLayer.removeLayer(currentTrack);
                currentTrack = null;
            }
            trackMarkers.forEach(marker => trackLayer.removeLayer(marker));
            trackMarkers = [];
        }
        
        // 初始化加载
        loadTrackData();
        
        // 绑定刷新按钮事件
        document.getElementById('refreshBtn').addEventListener('click', loadTrackData);
        
        // 定时自动刷新（5秒）
        setInterval(loadTrackData, 5000);
    </script>
</body>
</html>)";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

// 系统信息路径处理函数
static esp_err_t trace_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;

    context_ptr->trace_state.lock();

    nlohmann::json trace_response = {
        {"in_track", context_ptr->enable_track},
        {"distance", context_ptr->trace_state.get_distance()},
        {"trace", nlohmann::json::array()}
    };

    for (auto &point: context_ptr->trace_state.get_waypoints()) {
        nlohmann::json point_json = {
            {"lat", point.latitude},
            {"lon", point.longitude}
        };
        trace_response["trace"].push_back(point_json);
    }
    context_ptr->trace_state.unlock();

    httpd_resp_set_type(req, "text/json");
    return httpd_resp_send(req, trace_response.dump(4).c_str(), HTTPD_RESP_USE_STRLEN);
}


// 注册URI处理程序
static void register_uri_handlers(Context *context_ptr) {
    httpd_uri_t uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = context_ptr,
    };
    httpd_register_uri_handler(server, &uri);
    
    httpd_uri_t trace_uri = {
        .uri = "/trace",
        .method = HTTP_GET,
        .handler = trace_handler,
        .user_ctx = context_ptr,
    };
    httpd_register_uri_handler(server, &trace_uri);
}

// 启动Web服务器
static esp_err_t start_webserver(Context *context_ptr) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80; // 使用默认HTTP端口
    
    ESP_LOGI(MAIN_TAG, "Starting web server on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        register_uri_handlers(context_ptr);
        return ESP_OK;
    }
    
    ESP_LOGE(MAIN_TAG, "Error starting web server!");
    return ESP_FAIL;
}

// 停止Web服务器
static void stop_webserver(Context *context_ptr) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    Context *context_ptr = (Context*) arg;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char buffer[40];
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(buffer, sizeof(buffer), IPSTR, IP2STR(&event->ip_info.ip));
        context_ptr->ip = buffer;
        ESP_LOGI(MAIN_TAG, "connect wifi, got ip: %s", context_ptr->ip.c_str());

        if (start_webserver(context_ptr) != ESP_OK) {
            ESP_LOGE(MAIN_TAG, "Failed to start web server!");
        }
    }
}

/* Initialize Wi-Fi as sta and set scan method */
static void fast_scan(Context &context)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, &context, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, &context, NULL));

    // Initialize default station as network interface instance (esp-netif)
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Initialize and start WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "RESKIP",
            .password = "bibibibi",
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold = {
                .rssi = -127,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .rssi_5g_adjustment = 0,
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
}


extern "C" void app_main() {
    context.start_ts_ms = esp_timer_get_time() / 1000;
    srand((unsigned int) time(NULL));
    mount_sdcard_spi();
    load_config(context);


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    fast_scan(context);

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

    if (!display.init()) {
        ESP_LOGE(MAIN_TAG, "Failed to initialize OLED");
        return;
    }

    while (1) {
        encoder_state.encoder_press_handler();
        // gnss_state.print_debug_info();
        // battery_state.print_debug_info();
        display.updateDisplay(context);

        if (context.fresh_cnt && rand() % 10 == 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        context.gnss_state.lock();
        if (context.enable_track && context.fresh_cnt == 0 && context.gnss_state.valid) { // every second
            trace_state.add_waypoint(context.gnss_state);
        }
        context.gnss_state.unlock();

        if (!context.enable_track) {
            trace_state.try_close_trace();
        }
    }
}
