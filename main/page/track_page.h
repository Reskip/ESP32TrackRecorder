#include <vector>
#include <string>

#include "sidebar_page.h"

#ifndef TRACK_PAGE
#define TRACK_PAGE

class TrackPage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    TrackPage();

    // 虚析构函数
    virtual ~TrackPage() = default;

private:
    void render_expand(Context &context, OLED &oled) override;
};

#endif