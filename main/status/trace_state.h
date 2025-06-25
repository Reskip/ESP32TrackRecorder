#include <string>
#include <chrono>
#include <optional>
#include <vector>

#include "gnss_state.h"

#ifndef TRACESTATE
#define TRACESTATE

#define TRACE_TAG "TraceState"

struct WayPoint {
    double latitude;
    double longitude;

    std::optional<double> elevation;
    std::optional<std::chrono::system_clock::time_point> timestamp;
    std::optional<double> hdop;
    std::optional<double> speed;
    
    WayPoint(double lat, double lon)
        : latitude(lat), longitude(lon) {}

    WayPoint(GNSSState &gnss_state);

    void set_timestamp(int year, int month, int day, 
                      int hour = 0, int minute = 0, int second = 0, int microseconds = 0);
    std::string to_gpx_string() const;
};

class Trace {
public:
    Trace() = default;
    ~Trace() = default;

    void add_waypoint(GNSSState &gnss_state);

    std::optional<std::chrono::system_clock::time_point> get_start_time() const {
        return start_time;
    }

    double get_distance() const {
        return distance;
    }

    int get_waypoint_size() const {
        return current_trace.size();
    }

    const std::vector<WayPoint>& get_waypoints() const {
        return current_trace;
    }

private:
    std::vector<WayPoint> current_trace;
    std::optional<std::chrono::system_clock::time_point> start_time;
    double distance = 0.0;
};
#endif