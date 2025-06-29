#include <string>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "minmea.h"

#ifndef GNSSSTATE
#define GNSSSTATE

#define GSA_MAX_SAT_NUM 12
#define GSA_IN_USE_DELAY 3
#define GSA_TRACK_DELAY 3
#define GNSS_TAG "GNSSState"
#define BAUD_RATE 115200
#define UART_NUM UART_NUM_1
#define BUF_SIZE 2048
#define ACK_ROUND 10
#define POINT_CACHE_SIZE 5

#define UBX_SYNC_CHAR_1 0xB5
#define UBX_SYNC_CHAR_2 0x62
#define UBX_ACK 0x05
#define UBX_ACK_ACK 0x01
#define UBX_ACK_NAK 0x00

const std::vector<std::vector<uint8_t>> ubx_conf_command = {
    {0xB5, 0x62, 0x06, 0x8A, 0x21, 0x00, 0x00, 0x04, 0x00, 0x00, 0x21, 0x00, 0x11, 0x20, 0x00, 0x07, 0x00, 0x93, 0x20, 0x01, 0x01, 0x00, 0x93, 0x20, 0x28, 0x01, 0x00, 0x21, 0x30, 0xF4, 0x01, 0x01, 0x00, 0x52, 0x40, 0x00, 0xC2, 0x01, 0x00, 0x3B, 0xF4}
};

struct Satellite {
public:
    Satellite()
        : sat_type("Unknown"), nr(0), elevation(0), azimuth(0), snr(0), in_use(0), in_track(GSA_TRACK_DELAY) {}

    Satellite(const std::string& sat_type, int satellite_nr, int elevation, int azimuth, int snr)
        : sat_type(sat_type), nr(satellite_nr), elevation(elevation), azimuth(azimuth), snr(snr), in_use(0), in_track(GSA_TRACK_DELAY) {}

    ~Satellite() {}

    void deactive() {
        in_use = 0;
    }

    void active() {
        in_use = GSA_IN_USE_DELAY;
    }

    std::string sat_type;
    int nr;
    int elevation;
    int azimuth;
    int snr;
    int in_use;
    int in_track;
};

class GNSSState {
public:
    GNSSState(gpio_num_t tx, gpio_num_t rx);
    ~GNSSState();

    bool parse();
    void print_debug_info() const;
    bool init();
    bool send_ubx_command(const uint8_t* command, size_t length);
    bool wait_for_ack(uint8_t class_id, uint8_t msg_id);
    bool lock();
    bool unlock();

    gpio_num_t gpio_tx;
    gpio_num_t gpio_rx;

    double latitude;
    double longitude;
    double altitude;
    double doppler_speed;
    double ground_speed;
    double rms_deviation;
    int hdop;
    int fix_quality;
    int fix_type;
    bool valid;
    int year, month, day, hour, minute, second, microseconds;

    char data[BUF_SIZE];

    SemaphoreHandle_t mutex;
    std::string timestamp;
    std::vector<Satellite> satellites;
    std::vector<std::pair<double, double>> point_cache;
};

#endif