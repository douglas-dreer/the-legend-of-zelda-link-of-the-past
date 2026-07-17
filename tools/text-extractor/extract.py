#!/usr/bin/env python3
"""
Extrator de Textos - The Legend of Zelda: A Link to the Past
Extrai diálogos, textos de itens e mensagens do sistema
"""

import os
import sys
import json
from pathlib import Path
from typing import List, Dict, Optional

class TextExtractor:
    """Extrator de textos para ROMs SNES"""

    # Tabela de caracteres SNES (Zelda: A Link to the Past)
    CHAR_TABLE = {
        0x00: '[END]',
        0x01: '[PAUSE]',
        0x02: '\n',
        0x03: '[CHOICE]',
        0x04: '[WAIT]',
        0x05: '[SPEED:1]',
        0x06: '[SPEED:2]',
        0x07: '[SPEED:3]',
        0x08: '[SPEED:4]',
        0x09: '[SPEED:5]',
        0x0A: '[SPEED:6]',
        0x0B: '[SPEED:7]',
        0x0C: '[SPEED:8]',
        0x0D: '[SPEED:9]',
        0x0E: '[SPEED:10]',
        0x0F: '[SPEED:11]',
        0x10: '[SPEED:12]',
        0x11: '[SPEED:13]',
        0x12: '[SPEED:14]',
        0x13: '[SPEED:15]',
        0x14: '[SPEED:16]',
        0x15: '[SPEED:17]',
        0x16: '[SPEED:18]',
        0x17: '[SPEED:19]',
        0x18: '[SPEED:20]',
        0x19: '[COLOR:BLUE]',
        0x1A: '[COLOR:RED]',
        0x1B: '[COLOR:GREEN]',
        0x1C: '[COLOR:YELLOW]',
        0x1D: '[COLOR:PURPLE]',
        0x1E: '[COLOR:ORANGE]',
        0x1F: '[COLOR:WHITE]',
        0x20: ' ',
        0x21: '!',
        0x22: '"',
        0x23: '#',
        0x24: '$',
        0x25: '%',
        0x26: '&',
        0x27: "'",
        0x28: '(',
        0x29: ')',
        0x2A: '*',
        0x2B: '+',
        0x2C: ',',
        0x2D: '-',
        0x2E: '.',
        0x2F: '/',
        0x30: '0',
        0x31: '1',
        0x32: '2',
        0x33: '3',
        0x34: '4',
        0x35: '5',
        0x36: '6',
        0x37: '7',
        0x38: '8',
        0x39: '9',
        0x3A: ':',
        0x3B: ';',
        0x3C: '<',
        0x3D: '=',
        0x3E: '>',
        0x3F: '?',
        0x40: '@',
        0x41: 'A',
        0x42: 'B',
        0x43: 'C',
        0x44: 'D',
        0x45: 'E',
        0x46: 'F',
        0x47: 'G',
        0x48: 'H',
        0x49: 'I',
        0x4A: 'J',
        0x4B: 'K',
        0x4C: 'L',
        0x4D: 'M',
        0x4E: 'N',
        0x4F: 'O',
        0x50: 'P',
        0x51: 'Q',
        0x52: 'R',
        0x53: 'S',
        0x54: 'T',
        0x55: 'U',
        0x56: 'V',
        0x57: 'W',
        0x58: 'X',
        0x59: 'Y',
        0x5A: 'Z',
        0x5B: '[',
        0x5C: '\\',
        0x5D: ']',
        0x5E: '^',
        0x5F: '_',
        0x60: '`',
        0x61: 'a',
        0x62: 'b',
        0x63: 'c',
        0x64: 'd',
        0x65: 'e',
        0x66: 'f',
        0x67: 'g',
        0x68: 'h',
        0x69: 'i',
        0x6A: 'j',
        0x6B: 'k',
        0x6C: 'l',
        0x6D: 'm',
        0x6E: 'n',
        0x6F: 'o',
        0x70: 'p',
        0x71: 'q',
        0x72: 'r',
        0x73: 's',
        0x74: 't',
        0x75: 'u',
        0x76: 'v',
        0x77: 'w',
        0x78: 'x',
        0x79: 'y',
        0x7A: 'z',
        0x7B: '{',
        0x7C: '|',
        0x7D: '}',
        0x7E: '~',
        0x7F: ' ',
        # Caracteres especiais (0x80-0xFF)
        0x80: 'á',
        0x81: 'é',
        0x82: 'í',
        0x83: 'ó',
        0x84: 'ú',
        0x85: 'ã',
        0x86: 'õ',
        0x87: 'ç',
        0x88: 'ê',
        0x89: 'ô',
        0x8A: 'à',
        0x8B: 'è',
        0x8C: 'ì',
        0x8D: 'ò',
        0x8E: 'ù',
        0x8F: 'ñ',
        0x90: 'ä',
        0x91: 'ë',
        0x92: 'ï',
        0x93: 'ö',
        0x94: 'ü',
        0x95: 'ÿ',
        0x96: 'ß',
        0x97: '£',
        0x98: '©',
        0x99: '®',
        0x9A: '°',
        0x9B: '±',
        0x9C: '²',
        0x9D: '³',
        0x9E: 'µ',
        0x9F: '¶',
        0xA0: '·',
        0xA1: '¸',
        0xA2: 'º',
        0xA3: 'ª',
        0xA4: '¬',
        0xA5: '½',
        0xA6: '¼',
        0xA7: '¡',
        0xA8: '¿',
        0xA9: '«',
        0xAA: '»',
        0xAB: '…',
        0xAC: '×',
        0xAD: '÷',
        0xAE: '€',
        0xAF: '£',
        0xB0: '←',
        0xB1: '→',
        0xB2: '↑',
        0xB3: '↓',
        0xB4: '♦',
        0xB5: '♣',
        0xB6: '♠',
        0xB7: '♥',
        0xB8: '★',
        0xB9: '☆',
        0xBA: '○',
        0xBB: '●',
        0xBC: '□',
        0xBD: '■',
        0xBE: '△',
        0xBF: '▲',
        0xC0: '▼',
        0xC1: '▽',
        0xC2: '◇',
        0xC3: '◆',
        0xC4: '♪',
        0xC5: '♫',
        0xC6: '♬',
        0xC7: '☼',
        0xC8: '►',
        0xC9: '◄',
        0xCA: '↕',
        0xCB: '‼',
        0xCC: '¶',
        0xCD: '§',
        0xCE: '▬',
        0xCF: '↨',
        0xD0: '↑',
        0xD1: '↓',
        0xD2: '→',
        0xD3: '←',
        0xD4: '∟',
        0xD5: '↔',
        0xD6: '▲',
        0xD7: '▼',
        0xD8: '!',
        0xD9: '"',
        0xDA: '#',
        0xDB: '$',
        0xDC: '%',
        0xDD: '&',
        0xDE: "'",
        0xDF: '(',
        0xE0: ')',
        0xE1: '*',
        0xE2: '+',
        0xE3: ',',
        0xE4: '-',
        0xE5: '.',
        0xE6: '/',
        0xE7: '0',
        0xE8: '1',
        0xE9: '2',
        0xEA: '3',
        0xEB: '4',
        0xEC: '5',
        0xED: '6',
        0xEE: '7',
        0xEF: '8',
        0xF0: '9',
        0xF1: ':',
        0xF2: ';',
        0xF3: '<',
        0xF4: '=',
        0xF5: '>',
        0xF6: '?',
        0xF7: '@',
        0xF8: 'A',
        0xF9: 'B',
        0xFA: 'C',
        0xFB: 'D',
        0xFC: 'E',
        0xFD: 'F',
        0xFE: 'G',
        0xFF: 'H',
    }

    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.output_dir = Path("output/texts")

    def load_rom(self) -> bool:
        """Carrega a ROM na memória"""
        if not self.rom_path.exists():
            print(f"Erro: ROM não encontrada: {self.rom_path}")
            return False

        with open(self.rom_path, 'rb') as f:
            self.rom_data = f.read()

        print(f"ROM carregada: {self.rom_path.name}")
        print(f"Tamanho: {len(self.rom_data)} bytes ({len(self.rom_data) / 1024 / 1024:.2f} MB)")
        return True

    def extract_text(self, offset: int, max_length: int = 255) -> str:
        """
        Extrai texto de um offset específico

        Args:
            offset: Offset na ROM
            max_length: Comprimento máximo do texto

        Returns:
            Texto extraído
        """
        if self.rom_data is None:
            return ""

        text = ""
        i = 0

        while i < max_length and offset + i < len(self.rom_data):
            byte = self.rom_data[offset + i]

            # Fim do texto
            if byte == 0x00:
                break

            # Caracteres de controle
            if byte in self.CHAR_TABLE:
                char = self.CHAR_TABLE[byte]
                if char.startswith('['):
                    # Controle especial
                    text += char
                else:
                    text += char
            else:
                text += f"[0x{byte:02X}]"

            i += 1

        return text

    def scan_for_texts(self, start_offset: int = 0, end_offset: int = None,
                       min_length: int = 3) -> List[Dict]:
        """
        Escaneia a ROM procurando por textos

        Args:
            start_offset: Offset inicial
            end_offset: Offset final
            min_length: Comprimento mínimo do texto

        Returns:
            Lista de textos encontrados
        """
        if self.rom_data is None:
            return []

        if end_offset is None:
            end_offset = len(self.rom_data)

        texts = []
        i = start_offset

        while i < end_offset - min_length:
            # Procurar por início de texto (caractere ASCII)
            if 0x20 <= self.rom_data[i] <= 0x7E:
                # Tentar extrair texto
                text = self.extract_text(i, max_length=100)

                # Verificar se é um texto válido
                if len(text) >= min_length:
                    # Verificar se tem caracteres legíveis
                    readable_chars = sum(1 for c in text if c.isalnum() or c in ' .,!?')
                    if readable_chars >= min_length:
                        texts.append({
                            'offset': i,
                            'text': text,
                            'length': len(text),
                            'readable': readable_chars / len(text)
                        })

            i += 1

        return texts

    def find_text_tables(self) -> List[Dict]:
        """
        Procura tabelas de texto na ROM

        Returns:
            Lista de tabelas encontradas
        """
        if self.rom_data is None:
            return []

        tables = []

        # Procurar por padrões conhecidos de tabelas de texto
        # No Zelda, as tabelas geralmente começam com ponteiros
        for i in range(0, len(self.rom_data) - 4):
            # Verificar se é um ponteiro válido
            if self.rom_data[i] == 0x00 and self.rom_data[i+1] == 0x80:
                # Possível ponteiro de tabela
                tables.append({
                    'offset': i,
                    'type': 'pointer_table',
                    'description': 'Tabela de ponteiros de texto'
                })

        return tables

    def extract_dialogue(self, offset: int) -> Dict:
        """
        Extrai um diálogo completo

        Args:
            offset: Offset do diálogo

        Returns:
            Dicionário com informações do diálogo
        """
        text = self.extract_text(offset, max_length=500)

        # Analisar estrutura do diálogo
        lines = text.split('\n')
        choices = text.count('[CHOICE]')
        pauses = text.count('[PAUSE]')

        return {
            'offset': offset,
            'text': text,
            'lines': lines,
            'line_count': len(lines),
            'choices': choices,
            'pauses': pauses,
            'length': len(text)
        }

    def extract_all_texts(self, output_file: str = None) -> List[Dict]:
        """
        Extrai todos os textos da ROM

        Args:
            output_file: Arquivo de saída (opcional)

        Returns:
            Lista de textos extraídos
        """
        print("Procurando por textos na ROM...")

        # Escanear a ROM
        texts = self.scan_for_texts(
            start_offset=0x000000,
            end_offset=0x100000,
            min_length=3
        )

        print(f"Encontrados {len(texts)} textos")

        # Filtrar textos duplicados
        unique_texts = []
        seen = set()

        for text in texts:
            text_hash = text['text'][:20]  # Primeiros 20 chars como hash
            if text_hash not in seen:
                seen.add(text_hash)
                unique_texts.append(text)

        print(f"Textos únicos: {len(unique_texts)}")

        # Salvar se especificado
        if output_file:
            self.save_texts(unique_texts, output_file)

        return unique_texts

    def save_texts(self, texts: List[Dict], output_file: str):
        """Salva textos em arquivo JSON"""
        output_path = Path(output_file)
        output_path.parent.mkdir(parents=True, exist_ok=True)

        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump({
                'rom': str(self.rom_path),
                'total_texts': len(texts),
                'texts': texts
            }, f, indent=2, ensure_ascii=False)

        print(f"Textos salvos em: {output_path}")

    def create_translation_template(self, texts: List[Dict],
                                    target_lang: str) -> Dict:
        """
        Cria template de tradução

        Args:
            texts: Lista de textos originais
            target_lang: Idioma alvo

        Returns:
            Template de tradução
        """
        template = {
            'metadata': {
                'rom': str(self.rom_path),
                'original_language': 'en',
                'target_language': target_lang,
                'total_texts': len(texts)
            },
            'translations': []
        }

        for text in texts:
            template['translations'].append({
                'id': text['offset'],
                'offset': f"0x{text['offset']:08X}",
                'original': text['text'],
                'translated': '',
                'notes': '',
                'status': 'pending'
            })

        return template


def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python extract.py <caminho_para_rom>")
        print("\nExemplo:")
        print('  python extract.py "roms/Legend of Zelda, The - A Link to the Past (USA).sfc"')
        sys.exit(1)

    rom_path = sys.argv[1]
    extractor = TextExtractor(rom_path)

    if not extractor.load_rom():
        sys.exit(1)

    # Criar diretório de saída
    extractor.output_dir.mkdir(parents=True, exist_ok=True)

    # Extrair todos os textos
    print("\n[Fase 1] Extraindo textos...")
    output_file = extractor.output_dir / "texts_original.json"
    texts = extractor.extract_all_texts(str(output_file))

    if texts:
        # Mostrar exemplos
        print("\n[Exemplos de textos encontrados]")
        for i, text in enumerate(texts[:10]):
            print(f"  {i+1}. Offset: 0x{text['offset']:08X}")
            print(f"     Texto: {text['text'][:80]}...")
            print()

        # Criar template de tradução para português
        print("\n[Fase 2] Criando template de tradução (PT-BR)...")
        template = extractor.create_translation_template(texts, 'pt-BR')

        template_file = extractor.output_dir / "translation_template_pt.json"
        with open(template_file, 'w', encoding='utf-8') as f:
            json.dump(template, f, indent=2, ensure_ascii=False)

        print(f"Template salvo em: {template_file}")

        # Criar template para japonês
        print("\n[Fase 3] Criando template de tradução (JA)...")
        template_ja = extractor.create_translation_template(texts, 'ja')

        template_file_ja = extractor.output_dir / "translation_template_ja.json"
        with open(template_file_ja, 'w', encoding='utf-8') as f:
            json.dump(template_ja, f, indent=2, ensure_ascii=False)

        print(f"Template salvo em: {template_file_ja}")

    print("\n" + "="*60)
    print("EXTRAÇÃO DE TEXTOS CONCLUÍDA!")
    print("="*60)
    print("\nPróximos passos:")
    print("1. Revisar texts_original.json")
    print("2. Preencher traduções nos templates")
    print("3. Usar apply.py para aplicar traduções")
    print("\nConsulte docs/future-features/Traducao-Game.md para mais informações.")


if __name__ == "__main__":
    main()