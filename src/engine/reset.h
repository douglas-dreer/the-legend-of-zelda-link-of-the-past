#ifndef ENGINE_RESET_H
#define ENGINE_RESET_H

#include "../types.h"

// Reset sequence: $00:8000-$00:802F
// Initializes SNES hardware, boots SPC700, clears RAM, enables NMI
void engine_reset(void);

// Hardware initialization: $00:8000-$00:8021
// SEI, disable NMI/HDMA/DMA, clear APU ports, force blank, switch to native mode
void init_hardware(void);

// SPC700 boot sequence: $00:8901
// Uploads SPC700 program via IPL ROM transfer
void boot_spc700(void);

// RAM clear: $00:87C0
// Clears WRAM $0000-$1FFF (low) and $0400-$1FFF (extended pages)
void clear_ram(void);

// Enable NMI: $00:802F
// STA #$81 → $4200 (NMI + auto joypad read)
void enable_nmi(void);

#endif // ENGINE_RESET_H
