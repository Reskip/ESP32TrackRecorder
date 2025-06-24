#include "page.h"
#include <vector>
#include <string>

#ifndef MAIN_PAGE
#define MAIN_PAGE

class MainPage : public Page {
public:
    // 构造函数，初始化页面名称和选项列表
    MainPage();

    // 虚析构函数
    virtual ~MainPage() = default;
    void render(Context& context, OLED& oled) override;

private:
    void render_normal(Context &context, OLED &oled);
};

#endif