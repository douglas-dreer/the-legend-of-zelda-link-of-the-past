#include "ppu.h"
#include "dma.h"
#include "../game/state.h"
#include <cstring>
#include <cstdio>

// External references
extern PPURegisters ppu;
extern SNESPalette shadow_palette;
extern uint8 vram[];
extern uint8 cgram[];
extern uint8 rom[];
extern GlobalState shadow;

// ------------------------------------------------------------------
// ppu_write: Write to PPU register
// ------------------------------------------------------------------
void ppu_write(uint16 addr, uint8 value) {
    if (addr < 0x2100 || addr > 0x213F) return;

    switch (addr) {
        case 0x2100: ppu.inidisp = value; break;
        case 0x2101: ppu.objsel = value; break;
        case 0x2105: ppu.bgmode = value; break;
        case 0x2106: ppu.mosaic = value; break;
        case 0x210D:
            // BG1HOFS: write twice (low, then high+low>>8)
            ppu.bg1hofs = (ppu.bg1hofs << 8) | value;
            break;
        case 0x210E:
            ppu.bg1vofs = (ppu.bg1vofs << 8) | value;
            break;
        case 0x210F:
            ppu.bg2hofs = (ppu.bg2hofs << 8) | value;
            break;
        case 0x2110:
            ppu.bg2vofs = (ppu.bg2vofs << 8) | value;
            break;
        case 0x2111:
            ppu.bg3hofs = (ppu.bg3hofs << 8) | value;
            break;
        case 0x2112:
            ppu.bg3vofs = (ppu.bg3vofs << 8) | value;
            break;
        case 0x2115: ppu.vram_ctrl = value; break;
        case 0x2116: ppu.vram_addr = (ppu.vram_addr & 0xFF00) | value; break;
        case 0x2117: ppu.vram_addr = (ppu.vram_addr & 0x00FF) | (value << 8); break;
        case 0x2121: ppu.cgadd = value; break;
        case 0x2123: ppu.w12sel = value; break;
        case 0x2124: ppu.w34sel = value; break;
        case 0x2125: ppu.wobjsel = value; break;
        case 0x212C: ppu.tm = value; break;
        case 0x212D: ppu.ts = value; break;
        case 0x212E: ppu.tmw = value; break;
        case 0x212F: ppu.tsw = value; break;
        case 0x2130: ppu.cgsel = value; break;
        case 0x2131: ppu.cgadsub = value; break;
        case 0x2132: ppu.coldata = value; break;
        case 0x211A: ppu.m7sel = value; break;
        case 0x211B: ppu.m7a = (ppu.m7a << 8) | value; break;
        case 0x211C: ppu.m7b = (ppu.m7b << 8) | value; break;
        case 0x211D: ppu.m7c = (ppu.m7c << 8) | value; break;
        case 0x211E: ppu.m7d = (ppu.m7d << 8) | value; break;
        case 0x211F: ppu.m7x = (ppu.m7x << 8) | value; break;
        case 0x2120: ppu.m7y = (ppu.m7y << 8) | value; break;
        default: break;
    }
}

// ------------------------------------------------------------------
// ppu_read: Read from PPU register
// ------------------------------------------------------------------
uint8 ppu_read(uint16 addr) {
    if (addr < 0x2100 || addr > 0x213F) return 0;

    switch (addr) {
        case 0x2134: return (uint8)(ppu.m7a * (ppu.m7b >> 8)); // MPYL
        case 0x2135: return (uint8)((ppu.m7a * (ppu.m7b >> 8)) >> 8); // MPYM
        case 0x2136: return (uint8)((ppu.m7a * (ppu.m7b >> 8)) >> 16); // MPYH
        case 0x2137: return 0; // SLHV — latch H/V
        case 0x213E: return 0; // OAM data read
        case 0x213F: return 0; // CGRAM data read
        default: return 0;
    }
}

// ------------------------------------------------------------------
// write_vram: Write byte to VRAM
// ------------------------------------------------------------------
void write_vram(uint16 addr, uint8 value) {
    if (addr < VRAM_SIZE) {
        vram[addr] = value;
    }
}

// ------------------------------------------------------------------
// read_vram: Read byte from VRAM
// ------------------------------------------------------------------
uint8 read_vram(uint16 addr) {
    if (addr < VRAM_SIZE) {
        return vram[addr];
    }
    return 0;
}

// ------------------------------------------------------------------
// set_scroll_bg1-$3: Update shadow scroll values
// ------------------------------------------------------------------
void set_scroll_bg1(uint16 h, uint16 v) {
    shadow.scroll_bg1_h = h;
    shadow.scroll_bg1_v = v;
}

void set_scroll_bg2(uint16 h, uint16 v) {
    shadow.scroll_bg2_h = h;
    shadow.scroll_bg2_v = v;
}

void set_scroll_bg3(uint16 h, uint16 v) {
    shadow.bg3hofs_low = h & 0xFF;
    shadow.bg3hofs_high = (h >> 8) & 0xFF;
    shadow.bg3vofs_low = v & 0xFF;
    shadow.bg3vofs_high = (v >> 8) & 0xFF;
}

// ------------------------------------------------------------------
// set_bg_mode: Set BG mode shadow register
// ------------------------------------------------------------------
void set_bg_mode(uint8 mode) {
    shadow.bg_mode = mode;
}

// ------------------------------------------------------------------
// set_mosaic: Set mosaic shadow register
// ------------------------------------------------------------------
void set_mosaic(uint8 size) {
    shadow.mosaic = size;
}

// ------------------------------------------------------------------
// snes_color_to_rgb: Convert SNES 15-bit BGR to RGB 8-bit
// ------------------------------------------------------------------
ColorRGB snes_color_to_rgb(uint16 snes_color) {
    ColorRGB c;
    c.r = (snes_color & 0x1F) << 3;
    c.g = ((snes_color >> 5) & 0x1F) << 3;
    c.b = ((snes_color >> 10) & 0x1F) << 3;
    return c;
}

// ------------------------------------------------------------------
// palette_dma_upload: $00:8B87-$00:8BA5
//
// Upload palette buffer to CGRAM via DMA channel 1:
//   1. Check $15 (palette_dirty flag)
//   2. STZ $2121 — CGRAM address = 0
//   3. Configure DMA channel 1: mode $01 (2 bytes), B-bus $22 (CGRAM)
//   4. Source: $7E:0000 (palette in WRAM staging area)
//   5. Size: $0200 (512 bytes = 256 colors × 2 bytes)
//   6. Fire DMA: $02 → $420B
// ------------------------------------------------------------------
void palette_dma_upload(void) {
    if (shadow.palette_dirty == 0) return;

    // CGRAM address = 0
    ppu.cgadd = 0;

    // Copy shadow palette to CGRAM buffer
    // The game maintains a 512-byte palette buffer at $7E:0000
    // in WRAM, which is then DMA'd to CGRAM
    const uint8* palette_src = reinterpret_cast<const uint8*>(&shadow_palette);

    for (int i = 0; i < CGRAM_SIZE && i < (int)sizeof(SNESPalette); i++) {
        cgram[i] = palette_src[i];
    }

    // Clear dirty flag
    shadow.palette_dirty = 0;
}

// ------------------------------------------------------------------
// oam_dma_upload: $00:8BAA-$00:8BCA
//
// Upload OAM buffer to PPU via DMA channel 0:
//   1. REP #$20; SEP #$10 (16-bit A, 8-bit XY)
//   2. STZ $2102 — OAM address = 0
//   3. LDA #$00; STA $4302 — source addr = $0800 (OAM buffer)
//   4. STZ $4304 — source bank = $00
//   5. LDA #$0200 — size = 512 bytes
//   6. STA $4305
//   7. LDY #$01; STY $420B — fire channel 0
// ------------------------------------------------------------------
void oam_dma_upload(void) {
    // OAM address = 0
    ppu.oam_addr = 0;

    // The OAM buffer lives at $0800-$09FF in WRAM
    // DMA channel 0 transfers 512 bytes to $2104 (OAM data)
    // Mode 0: 1 byte → PPU (write once)
    // We simulate by copying our OAM buffer to the PPU's OAM

    // In a real port, this would call the DMA engine:
    // dma_transfer(OAM_BUFFER_ADDR, 0x2104, 0x0200, 0, 0x00);

    // For PC: the OAM buffer is already in oam_buffer struct,
    // the rendering layer reads from it directly.
}

// ------------------------------------------------------------------
// load_palette: Load palette data from ROM
// ------------------------------------------------------------------
void load_palette(uint8 palette_id) {
    // In the original game, palette data is loaded from ROM
    // and stored in the shadow palette buffer.
    // The actual palette addresses depend on the room/scene.
    // TODO: Implement palette lookup table from ROM
}

// ------------------------------------------------------------------
// upload_palette: Upload palette to CGRAM (wrapper)
// ------------------------------------------------------------------
void upload_palette() {
    palette_dma_upload();
}

// ------------------------------------------------------------------
// get_pixel_4bpp: Get pixel color index from 4bpp tile data
// ------------------------------------------------------------------
uint8 get_pixel_4bpp(const uint8* tile_data, int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return 0;

    uint8 bit = 7 - x;

    // Plane 0 & 1 (low 2 bits)
    uint8 lo_byte = tile_data[y * 2];
    uint8 hi_byte = tile_data[y * 2 + 1];
    uint8 p0 = (lo_byte >> bit) & 1;
    uint8 p1 = (hi_byte >> bit) & 1;

    // Plane 2 & 3 (high 2 bits)
    uint8 plane2 = (tile_data[16 + y * 2] >> bit) & 1;
    uint8 plane3 = (tile_data[16 + y * 2 + 1] >> bit) & 1;

    return (plane3 << 3) | (plane2 << 2) | (p1 << 1) | p0;
}
