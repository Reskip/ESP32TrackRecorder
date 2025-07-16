#include <string>
#include <chrono>
#include <optional>
#include <vector>

#include "gnss_state.h"
#include "utils/json.hpp"
#include "utils/lock.hpp"

#ifndef TRACESTATE
#define TRACESTATE

#define TRACE_TAG "TraceState"

struct Context;

struct WayPoint {
    double latitude;
    double longitude;

    std::optional<double> elevation;
    std::optional<std::chrono::system_clock::time_point> timestamp;
    std::optional<double> hdop;
    std::optional<double> speed;
    
    WayPoint() 
        : latitude(0), longitude(0) {}
    WayPoint(double lat, double lon)
        : latitude(lat), longitude(lon) {}

    WayPoint(GNSSState &gnss_state);

    void set_timestamp(int year, int month, int day, 
                      int hour = 0, int minute = 0, int second = 0, int microseconds = 0);
    std::string to_gpx_string() const;
};

class Trace {
public:
    Trace() {
        distance = 0.0;
        sample_cnt = 0;
        local_start_time_ms = 0;
        last_update_time_ms = 0;
        closed = true;
        fp = nullptr;
        current_trace = nlohmann::json::array();
    };
    ~Trace() = default;

    void add_waypoint(GNSSState &gnss_state);

    std::optional<std::chrono::system_clock::time_point> get_start_time() const {
        return start_time;
    }

    double get_distance() const {
        return distance;
    }

    int get_waypoint_size() const {
        return sample_cnt;
    }

    const nlohmann::json& get_waypoints() const {
        return current_trace;
    }

    nlohmann::json get_last_waypoints() const {
        nlohmann::json response = nlohmann::json::array();
        if (closed) {
            return response;
        }
        nlohmann::json last_point_json = {
            {"lat", last_point.latitude},
            {"lon", last_point.longitude},
            {"alt", *(last_point.elevation)},
            {"speed", *(last_point.speed)},
            {"hdop", *(last_point.hdop)}
        };
        response.push_back(last_point_json);
        return response;
    }

    void try_close_trace();
    int get_duration_ms();
    void register_context(Context* context);

    RWlock mutex;
private:
    WayPoint last_point;
    nlohmann::json current_trace;
    std::string file_name;
    std::chrono::system_clock::time_point start_time;
    int64_t local_start_time_ms;
    int64_t last_update_time_ms;
    double distance;
    int sample_cnt;
    bool closed;
    FILE *fp;
    Context* context;
};
#endif