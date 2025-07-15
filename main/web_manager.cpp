#include <iostream>
#include <fstream>

#include "esp_log.h"
#include "nvs_flash.h"

#include "web_manager.h"
#include "context.h"
#include "utils/webpage.h"

std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hex_val;
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &hex_val);
            result += static_cast<char>(hex_val);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

esp_err_t WebManager::root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebManager::trace_full_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    context_ptr->trace_state.mutex.lock_read();
    context_ptr->gnss_state.mutex.lock_read();
    nlohmann::json trace_response = {
        {"in_track", context_ptr->enable_track},
        {"distance", context_ptr->trace_state.get_distance()},
        {"speed", context_ptr->gnss_state.ground_speed},
        {"course", context_ptr->gnss_state.course}
    };
    context_ptr->gnss_state.mutex.unlock_read();
    trace_response["trace"] = context_ptr->trace_state.get_waypoints();
    context_ptr->trace_state.mutex.unlock_read();

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, trace_response.dump(4).c_str(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebManager::trace_recent_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    context_ptr->trace_state.mutex.lock_read();
    context_ptr->gnss_state.mutex.lock_read();
    nlohmann::json trace_response = {
        {"in_track", context_ptr->enable_track},
        {"distance", context_ptr->trace_state.get_distance()},
        {"speed", context_ptr->gnss_state.ground_speed},
        {"course", context_ptr->gnss_state.course}
    };
    context_ptr->gnss_state.mutex.unlock_read();
    trace_response["trace"] = context_ptr->trace_state.get_last_waypoints();
    context_ptr->trace_state.mutex.unlock_read();

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, trace_response.dump(4).c_str(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebManager::satellites_get_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    nlohmann::json satellites_response;
    satellites_response["satellites"] = nlohmann::json::array();

    context_ptr->gnss_state.mutex.lock_read();
    for (const Satellite& sat : context_ptr->gnss_state.satellites) {
        nlohmann::json jsat;
        jsat["type"] = sat.sat_type;
        jsat["nr"] = sat.nr;
        jsat["elevation"] = sat.elevation;
        jsat["azimuth"] = sat.azimuth;
        jsat["snr"] = sat.snr;
        jsat["in_use"] = sat.in_use;
        satellites_response["satellites"].push_back(jsat);
    }
    context_ptr->gnss_state.mutex.unlock_read();
    std::string resp_str = satellites_response.dump();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str.c_str(), resp_str.length());
    return ESP_OK;
}

esp_err_t WebManager::sdcard_files_handler(httpd_req_t *req) {
    nlohmann::json jresp;
    jresp["files"] = nlohmann::json::array();

    std::string sd_path = MOUNT_POINT;

    for (const auto& entry : std::filesystem::directory_iterator(sd_path)) {
        if (entry.is_regular_file()) {
            const std::string filename = entry.path().filename().string();
            const size_t ext_pos = filename.rfind('.');

            if (ext_pos != std::string::npos && 
                filename.substr(ext_pos) == ".GPX") {
                nlohmann::json jfile;
                jfile["name"] = filename;
                jfile["size"] = entry.file_size();
                jresp["files"].push_back(jfile);
            }
        }
    }

    std::string resp_str = jresp.dump();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str.c_str(), resp_str.length());
    return ESP_OK;
}

esp_err_t WebManager::download_file_handler(httpd_req_t *req) {
    char query_str[256];
    if (httpd_req_get_url_query_str(req, query_str, sizeof(query_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
        return ESP_FAIL;
    }
    ESP_LOGI(WEB_TAG, "query file %s", query_str);

    const char* file_param = strstr(query_str, "file=");
    if (!file_param) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'file' parameter");
        return ESP_FAIL;
    }
    
    std::string file_name = urlDecode(file_param + 5);
    std::string file_path = MOUNT_POINT + std::string("/") + file_name;
    ESP_LOGI(WEB_TAG, "try load file %s %s", file_name.c_str(), file_path.c_str());

    FILE* file = fopen(file_path.c_str(), "rb");
    if (file == NULL) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char content_disposition[256];
    snprintf(content_disposition, sizeof(content_disposition), 
            "attachment; filename=\"%s\"", file_name.c_str());

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Content-Disposition", content_disposition);
    httpd_resp_set_hdr(req, "Content-Length", std::to_string(file_size).c_str());
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");

    const size_t buffer_size = 1024;
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        fclose(file);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, buffer_size, file)) > 0) {
        esp_err_t err = httpd_resp_send_chunk(req, buffer, bytes_read);
        if (err != ESP_OK) {
            break;
        }
    }

    free(buffer);
    fclose(file);

    httpd_resp_send_chunk(req, NULL, 0);
    if (bytes_read == 0) {
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t WebManager::delete_file_handler(httpd_req_t *req) {
    char query_str[256];
    if (httpd_req_get_url_query_str(req, query_str, sizeof(query_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
        return ESP_FAIL;
    }
    ESP_LOGI(WEB_TAG, "query delete file %s", query_str);

    const char* file_param = strstr(query_str, "file=");
    if (!file_param) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'file' parameter");
        return ESP_FAIL;
    }
    
    std::string file_name = urlDecode(file_param + 5);
    std::string file_path = MOUNT_POINT + std::string("/") + file_name;
    ESP_LOGI(WEB_TAG, "try delete file %s %s", file_name.c_str(), file_path.c_str());

    if (std::filesystem::remove(file_path)) {
        httpd_resp_set_type(req, "application/json");
        nlohmann::json response = {{"status", "success"}, {"message", "File deleted successfully"}};
        return httpd_resp_send(req, response.dump(4).c_str(), HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found or could not be deleted");
        return ESP_FAIL;
    }
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
    
    httpd_uri_t trace_full_uri = {
        .uri = "/trace_full",
        .method = HTTP_GET,
        .handler = trace_full_handler,
        .user_ctx = context_ptr,
    };
    httpd_register_uri_handler(server, &trace_full_uri);

    httpd_uri_t trace_recent_uri = {
        .uri = "/trace_recent",
        .method = HTTP_GET,
        .handler = trace_recent_handler,
        .user_ctx = context_ptr,
    };
    httpd_register_uri_handler(server, &trace_recent_uri);

    httpd_uri_t satellites_uri = {
        .uri = "/satellites",
        .method = HTTP_GET,
        .handler = satellites_get_handler,
        .user_ctx = context_ptr
    };
    httpd_register_uri_handler(server, &satellites_uri);

    httpd_uri_t sdcard_files_uri = {
        .uri       = "/sdcard_files",
        .method    = HTTP_GET,
        .handler   = sdcard_files_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &sdcard_files_uri);

    httpd_uri_t download_file_uri = {
        .uri       = "/download",
        .method    = HTTP_GET,
        .handler   = download_file_handler,
        .user_ctx  = context_ptr
    };
    httpd_register_uri_handler(server, &download_file_uri);

    httpd_uri_t delete_file_uri = {
        .uri       = "/delete",
        .method    = HTTP_GET,
        .handler   = delete_file_handler,
        .user_ctx  = context_ptr
    };
    httpd_register_uri_handler(server, &delete_file_uri);
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

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, this, NULL));

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

void WebManager::stop_webserver() {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

std::string WebManager::get_ip() const {
    Context *context = static_cast<Context*>(context_ptr);
    return context->ip;
}