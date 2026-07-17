#!/usr/bin/env python3
"""
Extrator de Áudio - The Legend of Zelda: A Link to the Past
Extrai samples de áudio da ROM
"""

import os
import sys
import struct
from pathlib import Path
from typing import List, Optional
import wave

class SNESAudioExtractor:
    """Extrator de áudio para ROMs SNES"""
    
    # Frequências de amostragem do SNES
    SAMPLE_RATES = {
        0: 8000,
        1: 11025,
        2: 22050,
        3: 44100,
    }
    
    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.output_dir = Path("output/audio")
        
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
        
    def extract_sample(self, offset: int, length: int, 
                       sample_rate: int = 22050) -> List[int]:
        """
        Extrai um sample de áudio da ROM
        
        Args:
            offset: Offset na ROM onde começa o sample
            length: Tamanho do sample em bytes
            sample_rate: Taxa de amostragem
            
        Returns:
            Lista de amostras (16-bit signed)
        """
        if self.rom_data is None:
            return []
            
        if offset + length > len(self.rom_data):
            print(f"Erro: Offset {offset} + {length} excede o tamanho da ROM")
            return []
            
        # Ler dados do sample
        sample_data = self.rom_data[offset:offset + length]
        
        # Converter de unsigned 8-bit para signed 16-bit
        samples = []
        for byte in sample_data:
            # SNES usa sample unsigned 8-bit
            # Converter para signed 16-bit
            sample = (byte - 128) * 256
            samples.append(sample)
            
        return samples
        
    def extract_spc700_sample(self, offset: int, length: int,
                              sample_rate: int = 32000) -> List[int]:
        """
        Extrai um sample no formato SPC700 (8-bit signed)
        
        Args:
            offset: Offset na ROM
            length: Tamanho em bytes
            sample_rate: Taxa de amostragem
            
        Returns:
            Lista de amostras (16-bit signed)
        """
        if self.rom_data is None:
            return []
            
        if offset + length > len(self.rom_data):
            return []
            
        sample_data = self.rom_data[offset:offset + length]
        
        samples = []
        for byte in sample_data:
            # SPC700 usa sample signed 8-bit
            sample = struct.unpack('b', bytes([byte]))[0] * 256
            samples.append(sample)
            
        return samples
        
    def samples_to_wav(self, samples: List[int], sample_rate: int = 22050,
                       filename: str = "output.wav"):
        """
        Converte samples para formato WAV
        
        Args:
            samples: Lista de amostras (16-bit signed)
            sample_rate: Taxa de amostragem
            filename: Nome do arquivo de saída
        """
        if not samples:
            print("Nenhum sample para converter")
            return
            
        # Criar arquivo WAV
        with wave.open(filename, 'w') as wav_file:
            wav_file.setnchannels(1)  # Mono
            wav_file.setsampwidth(2)  # 16-bit
            wav_file.setframerate(sample_rate)
            
            # Escrever samples
            for sample in samples:
                # Garantir que o sample está no range correto
                sample = max(-32768, min(32767, sample))
                wav_file.writeframes(struct.pack('<h', sample))
                
        print(f"Sample salvo em: {filename}")
        
    def extract_and_save_sample(self, offset: int, length: int,
                                sample_rate: int = 22050, output_file: str = None):
        """Extrai e salva um sample como WAV"""
        samples = self.extract_sample(offset, length, sample_rate)
        
        if samples:
            if output_file is None:
                output_file = f"sample_{offset:08X}.wav"
                
            self.samples_to_wav(samples, sample_rate, output_file)
            print(f"✓ Sample extraído: {len(samples)} amostras")
            return True
        return False
        
    def extract_samples_from_block(self, block_offset: int, block_size: int,
                                   sample_rate: int = 22050, output_dir: str = None):
        """
        Extrai múltiplos samples de um bloco de áudio
        
        Args:
            block_offset: Offset do bloco na ROM
            block_size: Tamanho do bloco
            sample_rate: Taxa de amostragem
            output_dir: Diretório de saída
        """
        if output_dir is None:
            output_dir = self.output_dir
            
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Ler cabeçalho do bloco
        if block_offset + 16 > len(self.rom_data):
            print("Erro: Bloco muito pequeno")
            return
            
        # Ler informações do bloco
        block_header = self.rom_data[block_offset:block_offset + 16]
        
        # Número de samples no bloco
        num_samples = struct.unpack('<H', block_header[0:2])[0]
        
        print(f"Bloco em 0x{block_offset:08X}: {num_samples} samples")
        
        # Extrair cada sample
        sample_offset = block_offset + 16
        
        for i in range(num_samples):
            if sample_offset + 4 > len(self.rom_data):
                break
                
            # Ler tamanho do sample
            sample_length = struct.unpack('<H', self.rom_data[sample_offset:sample_offset + 2])[0]
            sample_offset += 2
            
            # Ler sample
            if sample_offset + sample_length > len(self.rom_data):
                break
                
            samples = self.extract_sample(sample_offset, sample_length, sample_rate)
            
            if samples:
                output_file = output_path / f"sample_{i:04d}.wav"
                self.samples_to_wav(samples, sample_rate, str(output_file))
                print(f"  Sample {i}: {sample_length} bytes → {output_file}")
                
            sample_offset += sample_length


def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python audio_extractor.py <caminho_para_rom>")
        sys.exit(1)
        
    rom_path = sys.argv[1]
    extractor = SNESAudioExtractor(rom_path)
    
    if not extractor.load_rom():
        sys.exit(1)
        
    # Criar diretório de saída
    extractor.output_dir.mkdir(parents=True, exist_ok=True)
    
    # Exemplo: Extrair sample do início da ROM (ajustar conforme necessário)
    print("\nExemplo de extração de áudio:")
    print("  Offset: 0x000000")
    print("  Tamanho: 1024 bytes")
    print("  Sample Rate: 22050 Hz")
    
    # Extrair sample de exemplo
    output_file = extractor.output_dir / "sample_example.wav"
    extractor.extract_and_save_sample(0x000000, 1024, 22050, str(output_file))
    
    print("\nNota: Os offsets acima são exemplos. Ajuste conforme a estrutura da ROM.")
    print("Para áudio SPC700, ajuste o sample_rate para 32000 Hz.")


if __name__ == "__main__":
    main()