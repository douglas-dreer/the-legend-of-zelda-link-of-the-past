# Análise Geral da ROM - The Legend of Zelda: A Link to the Past

## Resumo Executivo

| Campo | Valor |
|-------|-------|
| **ROM** | THE LEGEND OF ZELDA: A LINK TO THE PAST |
| **Formato** | LoROM, 1MB (1048576 bytes) |
| **País** | USA/NTSC |
| **Checksum** | 0xAF0D (válido) |
| **Developer** | Nintendo (ID: 0x01) |

---

## 1. Vetores de Interrupção

| Vetor | Endereço SNES | ROM Offset | Propósito |
|-------|---------------|------------|-----------|
| **NMI** | `$822C` | `0x022C` | VBlank handler |
| **RESET** | `$8000` | `0x0000` | Inicialização power-on |
| **IRQ** | `$82D8` | `0x02D8` | H-IRQ/raster effects |

---

## 2. Rotinas Principais

### Sequência de Inicialização RESET

```
$8000: SEI              → Desabilita IRQs
$8001: STZ $4200        → Zera NMITIMEN
$8004: STZ $420C        → Desabilita HDMA
$8007: STZ $420B        → Desabilita DMA
$800A: STZ $2140-$2143  → Zera portas APU
$8016: LDA #$80         → Força blanking
$8018: STA $2100
$801B: CLC / XCE        → Muda para modo 65816 nativo
$801D: REP #$28         → Configura modo 16-bit
$8027: SEP #$30         → Volta para 8-bit A/X/Y
$8029: JSR $8901        → Chama init principal
$802C: JSR $87C0        → Inicialização complementar
$802F: LDA #$81         → Ativa NMI
$8031: STA $4200
```

### Tabela de Rotinas Identificadas

| Rotina | Endereço | Chamadas | Propósito |
|--------|----------|----------|-----------|
| Init Principal | `$8901` | 1 | Inicialização do jogo |
| Hardware Init | `$87C0` | 1 | Configuração de hardware |
| VBlank Transfer | `$9347` | Múltiplas | Transferência DMA durante VBlank |
| APU Interface | `$8D01` | 18 | Comunicação com S-SMP |
| Main Loop | `$841E` | Múltiplas | Loop principal do jogo |

### Registradores SNES Mais Acessados

| Registrador | Ocorrências | Função |
|-------------|-------------|--------|
| `$2116` (VRAM Addr L) | 33 | Endereço VRAM low byte |
| `$2118` (VRAM Data L) | 29 | Escrita de dados VRAM |
| `$2115` (VRAM Ctrl) | 25 | Modo de incremento VRAM |
| `$420B` (DMA Enable) | 28 | Ativação de transfers DMA |
| `$2140-$2143` (APU) | 18 | Comunicação com áudio |

### Estatísticas de Chamadas

- **JSR**: 209 chamadas → 82 rotinas únicas
- **JSL**: 54 chamadas → 50 rotinas únicas
- **JMP**: 23 chamadas

---

## 3. Tabelas de Dados

### Tabelas de Ponteiros

| Quantidade | Maiores Tabelas |
|------------|-----------------|
| 341 tabelas | 0x03F868 (972 entradas) |

### Paletas

| Quantidade | Localização Principal |
|------------|----------------------|
| 3,788 candidatas | Banco $00 (0x1900-0x5000) e $01 (0x8000-0x85E0) |

### Tiles

| Quantidade | Bancos Principais |
|------------|-------------------|
| 30,921 tiles | $11 (3760 tiles), $14 (3583 tiles) |

### Sequências RLE

| Quantidade | Maior Sequência |
|------------|-----------------|
| 1,879 | 0x03F89D (1891x 0xFF) |

---

## 4. Compressão

### Formato LZ Proprietário

- **Total de blocos**: 6,747
- **Opcodes**: $10-$1C (formato proprietário Nintendo)
- **Rotinas de descompressão**:
  - `$00:8874` - Dispatch de opcodes (201 chamadas)
  - `$01:B916` - Processamento com ASL/ROL

### Distribuição por Banco

| Banco | Blocos Comprimidos | Conteúdo |
|-------|-------------------|----------|
| $19 | 665 | Dados de mapa/world |
| $11 | 363 | Tiles gráficos |
| $16 | 344 | Sprites |

### Outros Formatos

- **RLE**: 1,902 sequências longas (8+ bytes)
- **Tabelas Huffman-like**: 590 estruturas

---

## 5. Assets Gráficos

### Tiles por Categoria

| Categoria | Quantidade | Localização |
|-----------|------------|-------------|
| Tiles 4bpp | ~17,423 tiles | Bancos $04, $0D, $10-$19 |
| Tiles UI/Fonte | 1,824 tiles | Banco $00-$01 (0x0013E0-0x012640) |
| Tiles de Mapa | 1,016 tiles | Banco $1C (0x0E0000-0x0E7F20) |
| Tiles Massivos | 9,186+ tiles | Bancos $0A-$14 (0x057BE0-0x0C3FC0) |

### Paletas

| Tipo | Quantidade | Localização |
|------|------------|-------------|
| BG Palettes | ~15 regiões | Banco $10 (0x080000-0x08FFFF) |
| Sprite Palettes | ~15 regiões | Banco $10 |

---

## 6. Áudio (SPC700/APU)

### Motor de Música

| Componente | Localização | Tamanho |
|------------|-------------|---------|
| Código SPC700 | Bancos $1A-$1F | ~192 KB |
| Samples BRR | Distribuído | 1,300+ regiões |

### Formato de Sample

- **Tipo**: BRR (Bit Rate Reduction)
- **Taxa**: 8-bit signed
- **Frequências**: 8000 Hz a 32000 Hz

### Bancos de Áudio

| Banco | Conteúdo |
|-------|----------|
| $1A | Motor de música + samples |
| $1B | Samples adicionais |
| $1C | Efeitos sonoros |
| $1D | Dados de instrumentos |
| $1E | Sequências musicais |
| $1F | Dados finais |

---

## 7. DMA/VRAM

### Transferências Detectadas

- **28 transferências DMA** no código
- **Tiles BG**: Carregados em VRAM $2000-$3400
- **Tilemaps**: VRAM $4000-$7D00

---

## 8. Mapa de Offsets Importantes

```
0x000000-0x007FFF  → Código banco $00 (Reset, NMI, IRQ, init)
0x008000-0x00FFFF  → Código banco $01
0x001900-0x005000  → Paletas principais (Banco $00)
0x0013E0-0x012640  → Tiles UI/fontes (Banco $00-$01)
0x03F868           → Maior tabela de ponteiros (972 entradas)
0x057BE0-0x0C3FC0  → Tiles massivos (Bancos $0A-$14)
0x0C0000-0x0C7FFF  → Dados de mapa/world (Banco $18)
0x0DBB92           → Tabela de caracteres ASCII (Banco $1B)
0x0E0000-0x0E7F20  → Tiles de mapa (Banco $1C)
0x0D0000-0x0FFFFF  → Motor SPC700 + samples (Bancos $1A-$1F)
0x7FFA-0x7FFF      → Vetores de interrupção
```

---

## 9. Próximos Passos

1. **Desassemblagem completa** - Usar rotinas identificadas para desassemblar todo o código
2. **Extração de tiles** - Usar formato LZ identificado para extrair todos os tiles
3. **Extração de paletas** - Converter paletas 15-bit para 24-bit
4. **Extração de áudio** - Extrair samples BRR e converter para WAV
5. **Conversão C/C++** - Mapear rotinas assembly para código C/C++ equivalente

---

## 10. Referências

- [Fullsnes](https://problemkaputt.de/fullsnes.htm)
- [SNES Development Manual](https://www.raphnet.net/donnees/tech/snes_architecture/)
- [bsnes Documentation](https://github.com/bsnes-emu/bsnes)
- [Zelda LTTP Disassembly](https://github.com/Zelda3 Decomp/zelda3)