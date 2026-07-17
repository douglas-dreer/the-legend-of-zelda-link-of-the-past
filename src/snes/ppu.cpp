#include "ppu.h"
#include <cstring>

// PPU Write
void ppu_write(uint16 addr, uint8 value) {
    // Simulate PPU register writes
    if (addr >= 0x2100 && addr <= 0x213F) {
        // This would normally write to actual PPU registers
        // For PC emulation, we update shadow state
    }
}

// PPU Read
uint8 ppu_read(uint16 addr) {
    // Simulate PPU register reads
    if (addr >= 0x2100 && addr <= 0x213F) {
        // Return shadow state values
        return 0;
    }
    return 0;
}

// Write VRAM
void write_vram(uint16 addr, uint8 value) {
    if (addr < VRAM_SIZE) {
        vram[addr] = value;
    }
}

// Read VRAM
uint8 read_vram(uint16 addr) {
    if (addr < VRAM_SIZE) {
        return vram[addr];
    }
    return 0;
}

// Set BG1 Scroll
void set_scroll_bg1(uint16 h, uint16 v) {
    // Update shadow registers
    // These would normally write to PPU registers
}

// Set BG2 Scroll
void set_scroll_bg2(uint16 h, uint16 v) {
    // Update shadow registers
}

// Set BG3 Scroll
void set_scroll_bg3(uint16 h, uint16 v) {
    // Update shadow registers
}

// Set BG Mode
void set_bg_mode(uint8 mode) {
    // Update shadow register
}

// Set Mosaic
void set_mosaic(uint8 size) {
    // Update shadow register
}

// Convert SNES 15-bit color to RGB 8-bit
ColorRGB snes_color_to_rgb(uint16 snes_color) {
    ColorRGB c;
    c.r = (snes_color & 0x1F) << 3;                      // 5 bits → 8 bits
    c.g = ((snes_color >> 5) & 0x1F) << 3;
    c.b = ((snes_color >> 10) & 0x1F) << 3;
    return c;
}

// Load palette from ROM
void load_palette(uint8 palette_id) {
    // In a real implementation, this would read from ROM
    // and copy to shadow_palette
}

// Upload palette to CGRAM
void upload_palette() {
    // In a real implementation, this would write to CGRAM
    // via DMA or direct writes
}

// Get pixel from 4bpp tile
uint8 get_pixel_4bpp(const uint8* tile_data, int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return 0;
    }
    
    uint8 lo_byte = tile_data[y * 2];
    uint8 hi_byte = tile_data[y * 2 + 1];
    uint8 plane0 = (tile_data[16 + y * 2] >> x) & 1;
    uint8 plane1 = (tile_data[16 + y * 2 + 1] >> x) & 1;
    
    uint8 bit = 7 - x;
    uint8 p0 = (lo_byte >> bit) & 1;
    uint8 p1 = (hi_byte >> bit) & 1;
    
    return (plane0 << 3) | (plane1 << 2) | (p1 << 1) | p0;
}