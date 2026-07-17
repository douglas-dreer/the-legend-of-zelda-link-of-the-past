#!/usr/bin/env python3
"""
Parser de ROM SNES - The Legend of Zelda: A Link to the Past
Analisa o header e estrutura da ROM
"""

import os
import sys
from pathlib import Path

class SNESROMParser:
    """Parser para ROMs SNES"""
    
    # Tamanhos de ROM suportados
    ROM_SIZES = {
        0x00: "1 MB (8 Mbit)",
        0x01: "2 MB (16 Mbit)",
        0x02: "4 MB (32 Mbit)",
        0x03: "8 MB (64 Mbit)",
        0x04: "16 MB (128 Mbit)",
    }
    
    # Países
    COUNTRIES = {
        0x00: "Japão",
        0x01: "América do Norte",
        0x02: "Europa",
        0x03: "Suécia",
        0x04: "Finlândia",
        0x05: "Dinamarca",
        0x06: "França",
        0x07: "Holanda",
        0x08: "Espanha",
        0x09: "Alemanha",
        0x0A: "Itália",
        0x0B: "China",
        0x0C: "Indonésia",
        0x0D: "Coreia",
        0x0E: "Common (NA/EU)",
        0x0F: "Canadá",
        0x10: "Brasil",
        0x11: "Austrália",
    }
    
    # Publishers conhecidos
    PUBLISHERS = {
        0x01: "Nintendo",
        0x02: "Konami",
        0x03: "Capcom",
        0x04: "Hudson Soft",
        0x05: "Namco",
        0x06: "LOZC",
        0x07: "LC58",
        0x08: "Taito",
        0x09: "Halan",
        0x0A: "Jaleco",
        0x0B: "Coconuts",
        0x0C: "Culture Brain",
        0x0D: "Data East",
        0x0E: "Sunsoft",
        0x0F: "Tomy",
        0x10: "Fujisan",
        0x11: "Victor",
        0x12: "Yojin",
        0x13: "Varie",
        0x14: "Tsuburava",
        0x15: "Sofmap",
        0x16: "Micro Academy",
        0x17: "Takara",
        0x18: "Sega",
        0x19: "Sammy",
        0x1A: "Magnet",
        0x1B: "Rix",
        0x1C: "Acclaim",
        0x1D: "ASCII",
        0x1E: "Bandai",
        0x1F: "Squaresoft",
        0x20: "Tradewest",
        0x21: "Valiant",
        0x22: "Konami/Yoshi",
        0x23: "Kemco",
        0x24: "SETA",
        0x25: "Infogrames",
        0x26: "Lucusarts",
        0x27: "TOMY",
        0x28: "Davidson",
        0x29: "Hudson",
        0x2A: "SNS",
        0x2B: "Pony Canyon",
        0x2C: "Culture Brain",
        0x2D: "Kaneco",
        0x2E: "Pack in Video",
        0x2F: "Monolith",
        0x30: "Vic Tokai",
        0x31: "Character Soft",
        0x32: "I'Max",
        0x33: "Chunsoft",
        0x34: "Spike",
        0x35: "Victor Musical",
        0x36: "TNT Team",
        0x37: "NTTC",
        0x38: "Capcom (EU)",
        0x39: "Park Place Productions",
        0x3A: "Culture Brain",
        0x3B: "Angel (CO)",
        0x3C: "Yumeta",
        0x3D: "Polaris",
        0x3E: "Tokuma Shoten",
        0x3F: "Data East (EU)",
        0x40: "Toshiba EMI",
        0x41: "Yojin",
        0x42: "Yokosuka",
        0x43: "KSS",
        0x44: "POW",
        0x45: "Option",
        0x46: "Pegasus",
        0x47: "Micro World",
        0x48: "Mackin",
        0x49: "Micro Design",
        0x4A: "Messe Sansobo",
        0x4B: "S'NEXT",
        0x4C: "NCS",
        0x4D: "Bandai (JP)",
        0x4E: "Takara",
        0x4F: "Royal Kyushu",
        0x50: "Yonezawa/S'Pal",
        0x51: "Kaneko",
        0x52: "Arc",
        0x53: "Nihon Bussan",
        0x54: "Tecmo",
        0x55: "Imagineer",
        0x56: "Banpresto",
        0x57: "Yoshihiro",
        0x58: "Gaps",
        0x59: "Valnet",
        0x5A: "Techno Soft",
        0x5B: "Kemco (EU)",
        0x5C: "SNK",
        0x5D: "Pioneer",
        0x5E: "Kyoto",
        0x5F: "Audio Entertainment",
        0x60: "Tatakae Nankai Ouen",
        0x61: "BPS",
        0x62: "Samurai",
        0x63: "T&ESoft",
        0x64: "Epoch",
        0x65: "Athena",
        0x66: "Asmik",
        0x67: "Tecmo (JP)",
        0x68: "Imageworks",
        0x69: "Konami (AU)",
        0x6A: "Kawada",
        0x6B: "Takara (JP)",
        0x6C: "Kemco (JP)",
        0x6D: "Squaresoft (JP)",
        0x6E: "Tokuma Shoten (JP)",
        0x6F: "Data East (JP)",
        0x70: "Toshiba EMI (JP)",
        0x71: "Yojin (JP)",
        0x72: "Yokosuka (JP)",
        0x73: "KSS (JP)",
        0x74: "POW (JP)",
        0x75: "Option (JP)",
        0x76: "Pegasus (JP)",
        0x77: "Micro World (JP)",
        0x78: "Mackin (JP)",
        0x79: "Micro Design (JP)",
        0x7A: "Messe Sansobo (JP)",
        0x7B: "S'NEXT (JP)",
        0x7C: "NCS (JP)",
        0x7D: "Bandai (JP)",
        0x7E: "Takara (JP)",
        0x7F: "Royal Kyushu (JP)",
        0x80: "Yonezawa/S'Pal (JP)",
        0x81: "Kaneko (JP)",
        0x82: "Arc (JP)",
        0x83: "Nihon Bussan (JP)",
        0x84: "Tecmo (JP)",
        0x85: "Imagineer (JP)",
        0x86: "Banpresto (JP)",
        0x87: "Yoshihiro (JP)",
        0x88: "Gaps (JP)",
        0x89: "Valnet (JP)",
        0x8A: "Techno Soft (JP)",
        0x8B: "Kemco (EU)",
        0x8C: "SNK (JP)",
        0x8D: "Pioneer (JP)",
        0x8E: "Kyoto (JP)",
        0x8F: "Audio Entertainment (JP)",
        0x90: "Tatakae Nankai Ouen (JP)",
        0x91: "BPS (JP)",
        0x92: "Samurai (JP)",
        0x93: "T&ESoft (JP)",
        0x94: "Epoch (JP)",
        0x95: "Athena (JP)",
        0x96: "Asmik (JP)",
        0x97: "Tecmo (JP)",
        0x98: "Imageworks (JP)",
        0x99: "Konami (JP)",
        0x9A: "Kawada (JP)",
        0x9B: "Takara (JP)",
        0x9C: "Kemco (JP)",
        0x9D: "Squaresoft (JP)",
        0x9E: "Tokuma Shoten (JP)",
        0x9F: "Data East (JP)",
        0xA0: "Toshiba EMI (JP)",
        0xA1: "Yojin (JP)",
        0xA2: "Yokosuka (JP)",
        0xA3: "KSS (JP)",
        0xA4: "POW (JP)",
        0xA5: "Option (JP)",
        0xA6: "Pegasus (JP)",
        0xA7: "Micro World (JP)",
        0xA8: "Mackin (JP)",
        0xA9: "Micro Design (JP)",
        0xAA: "Messe Sansobo (JP)",
        0xAB: "S'NEXT (JP)",
        0xAC: "NCS (JP)",
        0xAD: "Bandai (JP)",
        0xAE: "Takara (JP)",
        0xAF: "Royal Kyushu (JP)",
        0xB0: "Yonezawa/S'Pal (JP)",
        0xB1: "Kaneko (JP)",
        0xB2: "Arc (JP)",
        0xB3: "Nihon Bussan (JP)",
        0xB4: "Tecmo (JP)",
        0xB5: "Imagineer (JP)",
        0xB6: "Banpresto (JP)",
        0xB7: "Yoshihiro (JP)",
        0xB8: "Gaps (JP)",
        0xB9: "Valnet (JP)",
        0xBA: "Techno Soft (JP)",
        0xBB: "Kemco (EU)",
        0xBC: "SNK (JP)",
        0xBD: "Pioneer (JP)",
        0xBE: "Kyoto (JP)",
        0xBF: "Audio Entertainment (JP)",
        0xC0: "Tatakae Nankai Ouen (JP)",
        0xC1: "BPS (JP)",
        0xC2: "Samurai (JP)",
        0xC3: "T&ESoft (JP)",
        0xC4: "Epoch (JP)",
        0xC5: "Athena (JP)",
        0xC6: "Asmik (JP)",
        0xC7: "Tecmo (JP)",
        0xC8: "Imageworks (JP)",
        0xC9: "Konami (JP)",
        0xCA: "Kawada (JP)",
        0xCB: "Takara (JP)",
        0xCC: "Kemco (JP)",
        0xCD: "Squaresoft (JP)",
        0xCE: "Tokuma Shoten (JP)",
        0xCF: "Data East (JP)",
        0xD0: "Toshiba EMI (JP)",
        0xD1: "Yojin (JP)",
        0xD2: "Yokosuka (JP)",
        0xD3: "KSS (JP)",
        0xD4: "POW (JP)",
        0xD5: "Option (JP)",
        0xD6: "Pegasus (JP)",
        0xD7: "Micro World (JP)",
        0xD8: "Mackin (JP)",
        0xD9: "Micro Design (JP)",
        0xDA: "Messe Sansobo (JP)",
        0xDB: "S'NEXT (JP)",
        0xDC: "NCS (JP)",
        0xDD: "Bandai (JP)",
        0xDE: "Takara (JP)",
        0xDF: "Royal Kyushu (JP)",
        0xE0: "Yonezawa/S'Pal (JP)",
        0xE1: "Kaneko (JP)",
        0xE2: "Arc (JP)",
        0xE3: "Nihon Bussan (JP)",
        0xE4: "Tecmo (JP)",
        0xE5: "Imagineer (JP)",
        0xE6: "Banpresto (JP)",
        0xE7: "Yoshihiro (JP)",
        0xE8: "Gaps (JP)",
        0xE9: "Valnet (JP)",
        0xEA: "Techno Soft (JP)",
        0xEB: "Kemco (EU)",
        0xEC: "SNK (JP)",
        0xED: "Pioneer (JP)",
        0xEE: "Kyoto (JP)",
        0xEF: "Audio Entertainment (JP)",
        0xF0: "Tatakae Nankai Ouen (JP)",
        0xF1: "BPS (JP)",
        0xF2: "Samurai (JP)",
        0xF3: "T&ESoft (JP)",
        0xF4: "Epoch (JP)",
        0xF5: "Athena (JP)",
        0xF6: "Asmik (JP)",
        0xF7: "Tecmo (JP)",
        0xF8: "Imageworks (JP)",
        0xF9: "Konami (JP)",
        0xFA: "Kawada (JP)",
        0xFB: "Takara (JP)",
        0xFC: "Kemco (JP)",
        0xFD: "Squaresoft (JP)",
        0xFE: "Tokuma Shoten (JP)",
        0xFF: "Data East (JP)",
    }
    
    # Mapas de memória do SNES
    MEMORY_MAPS = {
        0x20: "LoROM",
        0x21: "HiROM",
        0x22: "ExLoROM",
        0x23: "ExHiROM",
    }
    
    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.header = {}
        
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
        
    def read_header(self) -> dict:
        """Lê o header da ROM"""
        if self.rom_data is None:
            return None
            
        # Header Nintendo está em 0x7FC0 ou 0xFFC0 dependendo do mapeamento
        # Para LoROM: 0x7FC0
        # Para HiROM: 0xFFC0
        
        # Primeiro, vamos verificar o checksum complemento para determinar o mapeamento
        # O checksum complemento está em 0x7FD4-0x7FD5 (LoROM) ou 0xFFD4-0xFFD5 (HiROM)
        
        # Vamos tentar LoROM primeiro (0x7FC0)
        lorom_header_addr = 0x7FC0
        
        # Verificar se o header parece válido
        if lorom_header_addr + 0x30 <= len(self.rom_data):
            # Ler dados do header
            header_data = self.rom_data[lorom_header_addr:lorom_header_addr + 0x30]
            
            # Nome do jogo (21 bytes)
            game_name = header_data[0:21].decode('ascii', errors='replace').rstrip('\x00')
            
            # Mapeamento de memória (1 byte)
            map_mode = header_data[21]
            
            # Tamanho da ROM (1 byte)
            rom_size = header_data[22]
            
            # Tamanho da RAM (1 byte)
            ram_size = header_data[23]
            
            # País/Região (1 byte)
            country = header_data[24]
            
            # Publisher (1 byte)
            publisher = header_data[25]
            
            # Versão (1 byte)
            version = header_data[26]
            
            # Checksum complemento (2 bytes)
            checksum_complement = header_data[27] | (header_data[28] << 8)
            
            # Checksum (2 bytes)
            checksum = header_data[29] | (header_data[30] << 8)
            
            self.header = {
                'game_name': game_name,
                'map_mode': map_mode,
                'map_mode_name': self.MEMORY_MAPS.get(map_mode, f"Desconhecido (0x{map_mode:02X})"),
                'rom_size': rom_size,
                'rom_size_name': self.ROM_SIZES.get(rom_size, f"Desconhecido (0x{rom_size:02X})"),
                'ram_size': ram_size,
                'country': country,
                'country_name': self.COUNTRIES.get(country, f"Desconhecido (0x{country:02X})"),
                'publisher': publisher,
                'publisher_name': self.PUBLISHERS.get(publisher, f"Desconhecido (0x{publisher:02X})"),
                'version': version,
                'checksum_complement': checksum_complement,
                'checksum': checksum,
                'header_address': lorom_header_addr,
            }
            
            return self.header
            
        return None
        
    def print_header(self):
        """Imprime o header da ROM"""
        if not self.header:
            print("Header não lido")
            return
            
        print("\n" + "="*60)
        print("HEADER DA ROM")
        print("="*60)
        print(f"Nome do Jogo: {self.header['game_name']}")
        print(f"Mapeamento: {self.header['map_mode_name']}")
        print(f"Tamanho da ROM: {self.header['rom_size_name']}")
        print(f"País/Região: {self.header['country_name']}")
        print(f"Publisher: {self.header['publisher_name']}")
        print(f"Versão: {self.header['version']}")
        print(f"Checksum: 0x{self.header['checksum']:04X}")
        print(f"Checksum Complemento: 0x{self.header['checksum_complement']:04X}")
        print(f"Endereço do Header: 0x{self.header['header_address']:08X}")
        print("="*60)
        
    def verify_checksum(self) -> bool:
        """Verifica o checksum da ROM"""
        if self.rom_data is None:
            return False
            
        # Para LoROM, o checksum está em 0x7FDC-0x7FDD
        # O checksum complemento está em 0x7FDE-0x7FDF
        checksum_addr = 0x7FDC
        complement_addr = 0x7FDE
        
        if checksum_addr + 2 > len(self.rom_data) or complement_addr + 2 > len(self.rom_data):
            return False
            
        checksum = self.rom_data[checksum_addr] | (self.rom_data[checksum_addr + 1] << 8)
        complement = self.rom_data[complement_addr] | (self.rom_data[complement_addr + 1] << 8)
        
        # O checksum + complemento deve ser 0xFFFF
        return (checksum + complement) & 0xFFFF == 0xFFFF
        
    def get_rom_info(self) -> dict:
        """Retorna informações completas da ROM"""
        return {
            'path': str(self.rom_path),
            'size': len(self.rom_data),
            'header': self.header,
        }


def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python rom_parser.py <caminho_para_rom>")
        sys.exit(1)
        
    rom_path = sys.argv[1]
    parser = SNESROMParser(rom_path)
    
    if not parser.load_rom():
        sys.exit(1)
        
    header = parser.read_header()
    if header is None:
        print("Erro ao ler header da ROM")
        sys.exit(1)
        
    parser.print_header()
    
    # Verificar checksum
    if parser.verify_checksum():
        print("\n✓ Checksum válido!")
    else:
        print("\n✗ Checksum inválido!")
        
    # Salvar informações em JSON
    import json
    info = parser.get_rom_info()
    output_file = Path("rom_info.json")
    with open(output_file, 'w') as f:
        json.dump(info, f, indent=2, ensure_ascii=False)
    print(f"\nInformações salvas em: {output_file}")


if __name__ == "__main__":
    main()