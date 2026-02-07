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

void DisplayManager::showGnssBootScreen(int configured_steps, int completed_steps) {
    const int bar_x = 12;
    const int bar_y = 22;
    const int bar_w = 104;
    const int bar_h = 8;

    const int fill_w = completed_steps * (bar_w - 2) / configured_steps;

    oled.clear();
    oled.draw_string(12, 0, "ESP32 Track", WHITE, BLACK);
    oled.draw_string(12, 10, "GNSS Init ...", WHITE, BLACK);
    oled.draw_rectangle(bar_x, bar_y, bar_w, bar_h, WHITE);
    if (fill_w > 0) {
        oled.fill_rectangle(bar_x + 1, bar_y + 1, fill_w, bar_h - 2, WHITE);
    }

    std::ostringstream progress_stream;
    progress_stream << completed_steps << "/" << configured_steps;
    oled.draw_string(92, 0, progress_stream.str().c_str(), WHITE, BLACK);

    oled.refresh(true);
}

void DisplayManager::updateDisplay(Context &context) {
    context.fresh_cnt += 1;
    context.fresh_ts_diff_ms = esp_timer_get_time() / THOUSAND - context.last_fresh_ts_ms;
    context.last_fresh_ts_ms = esp_timer_get_time() / THOUSAND;
    if (context.fresh_cnt % MIN_REFRESH_FRAME == 0) {
        context.status_updated = true;
    }
    if (context.last_fresh_ts_ms - context.last_fps_update_ts_ms > THOUSAND) {
        context.fps = context.fresh_cnt * THOUSAND / (context.last_fresh_ts_ms - context.last_fps_update_ts_ms);
        context.fresh_cnt = 0;
        context.last_fps_update_ts_ms = context.last_fresh_ts_ms;
        ESP_LOGI(DISPLAY_TAG, "FPS: %lf", context.fps);
    }

    if (context.brightness_change_flag) {
        ESP_LOGI(DISPLAY_TAG, "SET BRIGHTNESS: %d", context.brightness);
        oled.set_light(context.brightness);
        context.brightness_change_flag = false;
    }

    encoder_event_t event;
    while (xQueueReceive(context.encoder_state.press_queue, &event, 0)){
        if (!event.position) {
            continue;
        }
        context.status_updated = true;
        if (side_pages[context.select_page_id]->handle_press(context, oled)) {
            continue;
        }
        side_pages[context.select_page_id]->switch_state();
        ESP_LOGI(DISPLAY_TAG, "Encoder Event: %d, switch state", event.position);
    }

    while (xQueueReceive(context.encoder_state.encoder_queue, &event, 0)){
        context.status_updated = true;
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
            }
        }
        ESP_LOGI(DISPLAY_TAG, "Encoder Event: %d, select page: %d", event.position, context.select_page_id);
    }

    bool animating = context.status_updated;
    for (int i = 0; i < side_pages.size(); i++) {
        animating = animating || side_pages[i]->is_animating();
    }

    if (!animating) {
        vTaskDelay(pdMS_TO_TICKS(20));
        return;
    }

    oled.clear();
    context.gnss_state.mutex.lock_read();
    context.status_updated = false;
    main_page->render(context, oled);
    for (int i = 0; i < side_pages.size(); i++) {
        side_pages[i]->render(context, oled);
    }
    context.gnss_state.mutex.unlock_read();
    oled.refresh(true);
}
