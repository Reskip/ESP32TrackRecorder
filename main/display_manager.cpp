#include "display_manager.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "page/main_page.h"
#include "page/status_page.h"
#include "page/gnss_page.h"
#include "page/setting_page.h"
#include "page/wifi_page.h"
#include "page/track_page.h"

DisplayManager::DisplayManager(gpio_num_t scl, gpio_num_t sda)
    : oled(scl, sda, SSD1306_128x32) {
        main_page = std::make_unique<MainPage>();

        side_pages.push_back(std::make_unique<StatusPage>());
        side_pages.push_back(std::make_unique<TrackPage>());
        side_pages.push_back(std::make_unique<GNSSPage>());
        side_pages.push_back(std::make_unique<WifiPage>());
        side_pages.push_back(std::make_unique<SettingPage>());
        
        for (int i = 0; i < side_pages.size(); i++) {
            side_pages[i]->set_init_position(i, side_pages.size());
        }
    }

DisplayManager::~DisplayManager() {}

bool DisplayManager::init() {
    if (!oled.init()) {
        return false;
    }

    oled.clear();
    oled.refresh(true);
    oled.select_font(0);
    return true;
}

void DisplayManager::updateDisplay(Context &context) {
    context.fresh_cnt += 1;
    context.fresh_ts_diff_ms = esp_timer_get_time() / THOUSAND - context.last_fresh_ts_ms;
    context.last_fresh_ts_ms = esp_timer_get_time() / THOUSAND;
    if (context.last_fresh_ts_ms - context.last_fps_update_ts_ms > THOUSAND) {
        context.fps = context.fresh_cnt * THOUSAND / (context.last_fresh_ts_ms - context.last_fps_update_ts_ms);
        context.fresh_cnt = 0;
        context.last_fps_update_ts_ms = context.last_fresh_ts_ms;
        ESP_LOGI(DISPLAY_TAG, "FPS: %lf", context.fps);
    }

    encoder_event_t event;
    while (xQueueReceive(context.encoder_state.press_queue, &event, 0)){
        if (!event.position) {
            continue;
        }
        if (side_pages[context.select_page_id]->handle_press(context, oled)) {
            continue;
        }
        side_pages[context.select_page_id]->switch_state();
        ESP_LOGI(DISPLAY_TAG, "Encoder Event: %d, switch state", event.position);
    }

    while (xQueueReceive(context.encoder_state.encoder_queue, &event, 0)){
        if (side_pages[context.select_page_id]->handle_scroll(context, oled, event.position)) {
            continue;
        }

        if (side_pages[context.select_page_id]->get_state() == PageState::EXPANDED) {
            continue;
        }

        if (event.position) {
            context.select_page_id += event.position + side_pages.size();
            context.select_page_id %= side_pages.size();
            for (int i = 0; i < side_pages.size(); i++) {
                side_pages[i]->set_target_position(
                    (i - context.select_page_id + 2 + side_pages.size()) % side_pages.size(),
                    side_pages.size()
                );
                // ESP_LOGI(DISPLAY_TAG, "Encoder Event page mapping %d %d",
                //     i, (i - context.select_page_id + 2 + side_pages.size()) % side_pages.size()
                // );
            }
        }
        ESP_LOGI(DISPLAY_TAG, "Encoder Event: %d, select page: %d", event.position, context.select_page_id);
    }

    oled.clear();
    context.gnss_state.lock();
    main_page->render(context, oled);
    for (int i = 0; i < side_pages.size(); i++) {
        side_pages[i]->render(context, oled);
    }
    context.gnss_state.unlock();
    oled.refresh(true);
}
