# Formato da ROM - The Legend of Zelda: A Link to the Past

## Visão Geral

Este documento descreve o formato da ROM de The Legend of Zelda: A Link to the Past para SNES.

## Informações da ROM

| Campo | Valor |
|-------|-------|
| Nome | THE LEGEND OF ZELDA |
| Tamanho | 1 MB (1048576 bytes) |
| Mapeamento | LoROM |
| País | Suécia (0x03) |
| Publisher | Nintendo (0x01) |
| Versão | 1.1 |
| Checksum | 0x50F2 |
| Checksum Complement | 0xAF0D |

## Estrutura da ROM

### LoROM Memory Map

A ROM usa o mapeamento LoROM, onde a memória é dividida em bancos de 32KB:

| Banco | Endereço | Descrição |
|-------|----------|-----------|
| $00-$3F | $8000-$FFFF | ROM (primeiros 32MB) |
| $40-$7D | $0000-$FFFF | ROM (próximos 32MB) |
| $80-$FF | $0000-$FFFF | Espelho de $00-$7D |

### Header da ROM

O header está localizado em $7FC0-$7FFF:

| Offset | Tamanho | Descrição |
|--------|---------|-----------|
| $00-$14 | 21 bytes | Nome do jogo |
| $15 | 1 byte | Modo de mapeamento (0x20 = LoROM) |
| $16 | 1 byte | Tamanho da ROM (0x02 = 4 MB) |
| $17 | 1 byte | Tamanho da RAM (0x0A = 1 MB) |
| $18 | 1 byte | País/Região |
| $19 | 1 byte | Publisher |
| $1A | 1 byte | Versão |
| $1B-$1C | 2 bytes | Checksum |
| $1D-$1E | 2 bytes | Checksum Complement |

### Header Checksum

O checksum é calculado sobre todos os bytes da ROM. O checksum complement é o complemento de 16 bits do checksum, de modo que:
```
checksum + complement = 0xFFFF
```

## Segmentos da ROM

### Banco $00-$3F (LoROM)

Cada banco tem 32KB ($8000-$FFFF):

| Endereço | Tamanho | Descrição |
|----------|---------|-----------|
| $0000-$7FFF | 32KB | RAM baixa (mapeada externamente) |
| $8000-$FFFF | 32KB | ROM |

### Estrutura de Dados

#### Tiles e Sprites

- **Tiles**: Blobs de 8x8 pixels em 4bpp (32 bytes por tile)
- **Sprites**: Conjuntos de tiles com informações de posição e atributos

#### Mapas de Background

- **Mapas**: Grids de tiles de 32x32 tiles cada
- **Atributos**: Flags de prioridade, flip horizontal/vertical

#### Paletas

- **CGRAM**: 256 cores, 15 bits cada (5 bits por canal RGB)
- **Formato**: $gggrrrrr $0bbbbbgg

#### Áudio

- **Samples**: Dados de áudio 8-bit signed
- **Sequências**: Dados de nota e duração
- **Instrumentos**: Parâmetros de timbre e envelope

## Análise Específica do Zelda

### Banco Inicial ($00)

O código do jogo começa no banco $00:

| Offset | Tamanho | Descrição |
|--------|---------|-----------|
| $8000-$FFFF | 32KB | Código principal do jogo |

### Vetores de Interrupção

| Endereço | Descrição |
|----------|-----------|
| $FFFA-$FFFB | Vetor NMI |
| $FFFC-$FFFD | Vetor Reset |
| $FFFE-$FFFF | Vetor IRQ/BRK |

### Dados de Exemplo

#### Código de Inicialização (primeiras instruções)

```asm
$008000: SEI              ; Desabilitar interrupções
$008001: STZ $4200        ; Limpar NMI enable
$008004: STZ $420C        ; Limpar HDMA enable
$008007: STZ $420B        ; Limpar DMA enable
$00800A: STZ $2140        ; Limpar comunicação APU
$00800D: STZ $2141
$008010: STZ $2142
$008013: STZ $2143
$008016: LDA #$80         ; Forçar blanking
$008018: STA $2100
$00801B: CLC              ; Limpar carry
$00801C: XCE              ; Trocar carry por emulation mode
```

## Ferramentas de Análise

### Tools Python

O projeto inclui as seguintes ferramentas:

1. **rom_parser.py** - Analisa o header da ROM
2. **asset_extractor.py** - Extrai tiles e paletas
3. **audio_extractor.py** - Extrai samples de áudio
4. **disassembler.py** - Converte binário para Assembly

### Uso das Ferramentas

```bash
# Analisar ROM
python tools/parser/rom_parser.py "roms/Legend of Zelda, The - A Link to the Past (USA).sfc"

# Extrair tiles
python tools/extractor/asset_extractor.py "roms/Legend of Zelda, The - A Link to the Past (USA).sfc"

# Extrair áudio
python tools/extractor/audio_extractor.py "roms/Legend of Zelda, The - A Link to the Past (USA).sfc"

# Desassemblar código
python tools/disassembler/disassembler.py "roms/Legend of Zelda, The - A Link to the Past (USA).sfc"
```

## Próximos Passos

1. **Mapeamento completo** - Identificar todos os offsets de dados
2. **Extração de tiles** - Encontrar e extrair todos os tiles do jogo
3. **Extração de paletas** - Mapear todas as paletas usadas
4. **Extração de áudio** - Identificar todos os samples e sequências
5. **Desassemblagem completa** - Desassemblar todo o código do jogo

## Referências

1. SNES ROM Header Format - https://problemkaputt.de/fullsnes.htm
2. LoROM Memory Map - SNES Development Manual
3. SNES Tile Format - https://www.chibiakumas.com/65816/platform3.php