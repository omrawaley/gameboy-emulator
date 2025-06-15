#pragma once

#include <SDL2/SDL.h>
#include "../../deps/imgui/imgui.h"
#include "../../deps/imgui/backends/imgui_impl_sdl2.h"
#include "../../deps/imgui/backends/imgui_impl_sdlrenderer2.h"

class App;

namespace GUI
{
    extern bool menuEnable;

    void init(SDL_Window* window, SDL_Renderer* renderer);
    void free();

    void processEvent(SDL_Event& event);

    void drawMenu(App& app);
    void draw(SDL_Renderer* renderer, App& app);
};