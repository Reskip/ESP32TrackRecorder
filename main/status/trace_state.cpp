#include <iostream>

#include "esp_log.h"
#include "esp_timer.h"

#include "status/trace_state.h"
#include "utils/utils.h"
#include "context.h"

#define SAMPLE_RATE 10
#define SAVE_FILE_RATE 30
#define MOUNT_POINT "/spiflash"
#define CUT_SEG_TIME_DIFF 10

WayPoint::WayPoint(GNSSState &gnss_state) {
    latitude = gnss_state.latitude;
    longitude = gnss_state.longitude;
    elevation = gnss_state.altitude;
    hdop = gnss_state.hdop;
    speed = gnss_state.ground_speed;

    set_timestamp(gnss_state.year, gnss_state.month, gnss_state.day,
        gnss_state.hour, gnss_state.minute, gnss_state.second, gnss_state.microseconds);
}

void WayPoint::set_timestamp(int year, int month, int day, 
                            int hour, int minute, int second, int microseconds) {
    std::tm tm = {};
    tm.tm_year = 2000 + year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = 0;

    std::time_t time = std::mktime(&tm);
    auto time_point = std::chrono::system_clock::from_time_t(time);
    time_point += std::chrono::microseconds(microseconds);
    timestamp = time_point;
}

std::string WayPoint::to_gpx_string() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(9);  // 设置浮点数精度
    
    ss << "<trkpt lat=\"" << latitude << "\" lon=\"" << longitude << "\">";
    
    if (elevation.has_value())
        ss << "<ele>" << *elevation << "</ele>";
        
    if (timestamp.has_value()) {
        auto time_t = std::chrono::system_clock::to_time_t(*timestamp);
        std::tm* tm = std::gmtime(&time_t);
        
        auto duration = timestamp->time_since_epoch();
        int64_t micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
        micros /= 1000;
        
        char time_str[20];  // 存储 "YYYY-MM-DDTHH:MM:SS" 格式
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%S", tm);
        
        ss << "<time>" << time_str << "." 
           << std::setfill('0') << std::setw(3) << micros  // 毫秒部分
           << "Z</time>";
    }
    
    if (hdop.has_value())
        ss << "<hdop>" << *hdop << "</hdop>";

    if (speed.has_value())
        ss << "<extensions><gom:speed>" << *speed << "</gom:speed></extensions>";
        
    ss << "</trkpt>";
    return ss.str();
}

void Trace::add_waypoint(GNSSState &gnss_state) {
    WayPoint waypoint(gnss_state);
    int64_t update_time_diff_ms = 0;
    int64_t current_time_ms = esp_timer_get_time() / 1000;

    mutex.lock_write();

    if (closed || sample_cnt == 0) {
        start_time = *waypoint.timestamp;
        current_trace = nlohmann::json::array();

        distance = 0.0;
        sample_cnt = 0;
        closed = false;
        last_point = waypoint;
        char time_str[50];

        local_start_time_ms = current_time_ms;
        last_update_time_ms = current_time_ms;
        auto time_t = std::chrono::system_clock::to_time_t(start_time);
        std::tm* tm = std::gmtime(&time_t);
        std::strftime(time_str, sizeof(time_str), "/%Y-%m-%dT%H-%M-%S.GPX", tm);
        file_name = std::string(MOUNT_POINT) + std::string(time_str);
        fp = fopen(file_name.c_str(), "w");
        ESP_LOGI(TRACE_TAG, "file name %s", file_name.c_str());
        init_gpx(fp);
    } else {
        double _distance = haversine_distance(
            last_point.latitude, last_point.longitude,
            waypoint.latitude, waypoint.longitude
        );
        distance += _distance;
        last_point = waypoint;
        update_time_diff_ms = current_time_ms - last_update_time_ms;
        last_update_time_ms = current_time_ms;
    }

    if (update_time_diff_ms > 1000 * CUT_SEG_TIME_DIFF) {
        new_track_seg(fp);
    }

    fprintf(fp, "           %s\n", waypoint.to_gpx_string().c_str());

    if (sample_cnt % SAVE_FILE_RATE == 0) {
        fclose(fp);
        fp = fopen(file_name.c_str(), "a+");
    }

    if (sample_cnt % SAMPLE_RATE == 0) {
        nlohmann::json point_json = {
            {"lat", waypoint.latitude},
            {"lon", waypoint.longitude},
            {"alt", *(waypoint.elevation)},
            {"speed", *(waypoint.speed)},
            {"hdop", *(waypoint.hdop)}
        };
        current_trace.push_back(point_json);
    }
    sample_cnt += 1;
    context->status_updated = true;

    mutex.unlock_write();
}

void Trace::try_close_trace() {
    if (closed) {
        return;
    }
    closed = true;
    file_name = "";
    close_gpx(fp);
}

int Trace::get_duration_ms() {
    if (!local_start_time_ms) {
        return 0;
    }
    return (esp_timer_get_time() / 1000) - local_start_time_ms;
}

std::string Trace::get_file_name() {
    return file_name;
}

void Trace::register_context(Context* context) {
    this->context = context;
}