#include "reset.h"
#include "../snes/hardware.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../snes/input.h"
#include "../game/state.h"
#include "../audio/apu.h"
#include <cstring>
#include <cstdio>

// External references
extern PPURegisters ppu;
extern CPU65816 cpu;
extern DMAChannel dma_channels[5];
extern uint8 vram[];
extern uint8 cgram[];
extern uint8 wram_low[];
extern uint8 wram_extended[];
extern GlobalState shadow;

// Forward declarations from main.cpp
extern void init_ram(void);

// ------------------------------------------------------------------
// init_hardware: $00:8000-$00:8021
// SEI; STZ $4200; STZ $420C; STZ $420B; STZ $2140-$2143;
// LDA #$80; STA $2100; CLC; XCE; REP #$28; LDA #$00; BRK #$5B
// ------------------------------------------------------------------
void init_hardware(void) {
    // SEI — disable interrupts (PC: no-op, we control flow)
    cpu.flag_i = true;

    // STZ $4200 — disable NMI/timer enable
    ppu.nmitimen = 0;

    // STZ $420C — disable HDMA
    ppu.hdma_enable = 0;

    // STZ $420B — disable DMA
    ppu.dma_enable = 0;

    // STZ $2140-$2143 — reset APU communication ports
    ppu.apu_port0 = 0;
    ppu.apu_port1 = 0;
    ppu.apu_port2 = 0;
    ppu.apu_port3 = 0;

    // LDA #$80; STA $2100 — force blank
    ppu.inidisp = 0x80;

    // CLC; XCE — switch from emulation (65C02) to native (65816)
    cpu.flag_c = false;
    cpu.emulation = false;

    // REP #$28 — set 16-bit accumulator, clear decimal flag
    // bit 7 (n), bit 6 (v) unchanged; bit 5 (m) clear = 16-bit A; bit 3 (d) clear
    cpu.flag_m = false;  // 16-bit accumulator
    cpu.flag_d = false;  // no decimal mode

    // Initialize stack pointer (native mode)
    cpu.sp = 0x01FF;

    // LDA #$00; BRK #$5B — set direct page to bank 00
    // (BRK $5B is used as a far call to the HALT handler on real hardware)
    cpu.dp = 0x0000;
    cpu.db = 0x00;
    cpu.pb = 0x00;
}

// ------------------------------------------------------------------
// sub_00_8023: $00:8023-$00:8026
// LDA #$FF; ORA ($1B,X)
// Sets $1B to $FF (bank byte for far addresses in bank $00)
// ------------------------------------------------------------------
static void set_bank_ff(void) {
    // LDA #$FF
    // ORA ($1B,X) — indirect OR, sets byte at [$1B] = $FF
    // This establishes the data bank register for the engine
    cpu.db = 0xFF;
}

// ------------------------------------------------------------------
// boot_spc700: $00:8901-$00:8912
// Uploads SPC700 program via IPL ROM port handshake
// $00:8901: LDA #$00; STA $00  (src addr low)
// $00:8905: LDA #$80; STA $01  (src addr high = $8000, IPL ROM)
// $00:8909: LDA #$19; STA $02  (bank $19 = SPC700 program in ROM)
// $00:890D: SEI; JSR $8888     (transfer data)
// $00:8911: CLI; RTS
// ------------------------------------------------------------------
void boot_spc700(void) {
    // Set source pointer: $00 = $0000, $01 = $80, $02 = $19
    // This points to ROM bank $19:$8000 = SPC700 program data
    // In PC emulation, we just call the APU boot
    spc700_boot();
    printf("SPC700 boot completed.\n");
}

// ------------------------------------------------------------------
// clear_ram: $00:87C0-$00:87E5
// REP #$30; LDY $01FE; LDX #$FE03
// Loop: STA $0000,X; STA $0400,X; STA $0800,X; ... STA $1C00,X
// DEX; DEX; BNE loop
// Clears pages 00-1F in low WRAM ($0000-$1FFF)
// ------------------------------------------------------------------
void clear_ram(void) {
    // Clear low WRAM pages (0x0000 - 0x1FFF)
    // The original clears 8 pages in interleaved fashion
    // using 16-bit stores with X decrementing by 2
    memset(wram_low, 0, WRAM_LOW_SIZE);

    // Clear extended WRAM (0x0400 - 0x1FFF mirrors already covered)
    // Extended shadow RAM areas used by the game
    memset(wram_extended, 0, WRAM_EXTENDED_SIZE);

    // Reset shadow state
    memset(&shadow, 0, sizeof(GlobalState));

    printf("RAM clear completed.\n");
}

// ------------------------------------------------------------------
// enable_nmi: $00:802F-$00:8031
// LDA #$81; STA $4200
// bit 7 = NMI enable, bit 0 = auto joypad read
// ------------------------------------------------------------------
void enable_nmi(void) {
    // LDA #$81; STA $4200
    ppu.nmitimen = 0x81;
    cpu.flag_i = false;  // CLI — enable interrupts
}

// ------------------------------------------------------------------
// engine_reset: $00:8000-$00:8031
// Full reset sequence
// ------------------------------------------------------------------
void engine_reset(void) {
    printf("Engine reset: starting initialization...\n");

    // $00:8000-$00:8021: Hardware init
    init_hardware();

    // $00:8023: Set data bank to $FF
    set_bank_ff();

    // $00:8027: SEP #$30 — set 8-bit accumulator and index
    cpu.flag_m = true;   // 8-bit A
    cpu.flag_x = true;   // 8-bit X/Y

    // $00:8029: JSR $8901 — boot SPC700
    boot_spc700();

    // $00:802C: JSR $87C0 — clear RAM
    clear_ram();

    // $00:802F-$00:8031: Enable NMI
    enable_nmi();

    printf("Engine reset: complete.\n");
}
