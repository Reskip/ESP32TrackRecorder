#include <sstream>
#include <iomanip>
#include <iostream>

#include "status_page.h"
#include "icon/icon.h"
#include "icon/status_icon.h"

#define Y_MARGIN 10
#define X_MARGIN 36

StatusPage::StatusPage()
    : SidebarPage("StatusPage", (char*) status_icon) {}

void StatusPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());
    float battery = context.battery_state.read_soc();
    while (xSemaphoreTake(context.storage_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {}
    double flash_use_rate = (context.flash_used * 100.0) / context.flash_total;
    double ram_use_rate = (context.ram_used * 100.0) / context.ram_total;
    xSemaphoreGive(context.storage_mutex);

    oled.draw_bitmap(start_x + X_MARGIN + 48, 0, (const uint8_t*) battery_icon[4], 13, 7);
    oled.draw_string(start_x + X_MARGIN, 0, "FPS", WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 12, "RAM", WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 24, "FLASH", WHITE, BLACK);

    std::ostringstream battery_stream;
    battery_stream << int(battery) << "%";
    oled.draw_string(start_x + X_MARGIN + 64, 0, battery_stream.str().c_str(), WHITE, BLACK);

    std::ostringstream fps_stream;
    fps_stream << int(context.fps);
    oled.draw_string(start_x + X_MARGIN + 20, 0, fps_stream.str().c_str(), WHITE, BLACK);

    std::ostringstream ram_stream;
    ram_stream << std::fixed << std::setprecision(2) << ram_use_rate << "%";
    oled.draw_string(start_x + X_MARGIN + 48, 12, ram_stream.str().c_str(), WHITE, BLACK);

    std::ostringstream flash_stream;
    flash_stream << std::fixed << std::setprecision(2) << flash_use_rate << "%";
    oled.draw_string(start_x + X_MARGIN + 48, 24, flash_stream.str().c_str(), WHITE, BLACK);
}