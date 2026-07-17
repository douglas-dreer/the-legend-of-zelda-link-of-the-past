# Tradução Game - The Legend of Zelda: A Link to the Past

## Visão Geral

Este documento descreve o sistema de tradução do jogo para múltiplos idiomas, incluindo extração de diálogos, textos e suporte a localização.

## 1. Análise dos Textos do Jogo

### Idioma Original

- **Idioma**: Inglês (USA)
- **Encoding**: ASCII/Custom SNES
- **Tamanho estimado**: ~50KB de textos

### Tipos de Texto

| Tipo | Exemplo | Frequência |
|------|---------|------------|
| Diálogos NPCs | "Welcome to Kakariko Village!" | Alta |
| Itens | "Master Sword" | Média |
| Instruções | "Press START to pause" | Baixa |
| Mensagens sistema | "Game Over" | Baixa |
| Créditos | "Director: Shigeru Miyamoto" | Única |

### Estrutura de Textos

```
Texto no SNES:
- Encoding custom (não ASCII padrão)
- Controle de caracteres especiais
- Pausas para animação de texto
- Escolhas de diálogo
```

## 2. Sistema de Extração

### Algoritmo de Extração

```python
def extract_text_from_rom(rom_data, offset, max_length=255):
    """
    Extrai texto da ROM usando encoding SNES custom
    """
    text = ""
    i = 0

    while i < max_length:
        byte = rom_data[offset + i]

        # Fim do texto
        if byte == 0x00:
            break

        # Caracteres de controle
        if byte == 0x01:  # Pausa
            text += "[PAUSE]"
        elif byte == 0x02:  # Nova linha
            text += "\n"
        elif byte == 0x03:  # Escolha
            text += "[CHOICE]"
        elif byte == 0x04:  # Esperar input
            text += "[WAIT]"
        elif byte == 0x05:  # Velocidade
            speed = rom_data[offset + i + 1]
            text += f"[SPEED:{speed}]"
            i += 1
        # Caracteres ASCII
        elif 0x20 <= byte <= 0x7E:
            text += chr(byte)
        # Caracteres especiais
        elif byte == 0x7F:
            text += " "  # Espaço
        elif 0x80 <= byte <= 0x9F:
            # Caracteres acentuados
            text += get_accented_char(byte)
        elif 0xA0 <= byte <= 0xBF:
            # Símbolos
            text += get_symbol(byte)
        else:
            text += f"[0x{byte:02X}]"

        i += 1

    return text
```

### Mapeamento de Caracteres SNES

| Byte | Caractere | Descrição |
|------|-----------|-----------|
| 0x00 | [END] | Fim do texto |
| 0x01 | [PAUSE] | Pausa para leitura |
| 0x02 | [NEWLINE] | Nova linha |
| 0x03 | [CHOICE] | Início de escolha |
| 0x04 | [WAIT] | Esperar input |
| 0x05-0x08 | [SPEED:n] | Velocidade do texto |
| 0x09-0x1F | [CTRL:n] | Controles especiais |
| 0x20-0x7E | ASCII | Caracteres padrão |
| 0x7F | (espaço) | Espaço |
| 0x80-0x9F | Acentos | Caracteres acentuados |
| 0xA0-0xBF | Símbolos | Símbolos especiais |
| 0xC0-0xFF | Tiles | Ícones de tiles |

## 3. Localização

### Idiomas Suportados

| Idioma | Código | Prioridade | Notas |
|--------|--------|------------|-------|
| Inglês | EN | Original | USA |
| Japonês | JP | Alta | Versão original |
| Português | PT | Média | Brasil |
| Espanhol | ES | Média | América Latina |
| Francês | FR | Baixa | Europa |
| Alemão | DE | Baixa | Europa |
| Italiano | IT | Baixa | Europa |

### Desafios de Localização

#### 1. Limite de Espaço

```c
// Texto original em inglês
"Welcome to the Lost Woods!"

// Tradução português (mais longa)
"Bem-vindo às Florestas Perdidas!"

// Solução: Compactação ou reorganização
```

#### 2. Caracteres Especiais

```
Inglês:  A-Z, a-z, 0-9, !@#$%...
Português: A-Z, a-z, 0-9, ã, é, ç, etc.
Japonês: Hiragana, Katakana, Kanji
```

#### 3. Layout de Texto

```
Original (2 linhas):         Tradução (3 linhas):
+------------------+         +------------------+
| Welcome to the   |         | Bem-vindo às     |
| Lost Woods!      |         | Florestas        |
|                  |         | Perdidas!        |
+------------------+         +------------------+
```

## 4. Scripts de Extração

### Extrator Principal

```python
#!/usr/bin/env python3
"""
Extrator de textos - The Legend of Zelda: A Link to the Past
"""

import json
from pathlib import Path

class TextExtractor:
    def __init__(self, rom_path):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.text_table = self.load_text_table()

    def load_text_table(self):
        """Carrega tabela de caracteres"""
        # Tabela de caracteres SNES
        table = {}
        for i in range(0x20, 0x7F):
            table[i] = chr(i)
        # Adicionar caracteres acentuados
        table[0x80] = 'á'
        table[0x81] = 'é'
        table[0x82] = 'í'
        table[0x83] = 'ó'
        table[0x84] = 'ú'
        table[0x85] = 'ã'
        table[0x86] = 'õ'
        table[0x87] = 'ç'
        table[0x88] = 'ê'
        table[0x89] = 'ô'
        return table

    def extract_text(self, offset, max_length=255):
        """Extrai texto de um offset"""
        text = ""
        i = 0

        while i < max_length:
            byte = self.rom_data[offset + i]

            if byte == 0x00:
                break
            elif byte == 0x01:
                text += "[PAUSE]"
            elif byte == 0x02:
                text += "\n"
            elif 0x20 <= byte <= 0x7E:
                text += chr(byte)
            elif byte in self.text_table:
                text += self.text_table[byte]
            else:
                text += f"[0x{byte:02X}]"

            i += 1

        return text

    def extract_all_texts(self):
        """Extrai todos os textos da ROM"""
        texts = []

        # Procurar por textos em offsets conhecidos
        # (Estes offsets precisam ser ajustados para o jogo específico)
        text_offsets = [
            # Adicionar offsets encontrados na análise
        ]

        for offset in text_offsets:
            text = self.extract_text(offset)
            if text and len(text) > 2:
                texts.append({
                    'offset': offset,
                    'text': text,
                    'length': len(text)
                })

        return texts
```

### Gerador de Tradução

```python
#!/usr/bin/env python3
"""
Gerador de tradução - The Legend of Zelda: A Link to the Past
"""

import json

class TranslationGenerator:
    def __init__(self, original_texts):
        self.original = original_texts
        self.translations = {}

    def create_translation(self, target_lang):
        """Cria arquivo de tradução"""
        translation = {
            'language': target_lang,
            'original_language': 'en',
            'texts': []
        }

        for text in self.original:
            translation['texts'].append({
                'id': text['offset'],
                'original': text['text'],
                'translated': '',  # Preencher com tradução
                'notes': ''
            })

        return translation

    def apply_translation(self, rom_data, translation):
        """Aplica tradução à ROM"""
        modified = bytearray(rom_data)

        for text_entry in translation['texts']:
            if text_entry['translated']:
                offset = text_entry['id']
                encoded = self.encode_text(text_entry['translated'])
                modified[offset:offset+len(encoded)] = encoded

        return bytes(modified)

    def encode_text(self, text):
        """Codifica texto para formato SNES"""
        encoded = bytearray()

        for char in text:
            if char == '\n':
                encoded.append(0x02)
            elif char == '[':
                # Processar controle
                pass
            elif 0x20 <= ord(char) <= 0x7E:
                encoded.append(ord(char))
            else:
                # Procurar na tabela
                for code, table_char in self.text_table.items():
                    if table_char == char:
                        encoded.append(code)
                        break

        encoded.append(0x00)  # Fim do texto
        return bytes(encoded)
```

## 5. Fluxo de Trabalho

### Passo 1: Extração

```bash
# Extrair todos os textos
python tools/text-extractor/extract.py \
  --rom "roms/Legend of Zelda, The - A Link to the Past (USA).sfc" \
  --output texts_original.json
```

### Passo 2: Criação de Tradução

```bash
# Criar模板 de tradução
python tools/text-extractor/create_template.py \
  --input texts_original.json \
  --language pt \
  --output translation_pt.json
```

### Passo 3: Preenchimento

```bash
# Abrir editor de tradução
python tools/text-extractor/editor.py \
  --translation translation_pt.json
```

### Passo 4: Validação

```bash
# Validar tradução
python tools/text-extractor/validate.py \
  --translation translation_pt.json \
  --original texts_original.json
```

### Passo 5: Aplicação

```bash
# Aplicar tradução à ROM
python tools/text-extractor/apply.py \
  --rom "roms/original.sfc" \
  --translation translation_pt.json \
  --output "roms/zelda_pt.sfc"
```

## 6. Estrutura de Arquivos

```
text-extractor/
├── extract.py              # Extrator principal
├── create_template.py      # Gerador de templates
├── editor.py              # Editor de tradução
├── validate.py            # Validador
├── apply.py               # Aplicador
├── tables/                # Tabelas de caracteres
│   ├── en.json
│   ├── pt.json
│   ├── ja.json
│   └── ...
└── output/                # Saídas
    ├── texts_original.json
    ├── translation_pt.json
    └── ...
```

## 7. Referências

- [SNES Text Encoding](https://www.zeldix.net/t157-text-encoding)
- [Game Translation Guide](https://www.romhacking.net/guides/246/)
- [Localization Best Practices](https://www.gamedeveloper.com/localization)