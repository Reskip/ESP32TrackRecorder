#include <sstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "esp_log.h"

#include "status/gnss_state.h"

GNSSState::GNSSState(gpio_num_t tx, gpio_num_t rx)
    :
        gpio_tx(tx),
        gpio_rx(rx),
        latitude(0.0),
        longitude(0.0),
        altitude(0.0),
        doppler_speed(0.0),
        ground_speed(0.0),
        rms_deviation(0.0),
        hdop(0),
        fix_quality(0),
        valid(false),
        year(0),
        month(0),
        day(0),
        hour(0),
        minute(0),
        second(0),
        microseconds(0),
        timestamp(""),
        satellites()
{}

GNSSState::~GNSSState() {}

bool GNSSState::init() {
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,                // 波特率
        .data_bits = UART_DATA_8_BITS,      // 数据位
        .parity = UART_PARITY_DISABLE,      // 无校验位
        .stop_bits = UART_STOP_BITS_1,      // 停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // 无硬件流控
        .source_clk = UART_SCLK_APB,        // 使用 APB 时钟
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, gpio_tx, gpio_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE, 0, 0, NULL, 0);

    mutex = xSemaphoreCreateMutex();

    ESP_LOGI(GNSS_TAG, "GNSS UART initialized on TX=%d, RX=%d", gpio_tx, gpio_rx);

    for (const std::vector<uint8_t> &conf: ubx_conf_command) {
        uint8_t cmd_buff[128];
        std::copy(conf.begin(), conf.end(), cmd_buff);
        do {
            send_ubx_command(cmd_buff, conf.size());
        } while (!wait_for_ack(conf[2], conf[3]));
    }

    return true;
}

void GNSSState::print_debug_info() const {
    std::stringstream debugInfo;
    debugInfo << "GNSS State Debug Info:\n";
    debugInfo << "Latitude: " << latitude << "\n";
    debugInfo << "Longitude: " << longitude << "\n";
    debugInfo << "Altitude: " << altitude << " m\n";
    debugInfo << "Doppler Speed: " << doppler_speed << " m/s\n";
    debugInfo << "Ground Speed: " << ground_speed << " m/s\n";
    debugInfo << "RMS Deviation: " << rms_deviation << "\n";
    debugInfo << "HDOP: " << hdop << "\n";
    debugInfo << "Fix Quality: " << fix_quality << "\n";
    debugInfo << "Timestamp: " << timestamp << "\n";
    debugInfo << "Satellites: " << satellites.size() << "\n";

    // 打印每颗卫星的信息
    for (size_t i = 0; i < satellites.size(); ++i) {
        const Satellite& sat = satellites[i];
        debugInfo << "  Satellite " << i + 1 << ":\n";
        debugInfo << "    Type: " << sat.sat_type << "\n";
        debugInfo << "    Number: " << sat.nr << "\n";
        debugInfo << "    Elevation: " << sat.elevation << " degrees\n";
        debugInfo << "    Azimuth: " << sat.azimuth << " degrees\n";
        debugInfo << "    SNR: " << sat.snr << "\n";
        debugInfo << "    In Use: " << (sat.in_use ? "Yes" : "No") << "\n";
    }

    // 输出到 ESP-IDF 的调试日志
    ESP_LOGI("GNSSState", "%s", debugInfo.str().c_str());
}

bool GNSSState::parse() {
    int length = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(50));

    std::vector<Satellite> satellites_copy(satellites);
    double _latitude;
    double _longitude;
    double _altitude;
    double _doppler_speed;
    double _ground_speed;
    double _rms_deviation;
    int _hdop;
    int _fix_quality;
    int _fix_type;
    bool _valid;
    int _hour, _minute, _second, _microseconds;
    bool update_rmc = false;
    bool update_gga = false;
    bool update_gst = false;
    bool update_gsv = false;
    bool update_gsa = false;

    std::string line;
    for (size_t i = 0; i < length; ++i) {
        if (data[i] == '\n' || data[i] == '\r') {
            struct minmea_sentence_rmc rmc;
            struct minmea_sentence_gga gga;
            struct minmea_sentence_gst gst;
            struct minmea_sentence_gsv gsv;
            struct minmea_sentence_gsa gsa;
            struct minmea_sentence_vtg vtg;
            struct minmea_sentence_zda zda;

            switch (minmea_sentence_id(line.c_str(), false)) {
                case MINMEA_SENTENCE_RMC: {
                    if (minmea_parse_rmc(&rmc, line.c_str())) {
                        _latitude = minmea_tocoord(&rmc.latitude);
                        _longitude = minmea_tocoord(&rmc.longitude);
                        _doppler_speed = minmea_tofloat(&rmc.speed);
                        _hour = rmc.time.hours;
                        _minute = rmc.time.minutes;
                        _second = rmc.time.seconds;
                        _microseconds = rmc.time.microseconds;
                        _valid = rmc.valid;
                        if (_hour < 0 || _minute < 0 || _second < 0) {
                            _hour = 0;
                            _minute = 0;
                            _second = 0;
                        }
                        update_rmc = true;
                    }
                    else {
                        printf("$xxRMC sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GGA: {
                    if (minmea_parse_gga(&gga, line.c_str())) {
                        for (auto iter = satellites_copy.begin(); iter != satellites_copy.end();) {
                            iter->in_track -= 1;
                            if (iter->in_track <= 0) {
                                iter = satellites_copy.erase(iter);
                            } else {
                                iter ++;
                            }
                        }

                        _fix_quality = gga.fix_quality;
                        _hdop = minmea_tofloat(&gga.hdop);
                        _altitude = minmea_tofloat(&gga.altitude);
                        update_gga = true;
                    }
                    else {
                        printf("$xxGGA sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GST: {
                    if (minmea_parse_gst(&gst, line.c_str())) {
                        _rms_deviation = minmea_tofloat(&gst.rms_deviation);
                        update_gst = true;
                    }
                    else {
                        printf("$xxGST sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GSV: {
                    if (minmea_parse_gsv(&gsv, line.c_str())) {
                        for (int i = 0; i < 4; i++) {
                            if (gsv.sats[i].nr == 0) {
                                continue;
                            }
                            int find_same_sat_id = -1;
                            for (int sat_id = 0; sat_id < satellites_copy.size(); sat_id++) {
                                if (satellites_copy[sat_id].sat_type != std::string(gsv.sat_type)
                                        || satellites_copy[sat_id].nr != gsv.sats[i].nr) {
                                    continue;
                                }
                                find_same_sat_id = sat_id;
                                break;
                            }

                            Satellite sat(  gsv.sat_type,
                                            gsv.sats[i].nr,
                                            gsv.sats[i].elevation,
                                            gsv.sats[i].azimuth,
                                            gsv.sats[i].snr
                                        );

                            if (find_same_sat_id == -1) {
                                satellites_copy.push_back(sat);
                            } else {
                                sat.in_use = satellites_copy[find_same_sat_id].in_use - 1;
                                if (sat.in_use < 0) {
                                    sat.in_use = 0;
                                }
                                satellites_copy[find_same_sat_id] = sat;
                            }
                        }
                        update_gsv = true;
                    }
                    else {
                        printf("$xxGSV sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GSA: {
                    if (minmea_parse_gsa(&gsa, line.c_str())) {
                        _fix_type = gsa.fix_type;
                        for(int i = 0; i < GSA_MAX_SAT_NUM; i++) {
                            for (int sat_id = 0; sat_id < satellites_copy.size(); sat_id++) {
                                if (satellites_copy[sat_id].nr != gsa.sats[i]) {
                                    continue;
                                }
                                satellites_copy[sat_id].in_use = GSA_IN_USE_DELAY;
                                break;
                            }
                        }
                        update_gsa = true;
                    }
                    else {
                            printf("$xxGSA sentence is not parsed\n");
                    }
                } break;

                // case MINMEA_SENTENCE_ZDA: {
                //     if (minmea_parse_zda(&zda, line.c_str())) {
                //         year = zda.date.year;
                //         month = zda.date.month;
                //         day = zda.date.day;
                //         hour = zda.time.hours;
                //         minute = zda.time.minutes;
                //         second = zda.time.seconds;
                //         ESP_LOGI("GNSS", "Date-Time: %04d-%02d-%02d %02d:%02d:%02d", 
                //                 year, month, day,
                //                 hour, minute, second);
                //     } else {
                //         printf("$xxZDA sentence is not parsed\n");
                //     }
                //     xSemaphoreGive(mutex);
                // } break;

                default: {
                } break;
            }
            line.clear();
        } else {
            line += data[i];
        }
    }

    while (xSemaphoreTake(mutex, pdMS_TO_TICKS(5)) != pdTRUE) {}
    if (update_rmc) {
        if (valid && _valid) {
            double ts_before = hour + (minute / 60.0) + (second / 3600.0) + (microseconds / 3.6e9);
            double ts_now = _hour + (_minute / 60.0) + (_second / 3600.0) + (_microseconds / 3.6e9);
            std::cout << "time diff: " << ts_before << " " << ts_now << " " << _minute << " " << _second << " " << microseconds << "\n";
            if (ts_now - ts_before > 0) {
                ground_speed = haversine_distance(latitude, longitude, _latitude, _longitude) / (ts_now - ts_before);
            }
        }
        latitude = _latitude;
        longitude = _longitude;
        doppler_speed = _doppler_speed;
        valid = _valid;
        hour = _hour;
        minute = _minute;
        second = _second;
        microseconds = _microseconds;
    }
    if (update_gga) {
        fix_quality = _fix_quality;
        hdop = _hdop;
        altitude = _altitude;
    }
    if (update_gst) {
        rms_deviation = _rms_deviation;
    }
    if (update_gsa) {
        fix_type = _fix_type;
    }
    if (update_gga || update_gsv || update_gsa) {
        satellites = std::move(satellites_copy);
    }
    xSemaphoreGive(mutex);
    return true;
}

bool GNSSState::send_ubx_command(const uint8_t* command, size_t length) {
    int bytes_written = uart_write_bytes(UART_NUM, command, length);
    if (bytes_written < 0) {
        ESP_LOGE(GNSS_TAG, "Failed to write UBX command");
        return false;
    }
    ESP_LOGI(GNSS_TAG, "Sent UBX command, length: %d", bytes_written);
    return true;
}

bool GNSSState::wait_for_ack(uint8_t class_id, uint8_t msg_id) {
    int ack_round = ACK_ROUND;

    while (ack_round--) {
        uint8_t buffer[128];
        int len = uart_read_bytes(UART_NUM, buffer, sizeof(buffer)-10, 50);

        for (int i = 0; i < len; i++) {
            if (buffer[i] == UBX_SYNC_CHAR_1 && buffer[i+1] == UBX_SYNC_CHAR_2 && buffer[i+2] == UBX_ACK) {
                if (buffer[i+3] == UBX_ACK_NAK) {
                    ESP_LOGE(GNSS_TAG, "ERROR: UBX NAK");
                    return false;
                }
                if (buffer[i+3] == UBX_ACK_ACK && buffer[i+6] == class_id && buffer[i+7] == msg_id) {
                    ESP_LOGI(GNSS_TAG, "SUCCESS: UBX ACK");
                    return true;
                }
            }
        }
    }

    ESP_LOGE(GNSS_TAG, "Timeout waiting for UBX ACK");
    return false;
}

double GNSSState::haversine_distance(double lat1, double lon1, double lat2, double lon2) {
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