#include "error.h"

#include <iostream>

Error::Error( const std::string& text, const ErrorModule& module) : text(text), module(module) { }

void ErrorCollector::reportError(const std::string& text, const ErrorModule& module)
{
    ErrorCollector::errors.push_back(Error{text, module});
}

void ErrorCollector::reportFatalError(const std::string& text, const ErrorModule& module)
{
    ErrorCollector::reportError(text, module);
    ErrorCollector::printErrors(true);
}

void ErrorCollector::printErrors(bool release)
{
    for(auto& error : ErrorCollector::errors)
    {
        std::string message = "\x1B[31m--> ERROR";
        switch(error.module)
        {
            case ErrorModule::App:
                message += "::APP";
                break;
            case ErrorModule::GameBoy:
                message += "::GAMEBOY";
                break;
            case ErrorModule::Bus:
                message += "::BUS";
                break;
            case ErrorModule::Cart:
                message += "::CART";
                break;
            case ErrorModule::MBC:
                message += "::MBC";
                break;
            case ErrorModule::CPU:
                message += "::CPU";
                break;
            case ErrorModule::PPU:
                message += "::PPU";
                break;
            case ErrorModule::Interrupts:
                message += "::INTERRUPTS";
                break;
            case ErrorModule::Timer:
                message += "::TIMER";
                break;
            case ErrorModule::Joypad:
                message += "::JOYPAD";
                break;
        }
        message += "::" + error.text;

        std::cerr << message << "\n";
    }

    std::flush(std::cerr);
}
