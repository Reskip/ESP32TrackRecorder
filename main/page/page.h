#include <string>
#include <iostream>

#include "ssd1306.h"
#include "context.h"

#ifndef PAGE
#define PAGE

#define WIDTH 128
#define HEIGHT 32
#define FPS 60

enum PageState {
    NORMAL,
    EXPANDED,
};

class Page {
public:
    Page(const std::string& name) : name(name), state(PageState::NORMAL) {}

    std::string get_name() const {
        return name;
    }

    PageState get_state() const {
        return state;
    }

    void set_state(PageState new_state) {
        state = new_state;
    }

    virtual void render(Context &context, OLED &oled) = 0;
    virtual bool is_animating() { return false; }
    virtual ~Page() = default;

private:
    std::string name;
    PageState state;
};

#endif // page.h