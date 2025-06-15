#pragma once

#include <array>
#include "types.h"
#include "bus.h"
#include "interrupts.h"

class PPU
{
    private:
        enum class Color : u8
        {
            White = 255,
            LightGray = 192,
            DarkGray = 96,
            Black = 0,
        };

        enum class Mode : u8
        {
            HBlank,
            VBlank,
            OAM,
            Transfer,
        };

        Mode mode;

        enum class ControlBit : u8
        {
            BackgroundAndWindowEnable = 0x01,
            ObjectEnable = 0x02,
            ObjectSize = 0x04,
            BackgroundTileMapArea = 0x08,
            BackgroundAndWindowTileDataArea = 0x10,
            WindowEnable = 0x20,
            WindowTileMapArea = 0x40,
            LCDEnable = 0x80,
        };

        u8 lcdc;

        u8 scx;
        u8 scy;

        u8 wx;
        u8 wy;
        u8 windowInternalLineCounter;

        u8 ly;
        u8 lyc;
        u8 stat;

        u8 bgp;
        u8 obp0;
        u8 obp1;

        u32 cycleCounter;

        std::array<u8, 160 * 144 * 3> framebuffer;

        Interrupts& interrupts;
        Bus& bus;
        friend class Bus;
        friend class GameBoy;

        void setControlBit(ControlBit controlBit, const bool val);
        bool getControlBit(ControlBit controlBit) const;

        void compareScanline();

        void updateHBlankPeriod();
        void updateVBlankPeriod();
        void updateOAMScan();
        void updateTransfer();

        void drawBackgroundScanline();
        void drawWindowScanline();
        void drawSpritesScanline();

    public:
        PPU(Bus& bus, Interrupts& interrupts);

        void restart();

        void step(const u8 cycles);
};