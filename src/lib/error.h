#pragma once

#include <vector>
#include <string>

enum class ErrorModule
{
    App,
    GameBoy,
    Bus,
    Cart,
    MBC,
    CPU,
    PPU,
    Interrupts,
    Timer,
    Joypad
};

struct Error
{
    std::string text;
    ErrorModule module;

    Error(const std::string& text, const ErrorModule& module);
};

namespace ErrorCollector
{
    void reportError(const std::string& text, const ErrorModule& module);
    void reportFatalError(const std::string& text, const ErrorModule& module);

    void printErrors(bool release);

    static std::vector<Error> errors;
};
