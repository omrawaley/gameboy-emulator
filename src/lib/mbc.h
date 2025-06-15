#pragma once

#include "types.h"

class Cart;

class MBC
{
    public:
        Cart& cart;

        MBC(Cart& cart);
        virtual ~MBC() = default;

        virtual u8 readByte(const u16 addr) const = 0;
        virtual void writeByte(const u16 addr, const u8 val) = 0;
};

class MBC1 : public MBC
{
    private:
        using MBC::MBC;

    public:
        bool ramEnable = false;
        u8 romBankNumber = 1;
        u8 ramBankNumber = 0;
        bool bankingModeSelect = 0;

    public:
        u8 readByte(const u16 addr) const;
        void writeByte(const u16 addr, const u8 val);
};

class MBC3 : public MBC
{
    private:
        using MBC::MBC;

    public:
        u8 readByte(const u16 addr) const;
        void writeByte(const u16 addr, const u8 val);
};