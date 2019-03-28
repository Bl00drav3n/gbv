# gbv
Original Gameboy video render library

## What is it?
GBV emulates the original GB video hardware to draw tiles to the screen. It tries to act as close to what the real hardware would display as possible.

### Supported Features:
* memory mapped registers LCDC (lcd control), SCX and SCY (scroll x and y)
* background map rendering (map1 & map2)

### Supported LCDC flags
* GBV_LCDC_BG_ENABLE
* GBV_LCDC_BG_MAP_SELECT
* GBV_LCDC_BG_DATA_SELECT

### Upcoming Features
* window support (WND)
* sprite support (OAM) (8x8 and 16x8)
* additional LCDC flag support
* more render modes (F32, RGBA)

## API design
The API was designed in a lightweight way, it will not allocate any memory. The library is initialized by calling gbv_init and providing 64k of backing memory.

When you're finished setting up all your data, call gbv_render with a 8bpp framebuffer of size **GBV_SCREEN_SIZE**. The image will be rendered row by row, starting at the top left.
You also have to provide a grayscale palette, to map the pixel values 00, 01, 10 and 11 to their respective output color values.

A linearly spaced (non-gamma corrected) palette is
```cpp
gbv_palette palette = { 0xFF, 0xAA, 0x55, 0x00 };
```
where 00 (color index 0) is mapped to white, 11 (color index 3) to black.

### Compiling and linking
Just include gbv.h and gbv.cpp files into your project, or compile to a library and link against it.

## Usage
A usage example can be found in test_sdl.cpp, using [libSDL2](https://www.libsdl.org/) to draw to the screen.

![test1](https://github.com/Bl00drav3n/gbv/raw/master/test1.png "Test 1")