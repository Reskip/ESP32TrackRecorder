#include <vector>
#include <string>

#include "sidebar_page.h"

#ifndef STATUS_PAGE
#define STATUS_PAGE

class StatusPage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    StatusPage();

    // 虚析构函数
    virtual ~StatusPage() = default;

private:
    void render_expand(Context &context, OLED &oled) override;
};

#endif