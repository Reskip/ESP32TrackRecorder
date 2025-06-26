#ifndef BUTTON_H
#define BUTTON_H

#include <vector>
#include <string>

#include "ssd1306.h"
#include "context.h"

class Button;
typedef void (*Callback)(Button*, Context*);

class Button {
public:
    enum Type {
        PRESS,
        SCROLL,
        ICON_ONLY,
    };
    
    Button(Type type, const std::string& description, int x, int y);

    void set_callback(Callback callback);
    void configure_press(bool initial_state = false);
    void configure_scroll(const std::vector<int>& values = {}, size_t initial_index = 0);
    void configure_icon(char* button_icon, int icon_w, int icon_h, bool show_icon_value);
    void render(Context &context, OLED &oled, int start_x);
    bool handle_press(Context &context);

    Type get_type() const;
    int get_output_weight(OLED &oled);
    int get_x_pos() const;
    int get_y_pos() const;
    bool get_press_state() const;

private:
    Type type_;
    std::string description_;
    int x_pos_;
    int y_pos_; 
    char* button_icon_;
    int icon_w_, icon_h_;
    bool show_icon_value_;

    bool press_state_;
    std::vector<int> scroll_values_;
    size_t scroll_index_;

    Callback callback_;
};

#endif