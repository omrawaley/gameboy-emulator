#include "app.h"

#ifdef ERROR
    #include "../lib/error.h"
#endif

float App::refreshRatePeriod = 16.67;
App::App() : window(nullptr), renderer(nullptr), displayTexture(nullptr), quit(false)
{
    this->loadMedia();

    // this->gameboy.initNcurses();
}

App::~App()
{
    this->freeMedia();

    // this->gameboy.uninitNcurses();

    #ifdef ERROR
        ErrorCollector::printErrors(true);
    #endif
}

void App::loadMedia()
{
    this->window = SDL_CreateWindow("Game Boy Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 5, 144 * 5, SDL_WINDOW_SHOWN);
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);
    this->displayTexture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    GUI::init(this->window, this->renderer);
}

void App::freeMedia()
{
    GUI::free();

    SDL_DestroyTexture(this->displayTexture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void App::start(std::string bootROMPath, std::string romPath)
{
    this->lastCycleTime = std::chrono::steady_clock::now();

    // this->gameboy.loadBootROM("/Users/om/Downloads/GameBoyROMs/dmg_boot.bin");

    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/01-special.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/02-interrupts.gb"); // FAILED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/03-op sp,hl.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/04-op r,imm.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/05-op rp.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/06-ld r,r.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/07-jr,jp,call,ret,rst.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/08-misc instrs.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/09-op r,r.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/10-bit ops.gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs/11-op a,(hl).gb"); // PASSED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/cpu_instrs.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/interrupt_time.gb"); // FAILED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/instr_timing.gb"); // FAILED
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/mts-20240926-1737-443f6e1/acceptance/reti_timing.gb");


    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/Tetris.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/DrMario.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/SuperMarioLand.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/LinksAwakening.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/Tennis.gb");

    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/dmg-acid2.gb");
    // this->gameboy.loadROM("/Users/om/Downloads/GameBoyROMs/pocket.gb");

    // this->gameboy.createGameBoyDoctorLog();

    this->gameboy.loadROM(romPath);
    this->gameboy.loadBootROM(bootROMPath);
}

void App::update()
{
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                this->quit = true;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        this->quit = true;
                        break;
                    case SDLK_TAB:
                        this->refreshRatePeriod = 0.1;
                        break;
                    case SDLK_m:
                        GUI::menuEnable = !GUI::menuEnable;
                        break;
                    case SDLK_UP:
                        this->gameboy.pressButton(Joypad::Button::Up);
                        break;
                    case SDLK_DOWN:
                        this->gameboy.pressButton(Joypad::Button::Down);
                        break;
                    case SDLK_LEFT:
                        this->gameboy.pressButton(Joypad::Button::Left);
                        break;
                    case SDLK_RIGHT:
                        this->gameboy.pressButton(Joypad::Button::Right);
                        break;
                    case SDLK_z:
                        this->gameboy.pressButton(Joypad::Button::A);
                        break;
                    case SDLK_x:
                        this->gameboy.pressButton(Joypad::Button::B);
                        break;
                    case SDLK_RETURN:
                        this->gameboy.pressButton(Joypad::Button::Start);
                        break;
                    case SDLK_RSHIFT:
                        this->gameboy.pressButton(Joypad::Button::Select);
                        break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym)
                {
                    case SDLK_TAB:
                        this->refreshRatePeriod = 16.67;
                        break;
                    case SDLK_UP:
                        this->gameboy.releaseButton(Joypad::Button::Up);
                        break;
                    case SDLK_DOWN:
                        this->gameboy.releaseButton(Joypad::Button::Down);
                        break;
                    case SDLK_LEFT:
                        this->gameboy.releaseButton(Joypad::Button::Left);
                        break;
                    case SDLK_RIGHT:
                        this->gameboy.releaseButton(Joypad::Button::Right);
                        break;
                    case SDLK_z:
                        this->gameboy.releaseButton(Joypad::Button::A);
                        break;
                    case SDLK_x:
                        this->gameboy.releaseButton(Joypad::Button::B);
                        break;
                    case SDLK_RETURN:
                        this->gameboy.releaseButton(Joypad::Button::Start);
                        break;
                    case SDLK_RSHIFT:
                        this->gameboy.releaseButton(Joypad::Button::Select);
                        break;
                }
                break;
        }

        GUI::processEvent(event);
    }

    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - this->lastCycleTime).count();

    if(deltaTime > App::refreshRatePeriod) // 1000 / 60 Hz = ~16.67 ms period
    {
        lastCycleTime = currentTime;

        this->gameboy.step();
    }

    // This sucks, sleeping takes time... throttling needs to be changed to use manual tick counting
    // SDL_Delay(16.67); // 1000 / 60 Hz = ~16.67 ms period
}

void App::draw()
{
    // this->gameboy.ncursesDrawDebugger();

    SDL_RenderClear(this->renderer);

    SDL_UpdateTexture(this->displayTexture, nullptr, this->gameboy.getFramebuffer().data(), 160 * (sizeof(u8) * 3));
    SDL_RenderCopy(this->renderer, this->displayTexture, nullptr, nullptr);

    GUI::draw(this->renderer, *this);

    SDL_RenderPresent(this->renderer);
}
