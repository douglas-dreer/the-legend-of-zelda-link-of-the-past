# Objetivos do Projeto

## Visão
Criar uma reimplementação 1:1 do The Legend of Zelda: A Link to the Past
em C/C++ portável, sem nenhuma dependência da ROM original.

## Objetivos

### 1. Decompilação 1:1
- Cada rotina assembly → função C equivalente
- Comportamento idêntico à ROM original
- Validação: comparar saída frame-a-frame

### 2. Zero dependência da ROM
- Todos os assets embutidos no código ou em arquivos externos
- Nenhuma referência a offsets da ROM
- Código autocontido

### 3. Portabilidade
- C/C++ padrão (sem extensões GNU)
- Abstração de platform (renderer, input, áudio)
- Pode rodar em: PC, mobile, web (Emscripten), consoles

### 4. Extensibilidade
- AI de inimigos melhorável
- FX gráficos recriáveis
- Assets substituíveis
- Sistema de plugins/mods

### 5. Multilíngue
- Sistema de tradução de textos
- Suporte a idiomas via arquivos JSON
- Textos extraídos da ROM como base

### 6. Ferramentas Python
- Extração de assets (tiles, paletas, áudio, textos)
- Conversão para formatos modernos (PNG, WAV, JSON)
- Reconstrução de assets HD

### 7. Versão 3D (futuro)
- Engine 3D compatível com os dados
- Recriação em 3D estilo Ocarina of Time
- Mesma lógica de jogo, visual diferente
