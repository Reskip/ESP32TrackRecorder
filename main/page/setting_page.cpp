#include <sstream>
#include <iomanip>
#include <iostream>

#include "setting_page.h"
#include "icon/setting_icon.h"

SettingPage::SettingPage()
    : SidebarPage("SettingPage", (char*) setting_icon) {}

void SettingPage::render_expand(Context &context, OLED &oled) {
    return;
}