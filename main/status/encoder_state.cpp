#include "esp_log.h"
#include "status/encoder_state.h"

Encoder::Encoder(
    gpio_num_t ec_a, gpio_num_t ec_b, gpio_num_t ec_key)
    : ec_a(ec_a), ec_b(ec_b), ec_key(ec_key), last_interrupt_time(0) {
    press_status = 0;
}

Encoder::~Encoder() {
    gpio_isr_handler_remove(ec_a);
    gpio_isr_handler_remove(ec_key);
}

bool Encoder::init() {
    encoder_queue = xQueueCreate(50, sizeof(encoder_event_t));
    press_queue = xQueueCreate(50, sizeof(encoder_event_t));

    gpio_config_t io_conf_a = {};
    gpio_config_t io_conf_b = {};
    gpio_config_t io_conf_key = {};

    io_conf_a.intr_type = GPIO_INTR_NEGEDGE;
    io_conf_a.mode = GPIO_MODE_INPUT;
    io_conf_a.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf_a.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_a.pin_bit_mask = (1ULL << ec_a);
    gpio_config(&io_conf_a);

    io_conf_b.intr_type = GPIO_INTR_DISABLE;
    io_conf_b.mode = GPIO_MODE_INPUT;
    io_conf_b.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf_b.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_b.pin_bit_mask = (1ULL << ec_b);
    gpio_config(&io_conf_b);

    io_conf_key.intr_type = GPIO_INTR_DISABLE;
    io_conf_key.mode = GPIO_MODE_INPUT;
    io_conf_key.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf_key.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf_key.pin_bit_mask = (1ULL << ec_key);
    gpio_config(&io_conf_key);

    gpio_isr_handler_add(ec_a, encoder_isr_scroll_handler, (void*)this);
    return true;
}

void IRAM_ATTR Encoder::encoder_isr_scroll_handler(void *arg) {
    Encoder *encoder = static_cast<Encoder *>(arg);

    encoder_event_t event;
    int A = gpio_get_level(encoder->ec_a);
    int B = gpio_get_level(encoder->ec_b);
    if (A == 0 || !encoder->press_status) {
        return;
    }
    event.position = (A == B) ? -1 : 1;

    uint32_t current_time = xTaskGetTickCountFromISR();
    uint32_t min_diff_time = DEBOUNCE_SAME_TIME_MS;
    if (event.position != encoder->last_press_key) {
        min_diff_time = DEBOUNCE_SWITCH_TIME_MS;
    }
    if (current_time - encoder->last_interrupt_time < pdMS_TO_TICKS(min_diff_time)) {
        return;
    }
    encoder->last_interrupt_time = current_time;
    encoder->last_press_key = event.position;

    xQueueSendFromISR(encoder->encoder_queue, &event, NULL);
}

void Encoder::encoder_press_handler() {
    uint32_t current_time = xTaskGetTickCountFromISR();
    if (current_time - last_interrupt_time < pdMS_TO_TICKS(DEBOUNCE_SAME_TIME_MS)) {
        return;
    }
    
    uint8_t now_key_value = gpio_get_level(ec_key);
    if (press_status == now_key_value) {
        return;
    }

    encoder_event_t event;
    event.position = press_status == 1 ? 1 : 0;
    last_press_key = event.position;

    press_status = now_key_value;
    last_interrupt_time = current_time;
    xQueueSend(press_queue, &event, pdMS_TO_TICKS(0));
}