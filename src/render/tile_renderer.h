#ifndef RENDER_TILE_RENDERER_H
#define RENDER_TILE_RENDERER_H

#include "../types.h"
#include "../snes/ppu.h"

// Render a single 8x8 4bpp tile into an ARGB pixel buffer
// tile_data: 32 bytes of 4bpp tile data (4 planes × 8 rows)
// palette: 16-color palette for this sub-palette
// out: ARGB output buffer
// out_pitch: stride of output buffer in pixels
// x, y: destination position in output buffer
// flip_h, flip_v: tile mirroring
void render_tile_4bpp(const uint8* tile_data, const ColorRGB* palette,
                       uint32* out, int out_pitch, int x, int y,
                       bool flip_h, bool flip_v, uint8 priority);

// Render a BG layer to the framebuffer
// layer: 1=BG1, 2=BG2, 3=BG3
// vram: VRAM data
// tilemap_addr: address of tilemap in VRAM
// tile_data_addr: address of tile data in VRAM
// palette: BG palette array
// scroll_h, scroll_v: scroll offsets
// framebuffer: 256x224 ARGB output
void render_bg_layer(int layer, const uint8* vram, uint16 tilemap_addr,
                     uint16 tile_data_addr, const ColorRGB palette[][COLORS_PER_PALETTE],
                     uint16 scroll_h, uint16 scroll_v, uint32* framebuffer);

#endif // RENDER_TILE_RENDERER_H
