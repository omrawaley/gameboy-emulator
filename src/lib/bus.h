#pragma once

#include "types.h"

class Cart;
class CPU;
class Timer;
class PPU;
class Joypad;
class Interrupts;

class Bus
{
    private:
        u8 bootRom[0x100];
        u8 vram[0x2000];
        u8 wram[0x2000];
        u8 oam[0xA0];
        u8 hram[0x7F];

        bool disableBootRom;

        Cart& cart;
        CPU& cpu;
        Timer& timer;
        PPU& ppu;
        Joypad& joypad;
        Interrupts& interrupts;

        friend class GameBoy;

    public:
        Bus(Cart& cart, CPU& cpu, Timer& timer, PPU& ppu, Joypad& joypad, Interrupts& interrupts);

        void restart();

        u8 readByte(const u16 addr) const;
        void writeByte(const u16 addr, const u8 val);
        u16 readWord(const u16 addr) const;
        void writeWord(const u16 addr, const u16 val);
};