#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
#include "utils/json.hpp"

#define WEB_TAG "Web"

class WebManager {
private:
    httpd_handle_t server = NULL;
    void* context_ptr;
    
    static void event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data);
    
    static esp_err_t root_handler(httpd_req_t *req);
    static esp_err_t trace_full_handler(httpd_req_t *req);
    static esp_err_t trace_recent_handler(httpd_req_t *req);
    static esp_err_t satellites_get_handler(httpd_req_t *req);
    static esp_err_t sdcard_files_handler(httpd_req_t *req);
    static esp_err_t download_file_handler(httpd_req_t *req);

    void register_uri_handlers();
    esp_err_t start_webserver();
    
public:
    WebManager(void* context) : context_ptr(context) {}
    
    ~WebManager() {
        stop_webserver();
    }

    esp_err_t init();
    void stop_webserver();
    std::string get_ip() const;
};

#endif // WEB_MANAGER_H