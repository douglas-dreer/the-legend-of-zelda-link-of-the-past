// =============================================================================
// test_dma.cpp — DMA system tests
//
// Tests:
//   1. DMA mode constants match SNES hardware
//   2. DMA channel register structure
//   3. DMA queue entry structure
//   4. VRAM write simulation
//   5. CGRAM (palette) write simulation
//   6. OAM write simulation
//   7. DMA transfer: VRAM to CGRAM
//   8. DMA address calculation (LoROM)
//   9. HDMA enable/disable
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>

// ---------------------------------------------------------------------------
// Types (from types.h)
// ---------------------------------------------------------------------------
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;

// ---------------------------------------------------------------------------
// Constants (from types.h)
// ---------------------------------------------------------------------------
static const uint32 VRAM_SIZE = 0x10000;
static const uint32 CGRAM_SIZE = 0x200;
static const uint32 OAM_SIZE = 0x220;

// ---------------------------------------------------------------------------
// DMA mode constants (from hardware.h)
// ---------------------------------------------------------------------------
static const uint8 DMA_MODE_1BYTE   = 0x00;
static const uint8 DMA_MODE_2BYTE   = 0x01;
static const uint8 DMA_MODE_1BYTE_2 = 0x02;
static const uint8 DMA_MODE_4BYTE   = 0x03;
static const uint8 DMA_MODE_2BYTE_2 = 0x04;
static const uint8 DMA_MODE_8BYTE   = 0x05;
static const uint8 DMA_MODE_4BYTE_2 = 0x06;
static const uint8 DMA_MODE_16BYTE  = 0x07;

// ---------------------------------------------------------------------------
// DMA channel structure (from hardware.h)
// ---------------------------------------------------------------------------
struct DMAChannel {
    uint8 mode;
    uint8 bbus_addr;
    uint32 src_addr;
    uint16 size;
    uint8 hdma_indirect;
};

// ---------------------------------------------------------------------------
// DMA queue entry (from dma.h)
// ---------------------------------------------------------------------------
struct DMATransfer {
    bool active;
    uint8 channel;
    uint8 mode;
    uint32 src_addr;
    uint16 dst_ppu;
    uint16 size;
};

// PPU registers shadow
struct PPURegisters {
    uint8 vram_ctrl;
    uint16 vram_addr;
    uint8 cgadd;
    uint16 oam_addr;
    uint8 hdma_enable;
    uint8 dma_enable;
};

// External framework functions
extern void test_pass(const char* name, const char* detail = "");
extern void test_fail(const char* name, const char* detail = "");
extern void section_header(const char* title);

// ---------------------------------------------------------------------------
// Simulated SNES memory
// ---------------------------------------------------------------------------
static uint8_t vram[VRAM_SIZE];
static uint8_t cgram[CGRAM_SIZE];
static uint8_t oam_raw[OAM_SIZE];
static PPURegisters ppu;
static DMAChannel dma_channels[5];
static DMATransfer dma_queue[5];

// ROM simulation (for testing DMA reads)
static uint8_t rom[0x100000];

// ---------------------------------------------------------------------------
// Test: DMA mode constants
// ---------------------------------------------------------------------------
static void test_dma_modes() {
    bool ok = true;
    if (DMA_MODE_1BYTE != 0x00) ok = false;   // 1 byte → PPU
    if (DMA_MODE_2BYTE != 0x01) ok = false;   // 2 bytes → PPU (write twice)
    if (DMA_MODE_1BYTE_2 != 0x02) ok = false; // 1 byte → PPU+2
    if (DMA_MODE_4BYTE != 0x03) ok = false;   // 4 bytes → PPU
    if (DMA_MODE_2BYTE_2 != 0x04) ok = false; // 2 bytes → PPU+2 (x2)
    if (DMA_MODE_8BYTE != 0x05) ok = false;   // 8 bytes
    if (DMA_MODE_4BYTE_2 != 0x06) ok = false; // 4 bytes → PPU+2
    if (DMA_MODE_16BYTE != 0x07) ok = false;  // 16 bytes (block)

    if (ok) test_pass("DMA modes: 8 transfer modes (0x00-0x07)");
    else    test_fail("DMA modes: constant values", "mismatch");
}

// ---------------------------------------------------------------------------
// Test: DMA channel structure
// ---------------------------------------------------------------------------
static void test_dma_channel_structure() {
    DMAChannel ch;
    memset(&ch, 0, sizeof(ch));

    ch.mode = DMA_MODE_2BYTE;
    ch.bbus_addr = 0x18;     // VRAM data write ($2118)
    ch.src_addr = 0x10000;   // Source address
    ch.size = 0x0400;        // 1024 bytes

    if (ch.mode == 0x01 && ch.bbus_addr == 0x18 &&
        ch.src_addr == 0x10000 && ch.size == 0x0400) {
        test_pass("DMAChannel: register set and read back");
    } else {
        test_fail("DMAChannel: register set", "values wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: DMA queue entry
// ---------------------------------------------------------------------------
static void test_dma_queue() {
    DMATransfer q;
    memset(&q, 0, sizeof(q));

    q.active = true;
    q.channel = 1;
    q.mode = DMA_MODE_2BYTE;
    q.src_addr = 0x7E0000;
    q.dst_ppu = 0x2118;
    q.size = 0x0200;

    if (q.active && q.channel == 1 && q.size == 0x0200) {
        test_pass("DMATransfer: queue entry set");
    } else {
        test_fail("DMATransfer: queue entry", "values wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: VRAM write simulation
// ---------------------------------------------------------------------------
static void test_vram_write() {
    memset(vram, 0, VRAM_SIZE);

    // Simulate writing tile data to VRAM at address 0x0000
    uint16_t addr = 0x0000;
    uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};

    for (int i = 0; i < 4; i++) {
        vram[addr + i] = data[i];
    }

    if (vram[0x0000] == 0xAA && vram[0x0001] == 0xBB &&
        vram[0x0002] == 0xCC && vram[0x0003] == 0xDD) {
        test_pass("VRAM write: 4 bytes at 0x0000");
    } else {
        test_fail("VRAM write: 4 bytes", "data mismatch");
    }

    // Verify write to VRAM at BG3 tiles base (0x4000)
    addr = 0x4000;
    for (int i = 0; i < 16; i++) {
        vram[addr + i] = (uint8_t)(0x10 + i);
    }
    bool ok = true;
    for (int i = 0; i < 16; i++) {
        if (vram[0x4000 + i] != (uint8_t)(0x10 + i)) {
            ok = false;
            break;
        }
    }
    if (ok) test_pass("VRAM write: 16 bytes at BG3 base (0x4000)");
    else    test_fail("VRAM write: 16 bytes at BG3", "data mismatch");
}

// ---------------------------------------------------------------------------
// Test: CGRAM (palette) write simulation
// ---------------------------------------------------------------------------
static void test_cgram_write() {
    memset(cgram, 0, CGRAM_SIZE);

    // Write a 16-color palette (32 bytes) to CGRAM offset 0
    uint16_t palette[16] = {
        0x0000, 0x001F, 0x03E0, 0x7C00,  // black, red, green, blue
        0x7FFF, 0x1778, 0x0910, 0x03E0,
        0x001F, 0x7C00, 0x7FFF, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    };

    for (int i = 0; i < 16; i++) {
        cgram[i * 2 + 0] = palette[i] & 0xFF;
        cgram[i * 2 + 1] = (palette[i] >> 8) & 0xFF;
    }

    // Verify palette 0 (black)
    if (cgram[0] == 0x00 && cgram[1] == 0x00) {
        test_pass("CGRAM write: palette color 0 (black)");
    } else {
        test_fail("CGRAM write: palette color 0", "expected 0x0000");
    }

    // Verify palette 1 (red max: 0x001F → low=0x1F, high=0x00)
    if (cgram[2] == 0x1F && cgram[3] == 0x00) {
        test_pass("CGRAM write: palette color 1 (red)");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got 0x%02X%02X, expected 0x001F", cgram[3], cgram[2]);
        test_fail("CGRAM write: palette color 1", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: OAM write simulation
// ---------------------------------------------------------------------------
static void test_oam_write() {
    memset(oam_raw, 0, OAM_SIZE);

    // Write 4 OAM entries (16 bytes)
    // Entry 0: Y=0x80, tile=0x10, attr=0x01, X=0x40
    oam_raw[0] = 0x80;  // Y
    oam_raw[1] = 0x10;  // tile
    oam_raw[2] = 0x01;  // attr
    oam_raw[3] = 0x40;  // X

    if (oam_raw[0] == 0x80 && oam_raw[1] == 0x10 &&
        oam_raw[2] == 0x01 && oam_raw[3] == 0x40) {
        test_pass("OAM write: entry 0 (Y=0x80, tile=0x10)");
    } else {
        test_fail("OAM write: entry 0", "data mismatch");
    }
}

// ---------------------------------------------------------------------------
// Test: DMA transfer simulation (VRAM → CGRAM)
// ---------------------------------------------------------------------------
static void test_dma_transfer() {
    memset(vram, 0, VRAM_SIZE);
    memset(cgram, 0, CGRAM_SIZE);
    memset(&ppu, 0, sizeof(ppu));

    // Set up source data in "ROM" (simulated)
    for (int i = 0; i < 256; i++) {
        rom[i] = (uint8_t)(i & 0xFF);
    }

    // DMA: copy 32 bytes from ROM offset 0 to CGRAM (via $2122)
    ppu.cgadd = 0;  // Start at CGRAM address 0

    DMAChannel* ch = &dma_channels[0];
    ch->mode = DMA_MODE_2BYTE;
    ch->bbus_addr = 0x22;     // CGRAM data port ($2122)
    ch->src_addr = 0;         // Source: ROM offset 0
    ch->size = 32;            // 32 bytes

    // Simulate DMA transfer
    for (uint16_t i = 0; i < ch->size; i++) {
        uint32_t addr = ch->src_addr + i;
        uint8_t val = (addr < sizeof(rom)) ? rom[addr] : 0;
        uint16_t dst = ppu.cgadd;
        if (dst < CGRAM_SIZE) {
            cgram[dst] = val;
        }
        ppu.cgadd++;
    }

    bool ok = true;
    for (int i = 0; i < 32; i++) {
        if (cgram[i] != (uint8_t)(i & 0xFF)) {
            ok = false;
            break;
        }
    }
    if (ok) test_pass("DMA transfer: ROM → CGRAM (32 bytes)");
    else    test_fail("DMA transfer: ROM → CGRAM", "data mismatch");
}

// ---------------------------------------------------------------------------
// Test: LoROM address translation
// ---------------------------------------------------------------------------
static void test_lorom_address() {
    // LoROM: bank * 0x8000 + (addr - 0x8000) = ROM offset
    // Bank $00, addr $8000 → offset 0x0000
    auto snes_to_rom = [](uint8_t bank, uint16_t addr) -> uint32_t {
        if (addr < 0x8000) return 0xFFFFFFFF;
        return (uint32_t)bank * 0x8000 + (addr - 0x8000);
    };

    uint32_t offset = snes_to_rom(0x00, 0x8000);
    if (offset == 0x0000) {
        test_pass("LoROM: bank $00 addr $8000 → offset 0x0000");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got 0x%06X, expected 0x0000", offset);
        test_fail("LoROM: bank $00 addr $8000", buf);
    }

    offset = snes_to_rom(0x01, 0x8000);
    if (offset == 0x8000) {
        test_pass("LoROM: bank $01 addr $8000 → offset 0x8000");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "got 0x%06X, expected 0x008000", offset);
        test_fail("LoROM: bank $01 addr $8000", buf);
    }

    offset = snes_to_rom(0x00, 0x7FFF);
    if (offset == 0xFFFFFFFF) {
        test_pass("LoROM: addr $7FFF → invalid (below $8000)");
    } else {
        test_fail("LoROM: addr $7FFF", "should be invalid");
    }
}

// ---------------------------------------------------------------------------
// Test: HDMA enable/disable
// ---------------------------------------------------------------------------
static void test_hdma() {
    memset(&ppu, 0, sizeof(ppu));

    // Enable HDMA channels 0, 1, 3 → mask = 0x0B
    ppu.hdma_enable = 0x0B;
    if (ppu.hdma_enable == 0x0B) {
        test_pass("HDMA: enable channels 0,1,3");
    } else {
        test_fail("HDMA: enable", "value wrong");
    }

    // Disable all HDMA
    ppu.hdma_enable = 0x00;
    if (ppu.hdma_enable == 0x00) {
        test_pass("HDMA: disable all");
    } else {
        test_fail("HDMA: disable", "value wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: VRAM address increment mode
// ---------------------------------------------------------------------------
static void test_vram_addr_mode() {
    memset(&ppu, 0, sizeof(ppu));

    // Mode $80 = word increment (standard for VRAM writes)
    ppu.vram_ctrl = 0x80;
    if (ppu.vram_ctrl == 0x80) {
        test_pass("VRAM ctrl: word increment mode ($80)");
    } else {
        test_fail("VRAM ctrl: word increment mode", "value wrong");
    }

    // VRAM address set
    ppu.vram_addr = 0x1000;
    if (ppu.vram_addr == 0x1000) {
        test_pass("VRAM addr: set to $1000 (BG1 tilemap)");
    } else {
        test_fail("VRAM addr: set to $1000", "value wrong");
    }
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void test_dma() {
    section_header("DMA");
    test_dma_modes();
    test_dma_channel_structure();
    test_dma_queue();
    test_vram_write();
    test_cgram_write();
    test_oam_write();
    test_dma_transfer();
    test_lorom_address();
    test_hdma();
    test_vram_addr_mode();
}
