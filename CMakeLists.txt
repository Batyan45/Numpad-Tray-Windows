cmake_minimum_required(VERSION 3.16)
project(NumpadTray VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

# Include directory
include_directories(include)

# Windows GUI application
add_executable(NumpadTray WIN32
    src/main.cpp
    src/config.cpp
    src/button.cpp
    src/popup.cpp
    src/hotkeys.cpp
    resources.rc
)

target_link_libraries(NumpadTray PRIVATE
    dwmapi
    shell32
)

set_target_properties(NumpadTray PROPERTIES
    OUTPUT_NAME "NumpadTray"
)
