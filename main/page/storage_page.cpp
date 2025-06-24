#include <sstream>
#include <iomanip>
#include <iostream>

#include "storage_page.h"
#include "icon/storage_icon.h"

StoragePage::StoragePage()
    : SidebarPage("StoragePage", (char*) storage_icon) {}

void StoragePage::render_expand(Context &context, OLED &oled) {
    return;
}