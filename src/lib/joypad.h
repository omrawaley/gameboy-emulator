#pragma once

#include "types.h"
#include "bus.h"
#include "interrupts.h"

class Joypad
{
    private:
        u8 joyp;
        u8 actionButtonState;
        u8 directionalButtonState;

        bool areActionButtonsSelected();
        bool areDirectionalButtonsSelected();

        Interrupts& interrupts;
        Bus& bus;
        friend class Bus;
        friend class GameBoy;

    public:
        enum class Button : u8
        {
            A,
            B,
            Start,
            Select,
            Up,
            Down,
            Left,
            Right,
        };

        Joypad(Bus& bus, Interrupts& interrupts);

        void restart();

        void pressButton(Button button);
        void releaseButton(Button button);

        void checkButtons();
};