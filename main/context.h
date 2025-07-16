#include "status/gnss_state.h"
#include "status/battery_state.h"
#include "status/encoder_state.h"
#include "status/trace_state.h"

#ifndef CONTEXT
#define CONTEXT

#define MOUNT_POINT "/spiflash"

struct Context {
    Context(
        GNSSState& gnss_state,
        Battery& battery_state,
        Encoder& encoder_state,
        Trace& trace_state):
        gnss_state(gnss_state),
        battery_state(battery_state),
        encoder_state(encoder_state),
        trace_state(trace_state),
        select_page_id(2),
        select_btn_id(0),
        brightness(0x2f),
        enable_wifi(false),
        enable_web_server(false),
        ip(""),
        enable_track(false),
        last_fresh_ts_ms(0),
        fresh_ts_diff_ms(0),
        last_fps_update_ts_ms(0),
        fresh_cnt(0),
        fps(0.0),
        flash_total(0),
        flash_used(0),
        ram_total(0),
        ram_used(0),
        return_page_flag(false),
        brightness_change_flag(false),
        status_updated(true),
        timezone(0),
        wifi_ssid(""),
        wifi_passwd("") {
            battery_state.register_context(this);
            gnss_state.register_context(this);
            trace_state.register_context(this);
        }
    GNSSState& gnss_state;
    Battery& battery_state;
    Encoder& encoder_state;
    Trace& trace_state;

    int select_page_id;
    int select_btn_id;
    uint8_t brightness;
    bool enable_wifi;
    bool enable_web_server;
    std::string ip;
    bool enable_track;

    int64_t last_fresh_ts_ms;
    int64_t fresh_ts_diff_ms;
    int64_t last_fps_update_ts_ms;
    int64_t fresh_cnt;
    int64_t start_ts_ms;
    double fps;

    int64_t flash_total;
    int64_t flash_used;
    int64_t ram_total;
    int64_t ram_used;

    bool return_page_flag;
    bool brightness_change_flag;
    bool status_updated;

    int timezone;
    std::string wifi_ssid;
    std::string wifi_passwd;

    SemaphoreHandle_t storage_mutex = xSemaphoreCreateMutex();;
};

#endif // context.h