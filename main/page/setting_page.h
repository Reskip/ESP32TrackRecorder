#include <vector>
#include <string>

#include "sidebar_page.h"

#ifndef SETTING_PAGE
#define SETTING_PAGE

class SettingPage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    SettingPage();

    // 虚析构函数
    virtual ~SettingPage() = default;
    bool handle_press(Context& context, OLED &oled) override;
    bool handle_scroll(Context& context, OLED &oled, int value) override;

private:
    void render_expand(Context &context, OLED &oled) override;
    void select_new_btn(int btn_id, OLED &oled, bool immidiate=false);
};

#endif