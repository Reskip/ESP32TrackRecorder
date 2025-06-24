// SidebarPage.h
#ifndef SIDEBAR_PAGE_H
#define SIDEBAR_PAGE_H

#include "page.h"
#include "esp_system.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#define SCROLL_SPEED 50.0
#define HIDE_SCROLL_SPEED 1.0
#define ANIMATION_SPEED 60.0
#define ANIMATION_FRAME_CNT 40

#define SIDEBAR_ICON_WIDTH 24
#define SIDEBAR_ICON_HEIGHT 24
#define SIDEBAR_ICON_X_START 100
#define SIDEBAR_ICON_MARGIN 4
#define CENTER_ICON 2

#define SIDEBAR_PAGE_TAG "SidebarPage"

class SidebarPage : public Page {
public:
    SidebarPage(const std::string& name, char* icon);
    virtual ~SidebarPage() = default;

    void render(Context& context, OLED& oled) override;
    void switch_state();

    void set_init_position(int index, int item_num);
    void set_target_position(int index, int item_num);
    double get_sidebar_start_pos_x();
    void debug();

private:
    virtual void render_expand(Context &context, OLED &oled) = 0;
    void render_icon(Context &context, OLED &oled);
    double lock_fps_speed(Context &context, double speed);
    double index_to_position(int index, int item_num);
    double ease_out_quad(double progress);
    void smooth_easing_move(double &current, double target, double &progress, double speed);

    double sidebar_current_position;
    double sidebar_target_position;
    double sidebar_progress;
    double sidebar_speed_delta;

    double expand_current_position;
    double expand_target_position;
    double expand_progress;

    double animation_frame;

    char* sidebar_icon;
};

#endif