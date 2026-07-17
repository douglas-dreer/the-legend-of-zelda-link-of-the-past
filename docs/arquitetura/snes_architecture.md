# Arquitetura SNES - The Legend of Zelda: A Link to the Past

## Visão Geral

O Super Nintendo Entertainment System (SNES) é um console de 16 bits lançado em 1990. Este documento descreve a arquitetura do hardware e como ela se aplica ao jogo The Legend of Zelda: A Link to the Past.

## CPU - Ricoh 5A22

### Registradores

O 5A22 é baseado no 65816 (evolução do 6502), com os seguintes registradores:

| Registrador | Tamanho | Descrição |
|-------------|---------|-----------|
| A | 16-bit | Acumulador |
| X | 16-bit | Índice X |
| Y | 16-bit | Índice Y |
| S | 16-bit | Ponteiro de Pilha |
| D | 16-bit | Direta (base para endereçamento direto) |
| PB | 8-bit | Banco do Programa |
| DB | 8-bit | Banco de Dados |
| PC | 16-bit | Contador de Programa |
| P | 8-bit | Registro de Status |

### Modos de Operação

O 65816 pode operar em dois modos:
- **Modo 8-bit**: Compatível com o 6502 (A, X, Y de 8 bits)
- **Modo 16-bit**: A, X, Y podem ser 16 bits

### Registradores de Status (P)

| Bit | Nome | Descrição |
|-----|------|-----------|
| 0 | C | Carry |
| 1 | Z | Zero |
| 2 | I | IRQ Disable |
| 3 | D | Decimal Mode |
| 4 | X | Index Register Size (0=8bit, 1=16bit) |
| 5 | M | Accumulator Register Size (0=8bit, 1=16bit) |
| 6 | V | Overflow |
| 7 | N | Negative |

## Memória

### Mapa de Memória (LoROM)

Para o Zelda: A Link to the Past, que usa LoROM:

| Faixa | Tamanho | Descrição |
|-------|---------|-----------|
| $0000-$1FFF | 8KB | RAM Baixa (Zero Page + Stack + Direct Page) |
| $2000-$5FFF | 16KB | Hardware Registers (PPU, APU, DMA) |
| $6000-$7FFF | 8KB | RAM Expansão (SRAM) |
| $8000-$FFFF | 32KB | ROM (Bancos $00-$3F) |

### Bancos de Memória

| Banco | Endereço | Descrição |
|-------|----------|-----------|
| $00-$3F | $0000-$FFFF | LoROM: ROM mapeada em $8000-$FFFF |
| $40-$7D | $0000-$FFFF | LoROM: ROM mapeada em $0000-$FFFF |
| $80-$FF | $0000-$FFFF | Espelho de $00-$7D (acesso rápido) |

### LoROM vs HiROM

| Característica | LoROM | HiROM |
|----------------|-------|-------|
| Tamanho máximo | 2 MB | 4 MB |
| Header | $7FC0-$7FFF | $FFC0-$FFFF |
| Código | $8000-$FFFF | $0000-$FFFF |
| Vantagem | Mais simples | Mais espaço |

**O Zelda: A Link to the Past usa LoROM**

## PPU - Picture Processing Unit

### Registradores Principais

| Endereço | Nome | Descrição |
|----------|------|-----------|
| $2100 | INIDISP | Controle de Display |
| $2101 | OBJSEL | Seleção de OBJ |
| $2102-$2103 | OAMADD | Endereço OAM |
| $2104 | OAMDATA | Dados OAM |
| $2105 | BGMODE | Modo de Background |
| $2106 | MOSAIC | Efeito Mosaico |
| $2107-$210B | BGnSC | Endereço dos Backgrounds |
| $210C | OBJSEL | Seleção de OBJ |
| $210D-$2114 | BGnHOFS/BGnVOFS | Offset dos Backgrounds |
| $2115 | VMAIN | Modo de Auto-Incremento |
| $2116-$2117 | VMADD | Endereço VRAM |
| $2118-$2119 | VMDATA | Dados VRAM |
| $211A | M7SEL | Modo 7 Seleção |
| $211B-$2120 | M7A-M7F | Matriz Modo 7 |
| $2121 | CGADD | Endereço CGRAM |
| $2122 | CGDATA | Dados CGRAM |
| $2123-$2125 | TM/TS | Seleção de Main/Sub Screen |
| $212C-$212D | TM/TS | Seleção de Main/Sub Screen |
| $212E-$212F | TM/TS | Seleção de Main/Sub Screen |
| $2130 | CGWSEL | Controle de Color Math |
| $2131 | CGADSUB | Seleção de Color Math |
| $2132 | COLDATA | Cor para Color Math |
| $2133 | SETINI | Configuração de Display |

### Modos de Background

| Modo | Descrição | Usado em |
|------|-----------|----------|
| 0 | 4 backgrounds de 4 cores | Não usado no Zelda |
| 1 | 2 backgrounds de 16 cores + 1 de 4 cores | **Zelda usa este** |
| 2 | 2 backgrounds de 16 cores com offset | Não usado |
| 3 | 1 background de 256 cores + 1 de 16 cores | Não usado |
| 4 | 1 background de 256 cores + 1 de 4 cores | Não usado |
| 7 | Modo 7 (rotação/escala) | **Zelda usa este** |

### Tiles e Mapas

- **Tiles**: Blocos de 8x8 pixels
- **Mapas**: Grids de tiles que definem os backgrounds
- **CGRAM**: Paleta de cores (256 cores, 15 bits cada)

### Formato de Tile (4bpp)

Cada tile de 8x8 pixels em 4bpp usa 32 bytes:
- 2 bytes por linha × 8 linhas × 4 planos = 32 bytes

```
Plano 0: byte0 byte1
Plano 1: byte2 byte3
Plano 2: byte4 byte5
Plano 3: byte6 byte7
```

Cada byte contém 8 pixels (1 bit por pixel por plano).

## APU - Audio Processing Unit

### SPC700

O APU contém um processador SPC700 dedicado:
- CPU de 8 bits
- 64KB de RAM
- 4 canais de áudio
- DSP com efeitos

### Registradores SPC700

| Endereço | Nome | Descrição |
|----------|------|-----------|
| $2140-$2143 | CPU/SPC Communication | Comunicação entre CPU e SPC700 |

### Canais de Áudio

| Canal | Descrição |
|-------|-----------|
| 0 | VOICE 0 |
| 1 | VOICE 1 |
| 2 | VOICE 2 |
| 3 | VOICE 3 |

### Formato de Sample

- **Taxa**: 8-bit signed
- **Frequências**: 8000 Hz a 32000 Hz
- **Modulação**: Pitch modulável por canal

## DMA - Direct Memory Access

### Registradores DMA

| Endereço | Nome | Descrição |
|----------|------|-----------|
| $4200 | NMITIMEN | Controle de NMI/Timer |
| $4201-$4202 | WRIO/WRMPYA/WRMPYB | I/O e Multiplicação |
| $4203-$4206 | WRDIVL/WRDIVH/WRDIVB | Divisão |
| $4207-$420A | WRHDMAHL/WRVDMAHL | DMA Horizontal/Vertical |
| $420B | MDMAEN | Enable DMA |
| $420C | HDMAEN | Enable HDMA |
| $4300-$437F | DMA Registers | Registradores DMA por Canal |

### Modos de Transferência DMA

| Modo | Tamanho | Descrição |
|------|---------|-----------|
| 0 | 1 byte | 1 byte por write |
| 1 | 2 bytes | 2 bytes por write |
| 2 | 2 bytes | 2 bytes (byte 1, byte 1) |
| 3 | 4 bytes | 4 bytes por write |
| 4 | 4 bytes | 4 bytes (word, word) |
| 5 | 4 bytes | 4 bytes (byte, byte, byte, byte) |
| 6 | 2 bytes | 2 bytes (byte 1, byte 1) |
| 7 | 4 bytes | 4 bytes |

## Timer

### Registradores Timer

| Endereço | Nome | Descrição |
|----------|------|-----------|
| $4200 | NMITIMEN | Controle de Timer |
| $4207-$420A | WRHDMAHL/WRVDMAHL | Timer Horizontal/Vertical |

### Temporização

- **NMI**: Ocorre a cada scanline (1 scanline = ~3.5 μs)
- **Timer**: Configurável em 1-8 ms

## Interrupções

### NMI (Non-Maskable Interrupt)

- Ocorre no início de cada frame
- Usado para sincronização com o display
- Zelda: A Link to the Past usa NMI para:
  - Atualizar sprites
  - Trocar banks de tiles
  - Processar input do jogador
  - Atualizar estado do jogo

### IRQ (Maskable Interrupt)

- Pode ser configurado para ocorrer em scanlines específicas
- Usado para efeitos especiais (split screen)

## Mapa de Registradores - Zelda: A Link to the Past

### Endereços de Hardware Usados pelo Jogo

| Endereço | Uso no Zelda |
|----------|--------------|
| $2100 | Controle de display (fade in/out) |
| $2101 | Configuração de sprites |
| $2105 | Modo 1 (BGs) e Modo 7 |
| $2107-$210B | Endereços dos BGs na VRAM |
| $2115 | Modo de acesso VRAM |
| $2116-$2117 | Endereço VRAM |
| $2118-$2119 | Dados VRAM |
| $2121 | Endereço CGRAM |
| $2122 | Dados CGRAM |
| $2140-$2143 | Comunicação APU |
| $4200 | Controle NMI |
| $420B | DMA Enable |
| $4300-$437F | Registradores DMA |

## Referências

1. Fullsnes - https://problemkaputt.de/fullsnes.htm
2. SNES Development Manual - Nintendo
3. bsnes+ Documentation
4. SNES Hardware Reference