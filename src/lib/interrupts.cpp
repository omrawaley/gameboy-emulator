#include "interrupts.h"

#include "cpu.h"
#include "gameboy.h"

Interrupts::Interrupts()
{
    this->restart();
}

void Interrupts::restart()
{
    this->ime = true;
    this->enable = 0;

    if(GameBoy::skipBootROM)
        this->flag = 0xE1;
    else
        this->flag = 0;
}

bool Interrupts::getFlag(Interrupt interrupt)
{
    return this->flag & static_cast<u8>(interrupt);
}

void Interrupts::setFlag(Interrupt interrupt, bool val)
{
    if(val)
        this->flag |= static_cast<u8>(interrupt);
    else
        this->flag &= ~static_cast<u8>(interrupt);
}

#include <stdio.h>

void Interrupts::fire(Interrupt interrupt, CPU& cpu)
{
    this->ime = false;
    cpu.PUSH(cpu.pc);
    cpu.halted = false;

    switch(interrupt)
    {
        case Interrupt::VBlank:
            this->setFlag(Interrupt::VBlank, false);
            cpu.pc = 0x40;
            // printf("VBlank");
            break;
        case Interrupt::LCD:
            this->setFlag(Interrupt::LCD, false);
            cpu.pc = 0x48;
            // printf("LCD");
            break;
        case Interrupt::Timer:
            this->setFlag(Interrupt::Timer, false);
            cpu.pc = 0x50;
            // printf("Timer");
            break;
        case Interrupt::Serial:
            this->setFlag(Interrupt::Serial, false);
            cpu.pc = 0x58;
            // printf("Serial");
            break;
        case Interrupt::Joypad:
            this->setFlag(Interrupt::Joypad, false);
            cpu.pc = 0x60;
            // printf("Joypad");
            break;
    }
}

bool Interrupts::check(CPU& cpu)
{
    if(!this->ime)
        return false;

    if(!(this->flag & this->enable))
        return false;

    // if(this->getFlag(Interrupt::VBlank))
    // {
    //     this->fire(Interrupt::VBlank, cpu);
    //     return true;
    // }

    // if(this->getFlag(Interrupt::LCD))
    // {
    //     this->fire(Interrupt::LCD, cpu);
    //     return true;
    // }

    // if(this->getFlag(Interrupt::Timer))
    // {
    //     this->fire(Interrupt::Timer, cpu);
    //     return true;
    // }

    // if(this->getFlag(Interrupt::Serial))
    // {
    //     this->fire(Interrupt::Serial, cpu);
    //     return true;
    // }

    // if(this->getFlag(Interrupt::Joypad))
    // {
    //     this->fire(Interrupt::Joypad, cpu);
    //     return true;
    // }

    for (u8 i = 0; i < 5; ++i) 
    {
        Interrupts::Interrupt interrupt = static_cast<Interrupts::Interrupt>(1 << i);

        if (this->getFlag(interrupt)) 
        {
            this->fire(interrupt, cpu);
            return true;
        }
    }

    return false;
}