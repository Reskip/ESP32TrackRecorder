#include <vector>
#include <string>

#include "sidebar_page.h"
#include "button.h"

#ifndef WIFI_PAGE
#define WIFI_PAGE

class WifiPage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    WifiPage();

    // 虚析构函数
    virtual ~WifiPage() = default;
    bool handle_press(Context& context, OLED &oled) override;
    bool handle_scroll(Context& context, OLED &oled, int value) override;

private:
    void render_expand(Context &context, OLED &oled) override;
    void select_new_btn(int btn_id, OLED &oled);
    std::vector<Button> buttons;
    double select_x, select_y, select_w, select_h;
    double target_select_x, target_select_y, target_select_w, target_select_h;
    double select_progress;
};

#endif