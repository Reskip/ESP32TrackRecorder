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