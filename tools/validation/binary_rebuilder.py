#!/usr/bin/env python3
"""
Binary Rebuilder for SNES 65816 Disassembly
Reconstructs a ROM binary from disassembled assembly listing and compares with original.
"""

import re
import sys
from pathlib import Path

# Line format: $BB:AAAA  XX [YY ZZ ...]  MNEM/DIRECTIVE  OPERANDS  ; comment
# We extract bank, address, and all hex bytes (the raw opcode+operand encoding)
LINE_PATTERN = re.compile(
    r'^\$([0-9A-F]{2}):([0-9A-F]{4})\s+'
    r'([0-9A-F]{2}(?:\s+[0-9A-F]{2})*)'
)

# Pattern for .db "string" directives
DB_STRING_PATTERN = re.compile(
    r'^\$([0-9A-F]{2}):([0-9A-F]{4})\s+\.db\s+"(.*)"'
)

ROM_SIZE = 0x100000  # 1MB


def parse_hex_bytes(hex_str: str) -> list:
    """Parse space-separated hex bytes string into list of int values."""
    return [int(b, 16) for b in hex_str.strip().split()]


def bank_addr_to_lorom_offset(bank: int, addr: int) -> int:
    """Convert SNES bank:address to LoROM file offset for addresses >= $8000."""
    return (bank * 0x8000) + (addr - 0x8000)


def parse_db_string(line: str) -> tuple:
    """
    Parse .db "string" directive, returning (offset, bytes) or None.
    Format: $BB:AAAA            .db "string"
    Each character in the string becomes one byte (ASCII value).
    """
    m = DB_STRING_PATTERN.match(line)
    if not m:
        return None

    bank = int(m.group(1), 16)
    addr = int(m.group(2), 16)
    string_content = m.group(3)

    if addr < 0x8000:
        return None

    # Convert each character to its byte value
    # Handle escaped quotes: \" in the string means literal "
    bytes_list = []
    i = 0
    while i < len(string_content):
        if string_content[i] == '\\' and i + 1 < len(string_content):
            # Escaped character
            if string_content[i + 1] == '"':
                bytes_list.append(ord('"'))
            elif string_content[i + 1] == '\\':
                bytes_list.append(ord('\\'))
            else:
                bytes_list.append(ord(string_content[i]))
            i += 2
        else:
            bytes_list.append(ord(string_content[i]))
            i += 1

    offset = bank_addr_to_lorom_offset(bank, addr)
    return offset, bytes_list


def rebuild_from_assembly(asm_path: str, output_path: str) -> dict:
    """
    Parse assembly listing, extract raw bytes, and write to ROM binary.
    Returns stats about the rebuild.
    """
    rebuilt = bytearray(ROM_SIZE)
    filled = [False] * ROM_SIZE  # Track which bytes we've written

    stats = {
        'total_lines': 0,
        'parsed_lines': 0,
        'comment_lines': 0,
        'blank_lines': 0,
        'bytes_written': 0,
        'bytes_out_of_range': 0,
        'bank_addr_collisions': 0,
        'max_bank': 0,
    }

    with open(asm_path, 'r', encoding='utf-8') as f:
        for line in f:
            stats['total_lines'] += 1
            line = line.rstrip('\n\r')

            if not line.strip():
                stats['blank_lines'] += 1
                continue

            if line.strip().startswith(';'):
                stats['comment_lines'] += 1
                continue

            # Handle .db "string" directives first
            if '.db "' in line:
                result = parse_db_string(line)
                if result:
                    stats['parsed_lines'] += 1
                    offset, bytes_list = result
                    bank = offset // 0x8000
                    for i, byte_val in enumerate(bytes_list):
                        byte_offset = offset + i
                        if byte_offset < ROM_SIZE:
                            rebuilt[byte_offset] = byte_val
                            filled[byte_offset] = True
                    stats['bytes_written'] += len(bytes_list)
                    continue

            m = LINE_PATTERN.match(line)
            if not m:
                continue

            stats['parsed_lines'] += 1
            bank = int(m.group(1), 16)
            addr = int(m.group(2), 16)
            hex_bytes = parse_hex_bytes(m.group(3))

            if bank > stats['max_bank']:
                stats['max_bank'] = bank

            # Calculate ROM offset
            if addr >= 0x8000:
                offset = bank_addr_to_lorom_offset(bank, addr)
            else:
                # Addresses below $8000 in LoROM (rare, but handle)
                # Map: bank*0x8000 + addr for addr < $8000 would be wrong
                # These shouldn't exist in standard LoROM code/data
                stats['bytes_out_of_range'] += len(hex_bytes)
                continue

            for i, byte_val in enumerate(hex_bytes):
                byte_offset = offset + i
                if byte_offset < ROM_SIZE:
                    rebuilt[byte_offset] = byte_val
                    filled[byte_offset] = True
                else:
                    stats['bytes_out_of_range'] += 1

            stats['bytes_written'] += len(hex_bytes)

    # Write rebuilt ROM
    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, 'wb') as f:
        f.write(rebuilt)

    stats['total_filled_bytes'] = sum(filled)
    return stats, filled


def compare_roms(rebuilt_path: str, original_path: str, filled: list) -> dict:
    """Compare rebuilt ROM byte-by-byte with original."""
    with open(original_path, 'rb') as f:
        original = f.read()

    with open(rebuilt_path, 'rb') as f:
        rebuilt = f.read()

    assert len(original) == len(rebuilt) == ROM_SIZE, \
        f"Size mismatch: original={len(original)}, rebuilt={len(rebuilt)}"

    total_bytes = len(original)
    same = 0
    different = 0
    only_in_original = 0
    only_in_rebuilt = 0
    diffs = []

    for i in range(total_bytes):
        if filled[i]:
            # We wrote this byte from the disassembly
            if original[i] == rebuilt[i]:
                same += 1
            else:
                different += 1
                if len(diffs) < 200:
                    # Convert offset back to bank:addr for readability
                    bank = i // 0x8000
                    addr = (i % 0x8000) + 0x8000
                    diffs.append({
                        'offset': i,
                        'bank_addr': f'${bank:02X}:{addr:04X}',
                        'original': original[i],
                        'rebuilt': rebuilt[i],
                    })
        else:
            # We didn't write this byte (gap in disassembly)
            if original[i] != 0x00:
                only_in_original += 1
            # If original is 0x00 and we didn't write, it's a match by default

    compared_bytes = same + different
    coverage = (sum(filled) / total_bytes) * 100
    match_pct = (same / compared_bytes * 100) if compared_bytes > 0 else 0

    return {
        'total_bytes': total_bytes,
        'filled_bytes': sum(filled),
        'coverage_pct': coverage,
        'compared_bytes': compared_bytes,
        'same': same,
        'different': different,
        'match_pct': match_pct,
        'only_in_original_nonzero': only_in_original,
        'diffs': diffs,
    }


def generate_report(stats: dict, comparison: dict, report_path: str, filled: list):
    """Generate human-readable rebuild report."""
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write('=' * 70 + '\n')
        f.write('BINARY REBUILD REPORT\n')
        f.write('Reconstructed from assembly disassembly\n')
        f.write('=' * 70 + '\n\n')

        f.write('-' * 70 + '\n')
        f.write('PARSING STATISTICS\n')
        f.write('-' * 70 + '\n')
        f.write(f'  Total lines read:      {stats["total_lines"]:>10,}\n')
        f.write(f'  Instruction/data lines: {stats["parsed_lines"]:>10,}\n')
        f.write(f'  Comment lines:         {stats["comment_lines"]:>10,}\n')
        f.write(f'  Blank lines:           {stats["blank_lines"]:>10,}\n')
        f.write(f'  Max bank detected:     ${stats["max_bank"]:02X}\n')
        f.write('\n')

        f.write('-' * 70 + '\n')
        f.write('REBUILD STATISTICS\n')
        f.write('-' * 70 + '\n')
        f.write(f'  Bytes written to ROM:  {stats["bytes_written"]:>10,}\n')
        f.write(f'  Unique bytes in ROM:   {stats["total_filled_bytes"]:>10,}\n')
        f.write(f'  Bytes out of range:    {stats["bytes_out_of_range"]:>10,}\n')
        f.write(f'  ROM coverage:          {comparison["coverage_pct"]:>9.2f}%\n')
        f.write('\n')

        f.write('-' * 70 + '\n')
        f.write('COMPARISON WITH ORIGINAL ROM\n')
        f.write('-' * 70 + '\n')
        f.write(f'  Total ROM size:        {comparison["total_bytes"]:>10,} bytes\n')
        f.write(f'  Filled bytes:          {comparison["filled_bytes"]:>10,}\n')
        f.write(f'  Compared bytes:        {comparison["compared_bytes"]:>10,}\n')
        f.write(f'  Bytes identical:       {comparison["same"]:>10,}\n')
        f.write(f'  Bytes different:       {comparison["different"]:>10,}\n')
        f.write(f'  Match percentage:      {comparison["match_pct"]:>9.2f}%\n')
        f.write(f'  Non-zero unfilled:     {comparison["only_in_original_nonzero"]:>10,}\n')
        f.write('\n')

        # Per-bank breakdown
        f.write('-' * 70 + '\n')
        f.write('PER-BANK COVERAGE (disassembly coverage vs ROM)\n')
        f.write('-' * 70 + '\n')
        f.write(f'  {"Bank":<8} {"Filled":>10} {"Total":>10} {"Coverage":>10}\n')
        f.write(f'  {"----":<8} {"------":>10} {"-----":>10} {"--------":>10}\n')
        for bank in range(0x00, stats['max_bank'] + 1):
            bank_start = bank * 0x8000
            bank_end = bank_start + 0x8000
            bank_filled = sum(1 for i in range(bank_start, min(bank_end, ROM_SIZE)) if filled[i])
            bank_pct = (bank_filled / 0x8000) * 100
            f.write(f'  ${bank:02X}     {bank_filled:>10,} {0x8000:>10,} {bank_pct:>9.2f}%\n')
        f.write('\n')

        # Diffs
        if comparison['diffs']:
            f.write('-' * 70 + '\n')
            f.write(f'FIRST {len(comparison["diffs"])} BYTE DIFFERENCES (disassembly vs original)\n')
            f.write('-' * 70 + '\n')
            f.write(f'  {"Offset":>10}  {"Address":<12} {"Original":>10} {"Rebuilt":>10}\n')
            f.write(f'  {"------":>10}  {"-------":<12} {"--------":>10} {"-------":>10}\n')
            for d in comparison['diffs']:
                f.write(f'  {d["offset"]:>10}  {d["bank_addr"]:<12} ${d["original"]:02X}        ${d["rebuilt"]:02X}\n')
            f.write('\n')
        else:
            f.write('  No byte differences found in disassembled regions!\n\n')

        f.write('=' * 70 + '\n')
        if comparison['match_pct'] >= 99.9:
            f.write('RESULT: EXCELLENT - Disassembly is nearly byte-perfect\n')
        elif comparison['match_pct'] >= 99.0:
            f.write('RESULT: GOOD - High match rate, minor discrepancies\n')
        elif comparison['match_pct'] >= 95.0:
            f.write('RESULT: FAIR - Most bytes match, some issues to investigate\n')
        else:
            f.write('RESULT: POOR - Significant mismatches, needs investigation\n')
        f.write('=' * 70 + '\n')


def main():
    base_dir = Path('/home/douglasdreer/projects/decompiler/snes/the-legend-of-zelda-link-of-the-past')
    asm_path = base_dir / 'output' / 'assembly' / 'zelda_full.asm'
    original_rom = base_dir / 'roms' / 'Legend of Zelda, The - A Link to the Past (USA).sfc'
    rebuilt_path = base_dir / 'output' / 'zelda_rebuilt.sfc'
    report_path = base_dir / 'output' / 'rebuild_report.txt'

    if not asm_path.exists():
        print(f'ERROR: Assembly file not found: {asm_path}')
        sys.exit(1)
    if not original_rom.exists():
        print(f'ERROR: Original ROM not found: {original_rom}')
        sys.exit(1)

    print(f'Assembly: {asm_path}')
    print(f'Original ROM: {original_rom}')
    print(f'Rebuilt output: {rebuilt_path}')
    print(f'Report: {report_path}')
    print()

    print('Phase 1: Parsing assembly and rebuilding binary...')
    stats, filled = rebuild_from_assembly(str(asm_path), str(rebuilt_path))
    print(f'  Parsed {stats["parsed_lines"]:,} instruction/data lines')
    print(f'  Wrote {stats["bytes_written"]:,} bytes ({stats["total_filled_bytes"]:,} unique positions)')
    print(f'  Coverage: {stats["total_filled_bytes"] / ROM_SIZE * 100:.2f}%')
    print()

    print('Phase 2: Comparing with original ROM...')
    comparison = compare_roms(str(rebuilt_path), str(original_rom), filled)
    print(f'  Match: {comparison["same"]:,} / {comparison["compared_bytes"]:,} '
          f'({comparison["match_pct"]:.2f}%)')
    print(f'  Differences: {comparison["different"]:,}')
    print(f'  Non-zero unfilled: {comparison["only_in_original_nonzero"]:,}')
    print()

    print('Phase 3: Generating report...')
    generate_report(stats, comparison, str(report_path), filled)
    print(f'  Report saved to: {report_path}')
    print()

    print('=' * 60)
    print(f'RESULT: {comparison["match_pct"]:.2f}% match rate')
    print(f'  {comparison["same"]:,} identical / {comparison["different"]:,} different')
    print(f'  Coverage: {comparison["coverage_pct"]:.2f}% of ROM')
    print('=' * 60)


if __name__ == '__main__':
    main()
