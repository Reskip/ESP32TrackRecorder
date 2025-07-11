#include <cmath>
#include <utility>

#include "utils/utils.h"

double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0;

    auto to_radians = [](double degrees) {
        return degrees * M_PI / 180.0;
    };

    lat1 = to_radians(lat1);
    lon1 = to_radians(lon1);
    lat2 = to_radians(lat2);
    lon2 = to_radians(lon2);

    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;

    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon/2) * std::sin(dLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));

    return R * c;
}

void init_gpx(FILE *fp) {
    const char* header = R"(<?xml version="1.0" encoding="utf-8"?>
<gpx version="1.1" creator="Guru Maps/5.7.4" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.topografix.com/GPX/1/1" xmlns:gom="https://gurumaps.app/gpx/v3">
    <trk>
        <name>TRACK_FILE</name>
        <type>TrackStyle_1000000</type>
        <trkseg>
)";
    fprintf(fp, header);
}

void close_gpx(FILE *fp) {
    const char* header = R"(        </trkseg>
    </trk>
</gpx>
)";
    fprintf(fp, header);
    fclose(fp);
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