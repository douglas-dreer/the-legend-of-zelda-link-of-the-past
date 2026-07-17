#!/usr/bin/env python3
"""
Script Principal - Engenharia Reversa de ROM
The Legend of Zelda: A Link to the Past
"""

import os
import sys
from pathlib import Path

# Adicionar diretórios ao path
sys.path.insert(0, str(Path(__file__).parent / "parser"))
sys.path.insert(0, str(Path(__file__).parent / "extractor"))
sys.path.insert(0, str(Path(__file__).parent / "disassembler"))

from rom_parser import SNESROMParser
from asset_extractor import SNESAssetExtractor
from audio_extractor import SNESAudioExtractor
from disassembler import SNESDisassembler

def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python main.py <caminho_para_rom>")
        print("\nExemplo:")
        print("  python main.py \"roms/Legend of Zelda, The - A Link to the Past (USA).sfc\"")
        sys.exit(1)
        
    rom_path = sys.argv[1]
    
    print("="*70)
    print("ENGENHARIA REVERSA - THE LEGEND OF ZELDA: A LINK TO THE PAST")
    print("="*70)
    
    # 1. Analisar ROM
    print("\n[Fase 1] Analisando ROM...")
    parser = SNESROMParser(rom_path)
    if not parser.load_rom():
        sys.exit(1)
        
    header = parser.read_header()
    if header is None:
        print("Erro ao ler header da ROM")
        sys.exit(1)
        
    parser.print_header()
    
    if parser.verify_checksum():
        print("\n✓ Checksum válido!")
    else:
        print("\n✗ Checksum inválido!")
        
    # 2. Extrair assets (exemplo)
    print("\n[Fase 2] Extraindo assets...")
    extractor = SNESAssetExtractor(rom_path)
    if extractor.load_rom():
        # Criar diretório de saída
        extractor.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Exemplo: Extrair tiles de 4bpp
        # NOTA: Estes valores são exemplos - precisam ser ajustados para o jogo específico
        print("  Extraindo tiles de exemplo...")
        tiles = extractor.extract_tiles(0x000000, 256, bpp=4)
        palette = extractor.extract_palette(0x000000, 16)
        
        if tiles and palette:
            output_file = extractor.output_dir / "tiles_example.ppm"
            extractor.save_tiles_as_image(tiles, palette, str(output_file))
            print(f"  ✓ Extraído {len(tiles)} tiles")
        else:
            print("  ✗ Nenhum tile extraído")
            
    # 3. Extrair áudio (exemplo)
    print("\n[Fase 3] Extraindo áudio...")
    audio = SNESAudioExtractor(rom_path)
    if audio.load_rom():
        audio.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Exemplo: Extrair sample de áudio
        print("  Extraindo sample de exemplo...")
        output_file = audio.output_dir / "sample_example.wav"
        if audio.extract_and_save_sample(0x000000, 1024, 22050, str(output_file)):
            print("  ✓ Sample extraído")
        else:
            print("  ✗ Nenhum sample extraído")
            
    # 4. Desassemblar código (exemplo)
    print("\n[Fase 4] Desassemblando código...")
    disasm = SNESDisassembler(rom_path)
    if disasm.load_rom():
        disasm.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Exemplo: Desassemblar bloco de código
        print("  Desassemblando bloco de exemplo...")
        output_file = disasm.output_dir / "code_example.asm"
        if disasm.disassemble_and_save(0x000000, 256, str(output_file)):
            print("  ✓ Código desassembaldo")
        else:
            print("  ✗ Nenhum código desassembaldo")
            
    print("\n" + "="*70)
    print("ANÁLISE CONCLUÍDA!")
    print("="*70)
    print("\nPróximos passos:")
    print("1. Analisar a estrutura da ROM para encontrar offsets reais")
    print("2. Ajustar parâmetros nas ferramentas para extrair dados reais")
    print("3. Criar ferramentas mais avançadas de análise")
    print("\nConsulte a documentação em docs/ para mais informações.")


if __name__ == "__main__":
    main()