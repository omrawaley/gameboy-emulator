#pragma once

#include "types.h"
#include "mbc.h"
#include <memory>
#include <map>

class Cart
{
    public:
        enum class Type
        {
            ROM_ONLY = 0x00,
            MBC1 = 0x01,
            MBC1_RAM = 0x02,
            MBC1_RAM_Battery = 0x03,
            MBC2 = 0x05,
            MBC2_Battery = 0x06,
            ROM_RAM = 0x08,
            ROM_RAM_BETTERY = 0x09,
            MMM01 = 0x0B,
            MMM01_RAM = 0x0C,
            MMM01_RAM_BATTERY = 0x0D,
            MBC3_TIMER_BATTERY = 0x0F,
            MBC3_TIMER_RAM_BATTERY = 0x10,
            MBC3 = 0x11,
            MBC3_RAM = 0x12,
            MBC3_RAM_BATTERY = 0x13,
            MBC5 = 0x19,
            MBC5_RAM = 0x1A,
            MBC5_RAM_BATTERY = 0x1B,
            MBC5_RUMBLE = 0x1C,
            MBC5_RUMBLE_RAM = 0x1D,
            MBC5_RUMBLE_RAM_BATTERY = 0x1E,
            MBC6 = 0x20,
            MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
            POCKET_CAMERA = 0xFC,
            BANDAI_TAMA5 = 0xFD,
            HuC3 = 0xFE,
            HuC1_RAM_BATTERY = 0xFF,
        };

    private:
        const std::map<const u8, const u16> romBanksLookupTable = 
        {
            {0x00, 2},
            {0x01, 4},
            {0x02, 8},
            {0x03, 16},
            {0x04, 32},
            {0x05, 64},
            {0x06, 128},
            {0x07, 256},
            {0x08, 512},
            {0x52, 72},
            {0x53, 80},
            {0x54, 96},
        };

        const std::map<const u8, const u8> ramBanksLookupTable =
        {
            {0x00, 0},
            {0x01, 0},
            {0x02, 1},
            {0x03, 4},
            {0x04, 16},
            {0x05, 8},
        };

        std::unique_ptr<u8[]> rom;
        std::unique_ptr<u8[]> ram;

        Type type;
        u16 romBanks;
        u8 ramBanks;

        friend class GameBoy;
        friend class MBC1;

    public:
        std::unique_ptr<MBC> mbc;

        Cart();

        void restart();

        void createMBC();

        u8 readByte(const u16 addr) const;
        void writeByte(const u16 addr, const u8 val);
};