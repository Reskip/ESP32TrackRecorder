file(GLOB_RECURSE SOURCES 
    "*.cpp"
    "page/*.cpp"
    "status/*.cpp"
    "icon/*.cpp"
    "utils/*.cpp"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES minmea driver oled esp_psram fatfs esp_timer esp_wifi esp_http_server)