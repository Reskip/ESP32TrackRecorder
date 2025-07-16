#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <string>
#include <memory>
#include <map>
#include "ssd1306.h"
#include "page/page.h"
#include "page/sidebar_page.h"

#define THOUSAND 1000.0
#define MIN_REFRESH_FRAME 10
#define SKIP_FRAME_TIME_MS 20
#define DISPLAY_TAG "Display"

class DisplayManager {
public:
    DisplayManager(gpio_num_t scl, gpio_num_t sda);
    ~DisplayManager();

    bool init();
    void updateDisplay(Context &context);

private:
    OLED oled;
    std::unique_ptr<Page> main_page;
    std::vector<std::unique_ptr<SidebarPage>> side_pages;
};

#endif // DISPLAY_MANAGER_H
