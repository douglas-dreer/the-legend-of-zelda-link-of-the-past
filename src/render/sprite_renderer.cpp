#include "sprite_renderer.h"
#include "tile_renderer.h"
#include "../snes/ppu.h"
#include <cstdio>

extern uint8 vram[];
extern SNESPalette shadow_palette;
extern OAMBuffer oam_buffer;

SpriteSize get_sprite_sizes(uint8 objsel) {
    // OBJSEL bits 7-6 select the name (tile base) and size combination
    // Bit 0 = large size select for each object
    // Default sizes for most configurations:
    //   Mode 0:  8x8 / 16x16
    //   Mode 1:  8x8 / 16x16  (SNES default)
    //   Mode 2:  8x8 / 32x32
    //   Mode 3: 16x16 / 32x32
    //   Mode 4: 16x16 / 32x32
    //   Mode 5: 16x16 / 32x32
    //   Mode 6: 16x16 / 32x32
    //   Mode 7: 16x16 / 32x32
    // Bit 7-6 of OBJSEL:
    //   00: 8x8 / 16x16
    //   01: 8x8 / 32x32
    //   10: 8x8 / 64x64
    //   11: 16x16 / 32x32
    uint8 size_bits = (objsel >> 6) & 0x03;
    switch (size_bits) {
        case 0: return {8, 8, 16, 16};
        case 1: return {8, 8, 32, 32};
        case 2: return {8, 8, 64, 64};
        case 3: return {16, 16, 32, 32};
        default: return {8, 8, 16, 16};
    }
}

void render_sprites(const OAMBuffer* oam, const uint8* vram_data,
                    const ColorRGB palette[][COLORS_PER_PALETTE],
                    uint8 objsel, uint32* framebuffer) {
    SpriteSize sizes = get_sprite_sizes(objsel);
    uint16 tile_base = VRAM_SPRITES;  // $6000

    // OAM priority processing: sprites are rendered in OAM order
    // but priority bits affect draw order
    // Process from entry 127 down to 0 (OAM priority: lower index = higher priority)
    for (int i = MAX_SPRITES - 1; i >= 0; i--) {
        const OAMEntry& entry = oam->entries[i];

        // Skip off-screen sprites (Y = $F0 or higher)
        if (entry.y >= 0xF0) continue;

        // Get high bits (X high, Y high, size)
        int high_idx = i / 2;
        bool is_large = (i & 1)
            ? (oam->high[high_idx].size >> 1) & 1
            : oam->high[high_idx].size & 1;

        int x_high = (i & 1)
            ? (oam->high[high_idx].x_high >> 1) & 1
            : oam->high[high_idx].x_high & 1;

        int y_high = (i & 1)
            ? (oam->high[high_idx].y_high >> 1) & 1
            : oam->high[high_idx].y_high & 1;

        // Full 9-bit coordinates
        int sx = entry.x | (x_high << 8);
        int sy = entry.y | (y_high << 8);

        // Sign extend for negative coordinates
        if (sx > 255) sx -= 512;
        if (sy > 239) sy -= 512;

        // Sprite dimensions
        int spr_w = is_large ? sizes.large_w : sizes.small_w;
        int spr_h = is_large ? sizes.large_h : sizes.small_h;

        // Attributes
        uint8 palette_num = (entry.attr >> 1) & 0x07;
        uint8 priority = (entry.attr >> 4) & 0x03;
        bool flip_h = entry.attr & OAM_ATTR_FLIP_H;
        bool flip_v = entry.attr & OAM_ATTR_FLIP_V;

        // Sprites larger than 8x8 use multiple tiles
        // The tile number is the base; tiles are arranged in a grid
        int tiles_x = spr_w / 8;
        int tiles_y = spr_h / 8;

        for (int ty = 0; ty < tiles_y; ty++) {
            for (int tx = 0; tx < tiles_x; tx++) {
                int dx = sx + tx * 8;
                int dy = sy + ty * 8;

                // Handle horizontal flip for multi-tile sprites
                int actual_tx = flip_h ? (tiles_x - 1 - tx) : tx;
                int actual_ty = flip_v ? (tiles_y - 1 - ty) : ty;

                // Calculate tile index
                // SNES sprites: tiles are arranged row-by-row
                // For 16x16 sprites: tile layout is 0,1,2,3 (left-top, right-top, left-bottom, right-bottom)
                uint16 tile_idx = entry.tile + actual_ty * 8 + actual_tx;
                // Mask to sprite name area
                tile_idx &= 0x00FF;

                uint16 tile_data_offset = tile_base + tile_idx * 32;
                const uint8* tile_ptr = &vram_data[tile_data_offset];

                render_tile_4bpp(tile_ptr, palette[palette_num],
                                 framebuffer, SCREEN_WIDTH,
                                 dx, dy,
                                 flip_h, flip_v, priority);
            }
        }
    }
}
