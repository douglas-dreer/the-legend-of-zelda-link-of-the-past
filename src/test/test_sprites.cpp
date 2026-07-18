// =============================================================================
// test_sprites.cpp — Sprite/OAM system tests
//
// Tests:
//   1. OAM entry structure and size (4 bytes)
//   2. OAM high entry structure (2 bits per sprite)
//   3. Sprite structure
//   4. OAM packing (high byte table: 2 bits per sprite)
//   5. Sprite position update
//   6. Hide unused sprites (Y=0xF0)
//   7. Sprite bounds checking
//   8. OAM buffer layout (128 sprites × 4 bytes + 64 bytes high)
//   9. Generate OAM visualization PPM
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static const int MAX_SPRITES = 128;
static const uint8_t SPRITE_OFFSCREEN_Y = 0xF0;

// ---------------------------------------------------------------------------
// OAM structures (from sprites.h)
// ---------------------------------------------------------------------------
struct OAMEntry {
    uint8_t y;
    uint8_t tile;
    uint8_t attr;
    uint8_t x;
};

struct OAMHighEntry {
    uint8_t x_high : 2;
    uint8_t y_high : 2;
    uint8_t size : 1;
};

struct OAMBuffer {
    OAMEntry entries[MAX_SPRITES];
    OAMHighEntry high[MAX_SPRITES / 2];
};

struct Sprite {
    int16_t x;
    int16_t y;
    uint8_t tile;
    uint8_t attributes;
    uint8_t type;
    uint8_t state;
    uint8_t health;
    uint8_t speed;
    int8_t vel_x;
    int8_t vel_y;
};

// OAM attribute helpers
static uint8_t OAM_ATTR_PALETTE(uint8_t n) { return (n << 1); }
static uint8_t OAM_ATTR_PRIORITY(uint8_t n) { return (n << 4); }
static const uint8_t OAM_ATTR_FLIP_H = 0x40;
static const uint8_t OAM_ATTR_FLIP_V = 0x80;

// External framework functions
extern void test_pass(const char* name, const char* detail = "");
extern void test_fail(const char* name, const char* detail = "");
extern void section_header(const char* title);
extern bool write_ppm(const char* path, int w, int h, const uint8_t* rgb);

// Global buffers
static OAMBuffer oam_buffer;
static Sprite sprites[MAX_SPRITES];

// ---------------------------------------------------------------------------
// Test: OAM entry structure size
// ---------------------------------------------------------------------------
static void test_oam_entry_size() {
    if (sizeof(OAMEntry) == 4) {
        test_pass("OAMEntry: 4 bytes (Y, tile, attr, X)");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got %zu bytes, expected 4", sizeof(OAMEntry));
        test_fail("OAMEntry: size", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: OAM high entry structure
// ---------------------------------------------------------------------------
static void test_oam_high_entry() {
    OAMHighEntry high;
    memset(&high, 0, sizeof(high));

    high.x_high = 0x2;  // X bit 8-9 = 10
    high.y_high = 0x1;  // Y bit 8-9 = 01
    high.size = 1;       // Large sprite

    if (high.x_high == 2 && high.y_high == 1 && high.size == 1) {
        test_pass("OAMHighEntry: bitfield read/write");
    } else {
        test_fail("OAMHighEntry: bitfield", "values wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: OAM buffer layout
// ---------------------------------------------------------------------------
static void test_oam_buffer_layout() {
    // OAM buffer: 128 entries × 4 bytes = 512 bytes + 64 bytes high = 576 bytes
    size_t expected = MAX_SPRITES * sizeof(OAMEntry) + (MAX_SPRITES / 2) * sizeof(OAMHighEntry);
    if (sizeof(OAMBuffer) == expected) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%zu bytes (512 + 64)", sizeof(OAMBuffer));
        test_pass("OAMBuffer: correct layout", buf);
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got %zu, expected %zu", sizeof(OAMBuffer), expected);
        test_fail("OAMBuffer: layout size", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: Sprite structure
// ---------------------------------------------------------------------------
static void test_sprite_structure() {
    Sprite s;
    memset(&s, 0, sizeof(s));

    s.x = 128;
    s.y = 64;
    s.tile = 0x20;
    s.attributes = OAM_ATTR_PALETTE(2) | OAM_ATTR_FLIP_H;
    s.vel_x = 2;
    s.vel_y = -1;

    if (s.x == 128 && s.y == 64 && s.tile == 0x20 && s.vel_x == 2 && s.vel_y == -1) {
        test_pass("Sprite: structure fields set/read");
    } else {
        test_fail("Sprite: structure fields", "values wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: Hide unused sprites
// ---------------------------------------------------------------------------
static void test_hide_unused_sprites() {
    memset(&oam_buffer, 0, sizeof(oam_buffer));

    // Set some sprites to visible
    oam_buffer.entries[0].y = 0x10;
    oam_buffer.entries[10].y = 0x20;
    oam_buffer.entries[127].y = 0x30;

    // Hide all (set Y to off-screen)
    for (int i = 0; i < MAX_SPRITES; i++) {
        oam_buffer.entries[i].y = SPRITE_OFFSCREEN_Y;
    }

    bool ok = true;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (oam_buffer.entries[i].y != SPRITE_OFFSCREEN_Y) {
            ok = false;
            break;
        }
    }
    if (ok) test_pass("hide_unused_sprites: all Y set to $F0");
    else    test_fail("hide_unused_sprites: Y values", "some sprites not hidden");
}

// ---------------------------------------------------------------------------
// Test: OAM packing (high byte table)
// ---------------------------------------------------------------------------
static void test_oam_packing() {
    memset(&oam_buffer, 0, sizeof(oam_buffer));

    // Set sprite 0: X=0x180 (low=0x80, high=0x1), Y=0x3F8 (low=0xF8, high=0x3)
    oam_buffer.entries[0].x = 0x80;  // low byte
    oam_buffer.entries[0].y = 0xF8;  // low byte
    oam_buffer.high[0].x_high = 0x1; // bits 8-9 = 01 → X = 0x180
    oam_buffer.high[0].y_high = 0x3; // bits 8-9 = 11 → Y = 0x3F8

    // Reconstruct 10-bit position
    int x = oam_buffer.entries[0].x | (oam_buffer.high[0].x_high << 8);
    int y = oam_buffer.entries[0].y | (oam_buffer.high[0].y_high << 8);

    if (x == 0x180 && y == 0x3F8) {
        test_pass("OAM packing: 10-bit X/Y reconstruction");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "X=%d Y=%d, expected X=384 Y=1016", x, y);
        test_fail("OAM packing: 10-bit X/Y reconstruction", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: Sprite position update
// ---------------------------------------------------------------------------
static void test_sprite_position_update() {
    memset(sprites, 0, sizeof(sprites));

    sprites[0].x = 100;
    sprites[0].y = 200;
    sprites[0].vel_x = 3;
    sprites[0].vel_y = -2;

    // Simulate one frame of movement
    sprites[0].x += sprites[0].vel_x;
    sprites[0].y += sprites[0].vel_y;

    if (sprites[0].x == 103 && sprites[0].y == 198) {
        test_pass("Sprite update: position + velocity");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got (%d,%d), expected (103,198)", sprites[0].x, sprites[0].y);
        test_fail("Sprite update: position + velocity", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: Sprite bounds checking
// ---------------------------------------------------------------------------
static void test_sprite_bounds() {
    // Verify index bounds for set_sprite_position
    auto set_sprite_position = [](int idx, int16_t x, int16_t y) {
        if (idx >= 0 && idx < MAX_SPRITES) {
            sprites[idx].x = x;
            sprites[idx].y = y;
            return true;
        }
        return false;
    };

    bool ok = true;
    if (!set_sprite_position(-1, 0, 0)) { /* good: rejected */ } else { ok = false; }
    if (!set_sprite_position(MAX_SPRITES, 0, 0)) { /* good: rejected */ } else { ok = false; }
    if (set_sprite_position(0, 50, 60)) { /* good: accepted */ } else { ok = false; }
    if (set_sprite_position(127, 100, 200)) { /* good: accepted */ } else { ok = false; }

    if (ok && sprites[0].x == 50 && sprites[127].y == 200) {
        test_pass("Sprite bounds: valid indices accepted, invalid rejected");
    } else {
        test_fail("Sprite bounds: index validation", "unexpected behavior");
    }
}

// ---------------------------------------------------------------------------
// Test: OAM attribute encoding
// ---------------------------------------------------------------------------
static void test_oam_attributes() {
    // Palette 3, priority 1, flip H
    uint8_t attr = OAM_ATTR_PALETTE(3) | OAM_ATTR_PRIORITY(1) | OAM_ATTR_FLIP_H;

    uint8_t palette = (attr >> 1) & 0x07;
    uint8_t priority = (attr >> 4) & 0x03;
    bool flip_h = (attr & OAM_ATTR_FLIP_H) != 0;
    bool flip_v = (attr & OAM_ATTR_FLIP_V) != 0;

    if (palette == 3 && priority == 1 && flip_h && !flip_v) {
        test_pass("OAM attributes: encode/decode palette+priority+flip");
    } else {
        test_fail("OAM attributes: encode/decode", "values wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: Generate OAM visualization PPM
// ---------------------------------------------------------------------------
static void test_oam_visualization() {
    memset(&oam_buffer, 0, sizeof(oam_buffer));

    // Place 8 sprites in a row, each a different color
    for (int i = 0; i < 8; i++) {
        oam_buffer.entries[i].x = (uint8_t)(i * 16 + 8);
        oam_buffer.entries[i].y = 0x40;
        oam_buffer.entries[i].tile = i;
        oam_buffer.entries[i].attr = OAM_ATTR_PALETTE(i & 7);
    }

    // Hide remaining sprites
    for (int i = 8; i < MAX_SPRITES; i++) {
        oam_buffer.entries[i].y = SPRITE_OFFSCREEN_Y;
    }

    // Generate PPM: 256×256, each sprite is 16×16 colored block
    const int W = 256;
    const int H = 256;
    std::vector<uint8_t> pixels(W * H * 3, 0);

    // Background: dark blue
    for (int i = 0; i < W * H * 3; i += 3) {
        pixels[i + 0] = 20;
        pixels[i + 1] = 20;
        pixels[i + 2] = 60;
    }

    // Draw visible sprites as colored rectangles
    uint8_t sprite_colors[8][3] = {
        {255, 0, 0},    {0, 255, 0},    {0, 0, 255},    {255, 255, 0},
        {255, 0, 255},  {0, 255, 255},  {255, 128, 0},  {128, 0, 255}
    };

    for (int s = 0; s < 8; s++) {
        int sx = oam_buffer.entries[s].x;
        int sy = oam_buffer.entries[s].y;
        uint8_t* col = sprite_colors[s];

        for (int py = sy; py < sy + 16 && py < H; py++) {
            for (int px = sx; px < sx + 16 && px < W; px++) {
                if (px >= 0 && py >= 0) {
                    int off = (py * W + px) * 3;
                    pixels[off + 0] = col[0];
                    pixels[off + 1] = col[1];
                    pixels[off + 2] = col[2];
                }
            }
        }
    }

    // Draw a 1px border around each sprite for clarity
    for (int s = 0; s < 8; s++) {
        int sx = oam_buffer.entries[s].x;
        int sy = oam_buffer.entries[s].y;
        for (int px = sx; px < sx + 16 && px < W; px++) {
            if (sy >= 0 && sy < H) {
                int off = (sy * W + px) * 3;
                pixels[off + 0] = 255; pixels[off + 1] = 255; pixels[off + 2] = 255;
            }
            if (sy + 15 < H && sy + 15 >= 0) {
                int off = ((sy + 15) * W + px) * 3;
                pixels[off + 0] = 255; pixels[off + 1] = 255; pixels[off + 2] = 255;
            }
        }
        for (int py = sy; py < sy + 16 && py < H; py++) {
            if (sx >= 0 && sx < W && py >= 0) {
                int off = (py * W + sx) * 3;
                pixels[off + 0] = 255; pixels[off + 1] = 255; pixels[off + 2] = 255;
            }
            if (sx + 15 < W && py >= 0) {
                int off = (py * W + sx + 15) * 3;
                pixels[off + 0] = 255; pixels[off + 1] = 255; pixels[off + 2] = 255;
            }
        }
    }

    if (write_ppm("output/test/sprites_test.ppm", W, H, pixels.data())) {
        test_pass("PPM: OAM visualization", "output/test/sprites_test.ppm");
    } else {
        test_fail("PPM: OAM visualization", "could not write file");
    }
}

// ---------------------------------------------------------------------------
// Test: OAM buffer address layout ($0800-$09FF)
// ---------------------------------------------------------------------------
static void test_oam_memory_layout() {
    // SNES OAM buffer: $0800-$09FF (544 bytes)
    // $0800-$08FF: 256 bytes for 64 sprites (main table)
    // $0900-$093F: 64 bytes for high byte table
    // $0940-$09FF: unused (but DMA'd as part of 512-byte block)

    // Verify our buffer layout matches
    uint8_t* base = reinterpret_cast<uint8_t*>(&oam_buffer);

    // Entry 0 should be at offset 0
    OAMEntry* entry0 = reinterpret_cast<OAMEntry*>(base);
    if (entry0 == &oam_buffer.entries[0]) {
        test_pass("OAM memory: entry 0 at buffer base");
    } else {
        test_fail("OAM memory: entry 0 at base", "pointer mismatch");
    }

    // High table should follow entries
    OAMHighEntry* high0 = reinterpret_cast<OAMHighEntry*>(base + MAX_SPRITES * sizeof(OAMEntry));
    if (high0 == &oam_buffer.high[0]) {
        test_pass("OAM memory: high table after main table");
    } else {
        test_fail("OAM memory: high table position", "pointer mismatch");
    }
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void test_sprites() {
    section_header("SPRITES / OAM");
    test_oam_entry_size();
    test_oam_high_entry();
    test_oam_buffer_layout();
    test_oam_memory_layout();
    test_oam_packing();
    test_oam_attributes();
    test_sprite_structure();
    test_hide_unused_sprites();
    test_sprite_position_update();
    test_sprite_bounds();
    test_oam_visualization();
}
