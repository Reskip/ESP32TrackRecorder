#include <sstream>
#include <iomanip>
#include <iostream>

#include "main_page.h"
#include "icon/icon.h"
#include "utils/utils.h"

MainPage::MainPage()
    : Page("MainPage") {
        set_state(PageState::NORMAL);
    }

void MainPage::render_normal(Context &context, OLED &oled) {
    int battery_idx = 0;
    if (context.battery_state.is_charging()) {
        battery_idx = 4;
    } else {
        battery_idx = get_battery_level(context.battery_state.read_soc());
    }
    oled.draw_bitmap(0, 0, (const uint8_t*) battery_icon[battery_idx], 13, 7);

    std::ostringstream timeBatteryStream;
    int hour = (context.gnss_state.hour + context.timezone + 24) % 24;
    timeBatteryStream << (hour < 10 ? "0" : "") << hour << ":"
                      << (context.gnss_state.minute < 10 ? "0" : "") << context.gnss_state.minute;
    oled.draw_string(60, 0, timeBatteryStream.str().c_str(), WHITE, BLACK);

    std::ostringstream speedStream;
    speedStream << std::fixed << std::setprecision(2)
                << context.gnss_state.ground_speed << " KM/H ";
    oled.draw_string(0, 12, speedStream.str().c_str(), WHITE, BLACK);

    int valid_sat_nums = 0;
    int sat_in_use = 0;
    for (const Satellite& sat : context.gnss_state.satellites) {
        if (sat.snr > 0) {
            valid_sat_nums++;
        }
        if (sat.in_use) {
            sat_in_use++;
        }
    }
    std::ostringstream satInfoStream;
    satInfoStream << "SAT: " << sat_in_use << "/" << valid_sat_nums;
    oled.draw_string(0, 24, satInfoStream.str().c_str(), WHITE, BLACK);

    // 渲染右侧索引栏
    int index_x = 96;  // 索引栏 x 坐标起点
    int index_y_start = 3;  // 索引栏 y 坐标起点
    int index_gap = 6;  // 索引点间距
    int selected_page_id = context.select_page_id;

    for (int i = 0; i < 5; ++i) {
        int y = index_y_start + i * index_gap;
        if (i == selected_page_id) {
            oled.draw_circle(index_x, y, 3, WHITE); // 选中页面点放大
        } else {
            oled.draw_circle(index_x, y, 1, WHITE); // 普通页面点
        }
    }
}

void MainPage::render(Context &context, OLED &oled) {
    render_normal(context, oled);
}