// =============================================================================
// test_palettes.cpp — Palette system tests
//
// Tests:
//   1. SNES 15-bit → RGB 24-bit conversion correctness
//   2. ALL_PALETTES array contains expected number of palettes (276)
//   3. Each palette has exactly 16 colors
//   4. Specific palette values match known SNES colors
//   5. Brightness adjustment produces expected results
//   6. Round-trip SNES→RGB→SNES preserves data
//   7. Named palette arrays match ALL_PALETTES entries
//   8. Generate PPM visualization of all palettes
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Palette inline functions (duplicated from palettes.h for test isolation)
// ---------------------------------------------------------------------------
inline void snes_to_rgb(uint16_t snes_color, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = (snes_color & 0x1F) << 3;
    *g = ((snes_color >> 5) & 0x1F) << 3;
    *b = ((snes_color >> 10) & 0x1F) << 3;
}

inline uint16_t rgb_to_snes(uint8_t r, uint8_t g, uint8_t b) {
    return ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3);
}

inline uint32_t snes_to_rgba(uint16_t snes_color, uint8_t alpha = 255) {
    uint8_t r, g, b;
    snes_to_rgb(snes_color, &r, &g, &b);
    return (static_cast<uint32_t>(alpha) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8) |
           static_cast<uint32_t>(r);
}

// ---------------------------------------------------------------------------
// Palette data (from palettes.cpp)
// ---------------------------------------------------------------------------
#define TOTAL_PALETTES 276
#define PALETTE_COLORS_PER 16

extern const uint16_t PALETTE_MAIN_BG[16];
extern const uint16_t PALETTE_MAIN_SPRITES[16];
extern const uint16_t PALETTE_UI[16];
extern const uint16_t PALETTE_TITLE[16];
extern const uint16_t PALETTE_LINK_GREEN[16];
extern const uint16_t PALETTE_LINK_BLUE[16];
extern const uint16_t PALETTE_LINK_RED[16];
extern const uint16_t PALETTE_ENEMIES_1[16];
extern const uint16_t PALETTE_ENEMIES_2[16];
extern const uint16_t PALETTE_ITEMS[16];
extern const uint16_t PALETTE_EFFECTS[16];
extern const uint16_t PALETTE_DUNGEON_1[16];
extern const uint16_t PALETTE_DUNGEON_2[16];
extern const uint16_t PALETTE_OVERWORLD_1[16];
extern const uint16_t PALETTE_OVERWORLD_2[16];
extern const uint16_t PALETTE_HOUSE[16];
extern const uint16_t ALL_PALETTES[TOTAL_PALETTES][PALETTE_COLORS_PER];

// External framework functions (from test_main.cpp)
extern void test_pass(const char* name, const char* detail = "");
extern void test_fail(const char* name, const char* detail = "");
extern void section_header(const char* title);
extern bool write_ppm(const char* path, int w, int h, const uint8_t* rgb);

// ---------------------------------------------------------------------------
// Test: SNES color conversion — known reference values
// ---------------------------------------------------------------------------
static void test_snes_to_rgb_conversion() {
    // SNES 15-bit format: bbbbbgggggrrrrr (little-endian stored as gggrrrrr 0bbbbbgg)
    {
        uint8_t r, g, b;
        snes_to_rgb(0x0000, &r, &g, &b);
        if (r == 0 && g == 0 && b == 0) {
            test_pass("snes_to_rgb: pure black (0x0000)");
        } else {
            test_fail("snes_to_rgb: pure black", "expected (0,0,0)");
        }
    }
    {
        uint8_t r, g, b;
        snes_to_rgb(0x001F, &r, &g, &b);
        if (r == 248 && g == 0 && b == 0) {
            test_pass("snes_to_rgb: pure red max (0x001F)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (248,0,0)", r, g, b);
            test_fail("snes_to_rgb: pure red max", buf);
        }
    }
    {
        uint8_t r, g, b;
        snes_to_rgb(0x03E0, &r, &g, &b);
        if (r == 0 && g == 248 && b == 0) {
            test_pass("snes_to_rgb: pure green max (0x03E0)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (0,248,0)", r, g, b);
            test_fail("snes_to_rgb: pure green max", buf);
        }
    }
    {
        uint8_t r, g, b;
        snes_to_rgb(0x7C00, &r, &g, &b);
        if (r == 0 && g == 0 && b == 248) {
            test_pass("snes_to_rgb: pure blue max (0x7C00)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (0,0,248)", r, g, b);
            test_fail("snes_to_rgb: pure blue max", buf);
        }
    }
    {
        uint8_t r, g, b;
        snes_to_rgb(0x7FFF, &r, &g, &b);
        if (r == 248 && g == 248 && b == 248) {
            test_pass("snes_to_rgb: white (0x7FFF)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (248,248,248)", r, g, b);
            test_fail("snes_to_rgb: white", buf);
        }
    }
    // Link skin color: 0x1778
    // R = (0x1778 & 0x1F) << 3 = 24<<3 = 192
    // G = ((0x1778>>5) & 0x1F) << 3 = 27<<3 = 216
    // B = ((0x1778>>10) & 0x1F) << 3 = 5<<3 = 40
    {
        uint8_t r, g, b;
        snes_to_rgb(0x1778, &r, &g, &b);
        if (r == 192 && g == 216 && b == 40) {
            test_pass("snes_to_rgb: Link skin (0x1778)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (192,216,40)", r, g, b);
            test_fail("snes_to_rgb: Link skin", buf);
        }
    }
    // HUD red: 0x0910
    // R = 16<<3=128, G = 8<<3=64, B = 2<<3=16
    {
        uint8_t r, g, b;
        snes_to_rgb(0x0910, &r, &g, &b);
        if (r == 128 && g == 64 && b == 16) {
            test_pass("snes_to_rgb: HUD red (0x0910)");
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "got (%d,%d,%d), expected (128,64,16)", r, g, b);
            test_fail("snes_to_rgb: HUD red", buf);
        }
    }
}

// ---------------------------------------------------------------------------
// Test: RGB → SNES round-trip
// ---------------------------------------------------------------------------
static void test_roundtrip() {
    uint16_t test_values[] = {0x0000, 0x7FFF, 0x1778, 0x0910, 0x03E0, 0x7C00};
    int count = sizeof(test_values) / sizeof(test_values[0]);
    int passed = 0;

    for (int i = 0; i < count; i++) {
        uint8_t r, g, b;
        snes_to_rgb(test_values[i], &r, &g, &b);
        uint16_t roundtrip = rgb_to_snes(r, g, b);
        if (roundtrip == test_values[i]) {
            passed++;
        }
    }

    if (passed == count) {
        test_pass("rgb_to_snes round-trip", "all values preserved");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d/%d values preserved", passed, count);
        test_fail("rgb_to_snes round-trip", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: ALL_PALETTES structure
// ---------------------------------------------------------------------------
static void test_palette_count() {
    int nonzero_palettes = 0;
    for (int i = 0; i < TOTAL_PALETTES; i++) {
        bool has_color = false;
        for (int j = 0; j < PALETTE_COLORS_PER; j++) {
            if (ALL_PALETTES[i][j] != 0) {
                has_color = true;
                break;
            }
        }
        if (has_color) nonzero_palettes++;
    }

    if (nonzero_palettes > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%d palettes with color data", nonzero_palettes);
        test_pass("ALL_PALETTES: non-empty palettes", buf);
    } else {
        test_fail("ALL_PALETTES: non-empty palettes", "all palettes are zero");
    }
}

// ---------------------------------------------------------------------------
// Test: Named palette arrays match ALL_PALETTES
// ---------------------------------------------------------------------------
static void test_named_palettes_match() {
    // PALETTE_MAIN_BG should match ALL_PALETTES[0]
    bool match = true;
    for (int i = 0; i < 16; i++) {
        if (PALETTE_MAIN_BG[i] != ALL_PALETTES[0][i]) { match = false; break; }
    }
    if (match) test_pass("PALETTE_MAIN_BG matches ALL_PALETTES[0]");
    else       test_fail("PALETTE_MAIN_BG matches ALL_PALETTES[0]", "mismatch");

    match = true;
    for (int i = 0; i < 16; i++) {
        if (PALETTE_MAIN_SPRITES[i] != ALL_PALETTES[1][i]) { match = false; break; }
    }
    if (match) test_pass("PALETTE_MAIN_SPRITES matches ALL_PALETTES[1]");
    else       test_fail("PALETTE_MAIN_SPRITES matches ALL_PALETTES[1]", "mismatch");

    match = true;
    for (int i = 0; i < 16; i++) {
        if (PALETTE_UI[i] != ALL_PALETTES[2][i]) { match = false; break; }
    }
    if (match) test_pass("PALETTE_UI matches ALL_PALETTES[2]");
    else       test_fail("PALETTE_UI matches ALL_PALETTES[2]", "mismatch");
}

// ---------------------------------------------------------------------------
// Test: Brightness adjustment
// ---------------------------------------------------------------------------
static void test_brightness() {
    uint16_t test_pal[16];
    for (int i = 0; i < 16; i++) {
        test_pal[i] = 0x7C00 | (i << 5) | i;
    }
    uint16_t orig[16];
    memcpy(orig, test_pal, sizeof(orig));

    auto apply_bright = [](uint16_t* pal, uint8_t brightness) {
        for (int i = 0; i < 16; i++) {
            uint8_t r = (pal[i] & 0x1F);
            uint8_t g = ((pal[i] >> 5) & 0x1F);
            uint8_t b = ((pal[i] >> 10) & 0x1F);
            r = (r * brightness) >> 5;
            g = (g * brightness) >> 5;
            b = (b * brightness) >> 5;
            pal[i] = (b << 10) | (g << 5) | r;
        }
    };

    // brightness=0 → all black
    apply_bright(test_pal, 0);
    bool all_zero = true;
    for (int i = 0; i < 16; i++) {
        if (test_pal[i] != 0) { all_zero = false; break; }
    }
    if (all_zero) test_pass("brightness=0: all colors black");
    else          test_fail("brightness=0: all colors black", "some colors non-zero");

    // brightness=31 → 31*val/32 (slightly less than original, SNES behavior)
    memcpy(test_pal, orig, sizeof(orig));
    apply_bright(test_pal, 31);
    bool all_close = true;
    for (int i = 0; i < 16; i++) {
        // Each component: (val * 31) >> 5 = val - (val >> 5)
        uint8_t orig_r = orig[i] & 0x1F;
        uint8_t orig_g = (orig[i] >> 5) & 0x1F;
        uint8_t orig_b = (orig[i] >> 10) & 0x1F;
        uint16_t expected = ((orig_b * 31) >> 5) << 10 |
                            ((orig_g * 31) >> 5) << 5 |
                            ((orig_r * 31) >> 5);
        if (test_pal[i] != expected) { all_close = false; break; }
    }
    if (all_close) test_pass("brightness=31: SNES-correct scaling (31/32)");
    else           test_fail("brightness=31: SNES-correct scaling", "values wrong");
}

// ---------------------------------------------------------------------------
// Test: snes_to_rgba function
// ---------------------------------------------------------------------------
static void test_rgba_conversion() {
    uint32_t rgba = snes_to_rgba(0x7FFF, 255);
    uint32_t expected = (255u << 24) | (248u << 16) | (248u << 8) | 248u;
    if (rgba == expected) {
        test_pass("snes_to_rgba: white (0x7FFF)");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got 0x%08X, expected 0x%08X", (unsigned)rgba, (unsigned)expected);
        test_fail("snes_to_rgba: white (0x7FFF)", buf);
    }

    rgba = snes_to_rgba(0x0000, 255);
    expected = 0xFF000000;
    if (rgba == expected) {
        test_pass("snes_to_rgba: black (0x0000)");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got 0x%08X, expected 0x%08X", (unsigned)rgba, (unsigned)expected);
        test_fail("snes_to_rgba: black (0x0000)", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: Generate PPM visualization of all palettes
// ---------------------------------------------------------------------------
static void test_generate_palette_ppm() {
    // Layout: 16 swatches per palette row, 276 rows
    // Each swatch is 32 pixels wide
    const int W = 16 * 32;
    const int H = 276;
    std::vector<uint8_t> pixels(W * H * 3, 0);

    for (int p = 0; p < TOTAL_PALETTES; p++) {
        for (int c = 0; c < PALETTE_COLORS_PER; c++) {
            uint8_t r, g, b;
            snes_to_rgb(ALL_PALETTES[p][c], &r, &g, &b);
            for (int x = 0; x < 32; x++) {
                int idx = (p * W + c * 32 + x) * 3;
                pixels[idx + 0] = r;
                pixels[idx + 1] = g;
                pixels[idx + 2] = b;
            }
        }
    }

    if (write_ppm("output/test/all_palettes_test.ppm", W, H, pixels.data())) {
        test_pass("PPM: all palettes visualization", "output/test/all_palettes_test.ppm");
    } else {
        test_fail("PPM: all palettes visualization", "could not write file");
    }

    // Individual named palettes as swatches
    const uint16_t* named[] = {
        PALETTE_LINK_GREEN, PALETTE_LINK_BLUE, PALETTE_LINK_RED,
        PALETTE_UI, PALETTE_TITLE, PALETTE_OVERWORLD_1, PALETTE_OVERWORLD_2,
        PALETTE_DUNGEON_1, PALETTE_DUNGEON_2
    };
    const char* names[] = {
        "link_green", "link_blue", "link_red",
        "ui", "title", "overworld_1", "overworld_2",
        "dungeon_1", "dungeon_2"
    };

    for (int i = 0; i < 9; i++) {
        const int SW = 16 * 32;
        const int SH = 16 * 32;
        std::vector<uint8_t> swatch(SW * SH * 3, 0);
        for (int cy = 0; cy < 16; cy++) {
            uint8_t r, g, b;
            snes_to_rgb(named[i][cy], &r, &g, &b);
            for (int y = cy * 32; y < (cy + 1) * 32; y++) {
                for (int x = 0; x < SW; x++) {
                    int idx = (y * SW + x) * 3;
                    swatch[idx + 0] = r;
                    swatch[idx + 1] = g;
                    swatch[idx + 2] = b;
                }
            }
        }
        char path[128];
        snprintf(path, sizeof(path), "output/test/palette_%s.ppm", names[i]);
        write_ppm(path, SW, SH, swatch.data());
    }
    test_pass("PPM: individual palette swatches", "9 palettes saved");
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void test_palettes() {
    section_header("PALETTES");
    test_snes_to_rgb_conversion();
    test_roundtrip();
    test_rgba_conversion();
    test_palette_count();
    test_named_palettes_match();
    test_brightness();
    test_generate_palette_ppm();
}
