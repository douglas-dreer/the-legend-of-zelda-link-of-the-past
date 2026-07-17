# Guia de Aprendizado - Engenharia Reversa de ROM

## Visão Geral

Este guia auxilia no aprendizado de engenharia reversa de ROMs SNES, usando The Legend of Zelda: A Link to the Past como exemplo prático.

## Pré-requisitos

### Conhecimentos Necessários

1. **Assembly 65816** - Linguagem de máquina do SNES
2. **Arquitetura de Computadores** - Registradores, memória, I/O
3. **Python** - Para criar ferramentas de análise
4. **Análise de Dados** - Interpretação de padrões binários

### Ferramentas

- Python 3.x
- Editor de texto com syntax highlight
- Visualizador hex (hexdump, hexcurse)
- (Opcional) IDA Pro ou Ghidra

## Módulos de Aprendizado

### Módulo 1: Fundamentos SNES

**Objetivo**: Entender a arquitetura básica do SNES

**Conteúdo**:
- Arquitetura do 65816
- Mapa de memória
- Registradores do PPU
- Sistema de interrupções

**Prática**:
- Ler o header da ROM
- Analisar o mapa de memória
- Identificar registradores usados

### Módulo 2: Formato da ROM

**Objetivo**: Entender como os dados são organizados na ROM

**Conteúdo**:
- LoROM vs HiROM
- Header da ROM
- Bancos de memória
- Checksum

**Prática**:
- Analisar o header da ROM
- Verificar checksum
- Mapear bancos de memória

### Módulo 3: Assets Gráficos

**Objetivo**: Extrair e interpretar tiles e paletas

**Conteúdo**:
- Formato de tiles (2bpp, 4bpp, 8bpp)
- Paletas (CGRAM)
- Mapas de background
- Sprites

**Prática**:
- Extrair tiles de exemplo
- Converter para imagens
- Analisar paletas

### Módulo 4: Áudio

**Objetivo**: Extrair e interpretar dados de áudio

**Conteúdo**:
- SPC700 e APU
- Samples de áudio
- Sequências musicais
- Instrumentos

**Prática**:
- Extrair samples
- Converter para WAV
- Analisar sequências

### Módulo 5: Código de Máquina

**Objetivo**: Desassemblar e entender o código

**Conteúdo**:
- Instruções 65816
- Modos de endereçamento
- Calling conventions
- Rotinas comuns

**Prática**:
- Desassemblar código
- Analisar rotinas
- Identificar padrões

## Exercícios Práticos

### Exercício 1: Análise de Header

1. Use o `rom_parser.py` para ler o header
2. Identifique: nome, mapeamento, tamanho, país
3. Verifique o checksum manualmente

### Exercício 2: Extração de Tiles

1. Use o `asset_extractor.py` para extrair tiles
2. Analise o formato dos tiles extraídos
3. Compare com tiles esperados

### Exercício 3: Extração de Áudio

1. Use o `audio_extractor.py` para extrair samples
2. Converta para WAV e ouça
3. Analise a taxa de amostragem

### Exercício 4: Desassemblagem

1. Use o `disassembler.py` para desassemblar código
2. Analise as primeiras instruções
3. Identifique rotinas conhecidas

## Recursos

### Documentação

- [SNES Development Manual](https://www.raphnet.net/donnees/tech/snes_architecture/snes_architecture.pdf)
- [Fullsnes](https://problemkaputt.de/fullsnes.htm)
- [SNES Technical Reference](https://www.snescentral.org/)

### Ferramentas

- [bsnes](https://github.com/bsnes-emu/bsnes) - Emulador com debugger
- [SNESTool](https://www.snescentral.org/snestool.php) - Ferramentas de análise
- [Tile Molester](https://www.romhacking.net/utilities/127/) - Editor de tiles

### Comunidades

- [ROMhacking.net](https://www.romhacking.net/)
- [SnesCentral](https://www.snescentral.org/)
- [NESDev](https://www.nesdev.org/) (comunidade ativa para SNES também)

## Próximos Passos

1. Complete os exercícios práticos
2. Analise a ROM completa do Zelda
3. Extraia todos os assets
4. Documente a estrutura completa do jogo
5. Comece a converter código para C/C++

## Dicas de Aprendizado

1. **Comece simples** - Analise partes pequenas da ROM primeiro
2. **Use ferramentas** - Automatize tarefas repetitivas
3. **Documente tudo** - Registre descobertas e padrões
4. **Pratique regularmente** - A engenharia reversa requer experiência
5. **Aprenda com outros** - Estude ROMs já analisadas