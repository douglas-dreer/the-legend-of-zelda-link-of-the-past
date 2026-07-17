# Guia de Conversão: Assembly 65816 → C/C++

**The Legend of Zelda: A Link to the Past**  
**Target:** PC (x86/x64) em C/C++  
**ROM:** 1 MB LoROM | CPU: 65816 | Disassembly: 569.023 linhas

---

## Índice

1. [Arquitetura do Código Original](#1-arquitetura-do-código-original)
2. [Mapeamento de Instruções 65816 para C](#2-mapeamento-de-instruções-65816-para-c)
3. [Estrutura de Dados](#3-estrutura-de-dados)
4. [Sistemas do Jogo em C](#4-sistemas-do-jogo-em-c)
5. [Conversão de Endereços SNES](#5-conversão-de-endereços-snes)
6. [Plano de Decompição](#6-plano-de-decompição)
7. [Referências](#7-referências)

---

## 1. Arquitetura do Código Original

### 1.1 Mapa de Memória: 65816 vs PC

O 65816 endereça 24 bits (16 MB), mas o SNES usa apenas ~128 KB de mapeamento real.
O LoROM divide a ROM em bancos de 32 KB ($8000-$FFFF).

**SNES Memory Map:**

```
$0000-$1FFF  Low WRAM (8 KB)
$2000-$20FF  PPU registers (I/O)
$2100-$21FF  PPU registers
$4000-$41FF  APU I/O + controller I/O
$4200-$44FF  DMA/HDMA/Timer registers
$8000-$FFFF  ROM (32 KB per bank)
$7E0000-$7FFFFF  Extended WRAM (64 KB)
```

**Mapping para PC:**

```c
// SNES: Memória linear mapeada em arrays
uint8_t wram_low[0x2000];        // $0000-$1FFF
uint8_t wram_extended[0x10000];   // $7E0000-$7FFFFF (64 KB)
uint8_t rom[0x100000];            // 1 MB LoROM
uint8_t vram[0x10000];            // 64 KB VRAM (PPU)
uint8_t oam[0x220];               // 544 bytes OAM
uint8_t cgram[0x200];             // 512 bytes palette

// Registradores PPU como variáveis globais shadow
typedef struct {
    uint8_t inidisp;      // $2100
    uint8_t objsel;       // $2101
    uint16_t oam_addr;    // $2102-$2103
    uint8_t bgmode;       // $2105
    uint8_t mosaic;       // $2106
    uint16_t bg1hofs;     // $210D
    uint16_t bg1vofs;     // $210E
    uint16_t bg2hofs;     // $210F
    uint16_t bg2vofs;     // $2110
    uint16_t bg3hofs;     // $2111
    uint16_t bg3vofs;     // $2112
    uint8_t vram_ctrl;    // $2115
    uint16_t vram_addr;   // $2116-$2117
    uint16_t vram_data;   // $2118-$2119
    uint8_t cgadd;        // $2121
    uint8_t w12sel;       // $2123
    uint8_t w34sel;       // $2124
    uint8_t wobjsel;      // $2125
    uint8_t tm;           // $212C (main screen)
    uint8_t ts;           // $212D (sub screen)
    uint8_t tmw;          // $212E
    uint8_t tsw;          // $212F
    uint8_t cgsel;        // $2130
    uint8_t cgadsub;      // $2131
    uint8_t coldata;      // $2132
} PPURegisters;
```

### 1.2 Registradores SNES e Mapeamento para C

| Registrador 65816 | Tamanho | Emulação C | Observações |
|---|---|---|---|
| **A** (Accumulator) | 8/16-bit | `uint8_t` ou `uint16_t` | Muda com REP/SEP |
| **X** (Index) | 8/16-bit | `uint8_t` ou `uint16_t` | Muda com REP/SEP |
| **Y** (Index) | 8/16-bit | `uint8_t` ou `uint16_t` | Muda com REP/SEP |
| **S** (Stack Pointer) | 16-bit | `uint16_t sp` | Stack em WRAM $0100-$01FF |
| **D** (Direct Page) | 16-bit | `uint16_t dp` | Base para addressing direto |
| **DB** (Data Bank) | 8-bit | `uint8_t db` | Bank para endereçamento |
| **PB** (Program Bank) | 8-bit | `uint8_t pb` | Bank do PC |
| **P** (Processor) | 8-bit | `uint8_t flags` | N, V, M, X, D, I, Z, C |

**Struct de contexto do CPU:**

```c
typedef struct {
    uint16_t a;        // Accumulator (8 ou 16-bit)
    uint16_t x;        // Index X
    uint16_t y;        // Index Y
    uint16_t sp;       // Stack Pointer
    uint16_t dp;       // Direct Page
    uint8_t db;        // Data Bank
    uint8_t pb;        // Program Bank
    uint16_t pc;       // Program Counter
    
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
} CPU65816;
```

### 1.3 Modos 8-bit vs 16-bit do 65816

O 65816 pode operar com registradores de 8 ou 16 bits. O Zelda usa predominantemente **8-bit**:

```c
// SEP #$30 → A=8bit, X/Y=8bit (mais comum no jogo)
// REP #$20 → A=16bit (usado em transferências de endereço)
// REP #$30 → A=16bit, X/Y=16bit (raro)

// Emulação: verificar flags antes de operar
void lda_imm(CPU65816 *cpu, uint16_t value) {
    if (cpu->flag_m) {
        cpu->a = value & 0xFF;  // 8-bit: ignora high byte
        cpu->flag_z = (cpu->a == 0);
        cpu->flag_n = (cpu->a & 0x80) != 0;
    } else {
        cpu->a = value;         // 16-bit
        cpu->flag_z = (cpu->a == 0);
        cpu->flag_n = (cpu->a & 0x8000) != 0;
    }
}
```

### 1.4 Little-Endian vs Byte Order

O 65816 é **little-endian**, igual ao x86/x64. Não é necessário swap de bytes.

```c
// SNES: Ler word little-endian de ROM
uint16_t read16_le(const uint8_t *data) {
    return data[0] | (data[1] << 8);
}

// SNES: Endereço de 24 bits (bank:addr)
uint32_t snes_address(uint8_t bank, uint16_t addr) {
    return (bank << 16) | addr;
}
```

---

## 2. Mapeamento de Instruções 65816 para C

### 2.1 Referência Completa de Tradução

| Instrução 65816 | Descrição | Equivalente C |
|---|---|---|
| **LDA #imm** | Load Accumulator imediato | `A = imm;` (update N, Z) |
| **LDA addr** | Load Accumulator da memória | `A = mem[addr];` (update N, Z) |
| **LDA (dp,X)** | Load indirect indexed | `A = mem[dp + X];` |
| **LDA (dp),Y** | Load indexed indirect | `A = mem[dp] + Y;` |
| **LDA long** | Load de endereço longo (24-bit) | `A = mem[bank:addr];` |
| **STA addr** | Store Accumulator | `mem[addr] = A;` |
| **STA long** | Store longo | `mem[bank:addr] = A;` |
| **STZ addr** | Store Zero | `memset(&mem[addr], 0, size);` ou `mem[addr] = 0;` |
| **LDX #imm** | Load Index X | `X = imm;` (update N, Z) |
| **LDX addr** | Load X da memória | `X = mem[addr];` |
| **LDY #imm** | Load Index Y | `Y = imm;` (update N, Z) |
| **STX addr** | Store Index X | `mem[addr] = X;` |
| **STY addr** | Store Index Y | `mem[addr] = Y;` |
| **ADC #imm** | Add with Carry | `A = A + imm + C;` (update N, V, Z, C) |
| **ADC addr** | Add from memory | `A = A + mem[addr] + C;` |
| **SBC #imm** | Subtract with Carry | `A = A - imm - !C;` (update N, V, Z, C) |
| **SBC addr** | Subtract from memory | `A = A - mem[addr] - !C;` |
| **AND #imm** | Bitwise AND | `A = A & imm;` (update N, Z) |
| **AND addr** | AND com memória | `A = A & mem[addr];` |
| **ORA #imm** | Bitwise OR | `A = A \| imm;` (update N, Z) |
| **ORA addr** | OR com memória | `A = A \| mem[addr];` |
| **EOR #imm** | Bitwise XOR | `A = A ^ imm;` (update N, Z) |
| **EOR addr** | XOR com memória | `A = A ^ mem[addr];` |
| **ASL A** | Arithmetic Shift Left | `C = A >> 7; A <<= 1;` (8-bit) |
| **ASL addr** | Shift Left memória | `C = mem[addr] >> 7; mem[addr] <<= 1;` |
| **LSR A** | Logical Shift Right | `C = A & 1; A >>= 1;` (sem sinal) |
| **LSR addr** | Shift Right memória | `C = mem[addr] & 1; mem[addr] >>= 1;` |
| **ROL A** | Rotate Left through Carry | `new_carry = A >> 7; A = (A << 1) \| old_carry; C = new_carry;` |
| **ROR A** | Rotate Right through Carry | `new_carry = A & 1; A = (A >> 1) \| (old_carry << 7); C = new_carry;` |
| **INC A** | Increment Accumulator | `A++; update N, Z;` |
| **INC addr** | Increment memória | `mem[addr]++; update N, Z;` |
| **DEC A** | Decrement Accumulator | `A--; update N, Z;` |
| **DEC addr** | Decrement memória | `mem[addr]--; update N, Z;` |
| **INX** | Increment X | `X++; update N, Z;` |
| **INY** | Increment Y | `Y++; update N, Z;` |
| **DEX** | Decrement X | `X--; update N, Z;` |
| **DEY** | Decrement Y | `Y--; update N, Z;` |
| **CMP #imm** | Compare Accumulator | `set_NZC(A - imm);` (não altera A) |
| **CMP addr** | Compare com memória | `set_NZC(A - mem[addr]);` |
| **CPX #imm** | Compare X | `set_NZC(X - imm);` |
| **CPY #imm** | Compare Y | `set_NZC(Y - imm);` |
| **BIT #imm** | Test bits | `Z = !(A & imm); N = imm >> 7; V = imm >> 6;` |
| **BEQ rel** | Branch if Equal (Z=1) | `if (flag_z) goto target;` |
| **BNE rel** | Branch if Not Equal (Z=0) | `if (!flag_z) goto target;` |
| **BCC rel** | Branch if Carry Clear | `if (!flag_c) goto target;` |
| **BCS rel** | Branch if Carry Set | `if (flag_c) goto target;` |
| **BPL rel** | Branch if Plus (N=0) | `if (!flag_n) goto target;` |
| **BMI rel** | Branch if Minus (N=1) | `if (flag_n) goto target;` |
| **BVS rel** | Branch if Overflow Set | `if (flag_v) goto target;` |
| **BVC rel** | Branch if Overflow Clear | `if (!flag_v) goto target;` |
| **JMP addr** | Jump absoluto | `goto target;` ou `return;` (se tail call) |
| **JMP (addr,X)** | Jump indexed indirect | `goto table[X];` (dispatch) |
| **JMP long** | Jump longo (24-bit) | `pb = bank; pc = addr;` |
| **JSR addr** | Jump to Subroutine | `call function();` |
| **JSL long** | Jump to Subroutine Long | `call far function();` |
| **RTS** | Return from Subroutine | `return;` |
| **RTL** | Return from Long Subroutine | `return;` |
| **PHA** | Push Accumulator | `push(A);` ou inline |
| **PLA** | Pull Accumulator | `A = pop();` |
| **PHX** | Push X | `push(X);` |
| **PLX** | Pull X | `X = pop();` |
| **PHY** | Push Y | `push(Y);` |
| **PLY** | Pull Y | `Y = pop();` |
| **PHB** | Push Data Bank | `push(db);` |
| **PLB** | Pull Data Bank | `db = pop();` |
| **PHD** | Push Direct Page | `push(dp);` |
| **PLD** | Pull Direct Page | `dp = pop();` |
| **PHK** | Push Program Bank | `push(pb);` |
| **RTL** | Return Long | `return;` |
| **REP #imm** | Reset Processor Bits | `clear_flags(imm);` |
| **SEP #imm** | Set Processor Bits | `set_flags(imm);` |
| **CLC** | Clear Carry | `flag_c = false;` |
| **SEC** | Set Carry | `flag_c = true;` |
| **CLI** | Clear IRQ Disable | `flag_i = false;` |
| **SEI** | Set IRQ Disable | `flag_i = true;` |
| **CLD** | Clear Decimal | `flag_d = false;` |
| **NOP** | No Operation | `// nothing` |
| **XCE** | Exchange C and Emulation | `swap(&flag_c, &emulation);` |
| **TAX** | Transfer A to X | `X = A; update N, Z;` |
| **TAY** | Transfer A to Y | `Y = A; update N, Z;` |
| **TXA** | Transfer X to A | `A = X; update N, Z;` |
| **TYA** | Transfer Y to A | `A = Y; update N, Z;` |
| **TXS** | Transfer X to Stack | `sp = X;` |
| **TSX** | Transfer Stack to X | `X = sp;` |
| **TCD** | Transfer A to Direct Page | `dp = A;` |
| **TDC** | Transfer Direct Page to A | `A = dp;` |
| **TSC** | Transfer Stack to C | `A = sp;` |
| **NMI** (vetor) | Non-Maskable Interrupt | `vblank_callback();` |
| **IRQ** (vetor) | Interrupt Request | `irq_callback();` |

### 2.2 Exemplos de Conversão Prática

**Conversão do RESET Handler ($00:8000):**

```asm
; Assembly original
$8000: 78              SEI
$8001: 9C 00 42        STZ $4200
$8004: 9C 0C 42        STZ $420C
$8007: 9C 0B 42        STZ $420B
$800A: 9C 40 21        STZ $2140
$800D: 9C 41 21        STZ $2141
$8010: 9C 42 21        STZ $2142
$8013: 9C 43 21        STZ $2143
$8016: A9 80           LDA #$80
$8018: 8D 00 21        STA $2100
$801B: 18              CLC
$801C: FB              XCE
$801D: C2 28           REP #$28
$801F: A9 00 01        LDA #$01FF
$8022: 1B              TCS
$8023: E2 30           SEP #$30
$8025: 20 01 89        JSR $8901
$8028: 20 C0 87        JSR $87C0
```

```c
// Conversão para C
void snes_reset(void) {
    // Desabilita interrupções
    cpu.flag_i = true;
    
    // Zera registradores de controle
    ppu.nmitimen = 0;         // $4200
    ppu.hdmaen = 0;           // $420C
    ppu.dmaen = 0;            // $420B
    
    // Zera portas APU
    ppu.apu_port0 = 0;        // $2140
    ppu.apu_port1 = 0;        // $2141
    ppu.apu_port2 = 0;        // $2142
    ppu.apu_port3 = 0;        // $2143
    
    // Force blank
    ppu.inidisp = 0x80;       // $2100 = #$80
    
    // Transição para modo nativo (65816)
    cpu.flag_c = false;
    cpu.emulation = false;
    
    // Limpa decimal mode e A=16bit
    cpu.flag_d = false;
    cpu.flag_m = false;
    
    // Inicializa stack pointer
    cpu.sp = 0x01FF;
    
    // Modo 8-bit
    cpu.flag_m = true;   // SEP #$30 implica M=1, X=1
    cpu.flag_x = true;
    
    // Inicialização principal
    spc700_boot();
    ram_clear();
}
```

**Conversão do Main Loop ($00:802F):**

```asm
; Assembly original
$802F: A5 12           LDA $12
$8031: F0 FC           BEQ $802F
$8033: 58              CLI
$8034: EE 1A 00        INC $001A
$8037: 20 1E 84        JSR $841E
$803A: 22 B5 80 00     JSL $0080B5
$803E: 20 FC 85        JSR $85FC
$8041: 9C 12 00        STZ $0012
$8044: 80 E9           BRA $802F
```

```c
// Conversão para C
void main_loop(void) {
    while (true) {
        // Espera NMI (VBlank) - frame sync
        while (global.nmi_flag == 0) {
            // busy-wait (pode usar SDL_Delay em PC)
        }
        
        // Habilita IRQs
        cpu.flag_i = false;
        
        // Incrementa frame counter
        global.frame_counter++;       // $001A
        
        // Oculta sprites não usados
        hide_unused_sprites();
        
        // Lógica principal do jogo
        main_game_logic();             // $0080B5
        
        // Prepara parâmetros DMA
        prepare_dma_params();
        
        // Limpa flag NMI (espera próxima frame)
        global.nmi_flag = 0;           // $0012
    }
}
```

**Conversão do NMI Handler ($00:80CA):**

```asm
; Trecho: escreve scroll registers
$8279: AD 20 01        LDA $0120
$827C: 8D 0D 21        STA $210D    ; BG1HOFS low
$827F: AD 21 01        LDA $0121
$8282: 8D 0D 21        STA $210D    ; BG1HOFS high (double write)
$8285: AD 24 01        LDA $0124
$8288: 8D 0E 21        STA $210E    ; BG1VOFS low
$828B: AD 25 01        LDA $0125
$828E: 8D 0E 21        STA $210E    ; BG1VOFS high (double write)
```

```c
// Conversão para C
void vblank_callback(void) {
    // ... (outras escritas) ...
    
    // BG1 Horizontal Scroll (double write para latch de 16-bit)
    io_write(0x210D, shadow.scroll_bg1_h & 0xFF);      // $0120
    io_write(0x210D, shadow.scroll_bg1_h >> 8);         // $0121
    
    // BG1 Vertical Scroll
    io_write(0x210E, shadow.scroll_bg1_v & 0xFF);      // $0124
    io_write(0x210E, shadow.scroll_bg1_v >> 8);         // $0125
    
    // BG2 Horizontal Scroll
    io_write(0x210F, shadow.scroll_bg2_h & 0xFF);
    io_write(0x210F, shadow.scroll_bg2_h >> 8);
    
    // BG2 Vertical Scroll
    io_write(0x2110, shadow.scroll_bg2_v & 0xFF);
    io_write(0x2110, shadow.scroll_bg2_v >> 8);
    
    // BG3 Horizontal Scroll (HUD)
    io_write(0x2111, shadow.scroll_bg3_h & 0xFF);
    io_write(0x2111, shadow.scroll_bg3_h >> 8);
    
    // BG3 Vertical Scroll (HUD)
    io_write(0x2112, shadow.scroll_bg3_v & 0xFF);
    io_write(0x2112, shadow.scroll_bg3_v >> 8);
}
```

### 2.3 Tabela de Dispatch (Jump Tables)

O jogo usa extensivamente JMP indexed indirect para dispatch. Em C, isso vira `switch`:

```asm
; Assembly: Game mode dispatcher
$8C7B: 85 17           STA $17        ; game mode
$8C7D: 0A              ASL A
$8C7E: AA              TAX
$8C7F: 7C 8E 8C        JMP ($8C8E,X)  ; jump table
```

```c
// Conversão para C
typedef enum {
    GAME_MODE_INIT = 0,
    GAME_MODE_SECONDARY = 1,
    GAME_MODE_DMA_VRAM = 2,
    GAME_MODE_TILEMAP = 3,
    GAME_MODE_FULL_DMA = 4,
    GAME_MODE_VRAM_UPDATE = 5,
    GAME_MODE_IDLE = 6,
    GAME_MODE_VRAM_STREAM = 7,
    GAME_MODE_BG_UPDATE = 8,
    GAME_MODE_SPRITE_UPDATE = 9,
    GAME_MODE_OAM_UPDATE = 10,
    GAME_MODE_SCROLL_UPDATE = 11,
    GAME_MODE_DMA_PARTIAL = 12,
    GAME_MODE_DMA_ALT = 13,
    GAME_MODE_VRAM_WRITE = 14,
    GAME_MODE_MULTI_DMA = 15,
} GameState;

void game_mode_dispatch(uint8_t mode) {
    switch (mode) {
        case GAME_MODE_INIT:       game_mode_init(); break;
        case GAME_MODE_DMA_VRAM:   game_mode_dma_vram(); break;
        case GAME_MODE_TILEMAP:    game_mode_tilemap(); break;
        case GAME_MODE_FULL_DMA:   game_mode_full_dma(); break;
        // ... 12 modos restantes
        default: break;
    }
}
```

---

## 3. Estrutura de Dados

### 3.1 OAM Buffer ($0800-$0A3F) → Struct Sprite

```c
// Formato OAM do SNES
typedef struct {
    uint8_t y;         // Posição Y (byte baixo)
    uint8_t tile;      // Número do tile
    uint8_t attr;      // Atributos (paleta, prioridade, flip)
    uint8_t x;         // Posição X (byte baixo)
} OAMEntry;

// High table (bits 8/9 de X e Y)
typedef struct {
    uint8_t x_high    : 2;  // Bit 8 e 9 de X
    uint8_t y_high    : 2;  // Bit 8 e 9 de Y
    uint8_t size      : 1;  // 0=small, 1=large
} OAMHighEntry;

// OAM Attributes byte
#define OAM_ATTR_PALETTE(n)   ((n) << 1)       // Bits 1-3: paleta (0-7)
#define OAM_ATTR_PRIORITY(n)  ((n) << 4)       // Bits 4-5: prioridade
#define OAM_ATTR_FLIP_H       0x40             // Bit 6: flip horizontal
#define OAM_ATTR_FLIP_V       0x80             // Bit 7: flip vertical

// Buffer completo em RAM ($0800-$09FF)
typedef struct {
    OAMEntry     entries[128];    // 128 sprites × 4 bytes = 512 bytes
    OAMHighEntry high[64];        // High table = 64 bytes (128 entries × 2 bits)
} OAMBuffer;  // $0800-$0A3F

// Shadow RAM do jogo: $0A00-$0A3F (tabelas de tamanho/prioridade)
typedef struct {
    uint8_t size_table[128];      // 1=sprite grande, 0=pequeno
    uint8_t priority_table[128];  // Prioridade de desenho
} SpriteTables;  // $0A00-$0A3F
```

### 3.2 Shadow RAM → Global State Struct

```c
typedef struct {
    // --- Game State ($00-$1F) ---
    uint8_t temp_vars[7];         // $00-$06: variáveis temporárias
    uint8_t nmi_flag;             // $12: frame sync flag
    uint8_t palette_dirty;        // $15: flag de paleta suja
    uint8_t bg_mode_dirty;        // $16: flag de BG mode sujo
    uint8_t game_mode;            // $17: game mode index
    uint8_t frame_counter;        // $1A: contador de frames
    uint8_t main_screen;          // $1C: main screen designation
    uint8_t sub_screen;           // $1D: sub screen designation
    uint8_t window_mask1;         // $1E
    uint8_t window_mask2;         // $1F
    
    // --- Scroll Registers ($94-$EB) ---
    uint8_t bg_mode;              // $94: BG mode shadow
    uint8_t mosaic;               // $95: mosaic shadow
    uint8_t w12sel;               // $96
    uint8_t w34sel;               // $97
    uint8_t wobjsel;              // $98
    uint8_t cgsel;                // $99
    uint8_t cgadsub;              // $9A
    uint8_t hdma_enable;          // $9B
    uint8_t coldata_green;        // $9C
    uint8_t coldata_red;          // $9D
    uint8_t coldata_blue;         // $9E
    uint8_t bg3hofs_low;          // $E4
    uint8_t bg3hofs_high;         // $E5
    uint8_t bg3vofs_low;          // $EA
    uint8_t bg3vofs_high;         // $EB
    uint8_t transition_counter;   // $B0: transição de tela
    
    // --- Extended Scroll ($011E-$0125) ---
    uint16_t scroll_bg2_h;        // $011E-$011F
    uint16_t scroll_bg1_h;        // $0120-$0121
    uint16_t scroll_bg2_v;        // $0122-$0123
    uint16_t scroll_bg1_v;        // $0124-$0125
    
    // --- IRQ Config ($0128-$012A) ---
    uint8_t irq_config;           // $0128
    uint8_t nmi_skip;             // $012A
    
    // --- APU Communication ($012C-$0133) ---
    uint8_t apu_cmd_to_send;      // $012C: port 0 tx
    uint8_t apu_data_to_send;     // $012D: port 1 tx
    uint8_t apu_param2;           // $012E: port 2 tx
    uint8_t apu_param3;           // $012F: port 3 tx
    uint8_t apu_special_cmd;      // $0130: special $F2+
    uint8_t apu_last_sent_p1;     // $0131
    uint8_t apu_last_sent_p0;     // $0133
    
    // --- Input ($F0-$FB) ---
    uint16_t joypad1_held;        // $F0-$F2 (high, low)
    uint16_t joypad1_new;         // $F4-$F6 (high, low)
    uint16_t joypad1_prev;        // $F8-$FA (high, low)
    uint16_t joypad2_held;        // $F1-$F3
    uint16_t joypad2_new;         // $F5-$F7
    uint16_t joypad2_prev;        // $F9-$FB
    
    // --- BG3 Scroll (HUD) ($0630-$0631) ---
    uint16_t irq_scroll_bg3;      // $0630-$0631
    
    // --- DMA Parameters ($0AC0-$0AF8) ---
    uint32_t dma_sources[5];      // $0AC0, $0AC4, $0AC8, $0ADC, $0AF4
} ShadowRAM;
```

### 3.3 DMA Params → DMA_config Struct

```c
// Registradores DMA por canal (5 canais, $4300-$4340)
typedef struct {
    uint8_t  mode;         // $43x0: modo de transferência
    uint8_t  bbus_addr;    // $43x1: endereço B-bus (PPU)
    uint32_t src_addr;     // $43x2-$43x4: endereço A-bus (24-bit)
    uint16_t size;         // $43x5-$43x6: tamanho em bytes
    uint8_t  hdma_indirect; // $43x7: HDMA flag
} DMAChannel;

// Modos DMA
#define DMA_MODE_1BYTE    0x00  // 1 byte → PPU (write once)
#define DMA_MODE_2BYTE    0x01  // 2 bytes → PPU (write twice)
#define DMA_MODE_1BYTE_2  0x02  // 1 byte → PPU+2 (write, skip 2)
#define DMA_MODE_4BYTE    0x03  // 4 bytes → PPU (write 4x)
#define DMA_MODE_2BYTE_2  0x04  // 2 bytes → PPU+2 (write 2x, skip 2)
#define DMA_MODE_8BYTE    0x05  // 8 bytes (write 2x each of 4)
#define DMA_MODE_4BYTE_2  0x06  // 4 bytes → PPU+2
#define DMA_MODE_16BYTE   0x07  // 16 bytes (block transfer)

// Função DMA para PC
void dma_transfer(uint32_t src, uint16_t dst_ppu, uint16_t size,
                  uint8_t channel, uint8_t mode) {
    DMAChannel *ch = &dma_channels[channel];
    ch->mode = mode;
    ch->bbus_addr = 0x18;      // $2118: VRAM data write
    ch->src_addr = src;
    ch->size = size;
    
    // Fire DMA
    uint8_t bit = 1 << channel;
    io_write(0x420B, bit);
    
    // Em PC: copiar dados diretamente do ROM/RAM para VRAM
    for (int i = 0; i < size; i++) {
        uint8_t byte = read_bus(src + i);
        ppu_write_vram(dst_ppu + (i / 2), byte);
    }
}
```

### 3.4 Paletas → Color Palette Array

```c
// Formato SNES 15-bit: gggrrrrr 0bbbbbgg (2 bytes)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColorRGB;

// Converter SNES 15-bit para RGB 8-bit
ColorRGB snes_color_to_rgb(uint16_t snes_color) {
    ColorRGB c;
    c.r = (snes_color & 0x1F) << 3;                      // 5 bits → 8 bits
    c.g = ((snes_color >> 5) & 0x1F) << 3;
    c.b = ((snes_color >> 10) & 0x1F) << 3;
    return c;
}

// Palette completa do SNES
typedef struct {
    ColorRGB bg_palette[8][16];     // 8 sub-paletas × 16 cores
    ColorRGB sprite_palette[8][16]; // 8 sub-paletas × 16 cores
} SNESPalette;  // 256 cores total (512 bytes ROM)

// Shadow palette em WRAM (buffer de upload)
SNESPalette shadow_palette;  // carregada do ROM, enviada via DMA

// Upload para CGRAM via DMA
void load_palette(uint8_t palette_id) {
    uint32_t rom_addr = palette_table[palette_id];
    
    // Copia 512 bytes do ROM para shadow palette
    memcpy(&shadow_palette, &rom[rom_addr], sizeof(SNESPalette));
    
    // Marca como suja para upload no próximo NMI
    shadow.palette_dirty = 1;
}
```

### 3.5 Tiles → Tile Data Arrays

```c
// Formato 4bpp: 8×8 pixels, 32 bytes por tile
typedef struct {
    uint8_t planes[4][8];   // 4 planos, 8 linhas de 2 bytes cada
} Tile4bpp;  // 32 bytes

// Ler pixel de tile 4bpp
uint8_t get_pixel_4bpp(const uint8_t *tile_data, int x, int y) {
    uint8_t lo_byte = tile_data[y * 2];
    uint8_t hi_byte = tile_data[y * 2 + 1];
    uint8_t plane0 = (tile_data[16 + y * 2] >> x) & 1;
    uint8_t plane1 = (tile_data[16 + y * 2 + 1] >> x) & 1;
    
    uint8_t bit = 7 - x;
    uint8_t p0 = (lo_byte >> bit) & 1;
    uint8_t p1 = (hi_byte >> bit) & 1;
    
    return (plane0 << 3) | (plane1 << 2) | (p1 << 1) | p0;
}

// Tilemap entry (16-bit)
typedef struct {
    uint16_t tile_num   : 10;  // Bits 0-9: número do tile
    uint16_t palette    : 3;   // Bits 10-12: sub-paleta (0-7)
    uint16_t priority   : 2;   // Bits 13-14: prioridade
    uint16_t flip_h     : 1;   // Bit 15: flip horizontal
    // Nota: flip vertical está no bit 15 do high byte (16-bit entry)
} TilemapEntry;
```

### 3.6 VRAM Layout Constants

```c
// Layout de VRAM do Zelda: A Link to the Past
#define VRAM_BG1_TILES     0x0000   // BG1 tile data
#define VRAM_BG1_MAP       0x1000   // BG1 tilemap
#define VRAM_BG2_TILES     0x0000   // BG2 tile data
#define VRAM_BG2_MAP       0x0800   // BG2 tilemap
#define VRAM_BG3_TILES     0x4000   // BG3 tile data (HUD)
#define VRAM_BG3_MAP       0x5800   // BG3 tilemap (HUD)
#define VRAM_SPRITES       0x6000   // Sprite tiles
#define VRAM_SPRITES_MAP   0x7800   // Sprite maps

// Tamanhos
#define VRAM_SIZE          0x10000  // 64 KB
#define BG1_MAP_SIZE       0x0800   // 2 KB tilemap
#define BG3_MAP_SIZE       0x0800   // 2 KB tilemap

// VRAM control modes
#define VRAM_MODE_8BIT     0x80     // Word increment (main mode)
#define VRAM_MODE_4BIT     0x00     // Byte increment

// Endereços VRAM usados pelo DMA
typedef enum {
    VRAM_ADDR_BG1_MAP  = 0x0000,
    VRAM_ADDR_BG1_TILES = 0x0000,
    VRAM_ADDR_BG2_MAP  = 0x4000,
    VRAM_ADDR_BG2_TILES = 0x2000,
    VRAM_ADDR_BG3_MAP  = 0x5800,
    VRAM_ADDR_BG3_TILES = 0x4000,
    VRAM_ADDR_SPRITES  = 0x6000,
} VRAMAddress;
```

---

## 4. Sistemas do Jogo em C

### 4.1 Game State Machine

```c
typedef enum {
    STATE_BOOT,
    STATE_TITLE_SCREEN,
    STATE_OVERWORLD,
    STATE_DUNGEON,
    STATE_CUTSCENE,
    STATE_MENU,
    STATE_GAME_OVER,
    STATE_SAVING,
    STATE_TRANSITION,
} GameState;

typedef struct {
    GameState current_state;
    GameState next_state;
    uint8_t sub_state;
    uint8_t transition_counter;
} StateMachine;

void main_game_logic(void) {
    switch (global.game_mode) {
        case 0x00:  // Init/boot
            init_game_state();
            break;
        case 0x01:  // Secondary mode
            run_secondary_mode();
            break;
        case 0x02:  // VRAM DMA
            dma_vram_transfer();
            break;
        // ... 13 modos restantes
    }
}
```

### 4.2 NMI Handler → vblank_callback()

```c
void vblank_callback(void) {
    // 1. Reconhece NMI
    io_read(0x4210);  // acknowledge
    
    // 2. Handshake APU
    apu_handshake();
    
    // 3. Force blank (prepara para DMA)
    io_write(0x2100, 0x80);
    
    // 4. Desabilita HDMA
    io_write(0x420C, 0x00);
    
    // 5. Primeira frame: init
    if (shadow.first_frame) {
        initial_dma_setup();
        read_controllers();
        shadow.first_frame = false;
    }
    
    // 6. Se NMI skip flag, usa caminho rápido
    if (shadow.nmi_skip != 0) {
        nmi_fast_path();
        return;
    }
    
    // 7. Escreve registradores PPU
    io_write(0x2123, shadow.w12sel);
    io_write(0x212C, shadow.main_screen);
    io_write(0x212D, shadow.sub_screen);
    
    // 8. Scroll registers
    io_write(0x210D, shadow.scroll_bg1_h & 0xFF);
    io_write(0x210D, shadow.scroll_bg1_h >> 8);
    // ... BG1V, BG2H, BG2V, BG3H, BG3V
    
    // 9. BG Mode e Mosaic
    io_write(0x2105, shadow.bg_mode);
    io_write(0x2106, shadow.mosaic);
    
    // 10. Mode 7 (condicional)
    if (shadow.mode7_enabled) {
        io_write(0x211B, shadow.m7a & 0xFF);
        io_write(0x211B, shadow.m7a >> 8);
        // ... M7B, M7C, M7D, M7X, M7Y
    }
    
    // 11. H-Counter + HDMA trigger
    io_write(0x4209, 0xFF);
    io_write(0x420A, 0x00);
    io_write(0x420C, shadow.hdma_enable);
}
```

### 4.3 DMA Engine

```c
// DMA multi-canal (5 canais)
typedef struct {
    uint8_t channel;
    uint8_t mode;
    uint32_t src_addr;
    uint16_t dst_ppu;
    uint16_t size;
} DMATransfer;

// Engine DMA do Zelda
void dma_engine_run(void) {
    // Canal 0: Tilemap data
    if (dma_queue[0].active) {
        dma_transfer(
            dma_queue[0].src_addr,
            dma_queue[0].dst_ppu,
            dma_queue[0].size,
            0, DMA_MODE_2BYTE
        );
        dma_queue[0].active = false;
    }
    
    // Canal 1: Tile data
    if (dma_queue[1].active) {
        dma_transfer(
            dma_queue[1].src_addr,
            dma_queue[1].dst_ppu,
            dma_queue[1].size,
            1, DMA_MODE_1BYTE
        );
        dma_queue[1].active = false;
    }
    
    // Canais 2-4: efeitos, paleta, etc.
    // ...
    
    // Fire todos de uma vez
    io_write(0x420B, active_mask);
}
```

### 4.4 Input System

```c
typedef struct {
    uint16_t held;       // Botões pressionados (frame atual)
    uint16_t newly;      // Botões recém-pressionados (rising edge)
    uint16_t previous;   // Botões do frame anterior
} InputState;

// SNES button masks
#define BTN_B        0x8000
#define BTN_Y        0x4000
#define BTN_SELECT   0x2000
#define BTN_START    0x1000
#define BTN_UP       0x0800
#define BTN_DOWN     0x0400
#define BTN_LEFT     0x0200
#define BTN_RIGHT    0x0100
#define BTN_A        0x0080
#define BTN_X        0x0040
#define BTN_L        0x0020
#define BTN_R        0x0010

InputState joypad1;
InputState joypad2;

void read_controllers(void) {
    // Lê registradores SNES ($4218-$421B)
    uint16_t hw_joypad1 = (io_read(0x4219) << 8) | io_read(0x4218);
    uint16_t hw_joypad2 = (io_read(0x421B) << 8) | io_read(0x421A);
    
    // Atualiza joypad 1
    joypad1.previous = joypad1.held;
    joypad1.held = hw_joypad1;
    joypad1.newly = (joypad1.held ^ joypad1.previous) & joypad1.held;
    
    // Atualiza joypad 2
    joypad2.previous = joypad2.held;
    joypad2.held = hw_joypad2;
    joypad2.newly = (joypad2.held ^ joypad2.previous) & joypad2.held;
}
```

### 4.5 Collision System

```c
typedef struct {
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
} BoundingBox;

// Colisão tile-based (do código $01:893C)
typedef struct {
    uint8_t collision_type;  // 2 bits por tile
    uint8_t solid_h;         // $B2: colisão horizontal
    uint8_t solid_v;         // $B4: colisão vertical
} TileCollision;

// Colisão AABB (do código $01:B1E7)
bool check_collision_aabb(const BoundingBox *a, const BoundingBox *b) {
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

// Colisão tile do mapa (do código $01:CCA1)
TileCollision get_tile_collision(int x, int y) {
    uint8_t tile_data = read_map_tile(x, y);
    
    TileCollision col;
    col.collision_type = tile_data & 0x03;  // 2 bits
    col.solid_h = (tile_data & 0xFC) == 0xFC;
    col.solid_v = (tile_data & 0xFC) == 0xFC;
    return col;
}
```

### 4.6 Room Loading

```c
typedef struct {
    uint16_t room_id;
    uint8_t map_type;         // 0=overworld, 1=dungeon
    uint32_t tileset_addr;
    uint32_t tilemap_addr;
    uint32_t entity_list_addr;
    uint8_t palette_id;
    uint8_t bg1_scroll_h;
    uint8_t bg1_scroll_v;
    uint8_t bg2_scroll_h;
    uint8_t bg2_scroll_v;
} RoomHeader;

void load_room(uint16_t room_id) {
    // Lê cabeçalho da sala da ROM
    RoomHeader *header = get_room_header(room_id);
    
    // 1. Carrega tileset (tiles comprimidos)
    decompress_tiles(header->tileset_addr);
    
    // 2. Carrega tilemap
    load_tilemap(header->tilemap_addr);
    
    // 3. Carrega paleta
    load_palette(header->palette_id);
    
    // 4. Configura scroll
    shadow.scroll_bg1_h = header->bg1_scroll_h;
    shadow.scroll_bg1_v = header->bg1_scroll_v;
    
    // 5. Inicializa entidades
    init_room_entities(header->entity_list_addr);
    
    // 6. DMA para VRAM
    dma_room_to_vram();
}
```

### 4.7 APU Communication

```c
// Comunicação via portas $2140-$2143
typedef struct {
    uint8_t port0_send;   // $012C
    uint8_t port1_send;   // $012D
    uint8_t port2_send;   // $012E
    uint8_t port3_send;   // $012F
    uint8_t port0_recv;   // $0133
    uint8_t port1_recv;   // $0131
    uint8_t special_cmd;  // $0130
} APUState;

APUState apu;

void apu_handshake(void) {
    // Envia comando
    io_write(0x2140, apu.port0_send);
    io_write(0x2141, apu.port1_send);
    io_write(0x2142, apu.port2_send);
    io_write(0x2143, apu.port3_send);
    
    // Recebe resposta
    apu.port0_recv = io_read(0x2140);
    apu.port1_recv = io_read(0x2141);
    
    // Handshake especial para comandos $F2+
    if (apu.port0_recv == 0xF2) {
        apu.special_cmd = apu.port0_recv;
    }
}

void apu_send_command(uint8_t cmd, uint8_t data) {
    apu.port0_send = cmd;
    apu.port1_send = data;
}

// Block transfer para SPC700
void spc700_block_transfer(const uint8_t *data, uint16_t addr, uint16_t size) {
    // Espera handshake
    while (io_read(0x2140) != apu.port0_recv) {}
    
    // Envia comando $CC (block transfer mode)
    io_write(0x2141, 0xCC);
    io_write(0x2140, 0x01);
    
    // Envia dados byte-a-byte
    for (uint16_t i = 0; i < size; i++) {
        io_write(0x2140, addr & 0xFF);
        io_write(0x2141, addr >> 8);
        io_write(0x2142, data[i]);
        addr++;
    }
    
    // Zera portas
    io_write(0x2140, 0);
    io_write(0x2141, 0);
}
```

### 4.8 Sprite System

```c
#define MAX_SPRITES 128
#define SPRITE_OFFSCREEN_Y 0xF0

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t tile;
    uint8_t attributes;
    uint8_t type;
    uint8_t state;
    uint8_t health;
    uint8_t speed;
    int8_t  vel_x;
    int8_t  vel_y;
    BoundingBox hitbox;
} Sprite;

Sprite sprites[MAX_SPRITES];

void update_sprites(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].type == 0) continue;  // vazio
        
        // Move entidade
        sprites[i].x += sprites[i].vel_x;
        sprites[i].y += sprites[i].vel_y;
        
        // Atualiza OAM
        oam_buffer.entries[i].x = sprites[i].x & 0xFF;
        oam_buffer.entries[i].y = sprites[i].y & 0xFF;
        oam_buffer.entries[i].tile = sprites[i].tile;
        oam_buffer.entries[i].attributes = sprites[i].attributes;
    }
}

void hide_unused_sprites(void) {
    // $00:841E — oculta sprites não usados definindo Y = $F0
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].type == 0) {
            oam_buffer.entries[i].y = SPRITE_OFFSCREEN_Y;
        }
    }
}
```

### 4.9 Palette System

```c
#define PALETTE_BG_COUNT    8   // 8 sub-paletas BG
#define PALETTE_SPRITE_COUNT 8  // 8 sub-paletas Sprite
#define COLORS_PER_PALETTE  16  // 16 cores por sub-palette

typedef struct {
    uint16_t snes_colors[256];  // 128 BG + 128 Sprite (SNES format)
} PaletteBuffer;

PaletteBuffer palette_buffer;  // Shadow para upload

void load_palette(uint16_t palette_id) {
    // Busca offset da paleta na ROM
    uint32_t rom_offset = palette_table_offset + (palette_id * 512);
    
    // Copia para buffer shadow
    memcpy(palette_buffer.snes_colors, &rom[rom_offset], 512);
    
    // Marca como suja
    shadow.palette_dirty = 1;
}

// Chamado durante NMI
void upload_palette(void) {
    if (!shadow.palette_dirty) return;
    
    io_write(0x2121, 0x00);  // CGRAM address = 0
    
    for (int i = 0; i < 256; i++) {
        uint16_t color = palette_buffer.snes_colors[i];
        io_write(0x2122, color & 0xFF);        // Low byte
        io_write(0x2122, (color >> 8) & 0x7F); // High byte (bit 7 = 0)
    }
    
    shadow.palette_dirty = 0;
}
```

---

## 5. Conversão de Endereços SNES

### 5.1 LoROM Address Translation

**Fórmula LoROM:**

```
ROM File Offset = Bank × 0x8000 + (SNES Address - 0x8000)
```

**Exemplos:**

| SNES Address | Bank | Offset | Cálculo |
|---|---|---|---|
| $00:8000 | $00 | $0000 | 0×0x8000 + (0x8000-0x8000) |
| $00:822C | $00 | $022C | 0×0x8000 + (0x822C-0x8000) |
| $00:FFFF | $00 | $7FFF | 0×0x8000 + (0xFFFF-0x8000) |
| $01:8000 | $01 | $8000 | 1×0x8000 + (0x8000-0x8000) |
| $08:8000 | $08 | $40000 | 8×0x8000 + (0x8000-0x8000) |
| $10:8000 | $10 | $80000 | 16×0x8000 + (0x8000-0x8000) |

**Função C:**

```c
uint32_t snes_to_rom_offset(uint8_t bank, uint16_t addr) {
    // LoROM: bank × 0x8000 + (addr - 0x8000)
    if (addr < 0x8000) {
        // Low mirror (ram, io) — não existe na ROM
        return 0xFFFFFFFF;
    }
    return (uint32_t)bank * 0x8000 + (addr - 0x8000);
}

// Inverso: ROM offset → SNES address
void rom_offset_to_snes(uint32_t offset, uint8_t *bank, uint16_t *addr) {
    *bank = offset / 0x8000;
    *addr = (offset % 0x8000) + 0x8000;
}
```

### 5.2 Tabela de Registradores PPU para Constantes

```c
// Registradores PPU ($2100-$213F)
typedef enum {
    // Display
    PPU_INIDISP   = 0x2100,  // Screen display / force blank
    PPU_OBJSEL    = 0x2101,  // Object size + base address
    PPU_OAMADDL   = 0x2102,  // OAM address low
    PPU_OAMADDH   = 0x2103,  // OAM address high
    PPU_OAMDATA   = 0x2104,  // OAM data write
    
    // Background
    PPU_BGMODE    = 0x2105,  // BG mode + character size
    PPU_MOSAIC    = 0x2106,  // Mosaic size
    PPU_BG1SC     = 0x2107,  // BG1 tilemap address
    PPU_BG2SC     = 0x2108,  // BG2 tilemap address
    PPU_BG3SC     = 0x2109,  // BG3 tilemap address
    PPU_BG4SC     = 0x210A,  // BG4 tilemap address
    PPU_BG12NBA   = 0x210B,  // BG1/BG2 character data address
    PPU_BG34NBA   = 0x210C,  // BG3/BG4 character data address
    
    // Scroll
    PPU_BG1HOFS   = 0x210D,  // BG1 horizontal scroll (write x2)
    PPU_BG1VOFS   = 0x210E,  // BG1 vertical scroll (write x2)
    PPU_BG2HOFS   = 0x210F,  // BG2 horizontal scroll
    PPU_BG2VOFS   = 0x2110,  // BG2 vertical scroll
    PPU_BG3HOFS   = 0x2111,  // BG3 horizontal scroll
    PPU_BG3VOFS   = 0x2112,  // BG3 vertical scroll
    PPU_BG4HOFS   = 0x2113,  // BG4 horizontal scroll
    PPU_BG4VOFS   = 0x2114,  // BG4 vertical scroll
    
    // VRAM
    PPU_VMAIN     = 0x2115,  // VRAM address increment mode
    PPU_VRAMADDL  = 0x2116,  // VRAM address low
    PPU_VRAMADDH  = 0x2117,  // VRAM address high
    PPU_VRAMDATAW = 0x2118,  // VRAM data write low
    PPU_VRAMDATAH = 0x2119,  // VRAM data write high
    
    // Mode 7
    PPU_M7SEL     = 0x211A,  // Mode 7 settings
    PPU_M7A       = 0x211B,  // Mode 7 matrix A (write x2)
    PPU_M7B       = 0x211C,  // Mode 7 matrix B
    PPU_M7C       = 0x211D,  // Mode 7 matrix C
    PPU_M7D       = 0x211E,  // Mode 7 matrix D
    PPU_M7X       = 0x211F,  // Mode 7 center X
    PPU_M7Y       = 0x2120,  // Mode 7 center Y
    
    // Color
    PPU_CGADD     = 0x2121,  // CGRAM address
    PPU_CGDATAW   = 0x2122,  // CGRAM data write
    
    // Window
    PPU_W12SEL    = 0x2123,  // Window 1/2 settings BG1/2
    PPU_W34SEL    = 0x2124,  // Window 3/4 settings BG3/4
    PPU_WOBJSEL   = 0x2125,  // Window settings OBJ
    
    // Screen designation
    PPU_TM        = 0x212C,  // Main screen designation
    PPU_TS        = 0x212D,  // Sub screen designation
    PPU_TMW       = 0x212E,  // Window mask for main
    PPU_TSW       = 0x212F,  // Window mask for sub
    
    // Color math
    PPU_CGWSEL    = 0x2130,  // Color math control
    PPU_CGADSUB   = 0x2131,  // Color math add/subtract
    PPU_COLDATA   = 0x2132,  // Fixed color data
} PPURegister;

// CPU I/O ($4200-$44FF)
typedef enum {
    CPU_NMITIMEN  = 0x4200,  // NMI/Timer enable
    CPU_WRIO      = 0x4201,  // Joypad programmable I/O
    CPU_WRMPYA    = 0x4202,  // Multiplicand A
    CPU_WRMPYB    = 0x4203,  // Multiplicand B
    CPU_RDMPYL    = 0x4214,  // Quotient low
    CPU_RDMPYH    = 0x4215,  // Quotient high
    CPU_RDIVL     = 0x4214,  // Division result low
    CPU_RDIVH     = 0x4215,  // Division result high
    CPU_HVBJOY    = 0x4212,  // H/V blank + joypad busy
    
    // Joypad auto-read
    CPU_JOY1L     = 0x4218,  // Joypad 1 low byte
    CPU_JOY1H     = 0x4219,  // Joypad 1 high byte
    CPU_JOY2L     = 0x421A,  // Joypad 2 low byte
    CPU_JOY2H     = 0x421B,  // Joypad 2 high byte
    
    // DMA
    CPU_DMAEN     = 0x420B,  // DMA enable (trigger)
    CPU_HDMAEN    = 0x420C,  // HDMA enable
    
    // DMA channel 0
    CPU_DMAP0     = 0x4300,  // DMA mode
    CPU_BBAD0     = 0x4301,  // B-bus address
    CPU_A1T0L     = 0x4302,  // A-bus address low
    CPU_A1T0H     = 0x4303,  // A-bus address high
    CPU_A1B0      = 0x4304,  // A-bus bank
    CPU_DAS0L     = 0x4305,  // DMA size low
    CPU_DAS0H     = 0x4306,  // DMA size high
} CPURegister;

// APU ports
typedef enum {
    APU_PORT0     = 0x2140,  // Communication port 0
    APU_PORT1     = 0x2141,  // Communication port 1
    APU_PORT2     = 0x2142,  // Communication port 2
    APU_PORT3     = 0x2143,  // Communication port 3
} APURegister;
```

### 5.3 Endereçamento 65816 → C

```c
// Shadow RAM: endereço RAM → offset no array
uint8_t* get_ram_ptr(uint16_t addr) {
    if (addr < 0x2000) return &wram_low[addr];
    if (addr >= 0x7E0000 && addr < 0x800000) {
        return &wram_extended[addr - 0x7E0000];
    }
    return NULL;  // I/O ou ROM
}

// Read bus (simula leitura de qualquer endereço)
uint8_t read_bus(uint32_t addr) {
    uint8_t bank = (addr >> 16) & 0xFF;
    uint16_t offset = addr & 0xFFFF;
    
    // WRAM low ($0000-$1FFF)
    if (offset < 0x2000) return wram_low[offset];
    
    // PPU I/O ($2100-$21FF)
    if (offset >= 0x2100 && offset < 0x2200) {
        return ppu_read(offset);
    }
    
    // CPU I/O ($4200-$44FF)
    if (offset >= 0x4200 && offset < 0x4500) {
        return cpu_io_read(offset);
    }
    
    // ROM ($8000-$FFFF)
    if (offset >= 0x8000) {
        uint32_t rom_off = snes_to_rom_offset(bank, offset);
        if (rom_off < ROM_SIZE) return rom[rom_off];
    }
    
    // WRAM extended ($7E0000-$7FFFFF)
    if (bank == 0x7E || bank == 0x7F) {
        return wram_extended[(bank - 0x7E) * 0x10000 + offset];
    }
    
    return 0xFF;
}
```

---

## 6. Plano de Decompição

### 6.1 Ordem de Prioridade dos Sistemas

```
FASE 1 — Fundação (2-3 semanas)
├── [1.1] CPU context struct + I/O simulation
├── [1.2] RAM/ROM memory model
├── [1.3] Basic instruction emulation (LDA, STA, JMP, JSR, branches)
└── [1.4] Output: RESET → Main Loop executando

FASE 2 — Interações (2-3 semanas)
├── [2.1] NMI handler → vblank_callback()
├── [2.2] DMA engine
├── [2.3] Input system (read_controllers)
└── [2.4] Output: frame render loop funcionando

FASE 3 — Gameplay (3-4 semanas)
├── [3.1] Game state machine
├── [3.2] Sprite system + OAM
├── [3.3] Collision system
├── [3.4] Room loading
└── [3.5] Output: sala inicial carregando

FASE 4 — Mundo (3-4 semanas)
├── [4.1] Overworld map system
├── [4.2] Dungeon system
├── [4.3] Room transitions
└── [4.4] Output: navegação entre salas

FASE 5 — Áudio (2-3 semanas)
├── [5.1] APU communication protocol
├── [5.2] SPC700 boot/upload
├── [5.3] Sound command dispatch
└── [5.4] Output: música e SFX funcionando

FASE 6 — Polish (2-4 semanas)
├── [6.1] Full gameplay integration
├── [6.2] Performance optimization
├── [6.3] Save/load state
└── [6.4] Platform abstraction (SDL2, OpenGL, etc.)
```

### 6.2 Quais Sistemas Converter Primeiro

**Por que este ordem:**

1. **Init/Reset** — é o ponto de entrada, sem ele nada roda
2. **NMI + DMA** — são o heartbeat do SNES; sem VBlank, nenhum gráfico aparece
3. **Input** — permite interação básica (testar se o game loop responde)
4. **Game state machine** — orquestra tudo; converter early permite isolar sistemas
5. **Sprites + OAM** — visual feedback imediato
6. **Room loading** — permite ver o mundo

### 6.3 Dependências entre Sistemas

```
                    ┌─────────────┐
                    │   RESET/Init │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │SPC700 Boot│ │RAM Clear │ │VRAM Init │
        └─────┬────┘ └──────────┘ └─────┬────┘
              │                          │
              ▼                          ▼
        ┌──────────┐              ┌──────────┐
        │  APU Comm │              │ DMA Engine│
        └──────────┘              └─────┬────┘
                                        │
              ┌─────────────────────────┤
              │                         │
              ▼                         ▼
        ┌──────────┐              ┌──────────┐
        │  Palette  │              │ OAM/Sprite│
        └──────────┘              └──────────┘
              │                         │
              └──────────┬──────────────┘
                         │
                         ▼
                  ┌─────────────┐
                  │ Game State  │
                  │  Machine    │
                  └──────┬──────┘
                         │
           ┌─────────────┼─────────────┐
           │             │             │
           ▼             ▼             ▼
     ┌──────────┐ ┌──────────┐ ┌──────────┐
     │ Room Load│ │  Input   │ │ Collision│
     └────┬─────┘ └──────────┘ └──────────┘
          │
          ▼
    ┌──────────┐
    │ Overworld │
    │ Dungeon   │
    └──────────┘
```

### 6.4 Como Testar Cada Sistema

**Fase 1 — Smoke Test:**

```c
// Verificar que o loop básico funciona
int main(void) {
    snes_reset();
    
    // Verifica: NMI foi habilitado?
    assert(cpu.flag_i == false);
    assert(ppu.nmitimen == 0x81);
    
    // Verifica: RAM foi zerada?
    for (int i = 0x0400; i < 0x2000; i++) {
        assert(wram_low[i] == 0);
    }
    
    // Verifica: SPC700 boot enviou dados
    assert(apu.port0_send != 0 || apu.port1_send != 0);
    
    printf("Smoke test passed!\n");
    return 0;
}
```

**Fase 2 — Frame Test:**

```c
// Verificar que NMI produz frame válido
void test_frame_rendering(void) {
    // Executa 3 frames
    for (int f = 0; f < 3; f++) {
        global.nmi_flag = 1;
        main_loop_iteration();  // uma iteração do loop
    }
    
    // Verifica: scroll registers foram escritos
    assert(ppu.bg1hofs != 0 || ppu.bg1vofs != 0);
    
    // Verifica: OAM foi atualizado
    assert(oam_buffer.entries[0].y != 0xF0 || 
           sprites[0].type != 0);
}
```

**Fase 3 — Compare com Screenshot:**

```c
// Renderizar frame e comparar com reference
void compare_frame(uint32_t frame_number, const char *ref_image) {
    // Renderiza para buffer
    render_frame_to_buffer(frame_buffer);
    
    // Compara pixel-a-pixel
    uint32_t diff = compare_buffers(frame_buffer, load_reference(ref_image));
    
    if (diff > 100) {  // tolerance
        printf("Frame %d: %u pixel differences!\n", frame_number, diff);
        save_debug_image(frame_buffer, "debug_frame.bmp");
    }
}
```

### 6.5 Estrutura de Projeto Recomendada

```
zelda-lttp-c/
├── src/
│   ├── main.c                 // Entry point
│   ├── cpu/
│   │   ├── cpu.h              // CPU context struct
│   │   ├── cpu.c              // Instruction implementation
│   │   └── instructions.c     // All 65816 instructions
│   ├── memory/
│   │   ├── memory.h           // Address space model
│   │   ├── memory.c           // read/write bus
│   │   └── rom.c              // ROM loading
│   ├── ppu/
│   │   ├── ppu.h              // PPU registers + rendering
│   │   ├── ppu.c              // PPU emulation
│   │   └── sprites.c          // OAM + sprite rendering
│   ├── audio/
│   │   ├── apu.h              // APU interface
│   │   ├── apu.c              // APU communication
│   │   └── spc700.c           // SPC700 boot/upload
│   ├── game/
│   │   ├── game_state.c       // State machine
│   │   ├── input.c            // Controller input
│   │   ├── collision.c        // Collision detection
│   │   ├── room.c             // Room loading
│   │   └── entities.c         // Entity system
│   ├── systems/
│   │   ├── dma.c              // DMA engine
│   │   ├── palette.c          // Palette management
│   │   └── decompress.c       // LZ decompression
│   └── platform/
│       ├── platform.h         // Platform abstraction
│       └── sdl2_backend.c     // SDL2 implementation
├── include/
│   ├── snes.h                 // SNES constants
│   ├── zelda.h                // Game-specific types
│   └── config.h               // Build configuration
├── data/
│   └── zelda.sfc              // ROM file
├── tests/
│   ├── test_cpu.c
│   ├── test_memory.c
│   └── test_ppu.c
└── CMakeLists.txt
```

---

## 7. Referências

### 7.1 Projetos de Decompilação SNES

| Projeto | Jogo | Linguagem | URL |
|---|---|---|---|
| **Zelda3** | Zelda: A Link to the Past | C (parcial) | github.com/snesrev/zelda3 |
| **SM64** | Super Mario 64 | C (completo) | github.com/n64decomp/sm64 |
| **OOT** | Ocarina of Time | C (completo) | github.com/zeldaret/oot |
| **KMFDManic** | SNES Decomps | Various | github.com/KMFDManic |
| **ctdp** | Chrono Trigger | C | github.com/akouzoukos/snesdisasm |

### 7.2 Documentação 65816

- **WDC 65C816 Datasheet** — Documentação oficial do processador
- **SNES Developer Manual** — Guia completo do hardware SNES
- **Fullsnes by byuu** — Enciclopédia mais completa do hardware SNES
- **Anomie's SNES Docs** — Detalhes técnicos dos registradores PPU
- **nocash SNES specs** — Documentação completa de todos os registradores

### 7.3 Guias de Decompilação

- **HarmlessLugia's SNES decomp guide** — Passo-a-passo de decompilação
- **SM64 decomp wiki** — Metodologia usada no SM64 (referência para qualquer decomp)
- **Zelda3 project README** — Abordagem específica para Zelda
- **asar assembler docs** — Para reassemblar e testar contra a ROM original

### 7.4 Ferramentas

| Ferramenta | Uso |
|---|---|
| **asar** | Assembler 65816 para reassemblar e checksum |
| **bsnes/higan** | Emulador ciclo-exato para debugging |
| **no$sns** | Emulador com debug symbols |
| **Tilemap Studio** | Visualizar tilemaps e tiles |
| **SNES Palconv** | Converter paletas |
| **SFX Sound Resource Editor** | Analisar samples BRR |

---

## A. Glossário 65816 → C

| Termo 65816 | Equivalente C |
|---|---|
| Accumulator | Variável `A` ou resultado de operação |
| Addressing mode | Forma de acessar operandos |
| Bank | Segmento de 64 KB (256 bancos × 64 KB = 16 MB) |
| Branch | `if (condition) goto label;` ou `if (!condition) return;` |
| Byte | `uint8_t` |
| Carry flag | `bool flag_c` |
| Direct Page | `dp` register → base para addressing rápido |
| Emulation mode | `cpu.emulation = true` (65C02 compat) |
| Half-carry | Não existe no 65816 |
| Index register | `uint16_t X` ou `uint16_t Y` |
| Interrupt | Callback assíncrono (NMI/IRQ) |
| Little-endian | Ordem de bytes igual ao x86 |
| Long address | Endereço de 24 bits (bank + addr) |
| Native mode | `cpu.emulation = false` (65816 nativo) |
| Negative flag | `bool flag_n` (bit 7/15 do resultado) |
| Page | Bloco de 256 bytes (direct page = 256 bytes) |
| Stack | Array de 256 bytes em WRAM $0100-$01FF |
| Vector | Endereço de handler de interrupção |
| Word | `uint16_t` (16 bits no 65816) |
| Zero flag | `bool flag_z` (resultado == 0) |
