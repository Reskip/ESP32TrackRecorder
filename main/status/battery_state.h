#include <string>
#include <vector>
#include "driver/uart.h"
#include "driver/gpio.h"

#ifndef BATTERYSTATE
#define BATTERYSTATE

#define BATTERY_TAG "BatteryState"

class Battery {
public:
    Battery(gpio_num_t scl, gpio_num_t sda, gpio_num_t charging);
    ~Battery();

    void print_debug_info() const;
    bool init();
    float read_voltage() const;
    float read_soc() const;
    bool is_charging() const;
    bool read_register(uint8_t reg, uint16_t *value) const;

    gpio_num_t scl, sda, charging;
};

#endif