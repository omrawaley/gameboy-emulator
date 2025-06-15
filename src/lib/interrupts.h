#pragma once

#include "types.h"

class CPU;

class Interrupts
{
    public:
        enum class Interrupt : u8
        {
            VBlank = 1,
            LCD = 2,
            Timer = 4,
            Serial = 8,
            Joypad = 16,
        };

    private:
        bool ime;
        u8 flag;
        u8 enable;

        friend class Bus;
        friend class GameBoy;
        friend class CPU;

        void fire(Interrupt interrupt, CPU& cpu);

    public:
        Interrupts();

        void restart();

        bool getFlag(Interrupt interrupt);
        void setFlag(Interrupt interrupt, bool val);

        bool check(CPU& cpu);

};