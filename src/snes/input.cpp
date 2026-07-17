#include "input.h"
#include "../game/state.h"
#include <cstdio>

// External references
extern InputState joypad1;
extern InputState joypad2;
extern GlobalState shadow;

// ------------------------------------------------------------------
// read_controllers: $00:83D1-$00:83F8
//
// Joypad 1 read:
//   $00:83D1: STZ $4016          — release joypad strobe (latch)
//   $00:83D4: LDA $4218          — read joypad 1 low byte (held)
//   $00:83D7: STA $00            — temp
//   $00:83D9: LDA $4219          — read joypad 1 high byte
//   $00:83DC: STA $01            — temp
//
//   Low byte edge detection ($F2/$FA/$F6):
//   $00:83DE: LDA $00; STA $F2   — joypad1_held_low = current
//   $00:83E2: TAY                — save current
//   $00:83E3: EOR $FA            — XOR with previous
//   $00:83E5: AND $F2            — AND with current (rising edge)
//   $00:83E7: STA $F6            — joypad1_new_low
//   $00:83E9: STY $FA            — previous = current
//
//   High byte edge detection ($F0/$F8/$F4):
//   $00:83EB: LDA $01; STA $F0   — joypad1_held_high = current
//   $00:83EF: TAY
//   $00:83F0: EOR $F8            — XOR with previous
//   $00:83F2: AND $F0            — AND with current (rising edge)
//   $00:83F4: STA $F4            — joypad1_new_high
//   $00:83F6: STY $F8            — previous = current
//
// Joypad 2: $00:83F9-$00:841D (same pattern, $421A/$421B)
// ------------------------------------------------------------------
void read_controllers(void) {
    // --- Joypad 1 ---
    // Latch joypad data
    // STZ $4016 — release strobe (on real SNES, writing 0 then 1 latches)
    // For PC: we'd read from SDL/gamepad, here we use shadow values

    // Read raw joypad bytes from hardware registers
    // $4218 = joypad 1 low, $4219 = joypad 1 high
    // On real SNES: bit 0=B, 1=Y, 2=sel, 3=start, 4=up, 5=down, 6=left, 7=right
    //               bit 8=A, 9=X, 10=L, 11=R (high byte)
    uint8 hw_low = 0;   // io_read(0x4218) — would read from SNES HW
    uint8 hw_high = 0;  // io_read(0x4219)

    // Store current held state
    // $00:83DE-$00:83E9: Low byte
    uint8 current_low = hw_low;
    uint8 prev_low = shadow.joypad1_prev & 0xFF;
    uint8 new_low = (current_low ^ prev_low) & current_low;  // rising edge
    shadow.joypad1_held = (shadow.joypad1_held & 0xFF00) | current_low;
    shadow.joypad1_new = (shadow.joypad1_new & 0xFF00) | new_low;
    shadow.joypad1_prev = (shadow.joypad1_prev & 0xFF00) | current_low;

    // $00:83EB-$00:83F6: High byte
    uint8 current_high = hw_high;
    uint8 prev_high = (shadow.joypad1_prev >> 8) & 0xFF;
    uint8 new_high = (current_high ^ prev_high) & current_high;  // rising edge
    shadow.joypad1_held = (shadow.joypad1_held & 0x00FF) | (current_high << 8);
    shadow.joypad1_new = (shadow.joypad1_new & 0x00FF) | (new_high << 8);
    shadow.joypad1_prev = (shadow.joypad1_prev & 0x00FF) | (current_high << 8);

    // Also update the InputState structs for compatibility
    joypad1.held = shadow.joypad1_held;
    joypad1.newly = shadow.joypad1_new;
    joypad1.previous = shadow.joypad1_prev;

    // --- Joypad 2: $00:83F9-$00:841D ---
    // Same pattern with $421A/$421B, shadow $F1/$F3/$F5/$F7/$F9/$FB
    uint8 hw2_low = 0;   // io_read(0x421A)
    uint8 hw2_high = 0;  // io_read(0x421B)

    uint8 cur2_low = hw2_low;
    uint8 prev2_low = shadow.joypad2_prev & 0xFF;
    uint8 new2_low = (cur2_low ^ prev2_low) & cur2_low;
    shadow.joypad2_held = (shadow.joypad2_held & 0xFF00) | cur2_low;
    shadow.joypad2_new = (shadow.joypad2_new & 0xFF00) | new2_low;
    shadow.joypad2_prev = (shadow.joypad2_prev & 0xFF00) | cur2_low;

    uint8 cur2_high = hw2_high;
    uint8 prev2_high = (shadow.joypad2_prev >> 8) & 0xFF;
    uint8 new2_high = (cur2_high ^ prev2_high) & cur2_high;
    shadow.joypad2_held = (shadow.joypad2_held & 0x00FF) | (cur2_high << 8);
    shadow.joypad2_new = (shadow.joypad2_new & 0x00FF) | (new2_high << 8);
    shadow.joypad2_prev = (shadow.joypad2_prev & 0x00FF) | (cur2_high << 8);

    joypad2.held = shadow.joypad2_held;
    joypad2.newly = shadow.joypad2_new;
    joypad2.previous = shadow.joypad2_prev;
}

// ------------------------------------------------------------------
// is_button_pressed: Check if button is currently held
// ------------------------------------------------------------------
bool is_button_pressed(uint16 button_mask) {
    return (joypad1.held & button_mask) != 0;
}

// ------------------------------------------------------------------
// is_button_newly_pressed: Check if button was just pressed this frame
// ------------------------------------------------------------------
bool is_button_newly_pressed(uint16 button_mask) {
    return (joypad1.newly & button_mask) != 0;
}
