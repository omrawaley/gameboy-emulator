#include "mbc.h"

#include "util.h"
#include "cart.h"

MBC::MBC(Cart& cart) : cart(cart)
{

}


u8 MBC1::readByte(const u16 addr) const
{
    // ROM Bank 0
    if(Util::isAddressBetween(addr, 0x0000, 0x3FFF))
        return this->cart.rom[addr];

    // ROM Bank 01-7F
    if(Util::isAddressBetween(addr, 0x4000, 0x7FFF))
    {
        // return this->cart.rom[(0x4000 * this->romBankNumber) + (addr - 0x4000)];
        const u8 bank = ((this->ramBankNumber << 5) | this->romBankNumber) % this->cart.romBanks;
        return this->cart.rom[(0x4000 * bank) + (addr - 0x4000)];
    }

    // RAM Bank 0-3
    if(Util::isAddressBetween(addr, 0xA000, 0xBFFF))
    {
        if(!this->ramEnable)
            return 0xFF;

        const u8 bank = this->bankingModeSelect * this->ramBankNumber % this->cart.ramBanks;
        return this->cart.ram[(addr - 0xA000) + (0x2000 * bank)];
    }

    return 0xFF;
}

void MBC1::writeByte(const u16 addr, const u8 val)
{
    // Enable RAM
    if(Util::isAddressBetween(addr, 0x0000, 0x1FFF))
        this->ramEnable = (val & 0xF) == 0xA;

    // ROM Bank
    if(Util::isAddressBetween(addr, 0x2000, 0x3FFF))
    {
        if(!val)
            this->romBankNumber = 1;
        else
            this->romBankNumber = val & 0x1F;
    }

    // RAM Bank
    if(Util::isAddressBetween(addr, 0x4000, 0x5FFF))
        this->ramBankNumber = val & 2;

    // Mode Select
    if(Util::isAddressBetween(addr, 0x6000, 0x7FFF))
        this->bankingModeSelect = val == 1;

    // External RAM
    if(Util::isAddressBetween(addr, 0xA000, 0xBFFF))
    {
        if(!this->ramEnable)
            return;

        // if(this->bankingModeSelect == 1 && this->cart.ramBanks == )

        const u8 bank = this->bankingModeSelect * this->ramBankNumber % this->cart.ramBanks;
        this->cart.ram[(addr - 0xA000) + (0x2000 * bank)] = val;
    }
}

u8 MBC3::readByte(const u16 addr) const
{

}

void MBC3::writeByte(const u16 addr, const u8 val)
{

}