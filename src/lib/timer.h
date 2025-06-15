#pragma once

#include "types.h"
#include "bus.h"
#include "interrupts.h"

class Timer
{
    private:
        u16 div;
        u8 tima;
        u8 tma;
        u8 tac;

        u16 divCycleCounter;
        u8 timaCycleCounter;

        Interrupts& interrupts;
        Bus& bus;
        friend class Bus;

    public:
        Timer(Bus& bus, Interrupts& interrupts);

        void restart();

        void step(const u8 cycles);
};