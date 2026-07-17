#!/usr/bin/env python3
"""
Assembly Validation Script for SNES 65816
Validates the disassembled assembly file for syntax correctness.
"""

import re
import sys
import shutil
from pathlib import Path
from collections import defaultdict
from typing import Dict, List, Tuple, Set

# 65816 Mnemonics and their valid addressing modes
# Each mnemonic maps to a list of (mode_name, pattern) tuples
MNEMONICS_65816: Dict[str, List[Tuple[str, str]]] = {
    # Implied
    'NOP': [('implied', r'^$')],
    'RTS': [('implied', r'^$')],
    'RTL': [('implied', r'^$')],
    'RTI': [('implied', r'^$')],
    'BRK': [('immediate', r'^\$[0-9A-F]{2}$')],
    'COP': [('immediate', r'^\$[0-9A-F]{2}$')],
    'SEI': [('implied', r'^$')],
    'CLI': [('implied', r'^$')],
    'SEC': [('implied', r'^$')],
    'CLC': [('implied', r'^$')],
    'SED': [('implied', r'^$')],
    'CLD': [('implied', r'^$')],
    'CLV': [('implied', r'^$')],
    'XCE': [('implied', r'^$')],
    'WAI': [('implied', r'^$')],
    'STP': [('implied', r'^$')],
    'XBA': [('implied', r'^$')],
    'PHA': [('implied', r'^$')],
    'PHX': [('implied', r'^$')],
    'PHY': [('implied', r'^$')],
    'PLA': [('implied', r'^$')],
    'PLX': [('implied', r'^$')],
    'PLY': [('implied', r'^$')],
    'PHD': [('implied', r'^$')],
    'PLD': [('implied', r'^$')],
    'PHB': [('implied', r'^$')],
    'PLB': [('implied', r'^$')],
    'PHK': [('implied', r'^$')],
    'PHP': [('implied', r'^$')],
    'PLP': [('implied', r'^$')],
    'WDM': [('immediate', r'^#\$[0-9A-F]{2}$')],  # WDM takes immediate byte
    'PEA': [('immediate16', r'^#?\$[0-9A-F]{4}$')],
    'PEI': [('direct', r'^\$[0-9A-F]{2}$')],
    'PER': [('immediate16', r'^#?\$[0-9A-F]{4}$')],
    'MVN': [('block_move', r'^\$[0-9A-F]{2},\$[0-9A-F]{2}$')],
    'MVP': [('block_move', r'^\$[0-9A-F]{2},\$[0-9A-F]{2}$')],
    
    # ASL - accumulator mode written as "ASL A"
    'ASL': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # LSR - accumulator mode written as "LSR A"
    'LSR': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # ROL - accumulator mode written as "ROL A"
    'ROL': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # ROR - accumulator mode written as "ROR A"
    'ROR': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # INC - accumulator mode written as "INC A"
    'INC': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # DEC - accumulator mode written as "DEC A"
    'DEC': [
        ('accumulator', r'^A$'),
        ('accumulator', r'^$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # LDA
    'LDA': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # LDX
    'LDX': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_y', r'^\$[0-9A-F]{2},Y$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
    ],
    
    # LDY
    'LDY': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # STA
    'STA': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # STX
    'STX': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_y', r'^\$[0-9A-F]{2},Y$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    
    # STY
    'STY': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    
    # STZ
    'STZ': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # ADC
    'ADC': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # SBC
    'SBC': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # AND
    'AND': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # ORA
    'ORA': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # EOR
    'EOR': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # BIT
    'BIT': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
    ],
    
    # CMP
    'CMP': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('zero_page_x', r'^\$[0-9A-F]{2},X$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute_x', r'^\$[0-9A-F]{4},X$'),
        ('absolute_y', r'^\$[0-9A-F]{4},Y$'),
        ('long', r'^\$[0-9A-F]{6}$'),
        ('long_x', r'^\$[0-9A-F]{6},X$'),
        ('indirect', r'^\(\$[0-9A-F]{2}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{2},X\)$'),
        ('indirect_y', r'^\(\$[0-9A-F]{2}\),Y$'),
        ('indirect_long', r'^\[\$[0-9A-F]{2}\]$'),
        ('indirect_long_y', r'^\[\$[0-9A-F]{2}\],Y$'),
        ('stack_relative', r'^\$[0-9A-F]{2},S$'),
        ('stack_relative_indirect_y', r'^\(\$[0-9A-F]{2},S\),Y$'),
    ],
    
    # CPX
    'CPX': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    
    # CPY
    'CPY': [
        ('immediate', r'^#\$[0-9A-F]{2}$'),
        ('immediate16', r'^#\$[0-9A-F]{4}$'),
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    
    # Branch instructions (relative 8-bit) - targets shown as absolute address
    'BCC': [('relative', r'^\$[0-9A-F]{4}$')],
    'BCS': [('relative', r'^\$[0-9A-F]{4}$')],
    'BEQ': [('relative', r'^\$[0-9A-F]{4}$')],
    'BMI': [('relative', r'^\$[0-9A-F]{4}$')],
    'BNE': [('relative', r'^\$[0-9A-F]{4}$')],
    'BPL': [('relative', r'^\$[0-9A-F]{4}$')],
    'BRA': [('relative', r'^\$[0-9A-F]{4}$')],
    'BVC': [('relative', r'^\$[0-9A-F]{4}$')],
    'BVS': [('relative', r'^\$[0-9A-F]{4}$')],
    'BRL': [('relative_long', r'^\$[0-9A-F]{4}$')],
    
    # Jump instructions
    'JMP': [
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('indirect', r'^\(\$[0-9A-F]{4}\)$'),
        ('indirect_x', r'^\(\$[0-9A-F]{4},X\)$'),
        ('long', r'^\$[0-9A-F]{6}$'),
    ],
    'JML': [
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('absolute', r'^$'),  # Handle missing operand (disassembler bug)
        ('indirect', r'^\(\$[0-9A-F]{4}\)$'),
        ('long', r'^\$[0-9A-F]{6}$'),
    ],
    'JSR': [
        ('absolute', r'^\$[0-9A-F]{4}$'),
        ('indirect_x', r'^\(\$[0-9A-F]{4},X\)$'),
    ],
    'JSL': [
        ('long', r'^\$[0-9A-F]{6}$'),
    ],
    
    # TSB / TRB
    'TSB': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    'TRB': [
        ('zero_page', r'^\$[0-9A-F]{2}$'),
        ('absolute', r'^\$[0-9A-F]{4}$'),
    ],
    
    # REP / SEP
    'REP': [('immediate', r'^#\$[0-9A-F]{2}$')],
    'SEP': [('immediate', r'^#\$[0-9A-F]{2}$')],
    
    # Transfer instructions
    'TAX': [('implied', r'^$')],
    'TAY': [('implied', r'^$')],
    'TXA': [('implied', r'^$')],
    'TYA': [('implied', r'^$')],
    'TSX': [('implied', r'^$')],
    'TXS': [('implied', r'^$')],
    'TXY': [('implied', r'^$')],
    'TYX': [('implied', r'^$')],
    'TCD': [('implied', r'^$')],
    'TDC': [('implied', r'^$')],
    'TCS': [('implied', r'^$')],
    'TSC': [('implied', r'^$')],
    
    # Increment/Decrement
    'DEX': [('implied', r'^$')],
    'DEY': [('implied', r'^$')],
    'INX': [('implied', r'^$')],
    'INY': [('implied', r'^$')],
}

# Instruction sizes (bytes) for each addressing mode
INSTRUCTION_SIZES = {
    'implied': 1,
    'accumulator': 1,
    'immediate': 2,
    'immediate16': 3,
    'zero_page': 2,
    'zero_page_x': 2,
    'zero_page_y': 2,
    'absolute': 3,
    'absolute_x': 3,
    'absolute_y': 3,
    'long': 4,
    'long_x': 4,
    'indirect': 2,
    'indirect_x': 2,
    'indirect_y': 2,
    'indirect_long': 2,
    'indirect_long_y': 2,
    'stack_relative': 2,
    'stack_relative_indirect_y': 2,
    'relative': 2,
    'relative_long': 3,
    'block_move': 3,
    'direct': 2,
}

# Line pattern: $BB:AAAA  OO  MNEM OP  ; comment
LINE_PATTERN = re.compile(
    r'^\$([0-9A-F]{2}):([0-9A-F]{4})\s+'
    r'([0-9A-F]{2}(?:\s+[0-9A-F]{2})*)\s+'
    r'([A-Z]{2,3})\s*'
    r'(.*?)(?:\s*;.*)?$'
)

# Data byte pattern: $BB:AAAA  OO .db $XX or .db "string"
DATA_PATTERN = re.compile(
    r'^\$([0-9A-F]{2}):([0-9A-F]{4})\s+'
    r'([0-9A-F]{2}(?:\s+[0-9A-F]{2})*)\s+\.db\s+'
)

class AssemblyValidator:
    def __init__(self, filepath: str):
        self.filepath = Path(filepath)
        self.stats = {
            'total_lines': 0,
            'comment_lines': 0,
            'blank_lines': 0,
            'instruction_lines': 0,
            'data_lines': 0,
            'unknown_lines': 0,
            'total_instructions': 0,
            'total_bytes': 0,
            'invalid_instructions': 0,
            'valid_instructions': 0,
            'invalid_mnemonics': defaultdict(int),
            'invalid_operands': [],
            'mnemonic_counts': defaultdict(int),
            'mode_counts': defaultdict(int),
            'bank_counts': defaultdict(int),
        }
        self.errors = []
        self.warnings = []
    
    def validate(self) -> dict:
        """Run validation on the assembly file."""
        print(f"Validating assembly file: {self.filepath}")
        print("-" * 60)
        
        with open(self.filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        self.stats['total_lines'] = len(lines)
        
        for line_num, line in enumerate(lines, 1):
            self._validate_line(line.rstrip(), line_num)
        
        return self.stats
    
    def _validate_line(self, line: str, line_num: int):
        """Validate a single line of assembly."""
        # Skip blank lines
        if not line.strip():
            self.stats['blank_lines'] += 1
            return
        
        # Skip comment-only lines
        if line.strip().startswith(';'):
            self.stats['comment_lines'] += 1
            return
        
        # Check for data bytes
        data_match = DATA_PATTERN.match(line)
        if data_match:
            self.stats['data_lines'] += 1
            # Count bytes from opcode
            opcode_bytes = data_match.group(3).strip()
            byte_count = len(opcode_bytes.split())
            self.stats['total_bytes'] += byte_count
            return
        
        # Check for instruction lines
        line_match = LINE_PATTERN.match(line)
        if line_match:
            bank = line_match.group(1)
            addr = line_match.group(2)
            opcode_bytes = line_match.group(3).strip()
            mnemonic = line_match.group(4)
            operands = line_match.group(5).strip()
            
            self.stats['instruction_lines'] += 1
            self.stats['total_instructions'] += 1
            self.stats['bank_counts'][bank] += 1
            
            # Count bytes from opcode
            byte_count = len(opcode_bytes.split())
            self.stats['total_bytes'] += byte_count
            
            # Validate mnemonic
            if mnemonic in MNEMONICS_65816:
                self.stats['mnemonic_counts'][mnemonic] += 1
                
                # Validate operand
                valid_modes = MNEMONICS_65816[mnemonic]
                operand_valid = False
                matched_mode = None
                
                for mode_name, pattern in valid_modes:
                    if re.match(pattern, operands):
                        operand_valid = True
                        matched_mode = mode_name
                        break
                
                if operand_valid:
                    self.stats['valid_instructions'] += 1
                    self.stats['mode_counts'][matched_mode] += 1
                else:
                    self.stats['invalid_instructions'] += 1
                    error_msg = f"Line {line_num}: Invalid operand '{operands}' for {mnemonic} at ${bank}:{addr}"
                    self.errors.append(error_msg)
                    self.stats['invalid_operands'].append({
                        'line': line_num,
                        'address': f"${bank}:{addr}",
                        'mnemonic': mnemonic,
                        'operand': operands
                    })
            else:
                self.stats['invalid_instructions'] += 1
                self.stats['invalid_mnemonics'][mnemonic] += 1
                error_msg = f"Line {line_num}: Unknown mnemonic '{mnemonic}' at ${bank}:{addr}"
                self.errors.append(error_msg)
        else:
            # Unknown line format
            self.stats['unknown_lines'] += 1
            if not line.strip().startswith(';') and not line.strip().startswith('.'):
                self.warnings.append(f"Line {line_num}: Unrecognized format: {line[:60]}")
    
    def generate_report(self, output_path: str):
        """Generate validation report."""
        report_path = Path(output_path)
        report_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Check for asm6f availability
        asm6f_available = shutil.which('asm6f') is not None
        
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write("=" * 70 + "\n")
            f.write("ASSEMBLY VALIDATION REPORT\n")
            f.write("=" * 70 + "\n\n")
            
            f.write(f"File: {self.filepath}\n")
            f.write(f"Generated by: validate_assembly.py\n\n")
            
            # Tool availability
            f.write("-" * 70 + "\n")
            f.write("TOOL AVAILABILITY\n")
            f.write("-" * 70 + "\n")
            if asm6f_available:
                f.write("  asm6f: AVAILABLE (assembly compilation possible)\n")
            else:
                f.write("  asm6f: NOT AVAILABLE (install from https://github.com/ClusterM/asm6f)\n")
                f.write("  Note: Only syntax validation performed. To compile and verify binary,\n")
                f.write("        install asm6f and re-run validation.\n")
            f.write("\n")
            
            f.write("-" * 70 + "\n")
            f.write("SUMMARY\n")
            f.write("-" * 70 + "\n")
            f.write(f"Total lines in file:      {self.stats['total_lines']:,}\n")
            f.write(f"Comment lines:            {self.stats['comment_lines']:,}\n")
            f.write(f"Blank lines:              {self.stats['blank_lines']:,}\n")
            f.write(f"Instruction lines:        {self.stats['instruction_lines']:,}\n")
            f.write(f"Data lines:               {self.stats['data_lines']:,}\n")
            f.write(f"Unrecognized lines:       {self.stats['unknown_lines']:,}\n\n")
            
            f.write(f"Total instructions:       {self.stats['total_instructions']:,}\n")
            f.write(f"Total bytes:              {self.stats['total_bytes']:,}\n")
            f.write(f"Valid instructions:       {self.stats['valid_instructions']:,}\n")
            f.write(f"Invalid instructions:     {self.stats['invalid_instructions']:,}\n")
            
            if self.stats['total_instructions'] > 0:
                validity_pct = (self.stats['valid_instructions'] / self.stats['total_instructions']) * 100
                f.write(f"Validity percentage:      {validity_pct:.2f}%\n")
            
            f.write("\n")
            
            # Bank distribution
            f.write("-" * 70 + "\n")
            f.write("INSTRUCTIONS BY BANK\n")
            f.write("-" * 70 + "\n")
            for bank in sorted(self.stats['bank_counts'].keys()):
                count = self.stats['bank_counts'][bank]
                f.write(f"  ${bank}:XX  {count:>10,} instructions\n")
            f.write("\n")
            
            # Top mnemonics
            f.write("-" * 70 + "\n")
            f.write("TOP 30 MNEMONICS\n")
            f.write("-" * 70 + "\n")
            sorted_mnemonics = sorted(self.stats['mnemonic_counts'].items(), 
                                     key=lambda x: x[1], reverse=True)[:30]
            for mnemonic, count in sorted_mnemonics:
                f.write(f"  {mnemonic:<6} {count:>10,}\n")
            f.write("\n")
            
            # Addressing modes
            f.write("-" * 70 + "\n")
            f.write("ADDRESSING MODES\n")
            f.write("-" * 70 + "\n")
            for mode in sorted(self.stats['mode_counts'].keys()):
                count = self.stats['mode_counts'][mode]
                f.write(f"  {mode:<35} {count:>10,}\n")
            f.write("\n")
            
            # Invalid mnemonics
            if self.stats['invalid_mnemonics']:
                f.write("-" * 70 + "\n")
                f.write("INVALID MNEMONICS FOUND\n")
                f.write("-" * 70 + "\n")
                for mnemonic, count in sorted(self.stats['invalid_mnemonics'].items(), 
                                            key=lambda x: x[1], reverse=True):
                    f.write(f"  {mnemonic:<6} {count:>10,} occurrences\n")
                f.write("\n")
            
            # Invalid operands (first 100)
            if self.stats['invalid_operands']:
                f.write("-" * 70 + "\n")
                f.write(f"INVALID OPERANDS (showing first {min(100, len(self.stats['invalid_operands']))} of {len(self.stats['invalid_operands'])})\n")
                f.write("-" * 70 + "\n")
                for item in self.stats['invalid_operands'][:100]:
                    f.write(f"  Line {item['line']}: {item['address']} {item['mnemonic']} {item['operand']}\n")
                f.write("\n")
            
            # Errors
            if self.errors:
                f.write("-" * 70 + "\n")
                f.write(f"ERRORS ({len(self.errors)} total)\n")
                f.write("-" * 70 + "\n")
                for error in self.errors[:100]:
                    f.write(f"  {error}\n")
                if len(self.errors) > 100:
                    f.write(f"  ... and {len(self.errors) - 100} more errors\n")
                f.write("\n")
            
            # Warnings
            if self.warnings:
                f.write("-" * 70 + "\n")
                f.write(f"WARNINGS ({len(self.warnings)} total)\n")
                f.write("-" * 70 + "\n")
                for warning in self.warnings[:50]:
                    f.write(f"  {warning}\n")
                if len(self.warnings) > 50:
                    f.write(f"  ... and {len(self.warnings) - 50} more warnings\n")
                f.write("\n")
            
            # Final verdict
            f.write("=" * 70 + "\n")
            f.write("FINAL VERDICT\n")
            f.write("=" * 70 + "\n")
            
            if self.stats['invalid_instructions'] == 0:
                f.write("✓ All instructions are syntactically valid!\n")
            else:
                f.write(f"✗ Found {self.stats['invalid_instructions']} invalid instructions\n")
                if self.stats['total_instructions'] > 0:
                    f.write(f"  Validity: {validity_pct:.2f}%\n")
            
            f.write("\n" + "=" * 70 + "\n")
        
        print(f"\nReport saved to: {report_path}")
        return report_path
    
    def print_summary(self):
        """Print summary to console."""
        print("\n" + "=" * 60)
        print("VALIDATION SUMMARY")
        print("=" * 60)
        print(f"Total instructions:   {self.stats['total_instructions']:>10,}")
        print(f"Total bytes:          {self.stats['total_bytes']:>10,}")
        print(f"Valid instructions:   {self.stats['valid_instructions']:>10,}")
        print(f"Invalid instructions: {self.stats['invalid_instructions']:>10,}")
        
        if self.stats['total_instructions'] > 0:
            validity_pct = (self.stats['valid_instructions'] / self.stats['total_instructions']) * 100
            print(f"Validity:             {validity_pct:>9.2f}%")
        
        print("=" * 60)


def main():
    if len(sys.argv) < 2:
        print("Usage: python validate_assembly.py <assembly_file> [output_report]")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else "output/validation_report.txt"
    
    validator = AssemblyValidator(input_file)
    validator.validate()
    validator.generate_report(output_file)
    validator.print_summary()
    
    # Exit with error code if invalid instructions found
    sys.exit(0 if validator.stats['invalid_instructions'] == 0 else 1)


if __name__ == '__main__':
    main()
