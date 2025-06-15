#include "ppu.h"

#include <string>
#include <algorithm>
#include <vector>
#include "gameboy.h"

PPU::PPU(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts)
{
    this->restart();
}

void PPU::restart()
{
    this->mode = Mode::OAM;
    this->framebuffer.fill(0);

    if(GameBoy::skipBootROM)
    {
        this->lcdc = 0x91;
        this->scx = 0;
        this->scy = 0;
        this->wx = 0;
        this->wy = 0;
        this->windowInternalLineCounter = 0;
        this->ly = 0;
        this->lyc = 0;
        this->stat = 0x85;
        this->bgp = 0xFC;
        this->obp0 = 0;
        this->obp1 = 0;
        this->cycleCounter = 0;
    }
    else
    {
        this->lcdc = 0x00;
        this->scx = 0;
        this->scy = 0;
        this->wx = 0;
        this->wy = 0;
        this->windowInternalLineCounter = 0;
        this->ly = 0;
        this->lyc = 0;
        this->stat = 0x00;
        this->bgp = 0x00;
        this->obp0 = 0;
        this->obp1 = 0;
        this->cycleCounter = 0;
    }
}

// =================================================================================
// Helper Functions
// =================================================================================

void PPU::setControlBit(ControlBit controlBit, const bool val)
{
    if(val)
        this->lcdc |= static_cast<u8>(controlBit); // Set (1)
    else
        this->lcdc &= ~static_cast<u8>(controlBit); // Clear (0)
}

bool PPU::getControlBit(ControlBit controlBit) const
{
    return (this->lcdc & static_cast<u8>(controlBit)) ? true : false;
}

void PPU::compareScanline()
{
    // If LY == LYC, set the coincidence flag
    if(this->ly == this->lyc)
    {
        this->stat |= 0x04;

        // // If the LY == LYC condition is set for the STAT interrupts, fire a STAT interrupt
        if(this->stat & 0x40)
            this->interrupts.setFlag(Interrupts::Interrupt::LCD, true);
    }
    else
    {
        this->stat &= ~0x04;
    }
}

// =================================================================================
// Scanline Modes
// =================================================================================

void PPU::updateHBlankPeriod()
{
    if(this->cycleCounter <= 204)
        return;

    // If mode 0 for the STAT interrupt is enabled, fire a STAT interrupt
    if(this->stat & 0x08)
        this->interrupts.setFlag(Interrupts::Interrupt::LCD, true);

    this->drawBackgroundScanline();
    this->drawWindowScanline();
    this->drawSpritesScanline();

    ++this->ly;
    this->compareScanline();

    // If the PPU is done with the visible scanlines, fire a VBlank interrupt
    if(this->ly == 144)
    {
        this->mode = Mode::VBlank;
        this->interrupts.setFlag(Interrupts::Interrupt::VBlank, true);
    }

    this->cycleCounter -= 204;
}

void PPU::updateVBlankPeriod()
{
    if(this->cycleCounter <= 456)
        return;

    // If mode 1 for the STAT interrupt is enabled, fire a STAT interrupt
    if(this->stat & 0x10)
        this->interrupts.setFlag(Interrupts::Interrupt::LCD, true);

    ++this->ly;
    this->compareScanline();

    // If the PPU is at the last invisible scanline, go back to the beginning
    if(this->ly == 154)
    {
        this->mode = Mode::OAM;
        this->ly = 0;
        this->compareScanline();
        this->windowInternalLineCounter = 0;
    }

    this->cycleCounter -= 456;
}

void PPU::updateOAMScan()
{
    if(this->cycleCounter <= 80)
        return;

    // If mode 2 for the STAT interrupt is enabled, fire a STAT interrupt
    if(this->stat & 0x20)
        this->interrupts.setFlag(Interrupts::Interrupt::LCD, true);

    this->mode = Mode::Transfer;
    this->cycleCounter -= 80;
}

void PPU::updateTransfer()
{
    if(this->cycleCounter <= 172)
        return;

    this->mode = Mode::HBlank;
    this->cycleCounter -= 172;
}

// =================================================================================
// Layer Drawing
// =================================================================================

void PPU::drawBackgroundScanline()
{
    const u8 palette[4][3] =
    {
        // {0xFF, 0xFF, 0xFF},
        // {0xAA, 0xAA, 0xAA},
        // {0x55, 0x55, 0x55},
        // {0x00, 0x00, 0x00}
        {154, 158, 63},
        {73, 107, 34},
        {14, 69, 11},
        {27, 42, 9}
    };

    u8 backgroundColors[4][3];
    for(u8 i = 0; i < 4; ++i)
    {
        u8 colorIndex = (this->bgp >> (i * 2)) & 0x03;
        backgroundColors[i][0] = palette[colorIndex][0];
        backgroundColors[i][1] = palette[colorIndex][1];
        backgroundColors[i][2] = palette[colorIndex][2];
    }

    if(!this->getControlBit(ControlBit::BackgroundAndWindowEnable))
    {
        // u32 framebufferOffset = this->ly * 160 * 3;
        // for(u8 screenX = 0; screenX < 160; ++screenX)
        // {
        //     this->framebuffer[framebufferOffset + screenX * 3] = 0xFF;
        //     this->framebuffer[framebufferOffset + screenX * 3 + 1] = 0xFF;
        //     this->framebuffer[framebufferOffset + screenX * 3 + 2] = 0xFF;
        // }
        return;
    }

    u16 tileMapStartAddress = this->getControlBit(ControlBit::BackgroundTileMapArea) ? 0x9C00 : 0x9800;
    u16 tileDataStartAddress = this->getControlBit(ControlBit::BackgroundAndWindowTileDataArea) ? 0x8000 : 0x8800;

    u32 framebufferOffset = this->ly * 160 * 3;

    u8 backgroundY = (this->scy + this->ly) % 256;
    u8 tileRow = backgroundY / 8;
    u8 pixelRowInTile = backgroundY % 8;

    u8 backgroundX = this->scx % 256;
    u8 tileCol = backgroundX / 8;
    u8 pixelOffset = backgroundX % 8;

    for(u8 screenX = 0; screenX < 160;)
    {
        u16 tileMapOffset = tileMapStartAddress + (tileRow * 32) + tileCol;
        u8 tileIndex = this->bus.readByte(tileMapOffset);

        u16 tileDataOffset;
        if(this->getControlBit(ControlBit::BackgroundAndWindowTileDataArea))
        {
            tileDataOffset = 0x8000 + (tileIndex * 16);
        }
        else
        {
            i8 signedTileIndex = static_cast<i8>(tileIndex);
            tileDataOffset = 0x9000 + (signedTileIndex * 16);
        }

        u16 tileRowAddress = tileDataOffset + (pixelRowInTile * 2);
        u8 lowByte = this->bus.readByte(tileRowAddress);
        u8 highByte = this->bus.readByte(tileRowAddress + 1);

        for(u8 pixel = pixelOffset; pixel < 8 && screenX < 160; ++pixel, ++screenX)
        {
            u8 lowBit = (lowByte >> (7 - pixel)) & 1;
            u8 highBit = (highByte >> (7 - pixel)) & 1;
            u8 pixelValue = (highBit << 1) | lowBit;

            u32 framebufferPixelOffset = framebufferOffset + screenX * 3;
            this->framebuffer[framebufferPixelOffset] = backgroundColors[pixelValue][0];
            this->framebuffer[framebufferPixelOffset + 1] = backgroundColors[pixelValue][1];
            this->framebuffer[framebufferPixelOffset + 2] = backgroundColors[pixelValue][2];
        }

        tileCol = (tileCol + 1) % 32;
        pixelOffset = 0;
    }
}

void PPU::drawWindowScanline()
{
    if(!this->getControlBit(ControlBit::WindowEnable) || !this->getControlBit(ControlBit::BackgroundAndWindowEnable))
        return;

    if(this->ly < this->wy)
        return;

    if(this->wx >= 167)
        return;
        
    u16 tileMapStartAddress = this->getControlBit(ControlBit::WindowTileMapArea) ? 0x9C00 : 0x9800;
    u16 tileDataStartAddress = this->getControlBit(ControlBit::BackgroundAndWindowTileDataArea) ? 0x8000 : 0x8800;

    u32 framebufferOffset = this->ly * 160 * 3;

    u8 windowY = this->windowInternalLineCounter;
    u8 tileRow = windowY / 8;
    u8 pixelRowInTile = windowY % 8;

    u8 windowX = abs(this->wx - 7);
    u8 tileCol = 0;

    u8 pixelOffset = 0;
    if(this->wx < 7)
    {
        pixelOffset = windowX;
        windowX = 0;
    }

    const u8 palette[4][3] =
    {
        // {0xFF, 0xFF, 0xFF},
        // {0xAA, 0xAA, 0xAA},
        // {0x55, 0x55, 0x55},
        // {0x00, 0x00, 0x00}
        {154, 158, 63},
        {73, 107, 34},
        {14, 69, 11},
        {27, 42, 9}
    };

    u8 windowColors[4][3];
    for(u8 i = 0; i < 4; ++i)
    {
        u8 colorIndex = (this->bgp >> (i * 2)) & 0x03;
        windowColors[i][0] = palette[colorIndex][0];
        windowColors[i][1] = palette[colorIndex][1];
        windowColors[i][2] = palette[colorIndex][2];
    }

    for(u8 screenX = windowX; screenX < 160;)
    {
        u16 tileMapOffset = tileMapStartAddress + (tileRow * 32) + tileCol;
        u8 tileIndex = this->bus.readByte(tileMapOffset);

        u16 tileDataOffset;
        if(this->getControlBit(ControlBit::BackgroundAndWindowTileDataArea))
        {
            tileDataOffset = 0x8000 + (tileIndex * 16);
        }
        else
        {
            i8 signedTileIndex = static_cast<i8>(tileIndex);
            tileDataOffset = 0x9000 + (signedTileIndex * 16);
        }

        u16 tileRowAddress = tileDataOffset + (pixelRowInTile * 2);
        u8 lowByte = this->bus.readByte(tileRowAddress);
        u8 highByte = this->bus.readByte(tileRowAddress + 1);

        for(u8 pixel = pixelOffset; pixel < 8 && screenX < 160; ++pixel, ++screenX)
        {
            u8 lowBit = (lowByte >> (7 - pixel)) & 1;
            u8 highBit = (highByte >> (7 - pixel)) & 1;
            u8 pixelValue = (highBit << 1) | lowBit;

            u32 framebufferPixelOffset = framebufferOffset + screenX * 3;
            this->framebuffer[framebufferPixelOffset] = windowColors[pixelValue][0];
            this->framebuffer[framebufferPixelOffset + 1] = windowColors[pixelValue][1];
            this->framebuffer[framebufferPixelOffset + 2] = windowColors[pixelValue][2];
        }

        tileCol = (tileCol + 1) % 32;
        pixelOffset = 0;
    }

    ++this->windowInternalLineCounter;
}

void PPU::drawSpritesScanline()
{
    if(!getControlBit(ControlBit::ObjectEnable))
        return;

    const u8 palette[2][4][3] = 
    {
        {
            // {0xFF, 0xFF, 0xFF}, // Color 0 (transparent)
            // {0xAA, 0xAA, 0xAA}, // Color 1
            // {0x55, 0x55, 0x55}, // Color 2
            // {0x00, 0x00, 0x00}  // Color 3
            {154, 158, 63},
            {73, 107, 34},
            {14, 69, 11},
            {27, 42, 9}
        },
        {
            // {0xFF, 0xFF, 0xFF}, // Color 0 (transparent)
            // {0xAA, 0xAA, 0xAA}, // Color 1
            // {0x55, 0x55, 0x55}, // Color 2
            // {0x00, 0x00, 0x00}  // Color 3
            {154, 158, 63},
            {73, 107, 34},
            {14, 69, 11},
            {27, 42, 9}
        }
    };

    u8 objectColors[2][4][3];
    for (int i = 0; i < 4; ++i) {
        u8 colorIdx0 = (this->obp0 >> (i * 2)) & 0x03;
        u8 colorIdx1 = (this->obp1 >> (i * 2)) & 0x03;
        objectColors[0][i][0] = palette[0][colorIdx0][0];
        objectColors[0][i][1] = palette[0][colorIdx0][1];
        objectColors[0][i][2] = palette[0][colorIdx0][2];
        objectColors[1][i][0] = palette[1][colorIdx1][0];
        objectColors[1][i][1] = palette[1][colorIdx1][1];
        objectColors[1][i][2] = palette[1][colorIdx1][2];
    }

    u8 spriteHeight = getControlBit(ControlBit::ObjectSize) ? 16 : 8;

    u32 framebufferOffset = this->ly * 160 * 3;

    struct Sprite
    {
        // u8 index;
        u8 yPos;
        u8 xPos;
        u8 tileIndex;
        u8 flags;
        // bool active;
    };

    // Sprite sprites[40];
    std::vector<Sprite> sprites;
    u8 numberOfSprites = 0;
    for(u8 i = 0; i < 40 && numberOfSprites < 10; ++i)
    {
        u16 oamBase = 0xFE00 + (i * 4);

        u8 yPos = this->bus.readByte(oamBase) - 16;
        u8 xPos = this->bus.readByte(oamBase + 1) - 8;
        u8 tileIndex = this->bus.readByte(oamBase + 2);
        u8 flags = this->bus.readByte(oamBase + 3);

        if(this->ly < yPos || this->ly >= yPos + spriteHeight)
        {
            // sprites[i].active = false;
            continue;
        }

        // sprites[i].active = true;

        // sprites[i].yPos = yPos;
        // sprites[i].xPos = xPos;
        // sprites[i].tileIndex = tileIndex;
        // sprites[i].flags = flags;

        Sprite sprite;
        sprite.yPos = yPos;
        sprite.xPos = xPos;
        sprite.tileIndex = tileIndex;
        sprite.flags = flags;

        sprites.push_back(sprite);

        ++numberOfSprites;
    }

    // std::stable_sort(std::begin(sprites), std::end(sprites), [](Sprite a, Sprite b){return a.xPos < b.xPos;});
    // std::stable_sort(sprites.begin(), sprites.end(), [](Sprite a, Sprite b){return a.xPos < b.xPos;});

    for (u8 i = 0; i < sprites.size(); ++i)
    {
        // if(!sprites[i].active)
            // continue;

        // if (this->ly < sprites[i].yPos || this->ly >= sprites[i].yPos + spriteHeight) 
        //     continue;

        for(u8 j = i + 1; j < sprites.size(); ++j)
        {
            if(sprites[i].xPos == sprites[j].xPos)
            {
                ++i;
                return;
            }
        }

        bool priority = (sprites[i].flags & 0x80) != 0;
        bool flipY = (sprites[i].flags & 0x40) != 0;
        bool flipX = (sprites[i].flags & 0x20) != 0;
        u8 paletteIndex = (sprites[i].flags & 0x10) ? 1 : 0;

        u8 pixelRow = flipY ? (spriteHeight - 1 - (this->ly - sprites[i].yPos)) : (this->ly - sprites[i].yPos);

        if (spriteHeight == 16 && (sprites[i].tileIndex & 1)) 
            sprites[i].tileIndex &= ~1;

        u16 tileDataOffset = 0x8000 + (sprites[i].tileIndex * 16) + (pixelRow * 2);
        u8 lowByte = this->bus.readByte(tileDataOffset);
        u8 highByte = this->bus.readByte(tileDataOffset + 1);

        for (u8 pixel = 0; pixel < 8; ++pixel) 
        {
            u8 screenX = sprites[i].xPos + pixel;

            if (screenX >= 160) 
                continue;

            u8 pixelIndex = flipX ? (7 - pixel) : pixel;
            u8 lowBit = (lowByte >> (7 - pixelIndex)) & 1;
            u8 highBit = (highByte >> (7 - pixelIndex)) & 1;
            u8 pixelValue = (highBit << 1) | lowBit;

            if (pixelValue == 0) 
                continue;

            u32 framebufferPixelOffset = framebufferOffset + screenX * 3;
            
            if (priority && this->framebuffer[framebufferPixelOffset] != 154 && this->framebuffer[framebufferPixelOffset] != 158 && this->framebuffer[framebufferPixelOffset] != 63) 
                continue;

            this->framebuffer[framebufferPixelOffset] = objectColors[paletteIndex][pixelValue][0];
            this->framebuffer[framebufferPixelOffset + 1] = objectColors[paletteIndex][pixelValue][1];
            this->framebuffer[framebufferPixelOffset + 2] = objectColors[paletteIndex][pixelValue][2];
        }
    }
}

// =================================================================================
// Main Logic
// =================================================================================

void PPU::step(const u8 cycles)
{
    if(!this->getControlBit(ControlBit::LCDEnable))
        return;

    this->cycleCounter += cycles;

    this->stat = (this->stat & 0xFC) | static_cast<u8>(this->mode);

    switch(this->mode)
    {
        case Mode::HBlank:
            this->updateHBlankPeriod();
            break;
        case Mode::VBlank:
            this->updateVBlankPeriod();
            break;
        case Mode::OAM:
            this->updateOAMScan();
            break;
        case Mode::Transfer:
            this->updateTransfer();
            break;
    }
}