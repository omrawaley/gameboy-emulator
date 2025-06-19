
# Game Boy Emulator

```
   ___   _   __  __ ___   ___  _____   __
  / __| /_\ |  \/  | __| | _ )/ _ \ \ / /
 | (_ |/ _ \| |\/| | _|  | _ \ (_) \ V / 
  \___/_/ \_\_|  |_|___| |___/\___/ |_|                                    
```

A simple Game Boy Emulator written in C++ with a scanline renderer and MBC support.

The core emulator (`lib`) can be extracted and used with custom implementations (e.g. on embedded systems). A desktop implementation (`test`) with SDL2 and Dear ImGUI is the main way to use this emulator.

This emulator was primarily written for educational purposes, so don't expect anything extraordinary (˙ ͜ʟ˙)
## Authors

- [@omrawaley](https://www.github.com/omrawaley)


## Demo

<img alt="Link's Awakening Demo" src="zelda_demo.gif" width="400" />

(The actual emulator is less choppy)

## Features

- Core emulator library
- CLI ROM loading
- Scanline rendering
- Customizable palette
- Expandable MBC support (only MBC1 currently)
- GUI
## API Reference

`void reboot()` Reboot the emulator.

`std::array<u8, 160 * 144 * 3>& getFramebuffer();` Get the RGB24 framebuffer.

`std::string getTitle();` Get the [ROM title](https://gbdev.io/pandocs/The_Cartridge_Header.html#0134-0143--title). 

`void pressButton(Joypad::Button button);` Press a button.

`void releaseButton(Joypad::Button button);` Release a button.

`void loadBootROM(std::string path);` Load a Boot ROM/BIOS.

`void loadROM(std::string path);` Load a ROM.

`void step();` Render 1 frame.


## Installation

Use the bundled `CMakeLists.txt` to generate a Makefile and compile with `make`.
```
cmake CMakeLists.txt
make
```

When running `cmake`, you have the option to pass `-DERROR=ON` which enables printing errors to the console.
    
## Usage

```
./bin/gb <path-to-boot-rom> <path-to-rom>
```

### Keys

<kbd>M</kbd> Show the menu bar.

<kbd>Tab</kbd> Hold to speed up the emulator.

<kbd>Esc</kbd> Quit the program.
## To-Do

- [ ] Add more CLI options
- [ ] Add a debugger to view internal state and the bus
- [ ] Add the ability to take screenshots
- [ ] Add more robust palette handling
- [ ] Rewrite the PPU to improve time complexity
- [ ] Add MBC3, MBC2, and MBC5 support
- [ ] Add the ability to view the serial port via the GUI
- [x] Add a proper error collector/reporter
- [ ] Format code to expose public functions first
- [x] Remove the IO array in `bus.h` and handle unimplemented IO registers properly
## Contributing

Contributions are always welcome! I'd love to learn how I can improve this emulator.
## License

[Apache 2.0](https://apache.org/licenses/LICENSE-2.0)

### Dependancies

[Dear ImGui](https://github.com/ocornut/imgui) - [MIT](https://choosealicense.com/licenses/mit/)

(Not bundled) [SDL](https://www.libsdl.org/) - [zlib](https://zlib.net/zlib_license.html)

