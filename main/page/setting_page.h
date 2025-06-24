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

private:
    void render_expand(Context &context, OLED &oled) override;
};

#endif