#!/usr/bin/env python3
"""
Desassembler 65816 - The Legend of Zelda: A Link to the Past
Converte binário para Assembly 65816
"""

import os
import sys
from pathlib import Path
from typing import List, Dict, Optional, Tuple
import struct

class SNESDisassembler:
    """Desassembler para CPU 65816 do SNES"""
    
    # Instruções 65816 com seus opcodes
    # Formato: opcode -> (mnemônico, modo de endereçamento, tamanho)
    INSTRUCTIONS = {
        # Instruções de carga
        0xA9: ("LDA", "immediato", 2),
        0xAD: ("LDA", "absoluto", 3),
        0xBD: ("LDA", "absoluto_x", 3),
        0xB9: ("LDA", "absoluto_y", 3),
        0xA5: ("LDA", "direto", 2),
        0xB5: ("LDA", "direto_x", 2),
        0xA1: ("LDA", "indireto_x", 2),
        0xB1: ("LDA", "indireto_y", 2),
        0xA7: ("LDA", "indireto_long", 2),
        0xBF: ("LDA", "longo_x", 3),
        0xA2: ("LDX", "immediato", 2),
        0xAE: ("LDX", "absoluto", 3),
        0xB6: ("LDX", "direto_y", 2),
        0xA0: ("LDY", "immediato", 2),
        0xAC: ("LDY", "absoluto", 3),
        0xB4: ("LDY", "direto_x", 2),
        
        # Instruções de armazenamento
        0x8D: ("STA", "absoluto", 3),
        0x9D: ("STA", "absoluto_x", 3),
        0x99: ("STA", "absoluto_y", 3),
        0x85: ("STA", "direto", 2),
        0x95: ("STA", "direto_x", 2),
        0x81: ("STA", "indireto_x", 2),
        0x91: ("STA", "indireto_y", 2),
        0x87: ("STA", "indireto_long", 2),
        0x9F: ("STA", "longo_x", 3),
        0x8E: ("STX", "absoluto", 3),
        0x86: ("STX", "direto", 2),
        0x96: ("STX", "direto_y", 2),
        0x8C: ("STY", "absoluto", 3),
        0x84: ("STY", "direto", 2),
        0x94: ("STY", "direto_x", 2),
        
        # Instruções aritméticas
        0x69: ("ADC", "immediato", 2),
        0x6D: ("ADC", "absoluto", 3),
        0x7D: ("ADC", "absoluto_x", 3),
        0x79: ("ADC", "absoluto_y", 3),
        0x65: ("ADC", "direto", 2),
        0x75: ("ADC", "direto_x", 2),
        0x61: ("ADC", "indireto_x", 2),
        0x71: ("ADC", "indireto_y", 2),
        0xE9: ("SBC", "immediato", 2),
        0xED: ("SBC", "absoluto", 3),
        0xFD: ("SBC", "absoluto_x", 3),
        0xF9: ("SBC", "absoluto_y", 3),
        0xE5: ("SBC", "direto", 2),
        0xF5: ("SBC", "direto_x", 2),
        0xE1: ("SBC", "indireto_x", 2),
        0xF1: ("SBC", "indireto_y", 2),
        
        # Instruções lógicas
        0x09: ("ORA", "immediato", 2),
        0x0D: ("ORA", "absoluto", 3),
        0x1D: ("ORA", "absoluto_x", 3),
        0x19: ("ORA", "absoluto_y", 3),
        0x05: ("ORA", "direto", 2),
        0x15: ("ORA", "direto_x", 2),
        0x01: ("ORA", "indireto_x", 2),
        0x11: ("ORA", "indireto_y", 2),
        0x29: ("AND", "immediato", 2),
        0x2D: ("AND", "absoluto", 3),
        0x3D: ("AND", "absoluto_x", 3),
        0x39: ("AND", "absoluto_y", 3),
        0x25: ("AND", "direto", 2),
        0x35: ("AND", "direto_x", 2),
        0x21: ("AND", "indireto_x", 2),
        0x31: ("AND", "indireto_y", 2),
        0x49: ("EOR", "immediato", 2),
        0x4D: ("EOR", "absoluto", 3),
        0x5D: ("EOR", "absoluto_x", 3),
        0x59: ("EOR", "absoluto_y", 3),
        0x45: ("EOR", "direto", 2),
        0x55: ("EOR", "direto_x", 2),
        0x41: ("EOR", "indireto_x", 2),
        0x51: ("EOR", "indireto_y", 2),
        
        # Instruções de incremento/decremento
        0xE8: ("INX", "implícito", 1),
        0xC8: ("INY", "implícito", 1),
        0xCA: ("DEX", "implícito", 1),
        0x88: ("DEY", "implícito", 1),
        0xEE: ("INC", "absoluto", 3),
        0xFE: ("INC", "absoluto_x", 3),
        0xE6: ("INC", "direto", 2),
        0xF6: ("INC", "direto_x", 2),
        0xCE: ("DEC", "absoluto", 3),
        0xDE: ("DEC", "absoluto_x", 3),
        0xC6: ("DEC", "direto", 2),
        0xD6: ("DEC", "direto_x", 2),
        
        # Instruções de salto
        0x4C: ("JMP", "absoluto", 3),
        0x6C: ("JMP", "indireto", 3),
        0x7C: ("JMP", "indireto_x", 3),
        0x5C: ("JMP", "longo", 4),
        0x20: ("JSR", "absoluto", 3),
        0xFC: ("JSR", "indireto_x", 3),
        0x00: ("BRK", "imediato", 2),
        0x40: ("RTI", "implícito", 1),
        0x60: ("RTS", "implícito", 1),
        0x6B: ("RTL", "implícito", 1),
        
        # Instruções de branch
        0x10: ("BPL", "relativo", 2),
        0x30: ("BMI", "relativo", 2),
        0x50: ("BVC", "relativo", 2),
        0x70: ("BVS", "relativo", 2),
        0x90: ("BCC", "relativo", 2),
        0xB0: ("BCS", "relativo", 2),
        0xD0: ("BNE", "relativo", 2),
        0xF0: ("BEQ", "relativo", 2),
        0x80: ("BRA", "relativo", 2),
        0x82: ("BRL", "relativo_longo", 3),
        
        # Instruções de pilha
        0x48: ("PHA", "implícito", 1),
        0x8A: ("TXA", "implícito", 1),
        0x98: ("TYA", "implícito", 1),
        0xDA: ("PHX", "implícito", 1),
        0x5A: ("PHY", "implícito", 1),
        0x68: ("PLA", "implícito", 1),
        0xAA: ("TAX", "implícito", 1),
        0xA8: ("TAY", "implícito", 1),
        0xFA: ("PLX", "implícito", 1),
        0x7A: ("PLY", "implícito", 1),
        
        # Instruções de transferência
        0x9B: ("TXY", "implícito", 1),
        0xBB: ("TYX", "implícito", 1),
        0x18: ("CLC", "implícito", 1),
        0x38: ("SEC", "implícito", 1),
        0x58: ("CLI", "implícito", 1),
        0x78: ("SEI", "implícito", 1),
        0xD8: ("CLD", "implícito", 1),
        0xF8: ("SED", "implícito", 1),
        0xB8: ("CLV", "implícito", 1),
        
        # Instruções de bitwise
        0x4A: ("LSR", "acumulador", 1),
        0x4E: ("LSR", "absoluto", 3),
        0x46: ("LSR", "direto", 2),
        0x0A: ("ASL", "acumulador", 1),
        0x0E: ("ASL", "absoluto", 3),
        0x06: ("ASL", "direto", 2),
        0x2A: ("ROL", "acumulador", 1),
        0x2E: ("ROL", "absoluto", 3),
        0x26: ("ROL", "direto", 2),
        0x6A: ("ROR", "acumulador", 1),
        0x6E: ("ROR", "absoluto", 3),
        0x66: ("ROR", "direto", 2),
        
        # Instruções de comparação
        0xC9: ("CMP", "immediato", 2),
        0xCD: ("CMP", "absoluto", 3),
        0xDD: ("CMP", "absoluto_x", 3),
        0xD9: ("CMP", "absoluto_y", 3),
        0xC5: ("CMP", "direto", 2),
        0xD5: ("CMP", "direto_x", 2),
        0xC1: ("CMP", "indireto_x", 2),
        0xD1: ("CMP", "indireto_y", 2),
        0xE0: ("CPX", "immediato", 2),
        0xEC: ("CPX", "absoluto", 3),
        0xE4: ("CPX", "direto", 2),
        0xC0: ("CPY", "immediato", 2),
        0xCC: ("CPY", "absoluto", 3),
        0xC4: ("CPY", "direto", 2),
        
        # Instruções especiais
        0x08: ("PHP", "implícito", 1),
        0x28: ("PLP", "implícito", 1),
        0xEB: ("XBA", "implícito", 1),
        0xFB: ("XCE", "implícito", 1),
        0xDB: ("STP", "implícito", 1),
        0xCB: ("WAI", "implícito", 1),
        0x89: ("BIT", "immediato", 2),
        0x2C: ("BIT", "absoluto", 3),
        0x24: ("BIT", "direto", 2),
        0x3C: ("BIT", "absoluto_x", 3),
        0x14: ("TRB", "direto", 2),
        0x1C: ("TRB", "absoluto", 3),
        0x24: ("TSB", "direto", 2),
        0x0C: ("TSB", "absoluto", 3),
        
        # Instruções de manipulação de flags
        0xC2: ("REP", "imediato", 2),
        0xE2: ("SEP", "imediato", 2),
        0xEA: ("NOP", "implícito", 1),
        0x42: ("WDM", "immediato", 2),
        0x9C: ("STZ", "absoluto", 3),
        0x9E: ("STZ", "absoluto_x", 3),
        0x64: ("STZ", "direto", 2),
        0x74: ("STZ", "direto_x", 2),
    }
    
    # Nomes dos modos de endereçamento para formatação
    ADDRESSING_MODE_NAMES = {
        "implícito": "",
        "acumulador": "A",
        "immediato": "#${:02X}",
        "direto": "${:02X}",
        "direto_x": "${:02X},X",
        "direto_y": "${:02X},Y",
        "indireto": "(${:02X})",
        "indireto_x": "(${:02X},X)",
        "indireto_y": "(${:02X}),Y",
        "indireto_long": "[${:02X}]",
        "absoluto": "${:04X}",
        "absoluto_x": "${:04X},X",
        "absoluto_y": "${:04X},Y",
        "longo": "${:06X}",
        "longo_x": "${:06X},X",
        "relativo": "{}",
        "relativo_longo": "{}",
    }
    
    def __init__(self, rom_path: str):
        self.rom_path = Path(rom_path)
        self.rom_data = None
        self.output_dir = Path("output/assembly")
        
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
        
    def disassemble_instruction(self, offset: int) -> Tuple[str, int]:
        """
        Desassembla uma instrução
        
        Args:
            offset: Offset da instrução na ROM
            
        Returns:
            Tuple com (instrução formatada, tamanho da instrução)
        """
        if self.rom_data is None or offset >= len(self.rom_data):
            return ("; Endereço inválido", 1)
            
        opcode = self.rom_data[offset]
        
        if opcode not in self.INSTRUCTIONS:
            return (f".db ${opcode:02X} ; Opcode desconhecido", 1)
            
        mnemonic, addressing_mode, size = self.INSTRUCTIONS[opcode]
        
        # Ler operandos
        operand_bytes = []
        for i in range(1, size):
            if offset + i < len(self.rom_data):
                operand_bytes.append(self.rom_data[offset + i])
            else:
                operand_bytes.append(0)
                
        # Formatar instrução
        if addressing_mode == "implícito":
            instruction = mnemonic
        elif addressing_mode == "acumulador":
            instruction = f"{mnemonic} A"
        elif addressing_mode == "immediato":
            if len(operand_bytes) >= 1:
                value = operand_bytes[0]
                instruction = f"{mnemonic} #${value:02X}"
            else:
                instruction = f"{mnemonic} #$??"
        elif addressing_mode == "direto":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} ${addr:02X}"
            else:
                instruction = f"{mnemonic} $??"
        elif addressing_mode == "direto_x":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} ${addr:02X},X"
            else:
                instruction = f"{mnemonic} $??,X"
        elif addressing_mode == "direto_y":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} ${addr:02X},Y"
            else:
                instruction = f"{mnemonic} $??,Y"
        elif addressing_mode == "indireto":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} (${addr:02X})"
            else:
                instruction = f"{mnemonic} ($??)"
        elif addressing_mode == "indireto_x":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} (${addr:02X},X)"
            else:
                instruction = f"{mnemonic} ($??,X)"
        elif addressing_mode == "indireto_y":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} (${addr:02X}),Y"
            else:
                instruction = f"{mnemonic} ($??),Y"
        elif addressing_mode == "indireto_long":
            if len(operand_bytes) >= 1:
                addr = operand_bytes[0]
                instruction = f"{mnemonic} [{addr:02X}]"
            else:
                instruction = f"{mnemonic} [??]"
        elif addressing_mode == "absoluto":
            if len(operand_bytes) >= 2:
                addr = operand_bytes[0] | (operand_bytes[1] << 8)
                instruction = f"{mnemonic} ${addr:04X}"
            else:
                instruction = f"{mnemonic} $????"
        elif addressing_mode == "absoluto_x":
            if len(operand_bytes) >= 2:
                addr = operand_bytes[0] | (operand_bytes[1] << 8)
                instruction = f"{mnemonic} ${addr:04X},X"
            else:
                instruction = f"{mnemonic} $????,X"
        elif addressing_mode == "absoluto_y":
            if len(operand_bytes) >= 2:
                addr = operand_bytes[0] | (operand_bytes[1] << 8)
                instruction = f"{mnemonic} ${addr:04X},Y"
            else:
                instruction = f"{mnemonic} $????,Y"
        elif addressing_mode == "longo":
            if len(operand_bytes) >= 3:
                addr = operand_bytes[0] | (operand_bytes[1] << 8) | (operand_bytes[2] << 16)
                instruction = f"{mnemonic} ${addr:06X}"
            else:
                instruction = f"{mnemonic} $??????"
        elif addressing_mode == "longo_x":
            if len(operand_bytes) >= 3:
                addr = operand_bytes[0] | (operand_bytes[1] << 8) | (operand_bytes[2] << 16)
                instruction = f"{mnemonic} ${addr:06X},X"
            else:
                instruction = f"{mnemonic} $??????,X"
        elif addressing_mode == "relativo":
            if len(operand_bytes) >= 1:
                # Branch relativo (signed 8-bit)
                rel = operand_bytes[0]
                if rel >= 128:
                    rel -= 256
                target = offset + size + rel
                instruction = f"{mnemonic} ${target:04X}"
            else:
                instruction = f"{mnemonic} $????"
        elif addressing_mode == "relativo_longo":
            if len(operand_bytes) >= 2:
                # Branch relativo longo (signed 16-bit)
                rel = operand_bytes[0] | (operand_bytes[1] << 8)
                if rel >= 32768:
                    rel -= 65536
                target = offset + size + rel
                instruction = f"{mnemonic} ${target:04X}"
            else:
                instruction = f"{mnemonic} $????"
        else:
            instruction = f"{mnemonic} ; Modo desconhecido: {addressing_mode}"
            
        return (instruction, size)
        
    def disassemble_range(self, start_offset: int, length: int) -> List[str]:
        """
        Desassembla um intervalo de bytes
        
        Args:
            start_offset: Offset inicial
            length: Tamanho em bytes
            
        Returns:
            Lista de instruções formatadas
        """
        if self.rom_data is None:
            return []
            
        instructions = []
        offset = start_offset
        
        while offset < start_offset + length and offset < len(self.rom_data):
            instruction, size = self.disassemble_instruction(offset)
            
            # Formatar com endereço
            formatted = f"${offset:06X}: {instruction}"
            instructions.append(formatted)
            
            offset += size
            
        return instructions
        
    def save_assembly(self, instructions: List[str], output_file: str):
        """Salva instruções em arquivo Assembly"""
        with open(output_file, 'w') as f:
            f.write("; Desassemblado por SNES Disassembler\n")
            f.write("; The Legend of Zelda: A Link to the Past\n\n")
            
            for instruction in instructions:
                f.write(instruction + "\n")
                
        print(f"Assembly salvo em: {output_file}")
        
    def disassemble_and_save(self, start_offset: int, length: int, output_file: str = None):
        """Desassembla e salva em arquivo"""
        instructions = self.disassemble_range(start_offset, length)
        
        if instructions:
            if output_file is None:
                output_file = f"assembly_{start_offset:08X}_{length:08X}.asm"
                
            self.save_assembly(instructions, output_file)
            print(f"✓ Desassemblado: {len(instructions)} instruções")
            return True
        return False


def main():
    """Função principal"""
    if len(sys.argv) < 2:
        print("Uso: python disassembler.py <caminho_para_rom>")
        sys.exit(1)
        
    rom_path = sys.argv[1]
    disasm = SNESDisassembler(rom_path)
    
    if not disasm.load_rom():
        sys.exit(1)
        
    # Criar diretório de saída
    disasm.output_dir.mkdir(parents=True, exist_ok=True)
    
    # Exemplo: Desassemblar do início da ROM (ajustar conforme necessário)
    print("\nExemplo de desassemblagem:")
    print("  Offset: 0x000000")
    print("  Tamanho: 256 bytes")
    
    # Desassemblar exemplo
    output_file = disasm.output_dir / "example.asm"
    disasm.disassemble_and_save(0x000000, 256, str(output_file))
    
    print("\nNota: Os offsets acima são exemplos. Ajuste conforme a estrutura da ROM.")
    print("O código do jogo geralmente começa em 0x8000 para LoROM ou 0x0000 para HiROM.")


if __name__ == "__main__":
    main()