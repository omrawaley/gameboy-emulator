#pragma once

#include "types.h"
#include "string"

#include "bus.h"
#include "cart.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "interrupts.h"

class GameBoy
{
    private:
        std::string bootROMPath;
        std::string romPath;

        Bus bus;
        Cart cart;
        CPU cpu;
        Timer timer;
        PPU ppu;
        Joypad joypad;
        Interrupts interrupts;

    public:
        static bool skipBootROM;

        GameBoy();

        void reboot();

        std::array<u8, 160 * 144 * 3>& getFramebuffer();

        std::string getTitle();

        void pressButton(Joypad::Button button);
        void releaseButton(Joypad::Button button);

        void loadBootROM(std::string path);
        void loadROM(std::string path);

        void step();
        
        // void createGameBoyDoctorLog();
        // void initNcurses();
        // void uninitNcurses();
        // void ncursesDrawDebugger();
};