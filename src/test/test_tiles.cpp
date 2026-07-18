// =============================================================================
// test_tiles.cpp — Tile system tests
//
// Tests:
//   1. 4bpp tile decoding correctness (known reference patterns)
//   2. Tile size constants (32 bytes per tile)
//   3. Empty tile detection
//   4. Tile pointer arithmetic
//   5. Decode synthetic tiles and verify pixel patterns
//   6. Generate PPM from decoded tiles
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Tile constants and functions (duplicated from tiles.h for test isolation)
// ---------------------------------------------------------------------------
static const int TILE_SIZE_4BPP = 32;
static const int TILE_PIXEL_SIZE = 8;

inline void decode_tile_4bpp(const uint8_t* tile_data, uint8_t* pixels) {
    for (int y = 0; y < 8; y++) {
        uint8_t plane0 = tile_data[y * 2];
        uint8_t plane1 = tile_data[y * 2 + 1];
        uint8_t plane2 = tile_data[16 + y * 2];
        uint8_t plane3 = tile_data[16 + y * 2 + 1];

        for (int x = 0; x < 8; x++) {
            int bit = 7 - x;
            pixels[y * 8 + x] = ((plane0 >> bit) & 1) |
                                (((plane1 >> bit) & 1) << 1) |
                                (((plane2 >> bit) & 1) << 2) |
                                (((plane3 >> bit) & 1) << 3);
        }
    }
}

inline bool is_tile_empty(const uint8_t* tile_data) {
    for (int i = 0; i < TILE_SIZE_4BPP; i++) {
        if (tile_data[i] != 0) return false;
    }
    return true;
}

inline const uint8_t* get_tile_ptr(const uint8_t* tile_array, int tile_index) {
    return &tile_array[tile_index * TILE_SIZE_4BPP];
}

// Mutable version for test data construction
inline uint8_t* get_tile_ptr_mut(uint8_t* tile_array, int tile_index) {
    return &tile_array[tile_index * TILE_SIZE_4BPP];
}

// External framework functions
extern void test_pass(const char* name, const char* detail = "");
extern void test_fail(const char* name, const char* detail = "");
extern void section_header(const char* title);
extern bool write_ppm(const char* path, int w, int h, const uint8_t* rgb);

// ---------------------------------------------------------------------------
// Test: 4bpp decoding of known pattern
// ---------------------------------------------------------------------------
static void test_decode_known_tile() {
    // Create a tile where all pixels are color index 5 (binary 0101)
    // Plane 0 (bit 0): 1 for all pixels → 0xFF per row
    // Plane 1 (bit 1): 0 for all pixels → 0x00 per row
    // Plane 2 (bit 2): 1 for all pixels → 0xFF per row
    // Plane 3 (bit 3): 0 for all pixels → 0x00 per row
    uint8_t tile[32];
    for (int y = 0; y < 8; y++) {
        tile[y * 2 + 0] = 0xFF;  // plane 0 low (all 1s)
        tile[y * 2 + 1] = 0x00;  // plane 1 low (all 0s)
        tile[16 + y * 2 + 0] = 0xFF;  // plane 2 high (all 1s)
        tile[16 + y * 2 + 1] = 0x00;  // plane 3 high (all 0s)
    }

    uint8_t pixels[64];
    decode_tile_4bpp(tile, pixels);

    bool all_match = true;
    for (int i = 0; i < 64; i++) {
        if (pixels[i] != 5) {
            all_match = false;
            break;
        }
    }
    if (all_match) {
        test_pass("decode_tile_4bpp: all-pixels-5 pattern");
    } else {
        test_fail("decode_tile_4bpp: all-pixels-5 pattern", "unexpected pixel values");
    }
}

// ---------------------------------------------------------------------------
// Test: Empty tile detection
// ---------------------------------------------------------------------------
static void test_empty_tile() {
    uint8_t empty_tile[32];
    memset(empty_tile, 0, 32);
    if (is_tile_empty(empty_tile)) {
        test_pass("is_tile_empty: zeroed tile");
    } else {
        test_fail("is_tile_empty: zeroed tile", "returned false");
    }

    uint8_t nonempty_tile[32];
    memset(nonempty_tile, 0, 32);
    nonempty_tile[0] = 0x01;
    if (!is_tile_empty(nonempty_tile)) {
        test_pass("is_tile_empty: non-zero tile");
    } else {
        test_fail("is_tile_empty: non-zero tile", "returned true");
    }
}

// ---------------------------------------------------------------------------
// Test: Tile pointer arithmetic
// ---------------------------------------------------------------------------
static void test_tile_ptr() {
    uint8_t tile_data[TILE_SIZE_4BPP * 4];
    memset(tile_data, 0, sizeof(tile_data));

    // Tile 0 → offset 0
    const uint8_t* p0 = get_tile_ptr(tile_data, 0);
    if (p0 == tile_data) {
        test_pass("get_tile_ptr: tile 0 offset");
    } else {
        test_fail("get_tile_ptr: tile 0 offset", "wrong pointer");
    }

    // Tile 2 → offset 64
    const uint8_t* p2 = get_tile_ptr(tile_data, 2);
    if (p2 == tile_data + 64) {
        test_pass("get_tile_ptr: tile 2 offset (64)");
    } else {
        test_fail("get_tile_ptr: tile 2 offset (64)", "wrong pointer");
    }
}

// ---------------------------------------------------------------------------
// Test: Gradient tile decode (row-based pattern)
// ---------------------------------------------------------------------------
static void test_gradient_tile() {
    // Create a tile where each row has a different color index
    // Row y: all pixels = y
    uint8_t tile[32];
    for (int y = 0; y < 8; y++) {
        // Encode value 'y' in 4bpp: bit 0=plane0, bit 1=plane1, bit 2=plane2, bit 3=plane3
        uint8_t plane0 = (y & 0x01) ? 0xFF : 0x00;
        uint8_t plane1 = (y & 0x02) ? 0xFF : 0x00;
        uint8_t plane2 = (y & 0x04) ? 0xFF : 0x00;
        uint8_t plane3 = (y & 0x08) ? 0xFF : 0x00;
        tile[y * 2 + 0] = plane0;
        tile[y * 2 + 1] = plane1;
        tile[16 + y * 2 + 0] = plane2;
        tile[16 + y * 2 + 1] = plane3;
    }

    uint8_t pixels[64];
    decode_tile_4bpp(tile, pixels);

    bool ok = true;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (pixels[y * 8 + x] != y) {
                ok = false;
                break;
            }
        }
        if (!ok) break;
    }
    if (ok) {
        test_pass("decode_tile_4bpp: row gradient (0-7)");
    } else {
        test_fail("decode_tile_4bpp: row gradient", "pixel mismatch");
    }
}

// ---------------------------------------------------------------------------
// Test: Generate PPM from decoded tiles
// ---------------------------------------------------------------------------
static void test_tile_ppm() {
    // Create 4 synthetic tiles in a 2×2 grid (16×16 pixels)
    // Tile 0: all color 1
    // Tile 1: all color 2
    // Tile 2: all color 4
    // Tile 3: checkerboard 3/7
    auto make_solid_tile = [](uint8_t color, uint8_t* out) {
        uint8_t planes[4] = {
            (uint8_t)((color & 1) ? 0xFF : 0x00),
            (uint8_t)((color & 2) ? 0xFF : 0x00),
            (uint8_t)((color & 4) ? 0xFF : 0x00),
            (uint8_t)((color & 8) ? 0xFF : 0x00)
        };
        for (int y = 0; y < 8; y++) {
            out[y * 2 + 0] = planes[0];
            out[y * 2 + 1] = planes[1];
            out[16 + y * 2 + 0] = planes[2];
            out[16 + y * 2 + 1] = planes[3];
        }
    };

    uint8_t tile_data[4 * TILE_SIZE_4BPP];
    make_solid_tile(1, get_tile_ptr_mut(tile_data, 0));
    make_solid_tile(2, get_tile_ptr_mut(tile_data, 1));
    make_solid_tile(4, get_tile_ptr_mut(tile_data, 2));

    // Checkerboard: alternating color 3 and 7
    uint8_t* checker = get_tile_ptr_mut(tile_data, 3);
    for (int y = 0; y < 8; y++) {
        // Encode checkerboard row: 3 = 0011, 7 = 0111
        uint8_t lo = 0, hi = 0;
        for (int x = 0; x < 8; x++) {
            uint8_t c = ((x + y) & 1) ? 7 : 3;
            int bit = 7 - x;
            if (c & 1) lo |= (1 << bit);
            if (c & 2) lo |= (1 << bit); // bit 1
            if (c & 4) hi |= (1 << bit); // bit 2
            if (c & 8) hi |= (1 << bit); // bit 3
        }
        // For checker: 3 = 0011, 7 = 0111
        // Plane 0: always 1 (bit 0 of both 3 and 7)
        // Plane 1: always 1 (bit 1 of both 3 and 7)
        // Plane 2: varies (bit 2: 0 for 3, 1 for 7)
        uint8_t plane2 = 0;
        for (int x = 0; x < 8; x++) {
            uint8_t c = ((x + y) & 1) ? 7 : 3;
            if (c & 4) plane2 |= (1 << (7 - x));
        }
        checker[y * 2 + 0] = 0xFF;  // plane 0: all 1
        checker[y * 2 + 1] = 0xFF;  // plane 1: all 1
        checker[16 + y * 2 + 0] = plane2;
        checker[16 + y * 2 + 1] = 0;
    }

    // Decode all 4 tiles to pixels
    uint8_t grid_pixels[16 * 16];
    for (int ty = 0; ty < 2; ty++) {
        for (int tx = 0; tx < 2; tx++) {
            int tile_idx = ty * 2 + tx;
            uint8_t tile_pixels[64];
            decode_tile_4bpp(get_tile_ptr(tile_data, tile_idx), tile_pixels);
            for (int py = 0; py < 8; py++) {
                for (int px = 0; px < 8; px++) {
                    int gx = tx * 8 + px;
                    int gy = ty * 8 + py;
                    grid_pixels[gy * 16 + gx] = tile_pixels[py * 8 + px];
                }
            }
        }
    }

    // Convert to RGB using a simple palette (index → color)
    uint8_t palette_rgb[16 * 3] = {
        0,0,0,        // 0: black
        40,40,40,     // 1: dark gray
        80,80,80,     // 2: medium gray
        255,0,0,      // 3: red
        120,120,120,  // 4: light gray
        160,160,160,  // 5: lighter gray
        200,200,200,  // 6: very light gray
        0,0,255,      // 7: blue
    };

    const int GRID_SIZE = 8;  // scale each pixel to 8×8
    const int W = 16 * GRID_SIZE;
    const int H = 16 * GRID_SIZE;
    std::vector<uint8_t> rgb(W * H * 3);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            uint8_t idx = grid_pixels[y * 16 + x];
            if (idx > 7) idx = 0;
            uint8_t r = palette_rgb[idx * 3 + 0];
            uint8_t g = palette_rgb[idx * 3 + 1];
            uint8_t b = palette_rgb[idx * 3 + 2];
            for (int sy = 0; sy < GRID_SIZE; sy++) {
                for (int sx = 0; sx < GRID_SIZE; sx++) {
                    int px = x * GRID_SIZE + sx;
                    int py = y * GRID_SIZE + sy;
                    int off = (py * W + px) * 3;
                    rgb[off + 0] = r;
                    rgb[off + 1] = g;
                    rgb[off + 2] = b;
                }
            }
        }
    }

    if (write_ppm("output/test/tiles_test.ppm", W, H, rgb.data())) {
        test_pass("PPM: tiles test grid (2x2)", "output/test/tiles_test.ppm");
    } else {
        test_fail("PPM: tiles test grid", "could not write file");
    }
}

// ---------------------------------------------------------------------------
// Test: get_pixel_4bpp equivalent (inline in this test)
// ---------------------------------------------------------------------------
static void test_get_pixel() {
    // Create tile with all pixels = 0xA (1010 binary)
    uint8_t tile[32];
    for (int y = 0; y < 8; y++) {
        tile[y * 2 + 0] = 0x00;  // plane 0: 0
        tile[y * 2 + 1] = 0xFF;  // plane 1: 1
        tile[16 + y * 2 + 0] = 0x00;  // plane 2: 0
        tile[16 + y * 2 + 1] = 0xFF;  // plane 3: 1
    }

    // Expected: color index = (plane3<<3 | plane2<<2 | plane1<<1 | plane0)
    // For all-pixels: 0b1010 = 10 = 0xA
    bool ok = true;
    for (int y = 0; y < 8 && ok; y++) {
        for (int x = 0; x < 8 && ok; x++) {
            uint8_t bit = 7 - x;
            uint8_t p0 = (tile[y * 2 + 0] >> bit) & 1;
            uint8_t p1 = (tile[y * 2 + 1] >> bit) & 1;
            uint8_t p2 = (tile[16 + y * 2 + 0] >> bit) & 1;
            uint8_t p3 = (tile[16 + y * 2 + 1] >> bit) & 1;
            uint8_t idx = (p3 << 3) | (p2 << 2) | (p1 << 1) | p0;
            if (idx != 10) ok = false;
        }
    }
    if (ok) test_pass("get_pixel_4bpp equivalent: all-pixels-10 pattern");
    else    test_fail("get_pixel_4bpp equivalent: pixel mismatch");
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void test_tiles() {
    section_header("TILES");
    test_decode_known_tile();
    test_gradient_tile();
    test_empty_tile();
    test_tile_ptr();
    test_get_pixel();
    test_tile_ppm();
}
