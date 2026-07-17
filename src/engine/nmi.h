#ifndef ENGINE_NMI_H
#define ENGINE_NMI_H

#include "../types.h"

// NMI Handler: $00:80CA
// Main VBlank interrupt handler
void nmi_handler_asm(void);

// APU Port Handshake: $00:80DC-$00:812D
// Bidirectional communication with SPC700 via ports $2140-$2143
void apu_port_handshake(void);

// Write PPU Shadow Registers: $00:8144-$00:81E5
// Writes window, color math, screen designation, scroll, BG mode, Mode 7 regs
void write_ppu_shadow_regs(void);

// Timed NMI Path: $00:822D-$00:82C7
// Used during special transitions (e.g. timing-critical mid-frame effects)
void nmi_timed_path(void);

// Stack Swap Helper: $00:82C7
// Swaps main and IRQ stacks for nested interrupt handling
void nmi_stack_swap(void);

#endif // ENGINE_NMI_H
