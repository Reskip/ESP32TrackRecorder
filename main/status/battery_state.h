#include <string>
#include <vector>
#include "driver/uart.h"
#include "driver/gpio.h"

#ifndef BATTERYSTATE
#define BATTERYSTATE

#define BATTERY_TAG "BatteryState"
#define REQUEST_CACHE 100

class Battery {
public:
    Battery(gpio_num_t scl, gpio_num_t sda, gpio_num_t charging);
    ~Battery();

    void print_debug_info();
    bool init();
    float read_voltage();
    float read_soc();
    bool is_charging();
    bool read_register(uint8_t reg, uint16_t *value) const;
    void update_all_status();

    gpio_num_t scl, sda, charging;
    float voltage_, soc_;
    int charging_;
    int request_cache_cnt;
};

#endif