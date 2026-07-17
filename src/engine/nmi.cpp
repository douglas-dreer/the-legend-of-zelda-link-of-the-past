#include "nmi.h"
#include "../snes/hardware.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../snes/input.h"
#include "../game/state.h"
#include "../game/sprites.h"
#include "../audio/apu.h"
#include <cstdio>

// External references
extern PPURegisters ppu;
extern CPU65816 cpu;
extern GlobalState shadow;
extern InputState joypad1;
extern InputState joypad2;
extern OAMBuffer oam_buffer;

// ------------------------------------------------------------------
// apu_port_handshake: $00:80DC-$00:812D
//
// Port 0 ($2140) handshake:
//   $00:80DC: LDA $012C         (apu_cmd_to_send)
//   $00:80DF: BNE $80EE        (if non-zero, send)
//   $00:80E1: LDA $2140        (read APU response)
//   $00:80E4: CMP $0133        (compare with last received)
//   $00:80E7: BNE $8103        (if different, APU responded)
//   $00:80E9: STZ $2140        (clear port, no new data)
//   $00:80EC: BRA $8103
//   $00:80EE: CMP $0133        (compare with last sent)
//   $00:80F1: BEQ $8103        (same = not yet acked, skip)
//   $00:80F3: STA $2140        (write to port)
//   $00:80F6: STA $0133        (save last sent)
//   $00:80F9: CMP #$F2         (check if special command)
//   $00:80FB: BCS $8100        (if >= $F2, skip save)
//   $00:80FD: STA $0130        (save as special_cmd)
//   $00:8100: STZ $012C        (clear pending)
//
// Port 1 ($2141) handshake (similar pattern):
//   $012D → $2141, compare $0131
//
// Ports 2/3 ($2142/$2143): write-through from shadow, no handshake
// ------------------------------------------------------------------
void apu_port_handshake(void) {
    // --- Port 0 handshake ---
    if (shadow.apu_cmd_to_send != 0) {
        // Check if APU has acked previous send
        if (shadow.apu_cmd_to_send != shadow.apu_last_sent_p0) {
            // Write command to port 0
            ppu.apu_port0 = shadow.apu_cmd_to_send;
            shadow.apu_last_sent_p0 = shadow.apu_cmd_to_send;

            // Special command tracking ($F2+)
            if (shadow.apu_cmd_to_send < 0xF2) {
                shadow.apu_special_cmd = shadow.apu_cmd_to_send;
            }
            shadow.apu_cmd_to_send = 0;
        }
    } else {
        // No pending command — check if APU sent new data
        if (ppu.apu_port0 != shadow.apu_last_sent_p0) {
            // APU has responded (new value differs from last received)
            // Data is already in ppu.apu_port0
        } else {
            // No new data — clear port
            ppu.apu_port0 = 0;
        }
    }

    // --- Port 1 handshake ---
    if (shadow.apu_data_to_send != 0) {
        if (shadow.apu_data_to_send != shadow.apu_last_sent_p1) {
            ppu.apu_port1 = shadow.apu_data_to_send;
            shadow.apu_last_sent_p1 = shadow.apu_data_to_send;
            shadow.apu_data_to_send = 0;
        }
    } else {
        if (ppu.apu_port1 != shadow.apu_last_sent_p1) {
            // APU responded
        } else {
            ppu.apu_port1 = 0;
        }
    }

    // --- Ports 2 and 3: write-through ---
    // $00:811E-$00:812D
    ppu.apu_port2 = shadow.apu_param2;
    ppu.apu_port3 = shadow.apu_param3;
    shadow.apu_param2 = 0;
    shadow.apu_param3 = 0;
}

// ------------------------------------------------------------------
// write_ppu_shadow_regs: $00:8144-$00:81E5
//
// Writes all PPU shadow registers to actual PPU during VBlank:
//   - Window settings ($2123-$2125) from shadow $96-$98
//   - Color math ($2130-$2132) from shadow $99, $9A, $9C-$9E
//   - Screen designation ($212C-$212F) from shadow $1C-$1F
//   - BG1 scroll ($210D-$210E) from shadow $0120-$0125
//   - BG2 scroll ($210F-$2110) from shadow $011E-$0123
//   - BG3 scroll ($2111-$2112) from shadow $E4-$E5, $EA-$EB
//   - Mosaic ($2106) from shadow $95
//   - BG mode ($2105) from shadow $94
//   - Mode 7 regs ($211C-$2120) if mode == 7
// ------------------------------------------------------------------
void write_ppu_shadow_regs(void) {
    // Check nmi_skip flag — if set, skip register writes (fast path)
    if (shadow.nmi_skip != 0) {
        // $00:8149: JMP $822D → timed NMI path
        nmi_timed_path();
        return;
    }

    // Window settings
    // $00:814C-$00:8158
    ppu.w12sel = shadow.w12sel;       // $2123
    ppu.w34sel = shadow.w34sel;       // $2124
    ppu.wobjsel = shadow.wobjsel;     // $2125

    // Color math
    // $00:815B-$00:8171
    ppu.cgsel = shadow.cgsel;         // $2130
    ppu.cgadsub = shadow.cgadsub;     // $2131
    // $2132 (COLDATA) written 3 times for green, red, blue
    // (bitmask-based, last write wins for each component)
    ppu.coldata = shadow.coldata_green;  // $9C
    ppu.coldata = shadow.coldata_red;    // $9D
    ppu.coldata = shadow.coldata_blue;   // $9E

    // Screen designation
    // $00:8174-$00:8185
    ppu.tm = shadow.main_screen;      // $212C ← $1C
    ppu.ts = shadow.sub_screen;       // $212D ← $1D
    ppu.tmw = shadow.window_mask1;    // $212E ← $1E
    ppu.tsw = shadow.window_mask2;    // $212F ← $1F

    // --- Scroll registers ---
    // BG1 Horizontal: $00:8188-$00:8191 (write twice to $210D)
    // $0120 (low), $0121 (high) — 16-bit scroll value
    ppu.bg1hofs = shadow.scroll_bg1_h;

    // BG1 Vertical: $00:8194-$00:819D (write twice to $210E)
    ppu.bg1vofs = shadow.scroll_bg1_v;

    // BG2 Horizontal: $00:81A0-$00:81A9 (write twice to $210F)
    ppu.bg2hofs = shadow.scroll_bg2_h;

    // BG2 Vertical: $00:81AC-$00:81B5 (write twice to $2110)
    ppu.bg2vofs = shadow.scroll_bg2_v;

    // BG3 Horizontal: $00:81B8-$00:81BF (write twice to $2111)
    // From shadow $E4 (low), $E5 (high)
    ppu.bg3hofs = shadow.bg3hofs_low | (shadow.bg3hofs_high << 8);

    // BG3 Vertical: $00:81C2-$00:81C9 (write twice to $2112)
    // From shadow $EA (low), $EB (high)
    ppu.bg3vofs = shadow.bg3vofs_low | (shadow.bg3vofs_high << 8);

    // Mosaic: $00:81CC-$00:81CE
    ppu.mosaic = shadow.mosaic;       // $2106 ← $95

    // BG Mode: $00:81D1-$00:81DA
    ppu.bgmode = shadow.bg_mode;      // $2105 ← $94

    // Mode 7 check: $00:81D6-$00:81E5
    if ((shadow.bg_mode & 0x07) == 0x07) {
        // Mode 7 active — clear M7B/M7D (simple mode 7 init)
        ppu.m7b = 0;                  // $211C × 2
        ppu.m7d = 0;                  // $211D × 2

        // Mode 7 center X: $00:81E8-$00:81F1
        // From $0638 (low), $0639 (high) — write twice to $211F
        // These are in WRAM, not in shadow struct
        extern uint8 wram_low[];
        uint16 m7x_val = wram_low[0x0638] | (wram_low[0x0639] << 8);
        ppu.m7x = m7x_val;

        // Mode 7 center Y: $00:81F4-$00:81FA
        // From $063A (low), $063B (high) — write twice to $2120
        uint16 m7y_val = wram_low[0x063A] | (wram_low[0x063B] << 8);
        ppu.m7y = m7y_val;
    }
}

// ------------------------------------------------------------------
// nmi_timed_path: $00:822D-$00:82C7
//
// Special timed NMI used during transitions:
//   JSR $9347 (timed operation)
//   LDA $FF; STA $4209; STZ $420A  (set H/V IRQ timer)
//   LDA #$A1; STA $4200             (NMI + IRQ + auto-joypad)
//   Then same register writes as normal path
// ------------------------------------------------------------------
void nmi_timed_path(void) {
    // $00:822D: JSR $9347 — timed VRAM update
    // (in a real implementation, this does a precisely-timed write)

    // $00:8230-$00:8235: Set IRQ timer
    // LDA $FF; STA $4209; STZ $420A
    // This sets a horizontal IRQ for mid-frame effects
    // shadow.irq_timer = 0x00FF;  // timer value from $FF

    // $00:8238: LDA #$A1; STA $4200
    // Enable NMI + IRQ + auto joypad
    ppu.nmitimen = 0xA1;

    // Same PPU register writes as normal path
    write_ppu_shadow_regs();
}

// ------------------------------------------------------------------
// nmi_stack_swap: $00:82C7-$00:82D6
//
// Swaps the main stack pointer with the saved IRQ stack pointer:
//   REP #$30; TSC; TAX
//   LDA $1F0A; TCS; STX $1F0A
//   PLB; PLD; PLY; PLX; PLA
// Used for interrupt nesting management
// ------------------------------------------------------------------
void nmi_stack_swap(void) {
    // Swap SP with saved value at $1F0A
    uint16 saved_sp = cpu.sp;
    cpu.sp = 0x1F0A;  // Load saved stack pointer
    // (In real implementation: restore bank, direct page, Y, X, A from stack)
    cpu.sp = saved_sp;
}

// ------------------------------------------------------------------
// nmi_handler_asm: $00:80CA-$00:822C
//
// Full NMI handler sequence:
//   1. REP #$30; push A/X/Y/D/B (16-bit context save)
//   2. LDA #$00; BRK $5B (set bank); PHK; PLB
//   3. SEP #$30 (8-bit mode)
//   4. Read $4210 (acknowledge NMI)
//   5. APU port handshake
//   6. Force blank ($80 → $2100)
//   7. Disable HDMA ($00 → $420C)
//   8. If first frame ($12==0): run DMA engine + read controllers
//   9. If nmi_skip set: jump to timed path
//  10. Write PPU shadow registers (window, color, scroll, BG mode, Mode 7)
//  11. Restore force blank from shadow ($13 → $2100)
//  12. Enable HDMA ($9B → $420C)
//  13. REP #$30; restore B/D/Y/X/A; RTI
// ------------------------------------------------------------------
void nmi_handler_asm(void) {
    // 1. Save 16-bit context (in real 65816: push A, X, Y, D, B)
    // On PC, we rely on C calling conventions for preservation.

    // 2. Set program bank to current bank (PHK/PLB equivalent)
    // cpu.pb = cpu.pb;  // already correct

    // 3. Switch to 8-bit mode
    // SEP #$30
    cpu.flag_m = true;  // 8-bit A
    cpu.flag_x = true;  // 8-bit X/Y

    // 4. Acknowledge NMI by reading $4210
    // LDA $4210
    // (bit 7 of $4210 is NMI flag; reading clears it)

    // 5. APU port handshake: $00:80DC-$00:812D
    apu_port_handshake();

    // 6. Force blank: $00:8130
    // LDA #$80; STA $2100
    ppu.inidisp = 0x80;

    // 7. Disable HDMA: $00:8135
    // STZ $420C
    ppu.hdma_enable = 0;

    // 8. First frame check: $00:8138-$00:8141
    if (shadow.nmi_flag == 0) {
        // INC $12 — set NMI flag (frame sync)
        shadow.nmi_flag = 1;

        // JSR $89E0 — run DMA engine
        dma_engine_run();

        // JSR $83D1 — read controllers
        read_controllers();
    }

    // 9. Check nmi_skip: $00:8144-$00:8149
    if (shadow.nmi_skip != 0) {
        // Jump to timed NMI path ($822D)
        nmi_timed_path();
        return;
    }

    // 10. Write PPU shadow registers: $00:814C-$00:81E5
    write_ppu_shadow_regs();

    // 11. Restore display: $00:821B-$00:821D
    // LDA $13; STA $2100
    ppu.inidisp = shadow.main_screen;  // $13 = display brightness/force blank

    // 12. Enable HDMA: $00:8220-$00:8222
    // LDA $9B; STA $420C
    ppu.hdma_enable = shadow.hdma_enable;  // $9B

    // 13. Restore 16-bit context and RTI
    // (In C: just return, context is managed by the caller)
}
