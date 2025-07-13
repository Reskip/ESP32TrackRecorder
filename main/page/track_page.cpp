#include <sstream>
#include <iomanip>
#include <iostream>

#include "track_page.h"
#include "icon/track_icon.h"

#define X_MARGIN 36

TrackPage::TrackPage()
    : SidebarPage("BatteryPage", (char*) track_icon) {}

void TrackPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());

    std::ostringstream tracker_stream;
    tracker_stream << "POINT " << context.trace_state.get_waypoint_size();
    std::ostringstream distance_stream;
    distance_stream << std::fixed << std::setprecision(2) << context.trace_state.get_distance();
    std::string distance_fixed = distance_stream.str().substr(0, 5);
    if (distance_fixed[distance_fixed.size() - 1] == '.') {
        distance_fixed = distance_fixed.substr(0, distance_fixed.size() - 1);
    }
    distance_stream.str("");
    distance_stream << "DIST  " << distance_fixed << " KM";
    std::ostringstream duration_stream;
    uint64_t duration_ms = 0;
    if (context.enable_track) {
        duration_ms = context.trace_state.get_duration_ms();
    }
    uint64_t hour = duration_ms / 3600000;
    uint64_t minute = duration_ms % 3600000 / 60000;
    uint64_t second = duration_ms % 60000 / 1000;
    duration_stream << "TIME  " << (hour < 10 ? "0" : "") << hour << ":"
                                << (minute < 10 ? "0" : "") << minute << ":"
                                << (second < 10 ? "0" : "") << second;

    oled.draw_string(start_x + X_MARGIN, 0, tracker_stream.str().c_str(), WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 12, distance_stream.str().c_str(), WHITE, BLACK);
    oled.draw_string(start_x + X_MARGIN, 24, duration_stream.str().c_str(), WHITE, BLACK);
}