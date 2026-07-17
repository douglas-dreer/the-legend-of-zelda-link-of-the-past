# Engenharia Reversa - The Legend of Zelda: A Link to the Past

## Visão Geral

Este projeto visa realizar a engenharia reversa completa da ROM de **The Legend of Zelda: A Link to the Past** para SNES, com os seguintes objetivos:

1. **Extração de Assets** - Imagens, sons e efeitos
2. **Desassemblagem** - Conversão de binário para Assembly 65816
3. **Descompilação** - Conversão de Assembly para C/C++
4. **Validação** - Garantir que o comportamento seja idêntico à ROM original

## Estrutura do Projeto

```
snes-zelda-decompiler/
├── docs/                    # Documentação
│   ├── arquitetura/         # Arquitetura SNES
│   ├── rom-format/          # Formato da ROM
│   └── guias/               # Guias de aprendizado
├── tools/                   # Ferramentas Python
│   ├── parser/              # Analisador de ROM
│   ├── extractor/           # Extrator de assets
│   └── disassembler/        # Desassembler
├── roms/                    # Arquivo da ROM (para referência)
└── src/                     # Código fonte extraído
```

## Equipe de Agentes

| Agente | Especialidade | Responsabilidade |
|--------|---------------|------------------|
| Eng. Software Reversa | Engenharia Reversa | Coordenação, análise geral |
| Eng. Sistema Embarcado | SNES Hardware | Arquitetura, mapeamento memória |
| Eng. Hardware SNES | Hardware SNES | Registradores, I/O, timing |
| Des. Assembly | Assembly 65816 | Instruções, calling conventions |
| Des. C/C++ | C/C++ | Mapeamento para C moderno |

## Fases do Projeto

### Fase 1: Fundamentos
- Documentar arquitetura SNES (65816 CPU, memória, I/O)
- Analisar formato da ROM (header, mapas de memória)
- Entender estrutura de dados do jogo

### Fase 2: Ferramentas Python
- `rom_parser.py` - Ler header, mapear segmentos
- `asset_extractor.py` - Extrair gráficos, tiles, paletas
- `audio_extractor.py` - Extrair samples de áudio
- `disassembler.py` - Converter bytes para assembly

### Fase 3: Extração
- Extrair todos os assets gráficos
- Extrair samples de áudio
- Extrair código (assembly)

### Fase 4: Conversão
- Documentar mapeamento Assembly → C/C++
- Criar tradutor automático (ou semi-automático)
- Gerar código C/C++ compilar

## Referências

- SNES Technical Reference
- 65816 CPU Documentation
- The Legend of Zelda: A Link to the Past ROM Analysis