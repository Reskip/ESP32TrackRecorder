#include <sstream>
#include <iomanip>
#include <iostream>

#include "esp_wifi.h"
#include "esp_timer.h"

#include "wifi_page.h"
#include "icon/wifi_icon.h"
#include "icon/icon.h"

#define X_MARGIN 36
#define Y_MARGIN 2

WifiPage::WifiPage()
    : SidebarPage("WifiPage", (char*) wifi_icon) {
    Button wifi_btn(Button::PRESS, "WIFI ", 0, Y_MARGIN + 0);
    wifi_btn.set_callback([](Button* btn, Context* context) {
        context->enable_wifi = btn->get_press_state();
        if (context->enable_wifi) {
            ESP_ERROR_CHECK(esp_wifi_start());
        } else {
            ESP_ERROR_CHECK(esp_wifi_stop());
            context->ip = "";
        }
    });

    Button return_btn(Button::ICON_ONLY, "RETURN", 78, Y_MARGIN + 0);
    return_btn.configure_icon((char*) return_icon, 7, 7, false);
    return_btn.set_callback([](Button* btn, Context* context) {
        context->return_page_flag = true;
    });

    buttons.push_back(wifi_btn);
    buttons.push_back(return_btn);
}

void WifiPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());

    for (auto &btn: buttons) {
        btn.render(context, oled, start_x + X_MARGIN);
    }

    if (context.enable_wifi) {
        if (context.ip.size() > 0) {
            oled.draw_string(start_x + X_MARGIN, Y_MARGIN + 12, context.ip.c_str(), WHITE, BLACK);
        } else {
            std::string connecting = "CONNECTING";
            for (int i = 0; i < (esp_timer_get_time() / 800000) % 3; i++) {
                connecting += ".";
            }
            oled.draw_string(start_x + X_MARGIN, Y_MARGIN + 12, connecting.c_str(), WHITE, BLACK);
        }
    }

    smooth_easing_move(select_x, target_select_x, select_progress, lock_fps_speed(context, SCROLL_SPEED));
    smooth_easing_move(select_y, target_select_y, select_progress, lock_fps_speed(context, SCROLL_SPEED), false);
    smooth_easing_move(select_w, target_select_w, select_progress, lock_fps_speed(context, SCROLL_SPEED), false);
    smooth_easing_move(select_h, target_select_h, select_progress, lock_fps_speed(context, SCROLL_SPEED), false);

    oled.fill_rectangle(
        std::round(std::max(0.0, start_x + X_MARGIN + select_x)), std::round(std::max(0.0, select_y)),
        std::round(select_w), std::round(select_h), INVERT
    );
}

bool WifiPage::handle_press(Context& context, OLED &oled) {
    if (get_state() == PageState::NORMAL) {
        switch_state();

        context.select_btn_id = 1;
        select_new_btn(context.select_btn_id, oled);
        select_progress = 1.0;
        select_x = target_select_x;
        select_y = target_select_y;
        select_w = target_select_w;
        select_h = target_select_h;
        return true;
    }
    buttons[context.select_btn_id].handle_press(context);
    select_new_btn(context.select_btn_id, oled, true);
    if (context.return_page_flag) {
        context.return_page_flag = false;
        switch_state();
    }
    return true;
}

bool WifiPage::handle_scroll(Context& context, OLED &oled, int value) {
    if (get_state() == PageState::NORMAL) {
        return false;
    }
    context.select_btn_id += value + buttons.size();
    context.select_btn_id %= buttons.size();
    select_new_btn(context.select_btn_id, oled);

    return true;
}

void WifiPage::select_new_btn(int btn_id, OLED &oled, bool immidiate) {
    target_select_x = buttons[btn_id].get_x_pos() - 2;
    target_select_y = buttons[btn_id].get_y_pos() - 2;
    target_select_h = 11;
    target_select_w = buttons[btn_id].get_output_weight(oled) + 4;
    select_progress = 0.0;
    if (immidiate) {
        select_x = target_select_x;
        select_y = target_select_y;
        select_h = target_select_h;
        select_w = target_select_w;
        select_progress = 1.0;
    }
}