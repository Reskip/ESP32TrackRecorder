#include <sstream>
#include <iomanip>
#include <iostream>

#include "gnss_page.h"
#include "icon/gnss_icon.h"

#define X_MARGIN 36

GNSSPage::GNSSPage()
    : SidebarPage("GNSSPage", (char*) gnss_icon) {}

void GNSSPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());

    std::ostringstream lat_stream;
    std::ostringstream lon_stream;
    std::ostringstream alt_stream;

    lat_stream << "LAT ";
    lon_stream << "LON ";
    alt_stream << "ALT ";
    if (context.gnss_state.valid) {
        lat_stream << std::fixed << std::setprecision(6) << context.gnss_state.latitude;
        lon_stream << std::fixed << std::setprecision(6) << context.gnss_state.longitude;
    } else {
        lat_stream << "---";
        lon_stream << "---";
    }
    if (context.gnss_state.fix_quality > 0) {
        alt_stream << std::fixed << std::setprecision(6) << context.gnss_state.altitude;
    } else {
        alt_stream << "---";
    }

    oled.draw_string(start_x + X_MARGIN, 0, lon_stream.str().c_str(), WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 12, lat_stream.str().c_str(), WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 24, alt_stream.str().c_str(), WHITE, BLACK);
}