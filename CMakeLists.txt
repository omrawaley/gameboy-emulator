cmake_minimum_required(VERSION 3.2)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)

project(gameboy_emulator)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
find_package(SDL2_image REQUIRED COMPONENTS SDL2_image)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(gb 
    src/test/main.cpp 
    src/test/app.cpp 
    src/test/gui.cpp
    src/lib/cpu.cpp 
    src/lib/bus.cpp 
    src/lib/cart.cpp 
    src/lib/gameboy.cpp 
    src/lib/timer.cpp 
    src/lib/ppu.cpp 
    src/lib/joypad.cpp 
    src/lib/interrupts.cpp 
    src/lib/util.cpp 
    src/lib/mbc.cpp
    deps/imgui/imgui.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/backends/imgui_impl_sdl2.cpp
    deps/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

target_link_libraries(gb PRIVATE SDL2::SDL2)
target_link_libraries(gb PRIVATE SDL2_image::SDL2_image)
target_link_libraries(gb PRIVATE ${CURSES_LIBRARIES})
