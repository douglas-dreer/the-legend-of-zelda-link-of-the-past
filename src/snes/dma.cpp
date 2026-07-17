#include "dma.h"
#include "hardware.h"
#include "../game/state.h"
#include <cstring>
#include <cstdio>

// External references
extern PPURegisters ppu;
extern DMAChannel dma_channels[5];
extern DMATransfer dma_queue[DMA_QUEUE_SIZE];
extern GlobalState shadow;
extern uint8 vram[];
extern uint8 cgram[];
extern uint8 oam_buffer_raw[];  // Raw OAM buffer ($0800-$09FF)
extern uint8 rom[];

// DMA source addresses from shadow RAM $0AC0-$0AF8
// $0AC0-$0AC3: channel 0 source (tilemap 1)
// $0AC4-$0AC7: channel 1 source (tilemap 2)
// $0AC8-$0ACB: channel 2 source (tilemap 3)
// $0ACC-$0ACF: channel 3 source (tilemap alt 1)
// $0AD0-$0AD3: channel 4 source (tilemap alt 2)

// ------------------------------------------------------------------
// configure_dma_channel: Helper to set up a DMA channel register set
// $43x0: mode, $43x1: bbus addr, $43x2-$43x3: src low, $43x4: src bank
// $43x5-$43x6: transfer size
// ------------------------------------------------------------------
static void configure_dma_channel(uint8 ch, uint8 mode, uint8 bbus,
                                   uint32 src, uint16 size) {
    DMAChannel* channel = &dma_channels[ch];
    channel->mode = mode;
    channel->bbus_addr = bbus;
    channel->src_addr = src;
    channel->size = size;
}

// ------------------------------------------------------------------
// fire_dma: Write channel mask to $420B to trigger DMA transfers
// ------------------------------------------------------------------
static void fire_dma(uint8 mask) {
    ppu.dma_enable = mask;

    // Simulate the DMA transfer by copying data in PC memory
    for (int ch = 0; ch < 5; ch++) {
        if (!(mask & (1 << ch))) continue;

        DMAChannel* channel = &dma_channels[ch];
        if (channel->size == 0) continue;

        // Copy from source (ROM/RAM) to destination (PPU register)
        // Mode 1 ($01): 2 bytes → PPU (write twice)
        // This is the standard VRAM transfer mode
        if (channel->bbus_addr == 0x18 || channel->bbus_addr == 0x19) {
            // VRAM write: bytes written as word pairs
            uint16 dst = ppu.vram_addr;
            for (uint16 i = 0; i < channel->size; i++) {
                // Read source byte
                uint8 byte_val = 0;
                uint32 addr = channel->src_addr + i;
                if (addr < ROM_SIZE) {
                    byte_val = rom[addr];
                }
                // Write to VRAM (byte-addressed)
                if (dst < VRAM_SIZE) {
                    vram[dst] = byte_val;
                }
                dst++;
            }
        } else if (channel->bbus_addr == 0x04) {
            // OAM data write: copy to OAM buffer
            uint32 src_addr = channel->src_addr;
            for (uint16 i = 0; i < channel->size && i < OAM_SIZE; i++) {
                uint8 byte_val = 0;
                if (src_addr + i < ROM_SIZE) {
                    byte_val = rom[src_addr + i];
                }
                oam_buffer_raw[i] = byte_val;
            }
        } else if (channel->bbus_addr == 0x22) {
            // CGRAM data write: copy to palette buffer
            uint32 src_addr = channel->src_addr;
            for (uint16 i = 0; i < channel->size && i < CGRAM_SIZE; i++) {
                uint8 byte_val = 0;
                if (src_addr + i < ROM_SIZE) {
                    byte_val = rom[src_addr + i];
                }
                cgram[i] = byte_val;
            }
        }
    }
}

// ------------------------------------------------------------------
// dma_transfer: $00:89E0 entry point
// Configures and fires a single DMA transfer
// ------------------------------------------------------------------
void dma_transfer(uint32 src, uint16 dst_ppu, uint16 size,
                  uint8 channel, uint8 mode) {
    if (channel >= 5) return;

    DMAChannel* ch = &dma_channels[channel];
    ch->mode = mode;
    ch->bbus_addr = 0x18;      // Default: VRAM data write ($2118)
    ch->src_addr = src;
    ch->size = size;

    // Fire DMA for this single channel
    fire_dma(1 << channel);
}

// ------------------------------------------------------------------
// dma_engine_run: $00:89E0-$00:8B67
//
// Full DMA engine called once per frame during NMI.
// Executes in this order:
//   1. VRAM setup ($2115 = #$80, word select)
//   2. Check $0710 for special DMA mode
//   3. Multi-channel VRAM DMA (channels 0-2, mode 1, bank $10)
//   4. Bank $7E VRAM DMA (channels 0-4, tilemap data)
//   5. Additional VRAM DMA passes
//   6. Palette DMA ($15 check, channel 1)
//   7. OAM DMA ($2102 = 0, channel 0, 512 bytes)
//   8. Extended VRAM DMA ($19, $18 checks)
//   9. Game mode DMA ($17 dispatch)
// ------------------------------------------------------------------
void dma_engine_run(void) {
    // $00:89E0: REP #$10 — 16-bit XY
    // $00:89E2: LDA #$80; STA $2115 — VRAM address increment = word
    ppu.vram_ctrl = 0x80;

    // $00:89E7: LDA $0710; BEQ $89EF — check special DMA flag
    if (shadow.dma_sources[0] == 0) {
        // --- Normal VRAM DMA: $00:89EF-$00:8A3B ---
        // Configure channels 0-2 with mode 1 (2 bytes → PPU)
        // Bank $10 source addresses, tilemap data
        {
            // $00:89EF: LDX #$01; CLC
            // $00:89F2-$00:89FE: Set channels 0-4 mode = $01 (2-byte write)
            for (int ch = 0; ch < 5; ch++) {
                dma_channels[ch].mode = 0x01;  // 2 bytes → PPU
            }

            // $00:8A01-$00:8A09: Set A1B ($43x4) = $10 for channels 0-2
            dma_channels[0].src_addr = (dma_channels[0].src_addr & 0xFFFF) | 0x100000;
            dma_channels[1].src_addr = (dma_channels[1].src_addr & 0xFFFF) | 0x100000;
            dma_channels[2].src_addr = (dma_channels[2].src_addr & 0xFFFF) | 0x100000;

            // $00:8A33: LDA #$07; STA $420B — fire channels 0-2
            fire_dma(0x07);
        }

        // --- Bank $7E VRAM DMA: $00:8A5C-$00:8A9D ---
        {
            for (int ch = 0; ch < 5; ch++) {
                dma_channels[ch].src_addr = (dma_channels[ch].src_addr & 0xFFFF) | 0x7E0000;
            }
            // $00:8A9D: LDA #$1F; STA $420B — fire all 5 channels
            fire_dma(0x1F);
        }

        // --- Additional VRAM DMA passes: $00:8AA2-$00:8AF8 ---
        fire_dma(0x07);

        // --- Third VRAM DMA pass: $00:8B01-$00:8B22 ---
        fire_dma(0x07);
    }

    // --- Palette DMA: $00:8B87-$00:8BA5 ---
    // --- Palette DMA: $00:8B87-$00:8BA5 ---
    // $00:8B87: LDA $15; BEQ $8BAA — check palette dirty flag
    if (shadow.palette_dirty != 0) {
        // $00:8B8B: STZ $2121 — CGRAM address = 0
        ppu.cgadd = 0;

        // $00:8B8E: LDY #$00
        // $00:8B90: JSL $43108C — configure channel 1 for palette DMA
        // Mode: $01 (2 bytes), B-bus: $22 (CGRAM data)
        configure_dma_channel(1, 0x01, 0x22,
                             0x7E0000,  // source from WRAM
                             0x0200);   // 512 bytes = 256 colors

        // $00:8B9A: LDA #$7E; STA $4314 — source bank $7E
        dma_channels[1].src_addr = 0x7E0000;

        // $00:8BA5: LDA #$02; STA $420B — fire channel 1
        fire_dma(0x02);

        // Clear palette dirty flag
        shadow.palette_dirty = 0;
    }

    // --- OAM DMA: $00:8BAA-$00:8BCA ---
    // --- OAM DMA: $00:8BAA-$00:8BCA ---
    // $00:8BAA: REP #$20; SEP #$10
    // $00:8BAE: STZ $15 — clear palette dirty (already done)
    // $00:8BB0: STZ $2102 — OAM address = 0
    // $00:8BB3: LDA #$00; TSB $8D — configure channel 0
    // $00:8BB9: LDA #$00; STA $4302 — source low = $0000
    // $00:8BBF: STZ $4304 — source bank = $00
    // $00:8BC2: LDA #$20; STA $4305 — size = $0200 (512 bytes)
    // $00:8BC8: LDY #$01; STY $420B — fire channel 0

    // OAM buffer is at $0800-$09FF in WRAM
    configure_dma_channel(0, 0x00, 0x04,   // mode 0, OAM data port
                         0x000800,          // source: $0800
                         0x0200);           // 512 bytes
    fire_dma(0x01);

    // --- Extended VRAM DMA: $00:8BCD-$00:8CE3 ---
    // $14, $19, $18 checks — additional tilemap/DMA operations
    // These are handled by the game mode dispatcher and are
    // specific to the current room/scene being loaded.
}

// ------------------------------------------------------------------
// dma_queue_transfer: Queue a DMA transfer for later execution
// ------------------------------------------------------------------
void dma_queue_transfer(uint8 channel, uint8 mode,
                        uint32 src, uint16 dst, uint16 size) {
    if (channel < DMA_QUEUE_SIZE) {
        dma_queue[channel].active = true;
        dma_queue[channel].channel = channel;
        dma_queue[channel].mode = mode;
        dma_queue[channel].src_addr = src;
        dma_queue[channel].dst_ppu = dst;
        dma_queue[channel].size = size;
    }
}

// ------------------------------------------------------------------
// dma_room_to_vram: $00:8B67-$00:8B82
// DMA for room tilemap data when $16 (bg_mode_dirty) is set
// ------------------------------------------------------------------
void dma_room_to_vram(void) {
    if (shadow.bg_mode_dirty == 0) return;

    // $00:8B67: LDA $16; BEQ $8B87
    // $00:8B6B: LDX $0219 — VRAM address from shadow
    // $00:8B6E: STX $2116 — set VRAM write address
    // $00:8B71: LDX #$0000; STX $4302 — source from $7E:0000
    // $00:8B77: LDA #$7E; STA $4304 — bank $7E
    // $00:8B7C: LDX #$014A; STX $4305 — size = $014A bytes
    // $00:8B82: LDA #$01; STA $420B — fire channel 0

    configure_dma_channel(0, 0x01, 0x18,   // mode 1, VRAM data
                         0x7E0000,          // source from WRAM
                         0x014A);           // 330 bytes
    fire_dma(0x01);

    shadow.bg_mode_dirty = 0;
}

// ------------------------------------------------------------------
// hdma_setup: Configure an HDMA channel
// ------------------------------------------------------------------
void hdma_setup(uint8 channel, uint8 mode, uint8 bbus_addr,
                const uint8* table_addr, uint8 indirect_mode) {
    if (channel >= 5) return;

    // Configure HDMA channel registers
    dma_channels[channel].mode = mode;
    dma_channels[channel].bbus_addr = bbus_addr;
    dma_channels[channel].hdma_indirect = indirect_mode;
}

// ------------------------------------------------------------------
// hdma_enable: Enable HDMA channels ($420C)
// ------------------------------------------------------------------
void hdma_enable(uint8 channels) {
    shadow.hdma_enable = channels;
}

// ------------------------------------------------------------------
// hdma_disable: Disable all HDMA ($420C = 0)
// ------------------------------------------------------------------
void hdma_disable(void) {
    shadow.hdma_enable = 0;
}
