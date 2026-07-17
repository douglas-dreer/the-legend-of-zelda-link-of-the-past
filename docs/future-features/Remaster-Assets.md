# Remaster de Assets - The Legend of Zelda: A Link to the Past

## Visão Geral

Este documento descreve o processo de remasterização dos assets do jogo para alta definição (HD), mantendo o estilo pixel art original.

## 1. Análise dos Assets Originais

### Resolução Original

| Componente | Resolução | Pixels |
|------------|-----------|--------|
| Tela completa | 256×224 | 57,344 |
| Tile BG | 8×8 | 64 |
| Sprite | 8×8 a 16×16 | 64-256 |
| HUD | 256×32 | 8,192 |

### Paleta de Cores

| Tipo | Cores | BPP | Total |
|------|-------|-----|-------|
| BG1/BG2 | 16 por tile | 4bpp | 256 cores |
| BG3 (HUD) | 4 por tile | 2bpp | 16 cores |
| Sprites | 16 por tile | 4bpp | 256 cores |
| Total CGRAM | 256 cores | - | 15-bit RGB |

### Limitações do Hardware

- **Sem alfa blending** - Apenas cores sólidas
- **Sem gradiente** - Cada pixel é uma cor exata
- **Sem anti-aliasing** - Bordas duras (pixel art)
- **Tamanho fixo** - Tiles sempre 8×8 pixels

## 2. Estratégia de Remaster

### Opção A: Upscale Simples (Nearest Neighbor)

```
Original (8×8) → Upscale (64×64) → Manter estilo pixel art
```

**Vantagens**:
- Mantém fidelidade 100% ao original
- Rápido de implementar
- Não requer trabalho artístico

**Desvantagens**:
- Pixels visíveis em telas grandes
- Sem melhoria visual real

### Opção B: Upscale com Interpolação (Bicubic/Lanczos)

```
Original (8×8) → Interpolação → Suavização → 64×64
```

**Vantagens**:
- Bordas mais suaves
- Melhor aparência em HD

**Desvantagens**:
- Perde estilo pixel art
- Pode parecer "borrado"

### Opção C: Recriação Artística (Recomendado)

```
Original → Análise → Redesenho HD → Novos tiles
```

**Vantagens**:
- Máxima qualidade visual
- Mantém estilo artístico
- Adiciona detalhes novos

**Desvantagens**:
- Requer artistas
- Tempo de desenvolvimento
- Custo

### Opção D: AI Upscaling (ESRGAN/SRGAN)

```
Original → Rede Neural → Upscale inteligente → 64×64
```

**Vantagens**:
- Automatizado
- Pode inferir detalhes
- Razoavelmente rápido

**Desvantagens**:
- Resultados inconsistentes
- Pode adicionar artifacts
- Requer treinamento

## 3. Detalhes Técnicos do Remaster

### Tamanho de Tiles HD

| Resolução | Tamanho Tile | Escala | Pixels |
|-----------|--------------|--------|--------|
| SD (original) | 8×8 | 1x | 64 |
| HD (720p) | 16×16 | 2x | 256 |
| Full HD (1080p) | 32×32 | 4x | 1,024 |
| 4K (2160p) | 64×64 | 8x | 4,096 |

### Conversão de Paleta

**Original (15-bit SNES)**:
```
R: 5 bits (0-31) → 8 bits (0-255)
G: 5 bits (0-31) → 8 bits (0-255)
B: 5 bits (0-31) → 8 bits (0-255)
```

**HD (24-bit)**:
```
R: 8 bits (0-255)
G: 8 bits (0-255)
B: 8 bits (0-255)
```

**Conversão**:
```python
def snes_to_hd(color_15bit):
    r = ((color_15bit >> 0) & 0x1F) * 255 // 31
    g = ((color_15bit >> 5) & 0x1F) * 255 // 31
    b = ((color_15bit >> 10) & 0x1F) * 255 // 31
    return (r, g, b)
```

### Estrutura de Tiles HD

**Original (4bpp)**:
```
32 bytes por tile
8 linhas × 2 bytes × 4 planos
```

**HD (32bpp)**:
```
1,024 bytes por tile (32×32)
32 linhas × 32 pixels × 4 bytes (RGBA)
```

## 4. Assets para Remaster

### BG1/BG2 - Backgrounds Principais

| Asset | Quantidade | Prioridade |
|-------|------------|------------|
| Masmorras | ~100 telas | Alta |
| Campo | ~50 telas | Alta |
| Castelo | ~30 telas | Média |
| Cidades | ~20 telas | Média |

### Sprites - Personagens e Objetos

| Asset | Quantidade | Prioridade |
|-------|------------|------------|
| Link (4 direções) | 4 sprites | Crítica |
| NPCs | ~50 sprites | Alta |
| Inimigos | ~100 sprites | Alta |
| Objetos | ~200 sprites | Média |

### BG3 - HUD

| Asset | Quantidade | Prioridade |
|-------|------------|------------|
| Barra de vida | 1 sprite | Crítica |
| Ícones de itens | ~20 sprites | Alta |
| Texto/numeros | ~30 tiles | Alta |

### Efeitos Visuais

| Asset | Quantidade | Prioridade |
|-------|------------|------------|
| Animações de ataque | ~20 frames | Alta |
| Explosões | ~10 frames | Média |
| Transições | ~5 frames | Baixa |

## 5. Processo de Extração

### Passo 1: Extrair Tiles Originais

```bash
python tools/extractor/asset_extractor.py \
  --offset 0x000000 \
  --count 1024 \
  --bpp 4 \
  --output tiles_original.png
```

### Passo 2: Extrair Paletas

```bash
python tools/extractor/asset_extractor.py \
  --palette-offset 0x000000 \
  --palette-count 16 \
  --output palettes.json
```

### Passo 3: Converter para HD

```bash
python tools/remaster/upscaler.py \
  --input tiles_original.png \
  --scale 4 \
  --method esrgan \
  --output tiles_hd.png
```

### Passo 4: Recriar tiles (se necessário)

```bash
python tools/remaster/tile_creator.py \
  --input reference_hd.png \
  --palette palette_hd.json \
  --output new_tiles.png
```

## 6. Ferramentas Necessárias

### Extração

- `asset_extractor.py` - Extrair tiles originais
- `palette_extractor.py` - Extrair paletas
- `sprite_extractor.py` - Extrair sprites

### Conversão

- `snes_to_hd.py` - Converter formato SNES para HD
- `palette_converter.py` - Converter paletas 15-bit para 24-bit

### Upscaling

- `esrgan_upscaler.py` - AI upscaling
- `pixel_art_upscaler.py` - Upscaling mantendo pixel art
- `tile_assembler.py` - Montar tiles em spritesheets

### Validação

- `tile_validator.py` - Verificar integridade dos tiles
- `palette_validator.py` - Verificar paletas
- `comparison_viewer.py` - Comparar original vs HD

## 7. Estimativa de Trabalho

### Fase 1: Extração (1-2 semanas)

- [ ] Extrair todos os tiles
- [ ] Extrair todas as paletas
- [ ] Documentar estrutura

### Fase 2: Conversão (2-4 semanas)

- [ ] Converter paletas
- [ ] Upscale automático
- [ ] Validação visual

### Fase 3: Recriação (4-8 semanas)

- [ ] Redesenhar tiles principais
- [ ] Adicionar detalhes
- [ ] Testar em jogo

### Fase 4: Integração (2-4 semanas)

- [ ] Substituir assets originais
- [ ] Testar performance
- [ ] Otimizar memória

## 8. Referências

- [Pixel Art Scaling](https://www.scale2x.it/)
- [ESRGAN](https://github.com/xinntao/Real-ESRGAN)
- [SNES Tile Format](https://www.chibiakumas.com/65816/platform3.php)
- [HD Texture Packs](https://www.snescentral.org/)