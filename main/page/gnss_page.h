#include <vector>
#include <string>

#include "sidebar_page.h"

#ifndef GNSS_PAGE
#define GNSS_PAGE

class GNSSPage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    GNSSPage();

    // 虚析构函数
    virtual ~GNSSPage() = default;

private:
    void render_expand(Context &context, OLED &oled) override;
};

#endif