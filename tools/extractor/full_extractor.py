#!/usr/bin/env python3
"""
Extrator completo de tiles e paletas do Zelda: A Link to the Past
Extrai paletas do banco $00 e tiles comprimidos via LZ
"""

import json
import os
import struct
import sys
from pathlib import Path
from typing import List, Tuple, Optional, Dict
from lz_decompressor import ZeldaLZDecompressor


class ALttPExtractor:
    """Extrator de assets para The Legend of Zelda: A Link to the Past"""
    
    # Mapeamento de memória LoROM
    # Banco $00: 0x000000-0x007FFF (offset 0x0000-0x7FFF na ROM)
    # Bancos $0A-$14: dados de tiles comprimidos
    
    # Offsets conhecidos na ROM (USA 1.0)
    PALETTE_START = 0x1900       # Início das paletas no banco $00
    PALETTE_END = 0x5000         # Fim das paletas
    
    # Locais de tiles conhecidos
    TILE_REGIONS = {
        'ui_tiles': (0x0013E0, 0x012640),      # Tiles UI e fontes
        'compressed_tiles': (0x057BE0, 0x0C3FC0),  # Tiles comprimidos (bancos $0A-$14)
    }
    
    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.output_dir = Path("output")
        self.palettes_dir = self.output_dir / "palettes"
        self.tiles_dir = self.output_dir / "tiles"
        
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
    
    def setup_dirs(self):
        """Cria diretórios de saída"""
        self.palettes_dir.mkdir(parents=True, exist_ok=True)
        self.tiles_dir.mkdir(parents=True, exist_ok=True)
        
    def snes_to_rgb(self, color_word: int) -> Tuple[int, int, int]:
        """
        Converte cor SNES 15-bit para RGB 24-bit
        
        Formato SNES: gggrrrrr 0bbbbbgg (little-endian)
        """
        r = (color_word & 0x1F) << 3
        g = ((color_word >> 5) & 0x1F) << 3
        b = ((color_word >> 10) & 0x1F) << 3
        return (r, g, b)
    
    def extract_palettes(self) -> Dict[str, List[List[Tuple[int, int, int]]]]:
        """
        Extrai todas as paletas do banco $00 (0x1900-0x5000)
        
        Returns:
            Dicionário com paletas organizadas por nome
        """
        print("\n=== Extraindo Paletas ===")
        print(f"Offset: 0x{self.PALETTE_START:04X} - 0x{self.PALETTE_END:04X}")
        
        palettes = {}
        
        # Cada paleta SNES tem 16 cores (32 bytes)
        # No Zelda, há múltiplas paletas para diferentes elementos
        palette_size = 16  # 16 cores por paleta
        bytes_per_palette = palette_size * 2  # 2 bytes por cor
        
        # Extrair paletas conhecidas
        palette_regions = {
            'main_bg': (0x1900, 16),        # Paleta principal de fundo
            'main_sprites': (0x1A00, 16),   # Paleta principal de sprites
            'ui': (0x1B00, 16),             # Paleta da interface
            'title': (0x1C00, 16),          # Paleta da tela título
            'link_green': (0x1D00, 16),     # Paleta do Link (verde)
            'link_blue': (0x1E00, 16),      # Paleta do Link (azul)
            'link_red': (0x1F00, 16),       # Paleta do Link (vermelho)
            'enemies_1': (0x2000, 16),      # Paleta de inimigos 1
            'enemies_2': (0x2100, 16),      # Paleta de inimigos 2
            'items': (0x2200, 16),          # Paleta de itens
            'effects': (0x2300, 16),        # Paleta de efeitos
            'dungeon_1': (0x2400, 16),      # Paleta masmorra 1
            'dungeon_2': (0x2500, 16),      # Paleta masmorra 2
            'overworld_1': (0x2600, 16),    # Paleta mundo aberto 1
            'overworld_2': (0x2700, 16),    # Paleta mundo aberto 2
            'house': (0x2800, 16),          # Paleta de casas
        }
        
        for name, (offset, num_colors) in palette_regions.items():
            palette = []
            
            for i in range(num_colors):
                color_offset = offset + (i * 2)
                
                if color_offset + 2 > len(self.rom_data):
                    break
                    
                color_word = struct.unpack('<H', 
                    self.rom_data[color_offset:color_offset + 2])[0]
                rgb = self.snes_to_rgb(color_word)
                palette.append(rgb)
            
            if palette:
                palettes[name] = palette
                print(f"  {name}: {len(palette)} cores (offset 0x{offset:04X})")
        
        # Extrair todas as paletas do range completo
        all_palettes = []
        offset = self.PALETTE_START
        
        while offset < self.PALETTE_END:
            palette = []
            
            for i in range(palette_size):
                color_offset = offset + (i * 2)
                
                if color_offset + 2 > len(self.rom_data):
                    break
                    
                color_word = struct.unpack('<H', 
                    self.rom_data[color_offset:color_offset + 2])[0]
                rgb = self.snes_to_rgb(color_word)
                palette.append(rgb)
            
            if palette:
                all_palettes.append(palette)
            
            offset += bytes_per_palette
        
        palettes['all_palettes'] = all_palettes
        print(f"\nTotal de paletas extraídas: {len(all_palettes)}")
        
        return palettes
    
    def save_palettes(self, palettes: Dict[str, List[List[Tuple[int, int, int]]]]):
        """Salva paletas em JSON e imagens"""
        print("\n=== Salvando Paletas ===")
        
        # Salvar como JSON
        json_data = {}
        for name, palette in palettes.items():
            json_data[name] = palette
        
        json_file = self.palettes_dir / "palettes.json"
        with open(json_file, 'w') as f:
            json.dump(json_data, f, indent=2)
        print(f"  JSON: {json_file}")
        
        # Salvar imagens PPM das paletas principais
        for name, palette in palettes.items():
            if name == 'all_palettes':
                continue
                
            if palette and len(palette) > 0:
                self._save_palette_image(palette, name)
        
        # Criar imagem de todas as paletas
        if 'all_palettes' in palettes:
            self._save_all_palettes_image(palettes['all_palettes'])
    
    def _save_palette_image(self, palette: List[Tuple[int, int, int]], name: str):
        """Salva uma paleta como imagem PPM"""
        # Organizar em grade 4x4
        width = 4 * 32  # 4 cores, 32 pixels cada
        height = 4 * 32  # 4 linhas
        
        ppm_lines = [f"P3\n{width} {height}\n255"]
        
        for y in range(height):
            row = []
            for x in range(width):
                color_idx = (y // 32) * 4 + (x // 32)
                if color_idx < len(palette):
                    r, g, b = palette[color_idx]
                else:
                    r, g, b = 0, 0, 0
                row.append(f"{r} {g} {b}")
            ppm_lines.append(" ".join(row))
        
        ppm_file = self.palettes_dir / f"palette_{name}.ppm"
        with open(ppm_file, 'w') as f:
            f.write("\n".join(ppm_lines))
    
    def _save_all_palettes_image(self, all_palettes: List[List[Tuple[int, int, int]]]):
        """Salva imagem com todas as paletas"""
        # 16 paletas por linha, cada paleta 4x4 cores de 8x8 pixels
        palettes_per_row = 8
        palette_pixel_size = 8
        
        cols = palettes_per_row
        rows = (len(all_palettes) + cols - 1) // cols
        
        width = cols * 4 * palette_pixel_size
        height = rows * 4 * palette_pixel_size
        
        ppm_lines = [f"P3\n{width} {height}\n255"]
        
        for y in range(height):
            row = []
            for x in range(width):
                # Determinar qual paleta e qual pixel
                pal_col = x // (4 * palette_pixel_size)
                pal_row = y // (4 * palette_pixel_size)
                pal_idx = pal_row * cols + pal_col
                
                if pal_idx < len(all_palettes):
                    palette = all_palettes[pal_idx]
                    # Posição dentro da paleta (4x4 grid)
                    inner_x = (x % (4 * palette_pixel_size)) // palette_pixel_size
                    inner_y = (y % (4 * palette_pixel_size)) // palette_pixel_size
                    color_idx = inner_y * 4 + inner_x
                    
                    if color_idx < len(palette):
                        r, g, b = palette[color_idx]
                    else:
                        r, g, b = 0, 0, 0
                else:
                    r, g, b = 0, 0, 0
                    
                row.append(f"{r} {g} {b}")
            ppm_lines.append(" ".join(row))
        
        ppm_file = self.palettes_dir / "all_palettes.ppm"
        with open(ppm_file, 'w') as f:
            f.write("\n".join(ppm_lines))
        print(f"  Imagem: {ppm_file}")
    
    def decode_4bpp_tile(self, data: bytes) -> List[int]:
        """
        Decodifica um tile 4bpp (32 bytes) para indices de pixel
        
        Formato SNES 4bpp:
        - 8 linhas, cada linha = 2 bytes planos (low/high)
        - Planos 0-1 nos primeiros 16 bytes
        - Planos 2-1 nos últimos 16 bytes
        """
        pixels = []
        
        if len(data) < 32:
            return pixels
            
        for y in range(8):
            # Planos 0 e 1
            plane0 = data[y * 2]
            plane1 = data[y * 2 + 1]
            
            # Planos 2 e 3
            plane2 = data[16 + y * 2]
            plane3 = data[16 + y * 2 + 1]
            
            for x in range(8):
                bit = 7 - x
                pixel = ((plane0 >> bit) & 1) | \
                        (((plane1 >> bit) & 1) << 1) | \
                        (((plane2 >> bit) & 1) << 2) | \
                        (((plane3 >> bit) & 1) << 3)
                pixels.append(pixel)
        
        return pixels
    
    def extract_raw_tiles(self, offset: int, count: int, 
                         palette: List[Tuple[int, int, int]],
                     name: str) -> Optional[str]:
        """
        Extrai tiles brutos (não comprimidos) da ROM
        
        Args:
            offset: Offset na ROM
            count: Número de tiles
            palette: Paleta para renderização
            name: Nome para o arquivo de saída
            
        Returns:
            Caminho do arquivo PPM ou None
        """
        tiles = []
        tile_size = 32  # 32 bytes por tile 4bpp
        
        for i in range(count):
            tile_offset = offset + (i * tile_size)
            
            if tile_offset + tile_size > len(self.rom_data):
                break
                
            tile_data = self.rom_data[tile_offset:tile_offset + tile_size]
            pixels = self.decode_4bpp_tile(tile_data)
            
            if pixels:
                tiles.append(pixels)
        
        if not tiles:
            return None
            
        return self._tiles_to_ppm(tiles, palette, name)
    
    def extract_compressed_tiles(self, start_offset: int, end_offset: int,
                                palette: List[Tuple[int, int, int]],
                                name: str) -> Optional[str]:
        """
        Extrai tiles comprimidos via LZ
        
        Args:
            start_offset: Offset inicial na ROM
            end_offset: Offset final na ROM
            palette: Paleta para renderização
            name: Nome para o arquivo de saída
            
        Returns:
            Caminho do arquivo PPM ou None
        """
        all_tiles = []
        
        offset = start_offset
        while offset < end_offset and offset < len(self.rom_data):
            # Verificar se há um opcode LZ válido
            opcode = self.rom_data[offset]
            
            if 0x10 <= opcode <= 0x17:
                # Tentar descomprimir
                decompressor = ZeldaLZDecompressor(self.rom_data)
                result = decompressor.decompress(offset, max_output=0x100000)
                
                if result and len(result) >= 32:
                    # Converter dados descomprimidos em tiles
                    num_tiles = len(result) // 32
                    
                    for i in range(min(num_tiles, 256)):  # Limitar a 256 tiles
                        tile_data = result[i * 32:(i + 1) * 32]
                        pixels = self.decode_4bpp_tile(tile_data)
                        
                        if pixels and any(p > 0 for p in pixels):
                            all_tiles.append(pixels)
                    
                    # Avançar além dos dados comprimidos
                    offset += len(result) + 10  # Aproximação
                else:
                    offset += 1
            else:
                offset += 1
        
        if not all_tiles:
            return None
            
        return self._tiles_to_ppm(all_tiles[:256], palette, name)
    
    def _tiles_to_ppm(self, tiles: List[List[int]], 
                     palette: List[Tuple[int, int, int]],
                     name: str,
                     tiles_per_row: int = 16) -> str:
        """Converte lista de tiles para imagem PPM"""
        if not tiles:
            return None
            
        tile_size = 8
        width = tiles_per_row * tile_size
        height = ((len(tiles) + tiles_per_row - 1) // tiles_per_row) * tile_size
        
        ppm_lines = [f"P3\n{width} {height}\n255"]
        
        for y in range(height):
            row = []
            for x in range(width):
                tile_idx = (y // tile_size) * tiles_per_row + (x // tile_size)
                pixel_idx = (y % tile_size) * tile_size + (x % tile_size)
                
                if tile_idx < len(tiles) and pixel_idx < len(tiles[tile_idx]):
                    color_idx = tiles[tile_idx][pixel_idx]
                    if color_idx < len(palette):
                        r, g, b = palette[color_idx]
                    else:
                        r, g, b = 0, 0, 0
                else:
                    r, g, b = 0, 0, 0
                    
                row.append(f"{r} {g} {b}")
            ppm_lines.append(" ".join(row))
        
        ppm_file = self.tiles_dir / f"{name}.ppm"
        with open(ppm_file, 'w') as f:
            f.write("\n".join(ppm_lines))
        
        print(f"  {name}: {len(tiles)} tiles -> {ppm_file}")
        return str(ppm_file)
    
    def extract_all_tiles(self, palettes: Dict[str, List[List[Tuple[int, int, int]]]]):
        """Extrai todos os tiles conhecidos"""
        print("\n=== Extraindo Tiles ===")
        
        # Usar paleta principal
        main_palette = palettes.get('main_bg', palettes.get('all_palettes', [[]])[0] if palettes.get('all_palettes') else [])
        
        if not main_palette:
            print("Nenhuma paleta disponível para renderização")
            return
        
        # 1. Tiles UI/Fontes (não comprimidos)
        print("\n--- Tiles UI/Fontes ---")
        ui_start, ui_end = self.TILE_REGIONS['ui_tiles']
        ui_count = (ui_end - ui_start) // 32  # 32 bytes por tile
        
        self.extract_raw_tiles(ui_start, min(ui_count, 512), main_palette, "ui_tiles")
        
        # 2. Tiles comprimidos
        print("\n--- Tiles Comprimidos ---")
        comp_start, comp_end = self.TILE_REGIONS['compressed_tiles']
        
        # Procurar e extrair blocos comprimidos
        offset = comp_start
        block_count = 0
        
        while offset < comp_end and block_count < 10:
            opcode = self.rom_data[offset] if offset < len(self.rom_data) else 0
            
            if 0x10 <= opcode <= 0x17:
                decompressor = ZeldaLZDecompressor(self.rom_data)
                result = decompressor.decompress(offset, max_output=0x20000)
                
                if result and len(result) >= 1024:
                    # Extrair tiles do bloco descomprimido
                    num_tiles = len(result) // 32
                    
                    if num_tiles > 0:
                        tiles = []
                        for i in range(min(num_tiles, 256)):
                            tile_data = result[i * 32:(i + 1) * 32]
                            pixels = self.decode_4bpp_tile(tile_data)
                            if pixels:
                                tiles.append(pixels)
                        
                        if tiles:
                            self._tiles_to_ppm(tiles, main_palette, 
                                              f"compressed_block_{block_count:02d}")
                            block_count += 1
                    
                    offset += len(result) + 10
                else:
                    offset += 1
            else:
                offset += 1
    
    def run(self):
        """Executa a extração completa"""
        print("=" * 60)
        print("EXTRATOR DE ASSETS - The Legend of Zelda: A Link to the Past")
        print("=" * 60)
        
        # Carregar ROM
        if not self.load_rom():
            return False
        
        # Criar diretórios
        self.setup_dirs()
        
        # Extrair paletas
        palettes = self.extract_palettes()
        
        # Salvar paletas
        self.save_palettes(palettes)
        
        # Extrair tiles
        self.extract_all_tiles(palettes)
        
        print("\n" + "=" * 60)
        print("EXTRAÇÃO CONCLUÍDA")
        print("=" * 60)
        print(f"Paletas salvas em: {self.palettes_dir}")
        print(f"Tiles salvos em: {self.tiles_dir}")
        
        # Listar arquivos gerados
        print("\nArquivos gerados:")
        for f in sorted(self.palettes_dir.glob("*")):
            print(f"  {f.name}")
        for f in sorted(self.tiles_dir.glob("*")):
            print(f"  {f.name}")
        
        return True


def main():
    """Função principal"""
    rom_path = "/home/douglasdreer/projects/decompiler/snes/the-legend-of-zelda-link-of-the-past/roms/Legend of Zelda, The - A Link to the Past (USA).sfc"
    
    extractor = ALttPExtractor(rom_path)
    success = extractor.run()
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
