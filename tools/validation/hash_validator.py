#!/usr/bin/env python3
"""
Hash Validator for SNES ROMs
Validates that rebuilt ROM matches original using multiple hash algorithms.
"""

import hashlib
import sys
import json
from pathlib import Path
from datetime import datetime


def compute_hashes(file_path: str) -> dict:
    """Compute MD5, SHA1, SHA256, and CRC32 hashes for a file."""
    md5 = hashlib.md5()
    sha1 = hashlib.sha1()
    sha256 = hashlib.sha256()

    with open(file_path, 'rb') as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            md5.update(chunk)
            sha1.update(chunk)
            sha256.update(chunk)

    # CRC32 (manual implementation)
    crc32_table = []
    for i in range(256):
        c = i
        for _ in range(8):
            if c & 1:
                c = 0xEDB88320 ^ (c >> 1)
            else:
                c = c >> 1
        crc32_table.append(c)

    crc32 = 0xFFFFFFFF
    with open(file_path, 'rb') as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            for byte in chunk:
                crc32 = crc32_table[(crc32 ^ byte) & 0xFF] ^ (crc32 >> 8)
    crc32 = crc32 ^ 0xFFFFFFFF

    # SNES checksum
    with open(file_path, 'rb') as f:
        data = f.read()
    snes_checksum = 0
    for i in range(0, len(data), 2):
        if i + 1 < len(data):
            snes_checksum += (data[i] << 8) | data[i + 1]
        else:
            snes_checksum += data[i] << 8
    snes_checksum = snes_checksum & 0xFFFF
    snes_complement = (~snes_checksum) & 0xFFFF

    return {
        'md5': md5.hexdigest(),
        'sha1': sha1.hexdigest(),
        'sha256': sha256.hexdigest(),
        'crc32': f'{crc32:08X}',
        'snes_checksum': f'{snes_checksum:04X}',
        'snes_complement': f'{snes_complement:04X}',
        'size': len(data),
    }


def validate_snes_header(file_path: str) -> dict:
    """Validate SNES ROM header integrity."""
    with open(file_path, 'rb') as f:
        data = f.read()

    # LoROM header at $7FC0-$7FFF (offset 0x7FC0-0x7FFF)
    header_offset = 0x7FC0
    if header_offset + 0x30 > len(data):
        return {'valid': False, 'error': 'File too small for header'}

    header = data[header_offset:header_offset + 0x30]

    game_name = header[0:21].decode('ascii', errors='replace').rstrip('\x00')
    map_mode = header[21]
    rom_size = header[22]
    country = header[24]
    publisher = header[25]
    version = header[26]

    # Validate checksum
    checksum_addr = 0x7FDC
    complement_addr = 0x7FDE
    checksum = data[checksum_addr] | (data[checksum_addr + 1] << 8)
    complement = data[complement_addr] | (data[complement_addr + 1] << 8)
    checksum_valid = (checksum + complement) & 0xFFFF == 0xFFFF

    return {
        'valid': True,
        'game_name': game_name,
        'map_mode': f'0x{map_mode:02X} ({"LoROM" if map_mode == 0x20 else "HiROM" if map_mode == 0x21 else "Unknown"})',
        'rom_size': f'{2**rom_size} MB',
        'country': f'0x{country:02X}',
        'publisher': f'0x{publisher:02X}',
        'version': f'1.{version}',
        'checksum': f'0x{checksum:04X}',
        'complement': f'0x{complement:04X}',
        'checksum_valid': checksum_valid,
    }


def compare_roms(original_path: str, rebuilt_path: str) -> dict:
    """Compare two ROM files byte-by-byte and report differences."""
    with open(original_path, 'rb') as f:
        original = f.read()
    with open(rebuilt_path, 'rb') as f:
        rebuilt = f.read()

    if len(original) != len(rebuilt):
        return {
            'identical': False,
            'error': f'Size mismatch: original={len(original)}, rebuilt={len(rebuilt)}',
        }

    diffs = []
    for i in range(len(original)):
        if original[i] != rebuilt[i]:
            bank = i // 0x8000
            addr_in_bank = i % 0x8000
            snes_addr = 0x8000 + addr_in_bank
            diffs.append({
                'offset': f'0x{i:06X}',
                'snes_addr': f'${bank:02X}:{snes_addr:04X}',
                'original': f'0x{original[i]:02X}',
                'rebuilt': f'0x{rebuilt[i]:02X}',
                'original_ascii': chr(original[i]) if 32 <= original[i] < 127 else '.',
            })

    return {
        'identical': len(diffs) == 0,
        'total_bytes': len(original),
        'different_bytes': len(diffs),
        'match_percentage': ((len(original) - len(diffs)) / len(original)) * 100,
        'diffs': diffs[:50],  # First 50 diffs
    }


def generate_validation_report(original_path: str, rebuilt_path: str, output_path: str):
    """Generate comprehensive validation report."""
    report = {
        'timestamp': datetime.now().isoformat(),
        'original': {
            'path': original_path,
            'hashes': compute_hashes(original_path),
            'header': validate_snes_header(original_path),
        },
        'rebuilt': {
            'path': rebuilt_path,
            'hashes': compute_hashes(rebuilt_path),
            'header': validate_snes_header(rebuilt_path),
        },
        'comparison': compare_roms(original_path, rebuilt_path),
    }

    # Check if hashes match
    hashes_match = report['original']['hashes']['md5'] == report['rebuilt']['hashes']['md5']
    report['validation_passed'] = hashes_match and report['comparison']['identical']

    # Write JSON report
    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=2, ensure_ascii=False)

    # Print summary
    print('=' * 60)
    print('ROM VALIDATION REPORT')
    print('=' * 60)
    print(f'Original: {original_path}')
    print(f'Rebuilt:  {rebuilt_path}')
    print()
    print(f'MD5 Original:  {report["original"]["hashes"]["md5"]}')
    print(f'MD5 Rebuilt:   {report["rebuilt"]["hashes"]["md5"]}')
    print(f'Hashes Match:  {"YES" if hashes_match else "NO"}')
    print()
    print(f'Bytes Compared: {report["comparison"]["total_bytes"]}')
    print(f'Bytes Different: {report["comparison"]["different_bytes"]}')
    print(f'Match: {report["comparison"]["match_percentage"]:.2f}%')
    print()
    print(f'SNES Header Valid (Original): {report["original"]["header"]["checksum_valid"]}')
    print(f'SNES Header Valid (Rebuilt):  {report["rebuilt"]["header"]["checksum_valid"]}')
    print()
    print(f'VALIDATION: {"PASSED" if report["validation_passed"] else "FAILED"}')
    print('=' * 60)

    return report


def main():
    if len(sys.argv) < 3:
        print("Uso: python hash_validator.py <original.sfc> <rebuilt.sfc> [output.json]")
        sys.exit(1)

    original = sys.argv[1]
    rebuilt = sys.argv[2]
    output = sys.argv[3] if len(sys.argv) > 3 else 'output/validation_report.json'

    generate_validation_report(original, rebuilt, output)


if __name__ == "__main__":
    main()
