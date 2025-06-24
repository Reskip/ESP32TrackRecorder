#include <sstream>
#include <iomanip>
#include <iostream>

#include "main_page.h"
#include "icon/icon.h"

MainPage::MainPage()
    : Page("MainPage") {
        set_state(PageState::NORMAL);
    }

int get_battery_level(int battery) {
    if (battery > 70) {
        return 3;
    } else if (battery > 40) {
        return 2;
    } else if (battery > 10) {
        return 1;
    } else {
        return 0;
    }
}

void MainPage::render_normal(Context &context, OLED &oled) {
    float battery = context.battery_state.read_soc();
    oled.draw_bitmap(0, 0, (const uint8_t*) battery_icon[get_battery_level(battery)], 13, 7);

    std::ostringstream timeBatteryStream;
    timeBatteryStream << (context.gnss_state.hour < 10 ? "0" : "") << context.gnss_state.hour << ":"
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