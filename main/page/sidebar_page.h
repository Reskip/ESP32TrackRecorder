// SidebarPage.h
#ifndef SIDEBAR_PAGE_H
#define SIDEBAR_PAGE_H

#include "esp_system.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include "page.h"
#include "button.h"

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
    virtual bool handle_press(Context& context, OLED &oled);
    virtual bool handle_scroll(Context& context, OLED &oled, int value);

    void set_init_position(int index, int item_num);
    void set_target_position(int index, int item_num);
    double get_sidebar_start_pos_x();
    void smooth_easing_move(double &current, double target, double &progress, double speed, bool update_progress=true);
    double lock_fps_speed(Context &context, double speed);
    bool is_animating() override;
    void debug();

protected:
    virtual void render_expand(Context &context, OLED &oled) = 0;
    void render_icon(Context &context, OLED &oled);
    double index_to_position(int index, int item_num);
    double ease_out_quad(double progress);

    double sidebar_current_position;
    double sidebar_target_position;
    double sidebar_progress;

    double expand_current_position;
    double expand_target_position;
    double expand_progress;

    double animation_frame;

    std::vector<Button> buttons;
    double select_x, select_y, select_w, select_h;
    double target_select_x, target_select_y, target_select_w, target_select_h;
    double select_progress;

    char* sidebar_icon;
};

#endif