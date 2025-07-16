// SidebarPage.cpp
#include "sidebar_page.h"

SidebarPage::SidebarPage(const std::string& name, char* icon) : Page(name) {
    set_state(PageState::NORMAL);
    sidebar_icon = icon;
    animation_frame = ANIMATION_FRAME_CNT;
}

void SidebarPage::render(Context& context, OLED& oled) {
    render_icon(context, oled);
    if (get_state() == PageState::EXPANDED || expand_progress < 1.0) {
        render_expand(context, oled);
    }
}

void SidebarPage::switch_state() {
    switch (get_state()) {
        case PageState::EXPANDED: {
            set_state(PageState::NORMAL);
            expand_progress = 0.0;
            expand_target_position = SIDEBAR_ICON_X_START;
        } break;

        case PageState::NORMAL: {
            set_state(PageState::EXPANDED);
            expand_progress = 0.0;
            expand_target_position = SIDEBAR_ICON_MARGIN;
        } break;
        default:
            break;
    }
}

bool SidebarPage::handle_press(Context& context, OLED &oled) {
    return false;
}

bool SidebarPage::handle_scroll(Context& context, OLED &oled, int value) {
    return false;
}

void SidebarPage::set_init_position(int index, int item_num) {
    sidebar_progress = 1.0;
    expand_progress = 1.0;
    select_progress = 1.0;
    sidebar_current_position = index_to_position(index, item_num);
    sidebar_target_position = index_to_position(index, item_num);
    expand_current_position = SIDEBAR_ICON_X_START;
    expand_target_position = SIDEBAR_ICON_X_START;
}

void SidebarPage::set_target_position(int index, int item_num) {
    sidebar_progress = 0.0;
    animation_frame = 0.0;
    expand_progress = 1.0;
    sidebar_target_position = index_to_position(index, item_num);
}

double SidebarPage::get_sidebar_start_pos_x() {
    return expand_current_position;
}

void SidebarPage::debug() {
    ESP_LOGI(SIDEBAR_PAGE_TAG, "x: %lf, y: %lf, target_x: %lf, target_y: %lf",
        expand_current_position,
        sidebar_current_position,
        expand_target_position,
        sidebar_target_position
    );
}

void SidebarPage::render_icon(Context &context, OLED &oled) {
    double speed = SCROLL_SPEED;
    if ((sidebar_current_position < -1 * HEIGHT || sidebar_current_position > HEIGHT) &&
        (sidebar_target_position < -1 * HEIGHT || sidebar_target_position > HEIGHT)) {
        speed = HIDE_SCROLL_SPEED;
    }

    smooth_easing_move(sidebar_current_position, sidebar_target_position, sidebar_progress, lock_fps_speed(context, speed));
    smooth_easing_move(expand_current_position, expand_target_position, expand_progress, lock_fps_speed(context, SCROLL_SPEED));

    if (sidebar_progress >= 1.0 && animation_frame < ANIMATION_FRAME_CNT) {
        animation_frame += lock_fps_speed(context, ANIMATION_SPEED) * ANIMATION_FRAME_CNT;
    }
    int now_frame = std::min((int) std::round(animation_frame), ANIMATION_FRAME_CNT);

    if (get_state() == PageState::EXPANDED || expand_progress < 1.0) {
        oled.fill_rectangle(
            std::round(expand_current_position) - (get_state() == PageState::EXPANDED ? SIDEBAR_ICON_MARGIN : 0),
            0, WIDTH, HEIGHT, BLACK);
    }

    oled.draw_bitmap(std::round(expand_current_position), std::round(sidebar_current_position),
                     (const uint8_t*) sidebar_icon + (now_frame * SIDEBAR_ICON_WIDTH * SIDEBAR_ICON_HEIGHT),
                     SIDEBAR_ICON_WIDTH, SIDEBAR_ICON_HEIGHT);
}

double SidebarPage::lock_fps_speed(Context &context, double speed) {
    return (context.fresh_ts_diff_ms / 1000.0) / (speed / FPS);
}

double SidebarPage::index_to_position(int index, int item_num) {
    return (index - CENTER_ICON) * HEIGHT + SIDEBAR_ICON_MARGIN;
}

double SidebarPage::ease_out_quad(double progress) {
    return 1.0 - (1.0 - progress) * (1.0 - progress);
}

void SidebarPage::smooth_easing_move(double &current, double target, double &progress, double speed, bool update_progress) {
    if (progress >= 1.0) {
        return;
    }
    if (update_progress) {
        progress = std::min(1.0, progress + speed);
    }
    double eased_progress = ease_out_quad(progress);
    current = current + (target - current) * eased_progress;
}

bool SidebarPage::is_animating() {
    return sidebar_progress < 1.0
        || expand_progress < 1.0
        || (!buttons.empty() && select_progress < 1.0)
        || animation_frame < ANIMATION_FRAME_CNT;
}