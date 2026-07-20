#include "tile_renderer.h"
#include "../snes/ppu.h"
#include <cstring>

extern uint8 vram[];
extern SNESPalette shadow_palette;

void render_tile_4bpp(const uint8* tile_data, const ColorRGB* palette,
                       uint32* out, int out_pitch, int x, int y,
                       bool flip_h, bool flip_v, uint8 priority) {
    for (int row = 0; row < 8; row++) {
        int src_y = flip_v ? (7 - row) : row;

        for (int col = 0; col < 8; col++) {
            int src_x = flip_h ? (7 - col) : col;
            uint8 color_idx = get_pixel_4bpp(tile_data, src_x, src_y);

            if (color_idx == 0) continue;  // Transparent

            int dst_x = x + col;
            int dst_y = y + row;
            if (dst_x < 0 || dst_x >= SCREEN_WIDTH || dst_y < 0 || dst_y >= SCREEN_HEIGHT)
                continue;

            const ColorRGB& c = palette[color_idx];
            uint32 argb = (0xFF << 24) | (c.r << 16) | (c.g << 8) | c.b;
            out[dst_y * out_pitch + dst_x] = argb;
        }
    }
}

void render_bg_layer(int layer, const uint8* vram, uint16 tilemap_addr,
                     uint16 tile_data_addr, const ColorRGB palette[][COLORS_PER_PALETTE],
                     uint16 scroll_h, uint16 scroll_v, uint32* framebuffer) {
    // SNES tilemap is 32x32 tiles (1024 bytes, 2 bytes per entry)
    // Each tilemap entry: bits 0-9 = tile number, 10-12 = palette, 13-14 = priority, 15 = flip H
    // Bit 31 (high byte bit 7) = flip V

    // Layer priority: BG1 and BG2 have 2 priority levels, BG3 has 1
    for (int prio = 1; prio >= 0; prio--) {
        for (int ty = 0; ty < 32; ty++) {
            for (int tx = 0; tx < 32; tx++) {
                // Calculate tilemap offset
                int map_offset = (ty * 32 + tx) * 2;
                uint16 entry_low = vram[tilemap_addr + map_offset];
                uint16 entry_high = vram[tilemap_addr + map_offset + 1];
                uint16 entry = entry_low | (entry_high << 8);

                uint16 tile_num = entry & 0x03FF;
                uint8 pal_idx = (entry >> 10) & 0x07;
                uint8 tile_prio = (entry >> 13) & 0x03;
                bool flip_h = (entry >> 15) & 1;
                bool flip_v = (entry_high >> 7) & 1;

                // Skip tiles not matching current priority pass
                if (tile_prio != prio) continue;

                // Calculate screen position with scroll
                int screen_x = tx * 8 - scroll_h;
                int screen_y = ty * 8 - scroll_v;

                // Wrap around (tilemap wraps at 256 pixels for 32 tiles)
                screen_x = ((screen_x % 256) + 256) % 256;
                screen_y = ((screen_y % 256) + 256) % 256;

                // If tile would be off-screen, wrap to other side
                if (screen_x > 248) screen_x -= 256;
                if (screen_y > 216) screen_y -= 256;

                // Calculate tile data address
                uint16 tile_data_offset = tile_data_addr + tile_num * 32;  // 32 bytes per 4bpp tile
                const uint8* tile_ptr = &vram[tile_data_offset];

                // Render the 8x8 tile
                render_tile_4bpp(tile_ptr, palette[pal_idx],
                                 framebuffer, SCREEN_WIDTH,
                                 screen_x, screen_y,
                                 flip_h, flip_v, tile_prio);
            }
        }
    }
}
