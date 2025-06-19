#include "bus.h"

#include <string.h>
#include <iostream>
#include <format>

#include "cart.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "joypad.h"
#include "interrupts.h"
#include "util.h"
#include "gameboy.h"
#include "error.h"

Bus::Bus(Cart& cart, CPU& cpu, Timer& timer, PPU& ppu, Joypad& joypad, Interrupts& interrupts) : disableBootRom(false), cart(cart), cpu(cpu), timer(timer), ppu(ppu), joypad(joypad), interrupts(interrupts)
{
    this->restart();
}

void Bus::restart()
{
    memset(this->bootRom, 0, 0x100);
    memset(this->vram, 0, 0x2000);
    memset(this->wram, 0, 0x2000);
    memset(this->oam, 0, 0xA0);
    memset(this->hram, 0, 0x7F);

    this->disableBootRom = GameBoy::skipBootROM ? true : false;
}

u8 Bus::readByte(const u16 addr) const
{
    if(!this->disableBootRom)
    {
        if(Util::isAddressBetween(addr, 0x0000, 0x00FF))
            return this->bootRom[addr];

        if(Util::isAddressBetween(addr, 0x0100, 0x7FFF))
            return this->cart.readByte(addr);
    }
    else
    {
        if(Util::isAddressBetween(addr, 0x0000, 0x7FFF))
            return this->cart.readByte(addr);
    }

    if(Util::isAddressBetween(addr, 0x8000, 0x9FFF))
        return this->vram[addr - 0x8000];

    if(Util::isAddressBetween(addr, 0xA000, 0xBFFF))
        return this->cart.readByte(addr);
    
    if(Util::isAddressBetween(addr, 0xC000, 0xDFFF))
        return this->wram[addr - 0xC000];

    if(Util::isAddressBetween(addr, 0xE000, 0xFDFF))
        return this->wram[addr - 0xE000];

    if(Util::isAddressBetween(addr, 0xFE00, 0xFE9F))
        return this->oam[addr - 0xFE00];

    if(Util::isAddressBetween(addr, 0xFF80, 0xFFFE))
        return this->hram[addr - 0xFF80];

    switch(addr)
    {
        case 0xFF00:
            return this->joypad.joyp;
            break;
        case 0xFF04:
            return (this->timer.div & 0xFF00) >> 8;
            break;
        case 0xFF05:
            return this->timer.tima;
            break;
        case 0xFF06:
            return this->timer.tma;
            break;
        case 0xFF07:
            return this->timer.tac;
            break;
        case 0xFF0F:
            return this->interrupts.flag & 0x1F;
            break;
        case 0xFF40:
            return this->ppu.lcdc;
            break;
        case 0xFF41:
            return this->ppu.stat;
            break;
        case 0xFF42:
            return this->ppu.scy;
            break;
        case 0xFF43:
            return this->ppu.scx;
            break;
        case 0xFF44:
            return this->ppu.ly;
            // return 0x90; // Needed for Game Boy Doctor
            break;
        case 0xFF45:
            return this->ppu.lyc;
            break;
        // case 0xFF46:
        //     return this->ppu.dma;
        //     break;
        case 0xFF47:
            return this->ppu.bgp;
            break;
        case 0xFF48:
            return this->ppu.obp0;
            break;
        case 0xFF49:
            return this->ppu.obp1;
            break;
        case 0xFF4A:
            return this->ppu.wy;
            break;
        case 0xFF4B:
            return this->ppu.wx;
            break;
        case 0xFFFF:
            return this->interrupts.enable & 0x1F;
            break;
        default:
            #ifdef ERROR
                ErrorCollector::reportError(std::format("INVALID_READ_ADDRESS {:X}\n", addr), ErrorModule::Bus);
            #endif
            break;
    }

    return 0xFF;
}

void Bus::writeByte(const u16 addr, const u8 val)
{
    if(Util::isAddressBetween(addr, 0x0000, 0x7FFF))
    {
        this->cart.writeByte(addr, val);
        return;
    }

    if(Util::isAddressBetween(addr, 0x8000, 0x9FFF))
    {
        this->vram[addr - 0x8000] = val;
        return;
    }

    if(Util::isAddressBetween(addr, 0xA000, 0xBFFF))
    {
        this->cart.writeByte(addr, val);
        return;
    }
    
    if(Util::isAddressBetween(addr, 0xC000, 0xDFFF))
    {
        this->wram[addr - 0xC000] = val;
        return;
    }

    if(Util::isAddressBetween(addr, 0xE000, 0xFDFF))
    {
        this->wram[addr - 0xE000] = val;
        return;
    }

    if(Util::isAddressBetween(addr, 0xFE00, 0xFE9F))
    {
        this->oam[addr - 0xFE00] = val;
        return;
    }

    if(Util::isAddressBetween(addr, 0xFF80, 0xFFFE))
    {
        this->hram[addr - 0xFF80] = val;
        return;
    }

    switch(addr)
    {
        case 0xFF00:
            this->joypad.joyp = (this->joypad.joyp & 0xF) | (val & 0xF0);
            break;
        case 0xFF04:
            this->timer.div = 0;
            this->timer.divCycleCounter = 0;
            break;
        case 0xFF05:
            this->timer.tima = val;
            break;
        case 0xFF06:
            this->timer.tma = val;
            break;
        case 0xFF07:
            this->timer.tac = val;
            break;
        case 0xFF0F:
            this->interrupts.flag = val & 0x1F;
            break;
        case 0xFF40:
            this->ppu.lcdc = val;
            break;
        case 0xFF41:
            this->ppu.stat = (this->ppu.stat & 0x07) | (val & 0xF8);
            break;
        case 0xFF42:
            this->ppu.scy = val;
            break;
        case 0xFF43:
            this->ppu.scx = val;
            break;
        case 0xFF45:
            this->ppu.lyc = val;
            this->ppu.compareScanline();
            break;
        case 0xFF46:
            for(u8 i = 0; i < 160; ++i)
            {
                this->oam[i] = this->readByte((val << 8) + i);
            }
            break;
        case 0xFF47:
            this->ppu.bgp = val;
            break;
        case 0xFF48:
            this->ppu.obp0 = val;
            break;
        case 0xFF49:
            this->ppu.obp1 = val;
            break;
        case 0xFF50:
            this->disableBootRom = true;
            break;
        case 0xFF4A:
            this->ppu.wy = val;
            break;
        case 0xFF4B:
            this->ppu.wx = val;
            break;
        case 0xFFFF:
            this->interrupts.enable = val & 0x1F;
            break;
        default:
            #ifdef ERROR
                ErrorCollector::reportError(std::format("INVALID_WRITE_ADDRESS {:X}\n", addr), ErrorModule::Bus);
            #endif
            break;
    }
}

u16 Bus::readWord(const u16 addr) const
{
    return (this->readByte(addr) | (this->readByte(addr + 1) << 8));
}

void Bus::writeWord(const u16 addr, const u16 val)
{
    this->writeByte(addr, val & 0xFF);
    this->writeByte(addr + 1, (val & 0xFF00) >> 8);
}