#ifndef HARDWARE_H
#define HARDWARE_H

#include "../types.h"

// SNES PPU Register Addresses ($2100-$213F)
enum class PPURegister : uint16 {
    // Display
    INIDISP = 0x2100,  // Screen display / force blank
    OBJSEL = 0x2101,   // Object size + base address
    OAMADDL = 0x2102,  // OAM address low
    OAMADDH = 0x2103,  // OAM address high
    OAMDATA = 0x2104,  // OAM data write
    
    // Background
    BGMODE = 0x2105,   // BG mode + character size
    MOSAIC = 0x2106,   // Mosaic size
    BG1SC = 0x2107,    // BG1 tilemap address
    BG2SC = 0x2108,    // BG2 tilemap address
    BG3SC = 0x2109,    // BG3 tilemap address
    BG4SC = 0x210A,    // BG4 tilemap address
    BG12NBA = 0x210B,  // BG1/BG2 character data address
    BG34NBA = 0x210C,  // BG3/BG4 character data address
    
    // Scroll
    BG1HOFS = 0x210D,  // BG1 horizontal scroll (write x2)
    BG1VOFS = 0x210E,  // BG1 vertical scroll (write x2)
    BG2HOFS = 0x210F,  // BG2 horizontal scroll
    BG2VOFS = 0x2110,  // BG2 vertical scroll
    BG3HOFS = 0x2111,  // BG3 horizontal scroll
    BG3VOFS = 0x2112,  // BG3 vertical scroll
    BG4HOFS = 0x2113,  // BG4 horizontal scroll
    BG4VOFS = 0x2114,  // BG4 vertical scroll
    
    // VRAM
    VMAIN = 0x2115,    // VRAM address increment mode
    VRAMADDL = 0x2116, // VRAM address low
    VRAMADDH = 0x2117, // VRAM address high
    VRAMDATAW = 0x2118, // VRAM data write low
    VRAMDATAH = 0x2119, // VRAM data write high
    
    // Mode 7
    M7SEL = 0x211A,    // Mode 7 settings
    M7A = 0x211B,      // Mode 7 matrix A (write x2)
    M7B = 0x211C,      // Mode 7 matrix B
    M7C = 0x211D,      // Mode 7 matrix C
    M7D = 0x211E,      // Mode 7 matrix D
    M7X = 0x211F,      // Mode 7 center X
    M7Y = 0x2120,      // Mode 7 center Y
    
    // Color
    CGADD = 0x2121,    // CGRAM address
    CGDATAW = 0x2122,  // CGRAM data write
    
    // Window
    W12SEL = 0x2123,   // Window 1/2 settings BG1/2
    W34SEL = 0x2124,   // Window 3/4 settings BG3/4
    WOBJSEL = 0x2125,  // Window settings OBJ
    
    // Screen designation
    TM = 0x212C,       // Main screen designation
    TS = 0x212D,       // Sub screen designation
    TMW = 0x212E,      // Window mask for main
    TSW = 0x212F,      // Window mask for sub
    
    // Color math
    CGWSEL = 0x2130,   // Color math control
    CGADSUB = 0x2131,  // Color math add/subtract
    COLDATA = 0x2132,  // Fixed color data
};

// CPU I/O Register Addresses ($4200-$44FF)
enum class CPURegister : uint16 {
    NMITIMEN = 0x4200,  // NMI/Timer enable
    WRIO = 0x4201,      // Joypad programmable I/O
    WRMPYA = 0x4202,    // Multiplicand A
    WRMPYB = 0x4203,    // Multiplicand B
    RDMPYL = 0x4214,    // Quotient low
    RDMPYH = 0x4215,    // Quotient high
    RDIVL = 0x4214,     // Division result low
    RDIVH = 0x4215,     // Division result high
    HVBJOY = 0x4212,    // H/V blank + joypad busy
    
    // Joypad auto-read
    JOY1L = 0x4218,     // Joypad 1 low byte
    JOY1H = 0x4219,     // Joypad 1 high byte
    JOY2L = 0x421A,     // Joypad 2 low byte
    JOY2H = 0x421B,     // Joypad 2 high byte
    
    // DMA
    DMAEN = 0x420B,     // DMA enable (trigger)
    HDMAEN = 0x420C,    // HDMA enable
    
    // DMA channel 0
    DMAP0 = 0x4300,     // DMA mode
    BBAD0 = 0x4301,     // B-bus address
    A1T0L = 0x4302,     // A-bus address low
    A1T0H = 0x4303,     // A-bus address high
    A1B0 = 0x4304,      // A-bus bank
    DAS0L = 0x4305,     // DMA size low
    DAS0H = 0x4306,     // DMA size high
};

// APU Port Addresses ($2140-$2143)
enum class APURegister : uint16 {
    PORT0 = 0x2140,  // Communication port 0
    PORT1 = 0x2141,  // Communication port 1
    PORT2 = 0x2142,  // Communication port 2
    PORT3 = 0x2143,  // Communication port 3
};

// PPU Registers Shadow State
struct PPURegisters {
    uint8 inidisp;      // $2100
    uint8 objsel;       // $2101
    uint16 oam_addr;    // $2102-$2103
    uint8 bgmode;       // $2105
    uint8 mosaic;       // $2106
    uint16 bg1hofs;     // $210D
    uint16 bg1vofs;     // $210E
    uint16 bg2hofs;     // $210F
    uint16 bg2vofs;     // $2110
    uint16 bg3hofs;     // $2111
    uint16 bg3vofs;     // $2112
    uint8 vram_ctrl;    // $2115
    uint16 vram_addr;   // $2116-$2117
    uint16 vram_data;   // $2118-$2119
    uint8 cgadd;        // $2121
    uint8 w12sel;       // $2123
    uint8 w34sel;       // $2124
    uint8 wobjsel;      // $2125
    uint8 tm;           // $212C (main screen)
    uint8 ts;           // $212D (sub screen)
    uint8 tmw;          // $212E
    uint8 tsw;          // $212F
    uint8 cgsel;        // $2130
    uint8 cgadsub;      // $2131
    uint8 coldata;      // $2132
    
    // CPU I/O Shadow Registers
    uint8 nmitimen;     // $4200: NMI/Timer enable
    uint8 hdma_enable;  // $420C: HDMA enable
    uint8 dma_enable;   // $420B: DMA enable (trigger)
    
    // Mode 7 Registers
    uint8 m7sel;        // $211A
    uint16 m7a;         // $211B
    uint16 m7b;         // $211C
    uint16 m7c;         // $211D
    uint16 m7d;         // $211E
    uint16 m7x;         // $211F
    uint16 m7y;         // $2120
    
    // APU Communication Ports
    uint8 apu_port0;    // $2140
    uint8 apu_port1;    // $2141
    uint8 apu_port2;    // $2142
    uint8 apu_port3;    // $2143
    
    // Mode 7 enabled flag
    bool mode7_enabled;
};

// CPU State
struct CPU65816 {
    uint16 a;        // Accumulator (8 ou 16-bit)
    uint16 x;        // Index X
    uint16 y;        // Index Y
    uint16 sp;       // Stack Pointer
    uint16 dp;       // Direct Page
    uint8 db;        // Data Bank
    uint8 pb;        // Program Bank
    uint16 pc;       // Program Counter
    
    // Flags do registrador P
    bool flag_n;       // Negative
    bool flag_v;       // Overflow
    bool flag_m;       // Accumulator 8/16 (true=8bit)
    bool flag_x;       // Index 8/16 (true=8bit)
    bool flag_d;       // Decimal mode (não usado no SNES)
    bool flag_i;       // IRQ disable
    bool flag_z;       // Zero
    bool flag_c;       // Carry
    
    bool emulation;    // Modo emulação 65C02
};

// DMA Channel Registers
struct DMAChannel {
    uint8 mode;         // $43x0: modo de transferência
    uint8 bbus_addr;    // $43x1: endereço B-bus (PPU)
    uint32 src_addr;    // $43x2-$43x4: endereço A-bus (24-bit)
    uint16 size;        // $43x5-$43x6: tamanho em bytes
    uint8 hdma_indirect; // $43x7: HDMA flag
};

// DMA Mode Constants
constexpr uint8 DMA_MODE_1BYTE = 0x00;    // 1 byte → PPU (write once)
constexpr uint8 DMA_MODE_2BYTE = 0x01;    // 2 bytes → PPU (write twice)
constexpr uint8 DMA_MODE_1BYTE_2 = 0x02;  // 1 byte → PPU+2 (write, skip 2)
constexpr uint8 DMA_MODE_4BYTE = 0x03;    // 4 bytes → PPU (write 4x)
constexpr uint8 DMA_MODE_2BYTE_2 = 0x04;  // 2 bytes → PPU+2 (write 2x, skip 2)
constexpr uint8 DMA_MODE_8BYTE = 0x05;    // 8 bytes (write 2x each of 4)
constexpr uint8 DMA_MODE_4BYTE_2 = 0x06;  // 4 bytes → PPU+2
constexpr uint8 DMA_MODE_16BYTE = 0x07;   // 16 bytes (block transfer)

// Global PPU and CPU state
extern PPURegisters ppu;
extern CPU65816 cpu;
extern DMAChannel dma_channels[5];

#endif // HARDWARE_H