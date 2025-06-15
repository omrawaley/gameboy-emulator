#include "joypad.h"

#include "gameboy.h"

Joypad::Joypad(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts)
{
    this->restart();
}

void Joypad::restart()
{
    this->actionButtonState = 0xFF;
    this->directionalButtonState = 0xFF;

    if(GameBoy::skipBootROM)
        this->joyp = 0xCF;
    else
        this->joyp = 0;
}

bool Joypad::areActionButtonsSelected()
{
    return !(this->joyp & 0x20);
}

bool Joypad::areDirectionalButtonsSelected()
{
    return !(this->joyp & 0x10);
}

void Joypad::pressButton(Button button)
{
    switch(button)
    {
        case Button::A:
            this->actionButtonState &= ~1;
            break;
        case Button::B:
            this->actionButtonState &= ~2;
            break;
        case Button::Start:
            this->actionButtonState &= ~8;
            break;
        case Button::Select:
            this->actionButtonState &= ~4;
            break;
        case Button::Up:
            this->directionalButtonState &= ~4;
            break;
        case Button::Down:
            this->directionalButtonState &= ~8;
            break;
        case Button::Left:
            this->directionalButtonState &= ~2;
            break;
        case Button::Right:
            this->directionalButtonState &= ~1;
            break;
    }

    this->interrupts.setFlag(Interrupts::Interrupt::Joypad, true);
}

void Joypad::releaseButton(Button button)
{
    switch(button)
    {
        case Button::A:
            this->actionButtonState |= 1;
            break;
        case Button::B:
            this->actionButtonState |= 2;
            break;
        case Button::Start:
            this->actionButtonState |= 8;
            break;
        case Button::Select:
            this->actionButtonState |= 4;
            break;
        case Button::Up:
            this->directionalButtonState |= 4;
            break;
        case Button::Down:
            this->directionalButtonState |= 8;
            break;
        case Button::Left:
            this->directionalButtonState |= 2;
            break;
        case Button::Right:
            this->directionalButtonState |= 1;
            break;
    }
}

void Joypad::checkButtons()
{
    if(this->areActionButtonsSelected())
    {
        this->joyp = (this->joyp & 0xF0) | this->actionButtonState & 0xF;
    }
    else if(this->areDirectionalButtonsSelected())
    {
        this->joyp = (this->joyp & 0xF0) | this->directionalButtonState & 0xF;
    }
    else
    {
        this->joyp = 0xCF;
    }
}