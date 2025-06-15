#pragma once

#include <SDL2/SDL.h>
#include "../lib/gameboy.h"
#include "gui.h"

class App
{
    private:
        std::chrono::time_point<std::chrono::steady_clock> lastCycleTime;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* displayTexture;

        void loadMedia();
        void freeMedia();

    public:
        static float refreshRatePeriod;

        bool quit;

        GameBoy gameboy;

        App();
        ~App();

        void start(std::string bootROMPath, std::string ROMPath);
        void update();
        void draw();
};