#include "button.h"
#include <iostream>
#include <sstream>

Button::Button(Type type, const std::string& description, int x, int y)
    : type_(type), description_(description), x_pos_(x), y_pos_(y),
      button_icon_(nullptr), icon_w_(0), icon_h_(0), show_icon_value_(false),
      press_state_(false), scroll_index_(0), callback_(nullptr) {
}

void Button::configure_press(bool initial_state) {
    if (type_ != PRESS) {
        std::cerr << "Error: Cannot configure as PRESS button.";
        return;
    }
    press_state_ = initial_state;
}

void Button::configure_scroll(const std::vector<std::string>& names, const std::vector<int>& values, size_t initial_index) {
    if (type_ != SCROLL) {
        std::cerr << "Error: Cannot configure as SCROLL button.";
        return;
    }
    scroll_names_ = names;
    scroll_values_ = values;
    scroll_index_ = initial_index;
    if (scroll_index_ >= scroll_values_.size()) {
        scroll_index_ = 0;
    }
}

void Button::configure_icon(char* button_icon, int icon_w, int icon_h, bool show_icon_value) {
    if (type_ != ICON_ONLY) {
        std::cerr << "Error: Cannot configure as ICON button.";
        return;
    }
    button_icon_ = button_icon;
    icon_w_ = icon_w;
    icon_h_ = icon_h;
    show_icon_value_ = show_icon_value;
}

void Button::render(Context &context, OLED &oled, int start_x) {
    std::ostringstream button_stream;
    if (type_ == PRESS) {
        button_stream << description_ << " ";
        button_stream << (press_state_ ? " ON" : "OFF");
    } else if (type_ == SCROLL) {
        button_stream << description_ << " ";
        button_stream << scroll_names_[scroll_index_];
    } else if (type_ == ICON_ONLY) {
        oled.draw_bitmap(start_x + x_pos_, y_pos_,
            (const uint8_t*) button_icon_, icon_w_, icon_h_);
        return;
    }
    oled.draw_string(start_x + x_pos_, y_pos_, button_stream.str().c_str(), WHITE, BLACK);
}

bool Button::handle_press(Context &context) {
    switch (type_) {
        case PRESS:
            press_state_ = !press_state_;
            break;

        case SCROLL:
            scroll_index_ = (scroll_index_ + 1) % scroll_values_.size();
            break;

        case ICON_ONLY:
            break;

        default:
            std::cerr << "Error: Unknown button type!" << std::endl;
            return false;
    }
    if (callback_) {
        callback_(this, &context);
    }
    return true;
}

Button::Type Button::get_type() const {
    return type_;
}

int Button::get_output_weight(OLED &oled) {
    if (type_ == PRESS) {
        return oled.measure_string(description_ + " OFF");
    } else if (type_ == SCROLL) {
        return oled.measure_string(description_ + " " + scroll_names_[scroll_index_]);
    } else if (type_ == ICON_ONLY) {
        return icon_w_;
    }
    return 0;
}

int Button::get_x_pos() const {
    return x_pos_;
}

int Button::get_y_pos() const {
    return y_pos_;
}

bool Button::get_press_state() const {
    return press_state_;
}

int Button::get_scroll_state() const {
    return scroll_values_[scroll_index_];
}

void Button::set_callback(Callback callback) {
    callback_ = callback;
}