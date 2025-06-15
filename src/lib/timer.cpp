#include "timer.h"

#include "gameboy.h"

Timer::Timer(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts)
{
    this->restart();
}

void Timer::restart()
{
    tima = 0;
    tma = 0;    
    divCycleCounter = 0; 
    timaCycleCounter = 0;

    if(GameBoy::skipBootROM)
    {
        tac = 0xF8;
        div = 0xAB;
    }
    else
    {
        tac = 0;
        div = 0;
    }
}

void Timer::step(const u8 cycles)
{
    this->divCycleCounter += cycles;

    while(this->divCycleCounter >= 256)
    {
        this->divCycleCounter -= 256;
        ++this->div;
    }

    if(!(this->tac & 0x4))
        return;

    this->timaCycleCounter += cycles;

    u16 frequency = 0;
    switch(this->tac & 0x3)
    {
        case 0:
            frequency = 1024;
            break;
        case 1:
            frequency = 16;
            break;
        case 2:
            frequency = 64;
            break;
        case 3:
            frequency = 256;
            break;
    }

    while(this->timaCycleCounter >= frequency)
    {
        this->timaCycleCounter -= frequency;
        ++this->tima;

        if(this->tima == 0)
        {
            this->tima = this->tma;
            this->interrupts.setFlag(Interrupts::Interrupt::Timer, true);
        }
    }
}