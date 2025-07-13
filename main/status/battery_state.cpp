#include "battery_state.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_FREQ_HZ 400000  // I2C 频率
#define I2C_MASTER_TIMEOUT_MS 1000
#define MAX17048_ADDR 0x36         // MAX17048 I2C 地址
#define VCELL_REG 0x02             // 电池电压寄存器
#define SOC_REG 0x04               // 电池电量寄存器

Battery::Battery(gpio_num_t scl, gpio_num_t sda, gpio_num_t charging) {
    this->scl = scl;
    this->sda = sda;
    this->charging = charging;
    init();
}

Battery::~Battery() {
}

bool Battery::init() {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    
    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(BATTERY_TAG, "I2C config failed");
        return false;
    }
    
    err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(BATTERY_TAG, "I2C driver install failed");
        return false;
    }

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pin_bit_mask = (1ULL << charging);
    gpio_config(&io_conf);

    voltage_ = 0;
    soc_ = 0;
    charging_ = false;
    request_cache_cnt = REQUEST_CACHE;
    return true;
}

void Battery::update_all_status() {
    if (request_cache_cnt < REQUEST_CACHE) {
        request_cache_cnt += 1;
        return;
    }
    request_cache_cnt = 0;
    uint16_t raw_value;

    if (!read_register(VCELL_REG, &raw_value)) {
        voltage_ = -1.0;
    } else {
        voltage_ = (raw_value >> 4) * 0.00125;
    }

    if (!read_register(SOC_REG, &raw_value)) {
        soc_ = -1.0;
    } else {
        soc_ = raw_value / 256.0;
    }

    charging_ = !gpio_get_level(charging);
    return;
}

float Battery::read_voltage() {
    update_all_status();
    return voltage_;
}

float Battery::read_soc() {
    update_all_status();
    return soc_;
}

bool Battery::is_charging() {
    update_all_status();
    return charging_;
}

bool Battery::read_register(uint8_t reg, uint16_t *value) const {
    uint8_t data[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MAX17048_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data[1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) {
        ESP_LOGE(BATTERY_TAG, "I2C read failed");
        return false;
    }
    *value = (data[0] << 8) | data[1];
    return true;
}

void Battery::print_debug_info() {
    float voltage_ = read_voltage();
    float soc_ = read_soc();
    int charging_ = is_charging();
    ESP_LOGI(BATTERY_TAG, "Voltage: %.3f V, SOC: %.1f%%, Charging: %d", voltage_, soc_, charging_);
}
