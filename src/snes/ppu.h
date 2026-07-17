#ifndef PPU_H
#define PPU_H

#include "../types.h"
#include "hardware.h"

// VRAM Layout Constants
constexpr uint16 VRAM_BG1_TILES = 0x0000;    // BG1 tile data
constexpr uint16 VRAM_BG1_MAP = 0x1000;      // BG1 tilemap
constexpr uint16 VRAM_BG2_TILES = 0x0000;    // BG2 tile data
constexpr uint16 VRAM_BG2_MAP = 0x0800;      // BG2 tilemap
constexpr uint16 VRAM_BG3_TILES = 0x4000;    // BG3 tile data (HUD)
constexpr uint16 VRAM_BG3_MAP = 0x5800;      // BG3 tilemap (HUD)
constexpr uint16 VRAM_SPRITES = 0x6000;      // Sprite tiles
constexpr uint16 VRAM_SPRITES_MAP = 0x7800;  // Sprite maps

// VRAM control modes
constexpr uint8 VRAM_MODE_8BIT = 0x80;  // Word increment (main mode)
constexpr uint8 VRAM_MODE_4BIT = 0x00;  // Byte increment

// Color structure (RGB 8-bit)
struct ColorRGB {
    uint8 r;
    uint8 g;
    uint8 b;
};

// SNES 15-bit color format: gggrrrrr 0bbbbbgg
struct SNESColor {
    uint16 value;
};

// Palette structures
constexpr int PALETTE_BG_COUNT = 8;      // 8 sub-paletas BG
constexpr int PALETTE_SPRITE_COUNT = 8;  // 8 sub-paletas Sprite
constexpr int COLORS_PER_PALETTE = 16;   // 16 cores por sub-palette

struct SNESPalette {
    ColorRGB bg_palette[PALETTE_BG_COUNT][COLORS_PER_PALETTE];
    ColorRGB sprite_palette[PALETTE_SPRITE_COUNT][COLORS_PER_PALETTE];
};

// Tile structures (4bpp format)
struct Tile4bpp {
    uint8 planes[4][8];  // 4 planos, 8 linhas de 2 bytes cada
};

// Tilemap entry (16-bit)
struct TilemapEntry {
    uint16 tile_num : 10;  // Bits 0-9: número do tile
    uint16 palette : 3;    // Bits 10-12: sub-paleta (0-7)
    uint16 priority : 2;   // Bits 13-14: prioridade
    uint16 flip_h : 1;     // Bit 15: flip horizontal
    // Nota: flip vertical está no bit 15 do high byte (16-bit entry)
};

// PPU Memory arrays
extern uint8 vram[VRAM_SIZE];
extern uint8 cgram[CGRAM_SIZE];

// Shadow palette buffer
extern SNESPalette shadow_palette;

// PPU Functions
void ppu_write(uint16 addr, uint8 value);
uint8 ppu_read(uint16 addr);

void write_vram(uint16 addr, uint8 value);
uint8 read_vram(uint16 addr);

void set_scroll_bg1(uint16 h, uint16 v);
void set_scroll_bg2(uint16 h, uint16 v);
void set_scroll_bg3(uint16 h, uint16 v);

void set_bg_mode(uint8 mode);
void set_mosaic(uint8 size);

ColorRGB snes_color_to_rgb(uint16 snes_color);
void load_palette(uint8 palette_id);
void upload_palette();

uint8 get_pixel_4bpp(const uint8* tile_data, int x, int y);

#endif // PPU_H