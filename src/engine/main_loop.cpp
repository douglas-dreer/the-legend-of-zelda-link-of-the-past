#include "main_loop.h"
#include "nmi.h"
#include "../snes/hardware.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../snes/input.h"
#include "../game/state.h"
#include "../game/sprites.h"
#include <cstdio>

// External references
extern PPURegisters ppu;
extern CPU65816 cpu;
extern GlobalState shadow;
extern InputState joypad1;

// ------------------------------------------------------------------
// wait_for_nmi: $00:8034-$00:8036
//   LDA $12; BEQ $8034
// Busy-loops until NMI handler sets $12 to non-zero.
// On PC, we block/sleep to avoid burning CPU.
// ------------------------------------------------------------------
void wait_for_nmi(void) {
    // $00:8034: LDA $12
    // $00:8036: BEQ $8034  (loop while $12 == 0)
    while (shadow.nmi_flag == 0) {
        // In a real PC port, this would yield or SDL_Delay(1)
        // to avoid spinning at 100% CPU. For decomp accuracy,
        // we spin, but a real port should sleep here.
    }
}

// ------------------------------------------------------------------
// enable_irq_cli: $00:8038
//   CLI — re-enable IRQs after NMI handler disabled them
// ------------------------------------------------------------------
static void enable_irq_cli(void) {
    // CLI — enable IRQ mask (allows IRQ to fire during game logic)
    cpu.flag_i = false;
}

// ------------------------------------------------------------------
// frame_debug_hotkeys: $00:803B-$00:804F
//
// Checks joypad for debug hotkeys:
//   $00:803B: LDA $F6           (joypad1 newly pressed low byte)
//   $00:803D: AND #$20          (check L button)
//   $00:803F: BEQ $8044
//   $00:8041: INC $0FD7         (increment debug frame skip counter)
//   $00:8044: LDA $F6
//   $00:8046: AND #$10          (check R button)
//   $00:8048: BNE $8051         (if R held, skip frame skip logic)
//   $00:804A: LDA $0FD7
//   $00:804D: AND #$01          (alternate frame skip)
//   $00:804F: BNE $805A         (if odd, skip sprite clear this frame)
// ------------------------------------------------------------------
static bool check_frame_skip(void) {
    // $F6 = joypad1 newly-pressed low byte
    uint8 newly_low = shadow.joypad1_new & 0xFF;

    // L button ($20 in low byte) → increment debug counter
    if (newly_low & 0x20) {
        shadow.frame_counter++;  // $0FD7 — reuse frame_counter
    }

    // R button ($10 in low byte) → force skip frame skip
    if (newly_low & 0x10) {
        return false;  // don't skip
    }

    // Check alternating frame skip
    if (shadow.frame_counter & 0x01) {
        return true;  // skip this frame's sprite clear
    }

    return false;
}

// ------------------------------------------------------------------
// game_logic_update: $00:80B5 (JSL $0080B5)
//
// The actual game logic update, called once per frame.
// In the original, this JSL goes to bank $00:$80B5 which is a
// far call trampoline. The real game logic lives in the game mode
// dispatcher at $00:8C7B.
// ------------------------------------------------------------------
void game_logic_update(void) {
    // Increment frame counter: $00:8051: INC $1A
    shadow.frame_counter++;

    // Hide unused sprites: $00:8053: JSR $841E
    hide_unused_sprites();

    // Game logic dispatch: $00:8056: JSL $0080B5
    // This calls the main game update through the game mode system
    // The game mode dispatcher at $8C7B reads $17 (game_mode)
    // and jumps through a 16-entry function pointer table
    game_mode_dispatch(shadow.game_mode);
}

// ------------------------------------------------------------------
// game_logic_sub_update: $00:85FC
//
// Secondary update routine called each frame after main logic.
// $00:85FC: LDY #$1C; TYA; ASL A; ASL A; TAX
// This indexes into a table of 7 (1C/4) update function pointers.
// ------------------------------------------------------------------
void game_logic_sub_update(void) {
    // In the original, this dispatches through a table of 7 function
    // pointers (Y=$1C = 28, /4 = 7 entries). Each entry handles a
    // different subsystem update (room transitions, effects, etc.)
    // For now, this is a placeholder that would be filled in with
    // the actual update table entries from the ROM.

    // TODO: Implement the 7-entry dispatch table from $85FC
    // Each entry updates a specific game subsystem.
}

// ------------------------------------------------------------------
// frame_logic: $00:8038-$00:805F
//
// Complete frame logic executed after NMI is acknowledged:
//   1. CLI (re-enable IRQs)
//   2. Check debug hotkeys (L/R buttons)
//   3. INC $1A (frame counter)
//   4. JSR $841E (hide unused sprites)
//   5. JSL $0080B5 (game logic update)
//   6. JSR $85FC (secondary update)
//   7. STZ $12 (clear NMI flag)
// ------------------------------------------------------------------
void frame_logic(void) {
    // $00:8038: CLI
    enable_irq_cli();

    // $00:803B-$00:804F: Debug hotkey checks
    bool skip_frame = check_frame_skip();

    // $00:8051: INC $1A — frame counter increment
    shadow.frame_counter++;

    if (!skip_frame) {
        // $00:8053: JSR $841E — hide unused sprites
        hide_unused_sprites();

        // $00:8056: JSL $0080B5 — main game logic
        game_mode_dispatch(shadow.game_mode);
    }

    // $00:805A: JSR $85FC — secondary update
    game_logic_sub_update();

    // $00:805D: STZ $12 — clear NMI flag
    shadow.nmi_flag = 0;
}

// ------------------------------------------------------------------
// main_loop_run: $00:8034-$00:805F (full loop)
//
// The main game loop:
//   wait_for_nmi() → frame_logic() → wait_for_nmi() → ...
// ------------------------------------------------------------------
void main_loop_run(void) {
    printf("Entering main loop...\n");

    while (true) {
        // $00:8034-$00:8036: Wait for NMI
        wait_for_nmi();

        // $00:8038-$00:805F: Frame logic
        frame_logic();
    }
}
