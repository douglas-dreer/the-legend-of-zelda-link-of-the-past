#!/usr/bin/env python3
"""
Extrator de Assets - The Legend of Zelda: A Link to the Past
Extrai gráficos, tiles e paletas da ROM
"""

import os
import sys
from pathlib import Path
from typing import List, Tuple, Optional
import struct

class SNESAssetExtractor:
    """Extrator de assets para ROMs SNES"""
    
    # Formatos de tile suportados
    TILE_FORMATS = {
        2: "2bpp (2 bits por pixel)",
        4: "4bpp (4 bits por pixel)",
        8: "8bpp (8 bits por pixel)",
    }
    
    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.output_dir = Path("output")
        
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
        
    def extract_tiles(self, offset: int, count: int, bpp: int = 4, 
                      tile_size: int = 8) -> List[List[int]]:
        """
        Extrai tiles da ROM
        
        Args:
            offset: Offset na ROM onde começam os tiles
            count: Número de tiles a extrair
            bpp: Bits por pixel (2, 4 ou 8)
            tile_size: Tamanho do tile (8x8 padrão)
            
        Returns:
            Lista de tiles, onde cada tile é uma lista de pixels
        """
        if self.rom_data is None:
            return []
            
        tiles = []
        bytes_per_tile = (tile_size * tile_size * bpp) // 8
        
        for i in range(count):
            tile_offset = offset + (i * bytes_per_tile)
            
            if tile_offset + bytes_per_tile > len(self.rom_data):
                break
                
            tile_data = self.rom_data[tile_offset:tile_offset + bytes_per_tile]
            tile = self._decode_tile(tile_data, bpp, tile_size)
            tiles.append(tile)
            
        return tiles
        
    def _decode_tile(self, data: bytes, bpp: int, tile_size: int) -> List[int]:
        """Decodifica um tile de bytes para pixels"""
        pixels = []
        
        if bpp == 2:
            # 2bpp: Cada byte contém 4 pixels
            for y in range(tile_size):
                for byte_idx in range(0, tile_size // 4, 2):
                    if byte_idx + 1 < len(data):
                        plane0 = data[byte_idx + y * 2]
                        plane1 = data[byte_idx + y * 2 + 1]
                        
                        for x in range(4):
                            pixel = ((plane0 >> (3 - x)) & 1) | \
                                   (((plane1 >> (3 - x)) & 1) << 1)
                            pixels.append(pixel)
                            
        elif bpp == 4:
            # 4bpp: Cada byte contém 8 pixels
            for y in range(tile_size):
                plane0 = data[y * 2]
                plane1 = data[y * 2 + 1]
                plane2 = data[y * 2 + 8]
                plane3 = data[y * 2 + 9]
                
                for x in range(8):
                    pixel = ((plane0 >> (7 - x)) & 1) | \
                           (((plane1 >> (7 - x)) & 1) << 1) | \
                           (((plane2 >> (7 - x)) & 1) << 2) | \
                           (((plane3 >> (7 - x)) & 1) << 3)
                    pixels.append(pixel)
                    
        elif bpp == 8:
            # 8bpp: 4 planos, cada 2 bytes por linha
            for y in range(tile_size):
                plane0 = data[y * 2]
                plane1 = data[y * 2 + 1]
                plane2 = data[y * 2 + 16]
                plane3 = data[y * 2 + 17]
                plane4 = data[y * 2 + 32]
                plane5 = data[y * 2 + 33]
                plane6 = data[y * 2 + 48]
                plane7 = data[y * 2 + 49]
                
                for x in range(8):
                    pixel = ((plane0 >> (7 - x)) & 1) | \
                           (((plane1 >> (7 - x)) & 1) << 1) | \
                           (((plane2 >> (7 - x)) & 1) << 2) | \
                           (((plane3 >> (7 - x)) & 1) << 3) | \
                           (((plane4 >> (7 - x)) & 1) << 4) | \
                           (((plane5 >> (7 - x)) & 1) << 5) | \
                           (((plane6 >> (7 - x)) & 1) << 6) | \
                           (((plane7 >> (7 - x)) & 1) << 7)
                    pixels.append(pixel)
                    
        return pixels
        
    def extract_palette(self, offset: int, count: int = 16) -> List[List[Tuple[int, int, int]]]:
        """
        Extrai paletas da ROM
        
        Args:
            offset: Offset na ROM onde começa a paleta
            count: Número de cores na paleta
            
        Returns:
            Lista de paletas, onde cada paleta é uma lista de cores RGB
        """
        if self.rom_data is None:
            return []
            
        palettes = []
        
        # Cada cor SNES é 2 bytes (15 bits: 5 bits por canal)
        for i in range(count):
            color_offset = offset + (i * 2)
            
            if color_offset + 2 > len(self.rom_data):
                break
                
            color_data = self.rom_data[color_offset:color_offset + 2]
            color = self._decode_color(color_data)
            palettes.append(color)
            
        return palettes
        
    def _decode_color(self, data: bytes) -> Tuple[int, int, int]:
        """Decodifica uma cor SNES para RGB"""
        # Formato SNES: gggrrrrr 0bbbbbgg
        color_word = data[0] | (data[1] << 8)
        
        r = (color_word & 0x1F) << 3
        g = ((color_word >> 5) & 0x1F) << 3
        b = ((color_word >> 10) & 0x1F) << 3
        
        return (r, g, b)
        
    def tiles_to_image(self, tiles: List[List[int]], palette: List[Tuple[int, int, int]],
                       tiles_per_row: int = 16) -> str:
        """
        Converte tiles para formato PPM (Portable Pixmap)
        
        Args:
            tiles: Lista de tiles
            palette: Paleta de cores
            tiles_per_row: Número de tiles por linha
            
        Returns:
            String no formato PPM
        """
        if not tiles:
            return ""
            
        tile_size = 8
        width = tiles_per_row * tile_size
        height = (len(tiles) // tiles_per_row + 1) * tile_size
        
        # Cabeçalho PPM
        ppm = f"P3\n{width} {height}\n255\n"
        
        # Gerar pixels
        for y in range(height):
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
                    
                ppm += f"{r} {g} {b}\n"
                
        return ppm
        
    def save_tiles_as_image(self, tiles: List[List[int]], palette: List[Tuple[int, int, int]],
                            output_file: str, tiles_per_row: int = 16):
        """Salva tiles como imagem PPM"""
        ppm_data = self.tiles_to_image(tiles, palette, tiles_per_row)
        
        with open(output_file, 'w') as f:
            f.write(ppm_data)
            
        print(f"Tiles salvos em: {output_file}")
        
    def extract_and_save_tiles(self, offset: int, count: int, bpp: int,
                               palette_offset: int, output_file: str):
        """Extrai e salva tiles com paleta"""
        tiles = self.extract_tiles(offset, count, bpp)
        palette = self.extract_palette(palette_offset, 16)
        
        if tiles and palette:
            self.save_tiles_as_image(tiles, palette, output_file)
            return True
        return False


def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python asset_extractor.py <caminho_para_rom>")
        sys.exit(1)
        
    rom_path = sys.argv[1]
    extractor = SNESAssetExtractor(rom_path)
    
    if not extractor.load_rom():
        sys.exit(1)
        
    # Criar diretório de saída
    extractor.output_dir.mkdir(exist_ok=True)
    
    # Exemplo: Extrair tiles do início da ROM (ajustar conforme necessário)
    # Estes valores são apenas exemplos - precisam ser ajustados para o jogo específico
    print("\nExemplo de extração de tiles:")
    print("  Offset: 0x000000")
    print("  Count: 256 tiles")
    print("  BPP: 4")
    print("  Palette: 0x0000")
    
    # Extrair tiles de exemplo
    tiles = extractor.extract_tiles(0x000000, 256, bpp=4)
    palette = extractor.extract_palette(0x000000, 16)
    
    if tiles and palette:
        output_file = extractor.output_dir / "tiles_example.ppm"
        extractor.save_tiles_as_image(tiles, palette, str(output_file))
        print(f"\n✓ Extraído {len(tiles)} tiles")
    else:
        print("\n✗ Nenhum tile extraído")
        
    print("\nNota: Os offsets acima são exemplos. Ajuste conforme a estrutura da ROM.")


if __name__ == "__main__":
    main()