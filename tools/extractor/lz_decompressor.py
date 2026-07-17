#!/usr/bin/env python3
"""
Descompressor LZ proprietário do Zelda: A Link to the Past
Formato: opcodes $10-$1C com dados comprimidos
"""

import struct
from typing import Optional


class ZeldaLZDecompressor:
    """Descompressor LZ para o formato proprietário do Zelda: A Link to the Past"""
    
    # Opcodes conhecidos
    OPCODE_END = 0x00
    OPCODE_LITERAL = 0x10      # Copia bytes literais
    OPCODE_BYTE_FILL = 0x11    # Preenche com byte repetido
    OPCODE_WORD_FILL = 0x12    # Preenche com word (16-bit) repetido
    OPCODE_INC_FILL = 0x13     # Preenche com incremento
    OPCODE_BACK_REF_LONG = 0x14  # Referência para trás (offset 16-bit)
    OPCODE_BACK_REF_SHORT = 0x15  # Referência para trás (offset 8-bit)
    OPCODE_VRAM_COPY = 0x16    # Cópia para VRAM (ignorado na descompressão)
    OPCODE_SPECIAL = 0x17      # Operação especial
    OPCODE_END_MARKER = 0x18   # Fim dos dados
    
    def __init__(self, data: bytes):
        """
        Inicializa o descompressor com dados da ROM
        
        Args:
            data: Bytes da ROM
        """
        self.data = data
        self.pos = 0
        self.output = bytearray()
        
    def reset(self, start_pos: int = 0):
        """Reseta a posição de leitura"""
        self.pos = start_pos
        self.output = bytearray()
        
    def read_byte(self) -> Optional[int]:
        """Lê um byte dos dados de entrada"""
        if self.pos >= len(self.data):
            return None
        byte = self.data[self.pos]
        self.pos += 1
        return byte
    
    def read_word(self) -> Optional[int]:
        """Lê um word (16-bit little-endian) dos dados de entrada"""
        if self.pos + 2 > len(self.data):
            return None
        word = struct.unpack('<H', self.data[self.pos:self.pos + 2])[0]
        self.pos += 2
        return word
    
    def decompress(self, start_pos: int = 0, max_output: int = 0x100000) -> Optional[bytes]:
        """
        Descomprime dados LZ do Zelda
        
        Args:
            start_pos: Posição inicial na ROM
            max_output: Tamanho máximo da saída
            
        Returns:
            Dados descomprimidos ou None se erro
        """
        self.reset(start_pos)
        
        try:
            while len(self.output) < max_output:
                opcode = self.read_byte()
                
                if opcode is None:
                    break
                    
                # Fim dos dados
                if opcode == self.OPCODE_END or opcode >= self.OPCODE_END_MARKER:
                    break
                    
                # Extrair tipo do opcode (bits 7-4)
                cmd_type = opcode & 0xF0
                
                if cmd_type == 0x00:
                    # Opcode não reconhecido ou fim
                    break
                    
                elif cmd_type == 0x10:
                    # Copia bytes literais
                    length = self._read_length(opcode)
                    for _ in range(length):
                        byte = self.read_byte()
                        if byte is None:
                            return None
                        self.output.append(byte)
                        
                elif cmd_type == 0x20:
                    # Preenche com byte repetido
                    length = self._read_length(opcode)
                    fill_byte = self.read_byte()
                    if fill_byte is None:
                        return None
                    for _ in range(length):
                        self.output.append(fill_byte)
                        
                elif cmd_type == 0x30:
                    # Preenche com word repetido
                    length = self._read_length(opcode)
                    fill_word = self.read_word()
                    if fill_word is None:
                        return None
                    lo = fill_word & 0xFF
                    hi = (fill_word >> 8) & 0xFF
                    for _ in range(length):
                        self.output.append(lo)
                        self.output.append(hi)
                        
                elif cmd_type == 0x40:
                    # Preenche com incremento
                    length = self._read_length(opcode)
                    start_byte = self.read_byte()
                    if start_byte is None:
                        return None
                    for i in range(length):
                        self.output.append((start_byte + i) & 0xFF)
                        
                elif cmd_type == 0x50:
                    # Referência para trás com offset 16-bit
                    length = self._read_length(opcode)
                    offset = self.read_word()
                    if offset is None:
                        return None
                    self._copy_backref(offset, length)
                    
                elif cmd_type == 0x60:
                    # Referência para trás com offset 8-bit
                    length = self._read_length(opcode)
                    offset = self.read_byte()
                    if offset is None:
                        return None
                    self._copy_backref(offset, length)
                    
                elif cmd_type == 0x70:
                    # Cópia para VRAM (ignorado)
                    length = self._read_length(opcode)
                    # Pular esses bytes
                    for _ in range(length):
                        self.read_byte()
                        
                else:
                    # Opcode não reconhecido, tentar pular
                    break
                    
        except Exception as e:
            return None
            
        return bytes(self.output)
    
    def _read_length(self, opcode: int) -> int:
        """
        Lê o comprimento baseado no opcode
        O nibble baixo do opcode é parte do comprimento
        """
        low_nibble = opcode & 0x0F
        
        # Ler byte extra para comprimento
        extra = self.read_byte()
        if extra is None:
            return 0
            
        # Combinar nibble baixo com byte extra
        # Formato típico: (low_nibble << 8) | extra + 1
        length = ((low_nibble << 8) | extra) + 1
        
        return length
    
    def _copy_backref(self, offset: int, length: int):
        """Copia dados de uma referência para trás"""
        src_pos = len(self.output) - offset
        
        if src_pos < 0:
            # Se offset é maior que o output atual, preencher com zeros
            for _ in range(length):
                self.output.append(0)
            return
            
        for _ in range(length):
            if src_pos < len(self.output):
                self.output.append(self.output[src_pos])
            else:
                self.output.append(0)
            src_pos += 1


def find_lz_blocks(rom_data: bytes, search_range: tuple = (0x1000, 0x100000)) -> list:
    """
    Procura por blocos LZ comprimidos na ROM
    
    Args:
        rom_data: Dados da ROM
        search_range: Range de offsets para buscar
        
    Returns:
        Lista de (offset, decompressed_size) dos blocos encontrados
    """
    blocks = []
    
    for offset in range(search_range[0], min(search_range[1], len(rom_data))):
        # Verificar se o byte atual parece ser um opcode válido
        opcode = rom_data[offset]
        
        # Opcodes válidos: $10-$17
        if 0x10 <= opcode <= 0x17:
            # Tentar descomprimir a partir deste offset
            decompressor = ZeldaLZDecompressor(rom_data)
            result = decompressor.decompress(offset, max_output=0x10000)
            
            if result and len(result) > 16:  # Blocos muito pequenos provavelmente não são tiles
                blocks.append((offset, len(result)))
    
    return blocks


def main():
    """Função de teste"""
    import sys
    
    if len(sys.argv) < 2:
        print("Uso: python lz_decompressor.py <rom_path> [offset]")
        sys.exit(1)
    
    rom_path = sys.argv[1]
    
    with open(rom_path, 'rb') as f:
        rom_data = f.read()
    
    print(f"ROM carregada: {len(rom_data)} bytes")
    
    if len(sys.argv) > 2:
        # Descomprimir de um offset específico
        offset = int(sys.argv[2], 16)
        print(f"\nDescomprimindo de offset 0x{offset:06X}...")
        
        decompressor = ZeldaLZDecompressor(rom_data)
        result = decompressor.decompress(offset)
        
        if result:
            print(f"Sucesso! {len(result)} bytes descomprimidos")
            # Salvar resultado
            output_file = f"decompressed_0x{offset:06X}.bin"
            with open(output_file, 'wb') as f:
                f.write(result)
            print(f"Salvo em: {output_file}")
        else:
            print("Falha na descompressão")
    else:
        # Procurar blocos comprimidos
        print("\nProcurando por blocos comprimidos...")
        blocks = find_lz_blocks(rom_data, (0x1000, 0x5000))
        
        print(f"\nEncontrados {len(blocks)} blocos potenciais:")
        for offset, size in blocks[:20]:
            print(f"  0x{offset:06X}: {size} bytes descomprimidos")


if __name__ == "__main__":
    main()
