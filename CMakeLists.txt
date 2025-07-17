# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME "ESPTrackRecorder")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# idf_build_set_property(MINIMAL_BUILD ON)
project(${PROJECT_NAME})