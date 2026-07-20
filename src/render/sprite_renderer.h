#ifndef RENDER_SPRITE_RENDERER_H
#define RENDER_SPRITE_RENDERER_H

#include "../types.h"
#include "../snes/ppu.h"
#include "../game/sprites.h"

// SNES sprite sizes based on OBJSEL register
// bit 7-6: Name select, bit 5-3: Name base, bit 2-1: Size 0, bit 0: Size 1
struct SpriteSize {
    int small_w, small_h;  // Size when bit 0 of OAM attr = 0
    int large_w, large_h;  // Size when bit 0 of OAM attr = 1
};

// Get sprite dimensions from OBJSEL register value
SpriteSize get_sprite_sizes(uint8 objsel);

// Render all visible sprites from OAM to the framebuffer
// oam: OAM buffer with sprite entries
// vram: VRAM data (sprite tiles start at VRAM_SPRITES)
// palette: sprite palette array
// objsel: OBJSEL register value
// framebuffer: 256x224 ARGB output
void render_sprites(const OAMBuffer* oam, const uint8* vram,
                    const ColorRGB palette[][COLORS_PER_PALETTE],
                    uint8 objsel, uint32* framebuffer);

#endif // RENDER_SPRITE_RENDERER_H
