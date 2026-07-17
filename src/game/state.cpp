#include "state.h"
#include "../snes/dma.h"
#include "../snes/ppu.h"
#include "../game/sprites.h"
#include "../game/room.h"
#include <cstdio>

// External references
extern GlobalState shadow;
extern StateMachine state_machine;

// ------------------------------------------------------------------
// init_game_state: Initialize state machine and shadow RAM
// ------------------------------------------------------------------
void init_game_state(void) {
    state_machine.current_state = GameState::BOOT;
    state_machine.next_state = GameState::BOOT;
    state_machine.sub_state = 0;
    state_machine.transition_counter = 0;

    shadow.game_mode = GAME_MODE_INIT;
}

// ------------------------------------------------------------------
// Game Mode Function Table: $00:8C7B
//
// JMP ($8C7E,X) — indirect indexed jump through 16 entry table.
// $17 (game_mode) is loaded, ASL (×2), used as index X.
// $17 is then cleared (STZ $17).
//
// Table at $00:8C7E (16 × 2-byte pointers):
//   0x00: $8CE3 → game_mode_init (mode 0)
//   0x01: $8CB0 → game_mode_secondary (mode 1)
//   0x02: $8CE4 → game_mode_dma_vram (mode 2)
//   0x03: $8D13 → game_mode_tilemap (mode 3)
//   0x04: $8D62 → game_mode_full_dma (mode 4)
//   0x05: $8D7C → game_mode_vram_update (mode 5)
//   0x06: $8D95 → game_mode_idle (mode 6)
//   0x07: $8E4B → game_mode_vram_stream (mode 7)
//   0x08: $8E57 → game_mode_bg_update (mode 8)
//   0x09: $8EA9 → game_mode_sprite_update (mode 9)
//   0x0A: $8E72 → game_mode_oam_update (mode 0xA)
//   0x0B: $8F16 → game_mode_scroll_update (mode 0xB)
//   0x0C: $8EE7 → game_mode_dma_partial (mode 0xC)
//   0x0D: $8F38 → game_mode_dma_alt (mode 0xD)
//   0x0E: $8F45 → game_mode_vram_write (mode 0xE)
//   0x0F: $8F4B → game_mode_multi_dma (mode 0xF)
// ------------------------------------------------------------------
void game_mode_dispatch(uint8 mode) {
    // $00:8C75: LDA $17 — load game mode
    // $00:8C77: ASL A — ×2 for word table
    // $00:8C78: TAX — index
    // $00:8C79: STZ $17 — clear game mode (one-shot)
    shadow.game_mode = 0;  // STZ $17 — clear after reading

    switch (mode) {
        case GAME_MODE_INIT:            // $00:8CE3
            game_mode_init();
            break;
        case GAME_MODE_SECONDARY:       // $00:8CB0
            game_mode_secondary();
            break;
        case GAME_MODE_DMA_VRAM:        // $00:8CE4
            game_mode_dma_vram();
            break;
        case GAME_MODE_TILEMAP:         // $00:8D13
            game_mode_tilemap();
            break;
        case GAME_MODE_FULL_DMA:        // $00:8D62
            game_mode_full_dma();
            break;
        case GAME_MODE_VRAM_UPDATE:     // $00:8D7C
            game_mode_vram_update();
            break;
        case GAME_MODE_IDLE:            // $00:8D95
            game_mode_idle();
            break;
        case GAME_MODE_VRAM_STREAM:     // $00:8E4B
            game_mode_vram_stream();
            break;
        case GAME_MODE_BG_UPDATE:       // $00:8E57
            game_mode_bg_update();
            break;
        case GAME_MODE_SPRITE_UPDATE:   // $00:8EA9
            game_mode_sprite_update();
            break;
        case GAME_MODE_OAM_UPDATE:      // $00:8E72
            game_mode_oam_update();
            break;
        case GAME_MODE_SCROLL_UPDATE:   // $00:8F16
            game_mode_scroll_update();
            break;
        case GAME_MODE_DMA_PARTIAL:     // $00:8EE7
            game_mode_dma_partial();
            break;
        case GAME_MODE_DMA_ALT:         // $00:8F38
            game_mode_dma_alt();
            break;
        case GAME_MODE_VRAM_WRITE:      // $00:8F45
            game_mode_vram_write();
            break;
        case GAME_MODE_MULTI_DMA:       // $00:8F4B
            game_mode_multi_dma();
            break;
        default:
            break;
    }
}

// ------------------------------------------------------------------
// Game Mode Implementations
// Each mode corresponds to a routine in the $8C7E dispatch table.
// ------------------------------------------------------------------

// Mode 0 ($00:8CE3): Init — configure VRAM for initial DMA
void game_mode_init(void) {
    // $00:8CE3: REP #$10 — 16-bit XY
    // Set VRAM address increment mode, configure DMA channel 0
    // Transfer initial tile/character data to VRAM
    // Clear $0710 flag when done
    printf("Game mode: INIT\n");
}

// Mode 1 ($00:8CB0): Secondary — run secondary game logic
void game_mode_secondary(void) {
    // $00:8CB0-$00:8CDE: Complex secondary mode handler
    // Handles sub-modes within the secondary state
    // Reads $1100-$1103 for VRAM transfer descriptors
    printf("Game mode: SECONDARY\n");
}

// Mode 2 ($00:8CE4): DMA VRAM — transfer data to VRAM
void game_mode_dma_vram(void) {
    // $00:8CE4-$00:8D0D:
    // REP #$10; LDA #$80; STA $2115
    // LDX #$01; CLC; STX $4300 (mode 1)
    // Read transfer descriptor from $1100
    // Configure source ($7F:xxxx), set VRAM dest
    // Fire DMA; clear $0710
    printf("Game mode: DMA_VRAM\n");
}

// Mode 3 ($00:8D13): Tilemap — transfer tilemap data
void game_mode_tilemap(void) {
    // $00:8D13-$00:8D5C:
    // Multi-pass tilemap DMA with loop
    // Reads $1100-$1103 for tilemap block descriptors
    // Each block has: VRAM addr, source, size
    // Loops until $1103 bit 7 set (last block flag)
    printf("Game mode: TILEMAP\n");
}

// Mode 4 ($00:8D62): Full DMA — comprehensive VRAM update
void game_mode_full_dma(void) {
    // $00:8D62-$00:8D7A:
    // Sets up full screen DMA: tile data + tilemap
    // Source bank $7F, word-increment mode
    // Fires multiple DMA passes for complete screen update
    printf("Game mode: FULL_DMA\n");
}

// Mode 5 ($00:8D7C): VRAM Update — partial screen DMA
void game_mode_vram_update(void) {
    // $00:8D7C-$00:8D95:
    // Similar to full DMA but for partial updates
    // Uses offset-based addressing into $7F bank data
    printf("Game mode: VRAM_UPDATE\n");
}

// Mode 6 ($00:8D95): Idle — no VRAM transfer needed
void game_mode_idle(void) {
    // $00:8D95-$00:8E08:
    // Complex idle mode with conditional DMA
    // Sets up channel 0 with $7E bank source
    // Performs 4-pass tile data DMA (4 × $10 bytes per tile row)
    // Loops through tile columns, fires DMA each iteration
    printf("Game mode: IDLE\n");
}

// Mode 7 ($00:8E4B): VRAM Stream — streaming DMA
void game_mode_vram_stream(void) {
    // $00:8E4B: RTS
    // Empty — reserved for streaming VRAM updates
    printf("Game mode: VRAM_STREAM\n");
}

// Mode 8 ($00:8E57): BG Update — background layer DMA
void game_mode_bg_update(void) {
    // $00:8E57-$00:8EA6:
    // Configures DMA for BG layer updates
    // Source bank $0A, multi-pass for 8 BG tile groups
    // Each group: $20 bytes × 7 rows = $E0 bytes
    // Fires DMA after each group
    printf("Game mode: BG_UPDATE\n");
}

// Mode 9 ($00:8EA9): Sprite Update — OBJ layer DMA
void game_mode_sprite_update(void) {
    // $00:8EA9-$00:8EE4:
    // LDA #$80; STA $2115 — word select
    // REP #$10; LDY #$0000; STY $2116 — VRAM addr
    // Configure channel 1 for sprite tile data
    // Fire DMA twice for two VRAM regions
    // This uploads sprite character data to VRAM $6000+
    printf("Game mode: SPRITE_UPDATE\n");
}

// Mode A ($00:8E72): OAM Update — update OAM buffer
void game_mode_oam_update(void) {
    // $00:8E72-$00:8E8A:
    // Writes tile data to specific VRAM offsets
    // Uses calculated address: (accumulator + $80) for VRAM dest
    // DMA size = $27 × 8 = $138 bytes
    // Fires channel 0 with mode 0 (1 byte)
    printf("Game mode: OAM_UPDATE\n");
}

// Mode B ($00:8F16): Scroll Update — update scroll registers
void game_mode_scroll_update(void) {
    // $00:8F16-$00:8F3D:
    // REP #$20; STA $2116 — set VRAM addr from A
    // LDA #$80; STA $2115 — word select
    // LDX #$01; STX $4300 — mode 1
    // DMA from $7F bank, size from descriptor
    // Fire DMA; clear $0710
    printf("Game mode: SCROLL_UPDATE\n");
}

// Mode C ($00:8EE7): DMA Partial — selective channel DMA
void game_mode_dma_partial(void) {
    // $00:8EE7-$00:8F15:
    // REP #$20; LDA #$00; STA $2116 — VRAM addr 0
    // Configure channel 0 with $7E bank, word-increment
    // Fire DMA; clear $0710
    printf("Game mode: DMA_PARTIAL\n");
}

// Mode D ($00:8F38): DMA Alt — alternative DMA configuration
void game_mode_dma_alt(void) {
    // $00:8F38-$00:8F4B:
    // Similar to DMA Partial with different source configuration
    printf("Game mode: DMA_ALT\n");
}

// Mode E ($00:8F45): VRAM Write — direct VRAM write
void game_mode_vram_write(void) {
    // $00:8F45-$00:8F4B:
    // Simple VRAM write of accumulated data
    printf("Game mode: VRAM_WRITE\n");
}

// Mode F ($00:8F4B): Multi DMA — multi-channel DMA
void game_mode_multi_dma(void) {
    // $00:8F4B-$00:8F6A:
    // Configure multiple DMA channels simultaneously
    // Fire all at once for maximum throughput
    printf("Game mode: MULTI_DMA\n");
}

// ------------------------------------------------------------------
// run_secondary_mode: Execute secondary game mode logic
// ------------------------------------------------------------------
void run_secondary_mode(void) {
    // Handle secondary game modes (sub-states within main states)
}

// ------------------------------------------------------------------
// Transition Functions
// ------------------------------------------------------------------
void start_transition(GameState next_state) {
    state_machine.next_state = next_state;
    state_machine.transition_counter = 0;
}

void update_transition(void) {
    if (state_machine.current_state != state_machine.next_state) {
        state_machine.transition_counter++;

        // Transition logic here
        // When complete:
        // state_machine.current_state = state_machine.next_state;
    }
}
