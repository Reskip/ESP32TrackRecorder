#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "esp_wifi.h"

#include "setting_page.h"
#include "icon/setting_icon.h"
#include "icon/icon.h"
#include "button.h"

#define X_MARGIN 36
#define Y_MARGIN 2

SettingPage::SettingPage()
    : SidebarPage("SettingPage", (char*) setting_icon) {

    Button wifi_btn(Button::PRESS, "WIFI ", 0, Y_MARGIN + 0);
    wifi_btn.set_callback([](Button* btn, Context* context) {
        context->enable_wifi = btn->get_press_state();
        if (context->enable_wifi) {
            ESP_ERROR_CHECK(esp_wifi_start());
        } else {
            ESP_ERROR_CHECK(esp_wifi_stop());
        }
    });

    Button track_btn(Button::PRESS, "TRACK", 0, Y_MARGIN + 12);
    track_btn.set_callback([](Button* btn, Context* context) {
        context->enable_track = btn->get_press_state();
    });

    Button return_btn(Button::ICON_ONLY, "RETURN", 78, Y_MARGIN + 0);
    return_btn.configure_icon((char*) return_icon, 7, 7, false);
    return_btn.set_callback([](Button* btn, Context* context) {
        context->statue_change_flag = true;
    });

    buttons.push_back(wifi_btn);
    buttons.push_back(track_btn);
    buttons.push_back(return_btn);
}

void SettingPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());

    for (auto &btn: buttons) {
        btn.render(context, oled, start_x + X_MARGIN);
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

bool SettingPage::handle_press(Context& context, OLED &oled) {
    if (get_state() == PageState::NORMAL) {
        switch_state();

        context.select_btn_id = 0;
        select_new_btn(0, oled);
        select_progress = 1.0;
        select_x = target_select_x;
        select_y = target_select_y;
        select_w = target_select_w;
        select_h = target_select_h;
        return true;
    }
    buttons[context.select_btn_id].handle_press(context);
    if (context.statue_change_flag) {
        context.statue_change_flag = false;
        switch_state();
    }
    return true;
}

bool SettingPage::handle_scroll(Context& context, OLED &oled, int value) {
    if (get_state() == PageState::NORMAL) {
        return false;
    }
    context.select_btn_id += value + buttons.size();
    context.select_btn_id %= buttons.size();
    select_new_btn(context.select_btn_id, oled);

    return true;
}

void SettingPage::select_new_btn(int btn_id, OLED &oled) {
    target_select_x = buttons[btn_id].get_x_pos() - 2;
    target_select_y = buttons[btn_id].get_y_pos() - 2;
    target_select_h = 11;
    target_select_w = buttons[btn_id].get_output_weight(oled) + 4;
    select_progress = 0.0;
}