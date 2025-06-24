#include <sstream>
#include <iomanip>
#include <iostream>

#include "track_page.h"
#include "icon/track_icon.h"

TrackPage::TrackPage()
    : SidebarPage("BatteryPage", (char*) track_icon) {}

void TrackPage::render_expand(Context &context, OLED &oled) {
    int start_x = std::round(get_sidebar_start_pos_x());
    for (int i = 0; i < context.gnss_state.satellites.size() && i < 3; i++) {
        std::ostringstream tracker_stream;
        tracker_stream << context.gnss_state.satellites[i].sat_type\
            << " " << context.gnss_state.satellites[i].nr\
            << " " << context.gnss_state.satellites[i].elevation\
            << " " << context.gnss_state.satellites[i].azimuth\
            << " " << context.gnss_state.satellites[i].snr\
            << " " << context.gnss_state.satellites[i].in_track;
        oled.draw_string(start_x + 30, 2+(i*10), tracker_stream.str().c_str(), WHITE, BLACK);
    }
}