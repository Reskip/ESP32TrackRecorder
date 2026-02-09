#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_crt_bundle.h"

#include "context.h"
#include "web_manager.h"
#include "utils/webpage.h"

std::string readRequestBody(httpd_req_t *req) {
    std::string body;
    body.resize(req->content_len);
    int remaining = req->content_len;
    int offset = 0;

    while (remaining > 0) {
        int ret = httpd_req_recv(req, body.data() + offset, remaining);
        if (ret <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            return "";
        }
        offset += ret;
        remaining -= ret;
    }

    return body;
}

std::string urlEncode(const std::string& value) {
    static const char* hex = "0123456789ABCDEF";
    std::string escaped;
    escaped.reserve(value.size() * 3);
    for (unsigned char c : value) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped.push_back(c);
        } else {
            escaped.push_back('%');
            escaped.push_back(hex[(c >> 4) & 0x0F]);
            escaped.push_back(hex[c & 0x0F]);
        }
    }
    return escaped;
}

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
        {"course", context_ptr->gnss_state.ground_speed > COURSE_SPEED_LIMIT?
            nlohmann::json(context_ptr->gnss_state.course):
            nlohmann::json(nullptr)
        }
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
        {"course", context_ptr->gnss_state.ground_speed > COURSE_SPEED_LIMIT?
            nlohmann::json(context_ptr->gnss_state.course):
            nlohmann::json(nullptr)
        }
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
    Context *context_ptr = (Context*) req->user_ctx;
    nlohmann::json jresp;
    jresp["files"] = nlohmann::json::array();
    jresp["current_file"] = context_ptr->trace_state.get_file_name();

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

bool WebManager::update_assist_token(Context* context_ptr, const std::string& token) {
    if (!context_ptr) {
        return false;
    }

    const std::string conf_file_path = MOUNT_POINT "/" CONFIG_FILE;
    nlohmann::json config = {
        {"timezone", context_ptr->timezone},
        {"wifi_ssid", context_ptr->wifi_ssid},
        {"wifi_passwd", context_ptr->wifi_passwd},
        {"assist_now_token", token}
    };

    config["assist_now_token"] = token;
    std::ofstream out(conf_file_path);
    if (!out.is_open()) {
        ESP_LOGE(WEB_TAG, "Failed to open config for writing");
        return false;
    }
    out << config.dump(4);
    context_ptr->assist_now_token = token;
    return true;
}

esp_err_t WebManager::assist_now_info_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    nlohmann::json response;

    response["token"] = context_ptr->assist_now_token;
    response["busy"] = context_ptr->assist_in_progress;

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response.dump().c_str(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t WebManager::assist_now_apply_handler(httpd_req_t *req) {
    Context *context_ptr = (Context*) req->user_ctx;
    if (context_ptr->assist_in_progress) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "AssistNow in progress");
        return ESP_FAIL;
    }
    const std::string body = readRequestBody(req);
    if (req->content_len > 0 && body.empty()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid body");
        return ESP_FAIL;
    }
    nlohmann::json request_json = nlohmann::json::parse(body, nullptr, false);
    if (request_json.is_discarded()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    const std::string token = request_json.value("token", "");
    const std::string mode = request_json.value("mode", "live");
    const double lat = request_json.value("lat", 0.0);
    const double lon = request_json.value("lon", 0.0);
    if (token.empty()) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing token");
        return ESP_FAIL;
    }

    context_ptr->assist_in_progress = true;

    auto url = std::make_unique<char[]>(512);
    if (!url) {
        context_ptr->assist_in_progress = false;
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory for AssistNow URL");
        return ESP_FAIL;
    }
    if (mode == "predictive_orbits") {
        snprintf(url.get(), 512,
            "https://offline-live1.services.u-blox.com/GetOfflineData.ashx?token=%s;gnss=gps,glo,bds,gal;format=mga;period=5;resolution=1",
            urlEncode(token).c_str());
    } else {
        snprintf(url.get(), 512,
            "https://online-live1.services.u-blox.com/GetOnlineData.ashx?token=%s&gnss=gps,glo,bds,gal&datatype=eph,aux,pos&pacc=0&alt=0&lat=%.2f&lon=%.2f",
            urlEncode(token).c_str(), lat, lon);
    }

    auto config = std::make_unique<esp_http_client_config_t>();
    if (!config) {
        context_ptr->assist_in_progress = false;
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory for AssistNow config");
        return ESP_FAIL;
    }
    *config = {};
    config->url = url.get();
    config->method = HTTP_METHOD_GET;
    config->timeout_ms = 10000;
    config->crt_bundle_attach = esp_crt_bundle_attach;
    esp_http_client_handle_t client = esp_http_client_init(config.get());
    if (client == nullptr) {
        context_ptr->assist_in_progress = false;
        ESP_LOGE(WEB_TAG, "AssistNow http client init failed");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "AssistNow client init failed");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        context_ptr->assist_in_progress = false;
        ESP_LOGE(WEB_TAG, "AssistNow http open failed: %d", err);
        esp_http_client_cleanup(client);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "AssistNow request failed");
        return ESP_FAIL;
    }
    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    if (status_code != 200) {
        context_ptr->assist_in_progress = false;
        ESP_LOGE(WEB_TAG, "AssistNow bad status: %d, content_length=%d", status_code, content_length);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "AssistNow request failed");
        return ESP_FAIL;
    }
    size_t total_bytes = 0;
    bool write_ok = true;
    auto read_buffer = std::make_unique<uint8_t[]>(2048);
    if (!read_buffer) {
        context_ptr->assist_in_progress = false;
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory for AssistNow buffer");
        return ESP_FAIL;
    }
    context_ptr->gnss_state.mutex.lock_write();
    while (true) {
        int read_len = esp_http_client_read(client, reinterpret_cast<char*>(read_buffer.get()), 1024);
        if (read_len < 0) {
            write_ok = false;
            ESP_LOGE(WEB_TAG, "AssistNow read failed: %d", read_len);
            break;
        }
        if (read_len == 0) {
            break;
        }

        if (!context_ptr->gnss_state.send_assist_data(read_buffer.get(), static_cast<size_t>(read_len))) {
            write_ok = false;
            break;
        }
        total_bytes += static_cast<size_t>(read_len);
    }
    context_ptr->gnss_state.mutex.unlock_write();
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (!write_ok || total_bytes == 0) {
        context_ptr->assist_in_progress = false;
        ESP_LOGE(WEB_TAG, "AssistNow request/write failed status=%d bytes=%d", status_code, static_cast<int>(total_bytes));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "AssistNow request failed");
        return ESP_FAIL;
    }
    bool save_ok = true;
    if (token != context_ptr->assist_now_token) {
        if (xSemaphoreTake(context_ptr->storage_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            save_ok = update_assist_token(context_ptr, token);
            xSemaphoreGive(context_ptr->storage_mutex);
        } else {
            save_ok = false;
        }
    }
    context_ptr->assist_in_progress = false;

    nlohmann::json response = {
        {"status", "ok"},
        {"bytes", total_bytes},
        {"token_saved", save_ok}
    };
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response.dump().c_str(), HTTPD_RESP_USE_STRLEN);
}

void WebManager::event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    WebManager* web_manager = static_cast<WebManager*>(arg);
    Context *context_ptr = static_cast<Context*>(web_manager->context_ptr);
    
    context_ptr->status_updated = true;
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
        .user_ctx  = context_ptr
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
    httpd_uri_t assist_info_uri = {
        .uri       = "/assist_now_info",
        .method    = HTTP_GET,
        .handler   = assist_now_info_handler,
        .user_ctx  = context_ptr
    };
    httpd_register_uri_handler(server, &assist_info_uri);
    httpd_uri_t assist_apply_uri = {
        .uri       = "/assist_now_apply",
        .method    = HTTP_POST,
        .handler   = assist_now_apply_handler,
        .user_ctx  = context_ptr
    };
    httpd_register_uri_handler(server, &assist_apply_uri);
}

esp_err_t WebManager::start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 12;
    config.stack_size = 102400;
    config.task_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;

    ESP_LOGI(WEB_TAG, "Starting web server on port: %d, stack=%d (PSRAM stack)", config.server_port, config.stack_size);
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
