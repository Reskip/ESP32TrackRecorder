#include <string>
#include <iostream>

#include "ssd1306.h"
#include "context.h"

#ifndef PAGE
#define PAGE

// 定义页面状态枚举

#define WIDTH 128
#define HEIGHT 32
#define FPS 60

enum PageState {
    NORMAL,
    EXPANDED,
};

// 页面基类
class Page {
public:
    // 构造函数
    Page(const std::string& name) : name(name), state(PageState::NORMAL) {}

    // 获取页面名称
    std::string get_name() const {
        return name;
    }

    // 获取页面状态
    PageState get_state() const {
        return state;
    }

    // 设置页面状态
    void set_state(PageState new_state) {
        state = new_state;
    }

    // 正常状态渲染接口
    virtual void render(Context &context, OLED &oled) = 0;

    // 虚析构函数
    virtual ~Page() = default;

private:
    std::string name;
    PageState state;
};

#endif // page.h