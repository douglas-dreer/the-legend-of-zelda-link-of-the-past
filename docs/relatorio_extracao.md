# Relatório de Extração - The Legend of Zelda: A Link to the Past

## Data: 2024

---

## 1. Resumo da Extração

### Desassemblagem

| Métrica | Valor |
|---------|-------|
| **Instruções totais** | 1,016,875 |
| **Rotinas descobertas** | 61,789 |
| **Cobertura de código** | 97.0% |
| **Tamanho do arquivo** | 24 MB (569,023 linhas) |
| **Bancos cobertos** | Todos os 32 ($00-$1F) |

### Paletas

| Métrica | Valor |
|---------|-------|
| **Paletas extraídas** | 440 |
| **Categorias** | 16 (main_bg, sprites, ui, title, link variants, enemies, items, effects, dungeons, overworld, house) |
| **Formato** | 24-bit RGB (convertido de 15-bit SNES) |
| **Localização** | Banco $00 (0x1900-0x5000) |

### Tiles

| Métrica | Valor |
|---------|-------|
| **Tiles UI/Fonte** | 512 (não comprimidos) |
| **Tiles Comprimidos LZ** | 405 (10 blocos) |
| **Formato** | 4bpp (32 bytes cada) |
| **Localização** | 0x0013E0-0x012640 (UI), Bancos $0A-$14 (comprimidos) |

### Áudio

| Métrica | Valor |
|---------|-------|
| **Regiões BRR encontradas** | 19,768 |
| **Samples decodificados** | 1,171,494 |
| **Arquivos WAV gerados** | 19,768 |
| **Tamanho total** | 3.06 MB |
| **Taxa de amostragem** | 32,000 Hz |

---

## 2. Arquivos Gerados

### Assembly

```
output/assembly/zelda_full.asm          (24 MB, 569,023 linhas)
```

### Paletas

```
tools/extractor/output/palettes/
├── palettes.json                       (388 KB)
├── all_palettes.ppm                    (4.4 MB)
├── palette_main_bg.ppm
├── palette_main_sprites.ppm
├── palette_ui.ppm
├── palette_title.ppm
├── palette_link_blue.ppm
├── palette_link_green.ppm
├── palette_link_red.ppm
├── palette_enemies_1.ppm
├── palette_enemies_2.ppm
├── palette_items.ppm
├── palette_effects.ppm
├── palette_dungeon_1.ppm
├── palette_dungeon_2.ppm
├── palette_overworld_1.ppm
├── palette_overworld_2.ppm
└── palette_house.ppm
```

### Tiles

```
tools/extractor/output/tiles/
├── ui_tiles.ppm                        (248 KB)
├── compressed_block_00.ppm
├── compressed_block_01.ppm
├── ...
└── compressed_block_09.ppm
```

### Áudio

```
output/audio/
├── extraction_report.txt
├── sample_000002.wav
├── sample_000053.wav
├── ...
└── sample_0F503E.wav                   (19,768 arquivos)
```

---

## 3. Scripts Criados

| Script | Tamanho | Função |
|--------|---------|--------|
| `tools/disassembler/full_disasm.py` | - | Desassembler 65816 completo |
| `tools/extractor/lz_decompressor.py` | 9.3 KB | Descompressor LZ proprietário |
| `tools/extractor/full_extractor.py` | 18.7 KB | Extrator de paletas e tiles |
| `tools/extractor/brr_extractor.py` | 6.8 KB | Decodificador BRR |
| `tools/extractor/audio_full_extractor.py` | 9.7 KB | Scanner de áudio BRR |
| `tools/text-extractor/extract.py` | - | Extrator de textos |

---

## 4. Dados Descobertos

### Formato LZ Proprietário

- **Opcodes**: $10-$1C
- **Estrutura**: opcode + comprimento (word) + dados
- **Tipos de operação**:
  - $10: Literal (copia dados diretos)
  - $20: Byte fill (repete um byte)
  - $30: Word fill (repete uma word)
  - $40: Inc fill (incrementa valor)
  - $50/$60: Back-reference (copia de posição anterior)

### Formato BRR

- **Block size**: 10 bytes (1 header + 9 data)
- **Header**: filter (bits 6-4), end flag (bit 4), range (bits 3-0)
- **Data**: 16 samples de 4 bits cada
- **Filtro**: sample + (prev1 * -0.9375) + (prev2 * 0.46875)

---

## 5. Próximos Passos

### Imediatos

1. **Validar tiles** - Verificar se os tiles extraídos são visuaismente corretos
2. **Identificar músicas** - Mapear samples BRR em músicas e efeitos
3. **Mapear sprites** - Identificar sprites de Link, NPCs e inimigos

### Médio Prazo

4. **Desassemblagem refinada** - Identificar mais rotinas e suas funções
5. **Mapeamento de gameplay** - Entender lógica de jogo (itens, inimigos, masmorras)
6. **Conversão C/C++** - Começar mapeamento de assembly para C

### Longo Prazo

7. **Decompilação completa** - Converter todo o código para C/C++
8. **Validação** - Garantir comportamento idêntico à ROM
9. **Port** - Criar versão PC do jogo

---

## 6. Referências

- [Zelda3 Decomp](https://github.com/Zelda3 Decomp/zelda3) - Projeto de referência
- [Fullsnes](https://problemkaputt.de/fullsnes.htm) - Documentação SNES
- [bsnes](https://github.com/bsnes-emu/bsnes) - Emulador com debugger