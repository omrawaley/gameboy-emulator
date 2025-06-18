#include "gameboy.h"

// #include <ncurses.h>

#ifdef ERROR
    #include "error.h"
#endif

bool GameBoy::skipBootROM = false;
GameBoy::GameBoy() : bus(this->cart, this->cpu, this->timer, this->ppu, this->joypad, this->interrupts), cpu(this->bus, this->interrupts), timer(this->bus, this->interrupts), ppu(this->bus, this->interrupts), joypad(this->bus, this->interrupts) 
{ 

}

void GameBoy::reboot()
{
    this->bus.restart();
    this->cpu.restart();
    this->ppu.restart();
    this->timer.restart();
    this->interrupts.restart();
    this->cart.restart();
    this->joypad.restart();

    if(!GameBoy::skipBootROM)
        this->loadBootROM(this->bootROMPath);

    this->loadROM(this->romPath);
}

std::array<u8, 160 * 144 * 3>& GameBoy::getFramebuffer()
{
    return this->ppu.framebuffer;
}

std::string GameBoy::getTitle()
{
    std::string title;

    for(u16 i = 0x134; i < 0x143; ++i)
    {
        title += static_cast<char>(this->cart.readByte(i));
    }

    return title;
}

void GameBoy::pressButton(Joypad::Button button)
{
    this->joypad.pressButton(button);
}

void GameBoy::releaseButton(Joypad::Button button)
{
    this->joypad.releaseButton(button);
}

void GameBoy::loadBootROM(std::string path)
{
    this->bootROMPath = path;

    FILE* file = fopen(path.c_str(), "rb");

    if(!file)
    {
        #ifdef ERROR
            ErrorCollector::reportError("COULD_NOT_OPEN_BOOT_ROM_FILE", ErrorModule::GameBoy);
        #endif
        GameBoy::skipBootROM = true;
        this->reboot();
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    if(size != 256)
    {
        #ifdef ERROR
            ErrorCollector::reportError("INCOMPATIBLE_BOOT_ROM", ErrorModule::GameBoy);
        #endif
        fclose(file);
        GameBoy::skipBootROM = true;
        this->reboot();
        return;
    }

    if(fread(this->bus.bootRom, sizeof(u8), size, file) != size)
    {
        #ifdef ERROR
            ErrorCollector::reportError("COULD_NOT_LOAD_BOOT_ROM_FROM_FILE", ErrorModule::GameBoy);
        #endif
        fclose(file);
        GameBoy::skipBootROM = true;
        this->reboot();
        return;
    }

    fclose(file);
}

void GameBoy::loadROM(std::string path)
{
    this->romPath = path;

    FILE* file = fopen(path.c_str(), "rb");

    if(!file)
    {
        #ifdef ERROR
            ErrorCollector::reportFatalError("COULD_NOT_OPEN_ROM_FILE", ErrorModule::GameBoy);
        #endif
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    std::unique_ptr<u8[]> buffer = std::make_unique<u8[]>(size);

    if(fread(buffer.get(), sizeof(u8), size, file) != size)
    {
        #ifdef ERROR
            ErrorCollector::reportFatalError("COULD_NOT_LOAD_ROM_FROM_FILE", ErrorModule::GameBoy);
        #endif
        fclose(file);
        return;
    }

    fclose(file);
    this->cart.type = static_cast<Cart::Type>(buffer[0x147]);
    this->cart.createMBC();

    this->cart.romBanks = this->cart.romBanksLookupTable.at(buffer[0x148]);
    this->cart.ramBanks = this->cart.ramBanksLookupTable.at(buffer[0x149]);

    this->cart.rom = std::make_unique<u8[]>(this->cart.romBanks * 0x4000);
    this->cart.rom.swap(buffer);
    this->cart.ram = std::make_unique<u8[]>(this->cart.ramBanks * 0x2000);
    memset(this->cart.ram.get(), 0, sizeof(*this->cart.ram.get()));
}

void GameBoy::step()
{
    u32 cycleCounter = 0;

    while(cycleCounter < 69905) // 4,194,304 Hz clock / 60 Hz refresh rate -> The actual refresh rate is 59.73 Hz but emulators run slowly
    {
        u8 cycles;

        bool interrupted = this->interrupts.check(this->cpu);
        if(!interrupted)
            cycles = this->cpu.step();
        else
            cycles = 20;
    
        cycleCounter += cycles;

        this->timer.step(cycles);

        this->ppu.step(cycles);

        this->joypad.checkButtons();
    }
}

// void GameBoy::createGameBoyDoctorLog()
// {
//     FILE* file = fopen("logs/gbdoctorlog.txt", "w");

//     for(size_t i = 0; i < 313216*25; ++i)
//     {
//         fprintf(file, "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X", cpu.af.hi, cpu.af.lo, cpu.bc.hi, cpu.bc.lo, cpu.de.hi, cpu.de.lo, cpu.hl.hi, cpu.hl.lo, cpu.sp, cpu.pc, cpu.bus.readByte(cpu.pc), cpu.bus.readByte(cpu.pc + 1), cpu.bus.readByte(cpu.pc + 2), cpu.bus.readByte(cpu.pc + 3));
//         fprintf(file, "\n");

//         cpu.step();
//         // this->step();
//     }

//     fclose(file);
//     printf("Finished creating GameBoy Doctor Log");
//     fflush(stdout);
// }

// void GameBoy::initNcurses()
// {
//     initscr();
//     noecho();
// }

// void GameBoy::uninitNcurses()
// {
//     endwin();
// }

// void GameBoy::ncursesDrawDebugger()
// {
//     erase();

//     printw("\n-- CPU --\n");
//     printw("A: 0x%X\n", this->cpu.af.hi);
//     printw("F: 0x%X\n", this->cpu.af.lo);
//     printw("B: 0x%X\n", this->cpu.bc.hi);
//     printw("C: 0x%X\n", this->cpu.bc.lo);
//     printw("D: 0x%X\n", this->cpu.de.hi);
//     printw("E: 0x%X\n", this->cpu.de.lo);
//     printw("H: 0x%X\n", this->cpu.hl.hi);
//     printw("L: 0x%X\n", this->cpu.hl.lo);
//     printw("SP: 0x%X\n", this->cpu.sp);
//     printw("PC: 0x%X\n", this->cpu.pc);
//     printw("IF: 0x%X\n", this->interrupts.flag);
//     printw("VBlank: %d\n", this->interrupts.getFlag(Interrupts::Interrupt::VBlank));
//     printw("LCD: %d\n", this->interrupts.getFlag(Interrupts::Interrupt::LCD));
//     printw("Timer: %d\n", this->interrupts.getFlag(Interrupts::Interrupt::Timer));
//     printw("Serial: %d\n", this->interrupts.getFlag(Interrupts::Interrupt::Serial));
//     printw("Joypad: %d\n", this->interrupts.getFlag(Interrupts::Interrupt::Joypad));

//     printw("\n-- PPU --\n");
//     printw("LCDC: 0x%X\n", this->ppu.lcdc);
//     printw("STAT: 0x%X\n", this->ppu.stat);
//     printw("LY: %d\n", this->ppu.ly);
//     printw("LYC: %d\n", this->ppu.lyc);
//     printw("SCX: %d\n", this->ppu.scx);
//     printw("SCY: %d\n", this->ppu.scy);

//     std::string mode;
//     switch(this->ppu.mode)
//     {
//         case PPU::Mode::HBlank:
//             mode = "HBlank";
//             break;
//         case PPU::Mode::VBlank:
//             mode = "VBlank";
//             break;
//         case PPU::Mode::OAM:
//             mode = "OAM Scan";
//             break;
//         case PPU::Mode::Transfer:
//             mode = "VRAM Transfer";
//             break;
//     }
//     printw("Mode: %s\n", mode.c_str());

//     printw("\n-- Joypad --\n");
//     printw("JOYP: %X\n", this->joypad.joyp);

//     u16 addr = 0xA000;

//     printw("\n-- MEM --\n");
//     for(size_t y = 0; y < 32; ++y) {
//         printw("%04X: ", addr);
//         for(size_t x = 0; x < 0x10; ++x) {
//             printw("%02X ", this->bus.readByte(addr));
//             ++addr;
//         }
//         printw("\n");
//     }

//     refresh();
// }