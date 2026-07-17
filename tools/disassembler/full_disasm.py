#!/usr/bin/env python3
"""
Full 65816 Disassembler for The Legend of Zelda: A Link to the Past (LoROM)
Approach: Code-flow tracing from known entry points + linear sweep.
Autonomous - no external dependencies, stdlib only.
"""

import os
import sys
import struct
from pathlib import Path

# ---------------------------------------------------------------------------
# 65816 opcode table: opcode -> (mnemonic, mode_name, size_bytes)
# Modes: imp, acc, imm, imm16, dp, dpX, dpY, (dp), (dp,X), (dp),Y,
#        [dp], long, longX, abs, absX, absY, (abs), (abs,X), rel8, rel16, dpdp
# ---------------------------------------------------------------------------

OPCODES = {}

def _o(mne, mode, size, op):
    OPCODES[op] = (mne, mode, size)

# ADC
_o("ADC","imm",2,0x69);_o("ADC","dp",2,0x65);_o("ADC","dpX",2,0x75)
_o("ADC","abs",3,0x6D);_o("ADC","absX",3,0x7D);_o("ADC","absY",3,0x79)
_o("ADC","(dp)",2,0x67);_o("ADC","(dp,X)",2,0x61);_o("ADC","(dp),Y",2,0x71)
_o("ADC","long",3,0x6F);_o("ADC","longX",3,0x7F)
# AND
_o("AND","imm",2,0x29);_o("AND","dp",2,0x25);_o("AND","dpX",2,0x35)
_o("AND","abs",3,0x2D);_o("AND","absX",3,0x3D);_o("AND","absY",3,0x39)
_o("AND","(dp)",2,0x27);_o("AND","(dp,X)",2,0x21);_o("AND","(dp),Y",2,0x31)
_o("AND","long",3,0x2F);_o("AND","longX",3,0x3F)
# ASL
_o("ASL","acc",1,0x0A);_o("ASL","dp",2,0x06);_o("ASL","dpX",2,0x16)
_o("ASL","abs",3,0x0E);_o("ASL","absX",3,0x1E)
# BIT
_o("BIT","imm",2,0x89);_o("BIT","dp",2,0x24);_o("BIT","abs",3,0x2C)
_o("BIT","dpX",2,0x34);_o("BIT","absX",3,0x3C)
# BRA/BRL
_o("BRA","rel8",2,0x80);_o("BRL","rel16",3,0x82)
# Branches
_o("BCC","rel8",2,0x90);_o("BCS","rel8",2,0xB0)
_o("BEQ","rel8",2,0xF0);_o("BNE","rel8",2,0xD0)
_o("BMI","rel8",2,0x30);_o("BPL","rel8",2,0x10)
_o("BVC","rel8",2,0x50);_o("BVS","rel8",2,0x70)
# BRK/COP
_o("BRK","dp",2,0x00);_o("COP","dp",2,0x02)
# Flags
_o("CLC","imp",1,0x18);_o("CLD","imp",1,0xD8);_o("CLI","imp",1,0x58);_o("CLV","imp",1,0xB8)
_o("SEC","imp",1,0x38);_o("SED","imp",1,0xF8);_o("SEI","imp",1,0x78)
# CMP
_o("CMP","imm",2,0xC9);_o("CMP","dp",2,0xC5);_o("CMP","dpX",2,0xD5)
_o("CMP","abs",3,0xCD);_o("CMP","absX",3,0xDD);_o("CMP","absY",3,0xD9)
_o("CMP","(dp)",2,0xC7);_o("CMP","(dp,X)",2,0xC1);_o("CMP","(dp),Y",2,0xD1)
_o("CMP","long",3,0xCF);_o("CMP","longX",3,0xDF)
# CPX/CPY
_o("CPX","imm",2,0xE0);_o("CPX","dp",2,0xE4);_o("CPX","abs",3,0xEC)
_o("CPY","imm",2,0xC0);_o("CPY","dp",2,0xC4);_o("CPY","abs",3,0xCC)
# DEC
_o("DEC","acc",1,0x3A);_o("DEC","dp",2,0xC6);_o("DEC","dpX",2,0xD6)
_o("DEC","abs",3,0xCE);_o("DEC","absX",3,0xDE)
# DEX/DEY
_o("DEX","imp",1,0xCA);_o("DEY","imp",1,0x88)
# EOR
_o("EOR","imm",2,0x49);_o("EOR","dp",2,0x45);_o("EOR","dpX",2,0x55)
_o("EOR","abs",3,0x4D);_o("EOR","absX",3,0x5D);_o("EOR","absY",3,0x59)
_o("EOR","(dp)",2,0x47);_o("EOR","(dp,X)",2,0x41);_o("EOR","(dp),Y",2,0x51)
_o("EOR","long",3,0x4F);_o("EOR","longX",3,0x5F)
# INC
_o("INC","acc",1,0x1A);_o("INC","dp",2,0xE6);_o("INC","dpX",2,0xF6)
_o("INC","abs",3,0xEE);_o("INC","absX",3,0xFE)
# INX/INY
_o("INX","imp",1,0xE8);_o("INY","imp",1,0xC8)
# JMP/JML
_o("JMP","abs",3,0x4C);_o("JMP","(abs)",3,0x6C);_o("JMP","(abs,X)",3,0x7C)
_o("JML","long",4,0x5C);_o("JML","(long)",4,0xDC)
# JSR/JSL
_o("JSR","abs",3,0x20);_o("JSR","(abs,X)",3,0xFC)
_o("JSL","long",4,0x22)
# LDA
_o("LDA","imm",2,0xA9);_o("LDA","dp",2,0xA5);_o("LDA","dpX",2,0xB5)
_o("LDA","abs",3,0xAD);_o("LDA","absX",3,0xBD);_o("LDA","absY",3,0xB9)
_o("LDA","(dp)",2,0xA7);_o("LDA","(dp,X)",2,0xA1);_o("LDA","(dp),Y",2,0xB1)
_o("LDA","long",3,0xAF);_o("LDA","longX",3,0xBF)
# LDX
_o("LDX","imm",2,0xA2);_o("LDX","dp",2,0xA6);_o("LDX","dpY",2,0xB6);_o("LDX","abs",3,0xAE)
# LDY
_o("LDY","imm",2,0xA0);_o("LDY","dp",2,0xA4);_o("LDY","dpX",2,0xB4)
_o("LDY","abs",3,0xAC);_o("LDY","absX",3,0xBC)
# LSR
_o("LSR","acc",1,0x4A);_o("LSR","dp",2,0x46);_o("LSR","dpX",2,0x56)
_o("LSR","abs",3,0x4E);_o("LSR","absX",3,0x5E)
# MVP/MVN
_o("MVN","dpdp",3,0x54);_o("MVP","dpdp",3,0x44)
# NOP
_o("NOP","imp",1,0xEA)
# ORA
_o("ORA","imm",2,0x09);_o("ORA","dp",2,0x05);_o("ORA","dpX",2,0x15)
_o("ORA","abs",3,0x0D);_o("ORA","absX",3,0x1D);_o("ORA","absY",3,0x19)
_o("ORA","(dp)",2,0x07);_o("ORA","(dp,X)",2,0x01);_o("ORA","(dp),Y",2,0x11)
_o("ORA","long",3,0x0F);_o("ORA","longX",3,0x1F)
# PEA/PEI/PER
_o("PEA","imm16",3,0xF4);_o("PEI","dp",2,0xD4);_o("PER","rel16",3,0x62)
# Push
_o("PHA","imp",1,0x48);_o("PHB","imp",1,0x8B);_o("PHD","imp",1,0x0B)
_o("PHK","imp",1,0x4B);_o("PHP","imp",1,0x08);_o("PHX","imp",1,0xDA);_o("PHY","imp",1,0x5A)
# Pull
_o("PLA","imp",1,0x68);_o("PLB","imp",1,0xAB);_o("PLD","imp",1,0x2B)
_o("PLP","imp",1,0x28);_o("PLX","imp",1,0xFA);_o("PLY","imp",1,0x7A)
# REP/SEP
_o("REP","imm",2,0xC2);_o("SEP","imm",2,0xE2)
# ROL
_o("ROL","acc",1,0x2A);_o("ROL","dp",2,0x26);_o("ROL","dpX",2,0x36)
_o("ROL","abs",3,0x2E);_o("ROL","absX",3,0x3E)
# ROR
_o("ROR","acc",1,0x6A);_o("ROR","dp",2,0x66);_o("ROR","dpX",2,0x76)
_o("ROR","abs",3,0x6E);_o("ROR","absX",3,0x7E)
# RTI/RTL/RTS
_o("RTI","imp",1,0x40);_o("RTL","imp",1,0x6B);_o("RTS","imp",1,0x60)
# SBC
_o("SBC","imm",2,0xE9);_o("SBC","dp",2,0xE5);_o("SBC","dpX",2,0xF5)
_o("SBC","abs",3,0xED);_o("SBC","absX",3,0xFD);_o("SBC","absY",3,0xF9)
_o("SBC","(dp)",2,0xE7);_o("SBC","(dp,X)",2,0xE1);_o("SBC","(dp),Y",2,0xF1)
_o("SBC","long",3,0xEF);_o("SBC","longX",3,0xFF)
# STA
_o("STA","dp",2,0x85);_o("STA","dpX",2,0x95);_o("STA","abs",3,0x8D)
_o("STA","absX",3,0x9D);_o("STA","absY",3,0x99);_o("STA","(dp)",2,0x87)
_o("STA","(dp,X)",2,0x81);_o("STA","(dp),Y",2,0x91)
_o("STA","long",3,0x8F);_o("STA","longX",3,0x9F)
# STP
_o("STP","imp",1,0xDB)
# STX/STY/STZ
_o("STX","dp",2,0x86);_o("STX","dpY",2,0x96);_o("STX","abs",3,0x8E)
_o("STY","dp",2,0x84);_o("STY","dpX",2,0x94);_o("STY","abs",3,0x8C)
_o("STZ","dp",2,0x64);_o("STZ","dpX",2,0x74);_o("STZ","abs",3,0x9C);_o("STZ","absX",3,0x9E)
# Tax/Tay/Tcd/Tcs/Tdc
_o("TAX","imp",1,0xAA);_o("TAY","imp",1,0xA8)
_o("TCD","imp",1,0x5B);_o("TCS","imp",1,0x1B);_o("TDC","imp",1,0x7B)
# TRB/TSB
_o("TRB","dp",2,0x14);_o("TRB","abs",3,0x1C)
_o("TSB","dp",2,0x04);_o("TSB","abs",3,0x0C)
# TSC/TSX
_o("TSC","imp",1,0x3B);_o("TSX","imp",1,0xBA)
# TXA/TXS/TXY/TYA/TYX
_o("TXA","imp",1,0x8A);_o("TXS","imp",1,0x9A)
_o("TXY","imp",1,0x9B);_o("TYA","imp",1,0x98);_o("TYX","imp",1,0xBB)
# WAI/WDM/XBA/XCE
_o("WAI","imp",1,0xCB);_o("WDM","imm",2,0x42)
_o("XBA","imp",1,0xEB);_o("XCE","imp",1,0xFB)


class Disasm:
    def __init__(self, rom_path):
        with open(rom_path, 'rb') as f:
            self.rom = f.read()
        self.size = len(self.rom)
        self.num_banks = self.size // 0x8000
        
        # State
        self.code = set()       # offsets confirmed as code
        self.data = set()       # offsets confirmed as data
        self.disasm = {}        # offset -> formatted line string
        self.routines = {}      # offset -> name
        self.labels = {}        # offset -> name (for references)
        self.total_inst = 0
        self.pending = set()    # offsets to process
        self._visited = set()   # already traced from
    
    def _b(self, off):
        return self.rom[off] if 0 <= off < self.size else 0
    
    def _w(self, off):
        return self._b(off) | (self._b(off+1) << 8)
    
    def _bank(self, off):
        return off // 0x8000
    
    def _addr(self, off):
        return (off % 0x8000) + 0x8000
    
    def _snes(self, off):
        return f"${self._bank(off):02X}:{self._addr(off):04X}"
    
    def _snes_ba(self, bank, addr):
        return f"${bank:02X}:{addr:04X}"
    
    def _rom_off(self, bank, addr):
        if addr < 0x8000:
            return -1
        return bank * 0x8000 + (addr - 0x8000)
    
    def _valid(self, off):
        return 0 <= off < self.size
    
    def _is_likely_code(self, off):
        """Check if bytes at offset look like valid 65816 code."""
        if not self._valid(off):
            return False
        op = self._b(off)
        if op not in OPCODES:
            return False
        _, _, sz = OPCODES[op]
        # Check that all operand bytes are within ROM
        for i in range(1, sz):
            if not self._valid(off + i):
                return False
        return True
    
    def _decode_one(self, off):
        """Decode one instruction. Returns (formatted_line, size, targets_list)."""
        bank = self._bank(off)
        addr = self._addr(off)
        op = self._b(off)
        
        if op not in OPCODES:
            return (f"{self._snes(off)}  {op:02X}       .db ${op:02X}", 1, [])
        
        mne, mode, sz = OPCODES[op]
        ops = [self._b(off + i) for i in range(1, sz)]
        targets = []
        target_comment = ""
        
        # Format operand string
        if mode == "imp":
            opstr = ""
        elif mode == "acc":
            opstr = "A"
        elif mode == "imm":
            opstr = f"#${ops[0]:02X}"
        elif mode == "imm16":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"#${val:04X}"
        elif mode == "dp":
            opstr = f"${ops[0]:02X}"
        elif mode == "dpX":
            opstr = f"${ops[0]:02X},X"
        elif mode == "dpY":
            opstr = f"${ops[0]:02X},Y"
        elif mode == "(dp)":
            opstr = f"(${ops[0]:02X})"
        elif mode == "(dp,X)":
            opstr = f"(${ops[0]:02X},X)"
        elif mode == "(dp),Y":
            opstr = f"(${ops[0]:02X}),Y"
        elif mode == "[dp]":
            opstr = f"[${ops[0]:02X}]"
        elif mode == "long":
            val = ops[0] | (ops[1] << 8) | (ops[2] << 16) if len(ops) >= 3 else 0
            opstr = f"${val:06X}"
            if mne in ("JML", "JSL"):
                tb = val >> 16; ta = val & 0xFFFF
                to = self._rom_off(tb, ta)
                if self._valid(to):
                    targets.append((to, True))
                    target_comment = f"; -> {self._snes_ba(tb, ta)}"
            elif mne == "JMP":
                tb = val >> 16; ta = val & 0xFFFF
                to = self._rom_off(tb, ta)
                if self._valid(to):
                    targets.append((to, True))
                    target_comment = f"; -> {self._snes_ba(tb, ta)}"
        elif mode == "longX":
            val = ops[0] | (ops[1] << 8) | (ops[2] << 16) if len(ops) >= 3 else 0
            opstr = f"${val:06X},X"
        elif mode == "abs":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"${val:04X}"
            if mne in ("JMP", "JSR", "JML"):
                to = self._rom_off(bank, val)
                if self._valid(to):
                    targets.append((to, True))
                    target_comment = f"; -> {self._snes_ba(bank, val)}"
        elif mode == "absX":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"${val:04X},X"
            if mne == "JMP":
                to = self._rom_off(bank, val)
                if self._valid(to):
                    targets.append((to, True))
                    target_comment = f"; -> {self._snes_ba(bank, val)}"
        elif mode == "absY":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"${val:04X},Y"
        elif mode == "(abs)":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"(${val:04X})"
        elif mode == "(abs,X)":
            val = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            opstr = f"(${val:04X},X)"
            if mne == "JMP":
                to = self._rom_off(bank, val)
                if self._valid(to):
                    targets.append((to, True))
                    target_comment = f"; -> {self._snes_ba(bank, val)}"
        elif mode == "rel8":
            rel = ops[0]
            if rel >= 128: rel -= 256
            tgt = off + sz + rel
            if self._valid(tgt):
                tb, ta = self._bank(tgt), self._addr(tgt)
            else:
                tb, ta = bank, addr
            opstr = f"${ta:04X}"
            if self._valid(tgt):
                targets.append((tgt, False))
                target_comment = f"; -> {self._snes_ba(tb, ta)}"
        elif mode == "rel16":
            rel = ops[0] | (ops[1] << 8) if len(ops) >= 2 else 0
            if rel >= 32768: rel -= 65536
            tgt = off + sz + rel
            if self._valid(tgt):
                tb, ta = self._bank(tgt), self._addr(tgt)
            else:
                tb, ta = bank, addr
            opstr = f"${ta:04X}"
            if self._valid(tgt):
                targets.append((tgt, False))
                target_comment = f"; -> {self._snes_ba(tb, ta)}"
        elif mode == "dpdp":
            opstr = f"${ops[0]:02X},${ops[1]:02X}" if len(ops) >= 2 else "$??,$??"
        else:
            opstr = "; ???"
        
        # Routine label comment
        rlabel = ""
        if off in self.routines:
            rlabel = f"; ======== {self.routines[off]} ========"
        
        comment = rlabel
        if target_comment:
            comment = f"{comment} {target_comment}".strip() if comment else target_comment
        
        hexbytes = ' '.join(f'{self._b(off+i):02X}' for i in range(sz))
        line = f"{self._snes(off)}  {hexbytes:<9} {mne:<4} {opstr:<16} {comment}"
        
        return (line, sz, targets)
    
    def _add_routine(self, off, name=None):
        if off not in self.routines:
            if name is None:
                name = f"sub_{self._bank(off):02X}_{self._addr(off):04X}"
            self.routines[off] = name
        self.pending.add(off)
    
    def _trace_from(self, start):
        """Follow control flow from start, disassembling sequentially."""
        off = start
        limit = 4096  # max instructions per routine
        
        for _ in range(limit):
            if not self._valid(off) or off in self._visited:
                break
            if off in self.data:
                break
            
            self._visited.add(off)
            self.code.add(off)
            self.total_inst += 1
            
            line, sz, targets = self._decode_one(off)
            self.disasm[off] = line
            op = self._b(off)
            
            # Schedule target code for tracing
            for toff, is_sub in targets:
                if is_sub and op in (0x20, 0x22):  # JSR/JSL
                    self._add_routine(toff)
                elif not is_sub:
                    self.pending.add(toff)
                else:
                    self.pending.add(toff)
            
            # Stop conditions
            if op in (0x60, 0x6B, 0x40):  # RTS/RTL/RTI
                break
            if op in (0x4C, 0x5C, 0x6C, 0x7C, 0xDC):  # JMP/JML
                break
            if op == 0x80:  # BRA
                break
            if op in (0x00, 0x02, 0xDB):  # BRK/COP/STP
                break
            
            off += sz
    
    def _add_entry_points(self):
        """Add known entry points."""
        # RESET vector
        self._add_routine(self._rom_off(0x00, 0x8000), "RESET")
        # NMI from known analysis
        self._add_routine(self._rom_off(0x00, 0x822C), "NMI_handler")
        # IRQ from known analysis
        self._add_routine(self._rom_off(0x00, 0x82D8), "IRQ_handler")
        
        # Read 65816 vectors at $FF00-$FFFF of bank $00
        vec_off = self._rom_off(0x00, 0xFFE0)
        if self._valid(vec_off):
            vec_names = [
                (0x00, "COP"), (0x02, "BRK"), (0x04, "ABORT"),
                (0x06, "NMI"), (0x08, "RESERVED"), (0x0A, "IRQ"), (0x0E, "IRQ2")
            ]
            for voff_rel, vname in vec_names:
                vaddr = self._w(vec_off + voff_rel)
                if vaddr >= 0x8000:
                    to = self._rom_off(0x00, vaddr)
                    if self._valid(to):
                        self._add_routine(to, f"vector_{vname}")
    
    def _linear_sweep(self):
        """Sweep through all banks trying to disassemble unclaimed bytes as code."""
        print("  Linear sweep to find more code...")
        
        # For each bank, sweep through all bytes
        for bank in range(self.num_banks):
            bank_start = bank * 0x8000
            bank_end = min(bank_start + 0x8000, self.size)
            
            off = bank_start
            while off < bank_end:
                if off in self.code or off in self.data or off in self._visited:
                    # Already handled - skip
                    if off in self.code and off in self.disasm:
                        # Find size from the disassembled instruction
                        op = self._b(off)
                        if op in OPCODES:
                            _, _, sz = OPCODES[op]
                            off += sz
                        else:
                            off += 1
                    else:
                        off += 1
                    continue
                
                # Try to disassemble from here
                if self._is_likely_code(off):
                    # Check if this looks like the start of a routine
                    # Heuristic: routines often start with PHA/PHX/PHY/PHD/PHB/SEI/CLC/XCE/etc.
                    op = self._b(off)
                    routine_starts = {
                        0x48, 0xDA, 0x5A, 0x0B, 0x8B,  # PHA/PHX/PHY/PHD/PHB
                        0x78, 0x18, 0xFB, 0xD8, 0x58,  # SEI/CLC/XCE/CLD/CLI
                        0xE2, 0xC2,  # SEP/REP
                        0xA9, 0xAD,  # LDA abs/imm
                    }
                    
                    if op in routine_starts:
                        self._add_routine(off)
                    else:
                        # Still try to trace from here
                        self.pending.add(off)
                
                off += 1
    
    def _mark_data_regions(self):
        """Mark regions between code as data."""
        # Simple approach: any byte not claimed as code that's surrounded by known
        # code/instruction boundaries gets marked as data
        for bank in range(self.num_banks):
            bank_start = bank * 0x8000
            bank_end = min(bank_start + 0x8000, self.size)
            
            off = bank_start
            while off < bank_end:
                if off in self.code:
                    op = self._b(off)
                    if op in OPCODES:
                        _, _, sz = OPCODES[op]
                        off += sz
                    else:
                        off += 1
                    continue
                
                if off in self.data:
                    off += 1
                    continue
                
                # Check if this is inside a known vector table area
                # Vectors at $FF00-$FFFF in each bank
                addr = self._addr(off)
                if 0xFF00 <= addr <= 0xFFFF:
                    self.data.add(off)
                    off += 1
                    continue
                
                # Mark as data
                self.data.add(off)
                off += 1
    
    def _format_data_line(self, off):
        """Format a single data byte for output."""
        b = self._b(off)
        # Try to detect if this starts a string
        s = self._try_string(off)
        if s:
            return f'{self._snes(off)}  {"":9} .db "{s}"'
        return f"{self._snes(off)}  {b:02X}       .db ${b:02X}"
    
    def _try_string(self, off, min_len=4):
        """Try to read an ASCII string starting at offset."""
        chars = []
        for i in range(128):
            b = self._b(off + i)
            if b == 0:
                break
            if 0x20 <= b <= 0x7E:
                chars.append(chr(b))
            else:
                break
        if len(chars) >= min_len:
            return ''.join(chars)
        return None
    
    def _generate_output(self):
        """Generate the final assembly file."""
        out_dir = Path("output/assembly")
        out_dir.mkdir(parents=True, exist_ok=True)
        out_file = out_dir / "zelda_full.asm"
        
        lines = []
        lines.append("; ============================================================")
        lines.append("; The Legend of Zelda: A Link to the Past")
        lines.append("; Full 65816 Disassembly - LoROM Format (1MB)")
        lines.append("; Auto-generated by tools/disassembler/full_disasm.py")
        lines.append("; ============================================================")
        lines.append("")
        lines.append(f"; Total instructions: {self.total_inst}")
        lines.append(f"; Total routines: {len(self.routines)}")
        lines.append(f"; Total data bytes: {len(self.data)}")
        lines.append("")
        
        # Sort routines for listing
        sorted_routines = sorted(self.routines.items())
        lines.append("; ==================== ROUTINE LIST ====================")
        for off, name in sorted_routines:
            lines.append(f"; {name}: {self._snes(off)}")
        lines.append("; ============================================================")
        lines.append("")
        
        # Emit per bank
        for bank in range(self.num_banks):
            bank_start = bank * 0x8000
            bank_end = min(bank_start + 0x8000, self.size)
            
            # Check if this bank has content
            has = any(o in self.code or o in self.disasm for o in range(bank_start, bank_end))
            if not has:
                continue
            
            lines.append(f"; ==================== BANK ${bank:02X} ====================")
            lines.append("")
            
            off = bank_start
            while off < bank_end:
                if off in self.disasm:
                    lines.append(self.disasm[off])
                    op = self._b(off)
                    if op in OPCODES and off in self.code:
                        _, _, sz = OPCODES[op]
                        off += sz
                    else:
                        off += 1
                elif off in self.data:
                    # Try to output as string
                    s = self._try_string(off)
                    if s:
                        bank_n = self._bank(off)
                        addr_n = self._addr(off)
                        escaped = s.replace('"', '\\"')
                        lines.append(f'{self._snes(off)}  {"":9} .db "{escaped}"')
                        # Skip all string bytes (including null)
                        slen = len(s)
                        # Check for null terminator
                        if self._b(off + slen) == 0:
                            slen += 1
                        off += slen
                    else:
                        lines.append(self._format_data_line(off))
                        off += 1
                else:
                    off += 1
            
            lines.append("")
        
        with open(out_file, 'w', encoding='ascii', errors='replace') as f:
            f.write('\n'.join(lines))
        
        fsize = out_file.stat().st_size
        print(f"  Output: {out_file}")
        print(f"  File size: {fsize:,} bytes ({fsize/1024:.1f} KB)")
        print(f"  Total lines: {len(lines)}")
    
    def run(self):
        print("=" * 60)
        print("65816 Full Disassembler - Zelda: A Link to the Past")
        print(f"LoROM format, {self.size:,} bytes, {self.num_banks} banks")
        print("=" * 60)
        
        # Phase 1: Entry points
        print("\n[Phase 1] Adding entry points...")
        self._add_entry_points()
        print(f"  {len(self.routines)} entry points added")
        
        # Phase 2: Trace code from entry points
        print("\n[Phase 2] Tracing code flow from entry points...")
        iteration = 0
        while self.pending:
            iteration += 1
            batch = list(self.pending)
            self.pending.clear()
            for off in batch:
                if off not in self._visited and self._valid(off) and off not in self.data:
                    self._trace_from(off)
            if iteration > 100:
                print(f"  Warning: {iteration} iterations, stopping")
                break
        print(f"  After flow tracing: {self.total_inst} instructions, {len(self.routines)} routines")
        
        # Phase 3: Linear sweep for more code
        print("\n[Phase 3] Linear sweep for additional code...")
        self._linear_sweep()
        
        # Trace any new pending targets
        iteration = 0
        while self.pending:
            iteration += 1
            batch = list(self.pending)
            self.pending.clear()
            for off in batch:
                if off not in self._visited and self._valid(off) and off not in self.data:
                    self._trace_from(off)
            if iteration > 100:
                break
        print(f"  After sweep: {self.total_inst} instructions, {len(self.routines)} routines")
        
        # Phase 4: Mark remaining as data
        print("\n[Phase 4] Marking data regions...")
        self._mark_data_regions()
        print(f"  Data regions: {len(self.data)} bytes")
        
        # Phase 5: Generate output
        print("\n[Phase 5] Generating assembly output...")
        self._generate_output()
        
        # Summary
        print("\n" + "=" * 60)
        print("DISASSEMBLY COMPLETE")
        print("=" * 60)
        print(f"  Instructions: {self.total_inst:,}")
        print(f"  Routines:     {len(self.routines):,}")
        print(f"  Data bytes:   {len(self.data):,}")
        print(f"  Code coverage: {len(self.code):,} / {self.size:,} ({100*len(self.code)/self.size:.1f}%)")
        
        return True


def main():
    if len(sys.argv) < 2:
        print("Usage: python full_disasm.py <rom_path>")
        sys.exit(1)
    
    rom_path = sys.argv[1]
    if not os.path.exists(rom_path):
        print(f"Error: ROM not found: {rom_path}")
        sys.exit(1)
    
    d = Disasm(rom_path)
    try:
        d.run()
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
