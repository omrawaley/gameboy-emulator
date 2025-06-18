#include "cart.h"

#ifdef ERROR
    #include "error.h"
#endif

Cart::Cart()
{
    this->restart();
}

void Cart::restart()
{
    this->rom.reset();
    this->ram.reset();
    this->mbc.reset();
}

void Cart::createMBC()
{
    switch(this->type)
    {
        case Type::ROM_ONLY:
            return;
            break;
        case Type::MBC1:
        case Type::MBC1_RAM:
        case Type::MBC1_RAM_Battery:
            this->mbc = std::make_unique<MBC1>(*this);
            break;
        // case Type::MBC2:
        // case Type::MBC2_Battery:
        // case Type::ROM_RAM:
        // case Type::ROM_RAM_BETTERY:
        // case Type::MMM01:
        // case Type::MMM01_RAM:
        // case Type::MMM01_RAM_BATTERY:
        case Type::MBC3_TIMER_BATTERY:
        case Type::MBC3_TIMER_RAM_BATTERY:
        case Type::MBC3:
        case Type::MBC3_RAM:
        case Type::MBC3_RAM_BATTERY:
            this->mbc = std::make_unique<MBC3>(*this);
            break;
        // case Type::MBC5:
        // case Type::MBC5_RAM:
        // case Type::MBC5_RAM_BATTERY:
        // case Type::MBC5_RUMBLE:
        // case Type::MBC5_RUMBLE_RAM:
        // case Type::MBC5_RUMBLE_RAM_BATTERY:
        // case Type::MBC6:
        // case Type::MBC7_SENSOR_RUMBLE_RAM_BATTERY:
        // case Type::POCKET_CAMERA:
        // case Type::BANDAI_TAMA5:
        // case Type::HuC3:
        // case Type::HuC1_RAM_BATTERY:
        //     break;
        default:
            #ifdef ERROR
                ErrorCollector::reportFatalError("UNIMPLEMENTED_MBC", ErrorModule::Cart);
            #endif
            exit(EXIT_FAILURE);
            break;
    }
}

u8 Cart::readByte(const u16 addr) const
{
    if(this->type == Type::ROM_ONLY)
        return this->rom[addr];
    else
        return this->mbc.get()->readByte(addr);
}

void Cart::writeByte(const u16 addr, const u8 val)
{
    if(this->type == Type::ROM_ONLY)
        return;
    else
        this->mbc.get()->writeByte(addr, val);
}