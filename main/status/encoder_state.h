#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#ifndef ENCODERSTATE
#define ENCODERSTATE

#define ENCODER_TAG "EncoderState"
#define DEBOUNCE_TIME_MS 50

typedef struct {
    int position;
} encoder_event_t;

class Encoder {
public:
    Encoder(
        gpio_num_t ec_a, gpio_num_t ec_b, gpio_num_t ec_key);
    ~Encoder();

    bool init();
    void encoder_press_handler();
    QueueHandle_t encoder_queue;
    QueueHandle_t press_queue;

private:
    gpio_num_t ec_a;
    gpio_num_t ec_b;
    gpio_num_t ec_key;

    uint32_t last_press_key;
    uint32_t last_interrupt_time;

    uint8_t encoder_a_last;
    uint8_t encoder_b_last;
    uint8_t value_count;

    uint8_t press_status;
    static void IRAM_ATTR encoder_isr_scroll_handler(void *arg);
};

#endif