#include "esp_log.h"
#include "nvs_flash.h"

#include "web_manager.h"
#include "context.h"
#include "utils/webpage.h"

esp_err_t WebManager::root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebManager::trace_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    context_ptr->trace_state.lock();

    nlohmann::json trace_response = {
        {"in_track", context_ptr->enable_track},
        {"distance", context_ptr->trace_state.get_distance()}
    };
    trace_response["trace"] = context_ptr->trace_state.get_waypoints();
    context_ptr->trace_state.unlock();

    httpd_resp_set_type(req, "text/json");
    return httpd_resp_send(req, trace_response.dump(4).c_str(), HTTPD_RESP_USE_STRLEN);
}

void WebManager::event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    WebManager* web_manager = static_cast<WebManager*>(arg);
    Context *context_ptr = static_cast<Context*>(web_manager->context_ptr);
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        web_manager->stop_webserver();
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        char buffer[40];
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(buffer, sizeof(buffer), IPSTR, IP2STR(&event->ip_info.ip));
        context_ptr->ip = buffer;
        ESP_LOGI(WEB_TAG, "connect wifi, got ip: %s", context_ptr->ip.c_str());

        if (web_manager->start_webserver() != ESP_OK) {
            ESP_LOGE(WEB_TAG, "Failed to start web server!");
        }
    }
}

void WebManager::register_uri_handlers() {
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

esp_err_t WebManager::start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    
    ESP_LOGI(WEB_TAG, "Starting web server on port: %d", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        register_uri_handlers();
        return ESP_OK;
    }
    
    ESP_LOGE(WEB_TAG, "Error starting web server!");
    return ESP_FAIL;
}

esp_err_t WebManager::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件处理程序
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, NULL));

    // 初始化STA网络接口
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    Context *context = static_cast<Context*>(context_ptr);
    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold = {
                .rssi = -127,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .rssi_5g_adjustment = 0,
            },
        },
    };

    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), 
            context->wifi_ssid.c_str(), 
            sizeof(wifi_config.sta.ssid) - 1);
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), 
            context->wifi_passwd.c_str(), 
            sizeof(wifi_config.sta.password) - 1);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    return ESP_OK;
}

// 停止Web服务器
void WebManager::stop_webserver() {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

// 获取当前IP地址
std::string WebManager::get_ip() const {
    Context *context = static_cast<Context*>(context_ptr);
    return context->ip;
}