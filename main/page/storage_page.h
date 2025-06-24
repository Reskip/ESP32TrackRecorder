#include <vector>
#include <string>

#include "sidebar_page.h"

#ifndef BLUETOOTH_PAGE
#define BLUETOOTH_PAGE

class StoragePage : public SidebarPage {
public:
    // 构造函数，初始化页面名称和选项列表
    StoragePage();

    // 虚析构函数
    virtual ~StoragePage() = default;

private:
    void render_expand(Context &context, OLED &oled) override;
};

#endif