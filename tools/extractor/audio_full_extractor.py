#!/usr/bin/env python3
"""
Full Audio Extractor - The Legend of Zelda: A Link to the Past
Scans entire ROM for BRR audio data and extracts all samples
"""

import struct
import wave
import sys
from pathlib import Path
from collections import defaultdict


# BRR filter coefficients (fixed-point representation)
BRR_FILTERS = [
    (0, 0),
    (240, -256),   # 15/16, -1
    (480, -224),   # 30/16, -7/8
    (384, -208),   # 24/16, -13/16
    (480, 240),    # 30/16, 15/16
    (384, 208),    # 24/16, 13/16
    (240, 256),    # 15/16, 1
    (160, 288),    # 10/16, 9/8
]


def decode_brr_block(block_data, prev_samples=(0, 0)):
    """
    Decode a single BRR block (10 bytes) into 16 samples.

    BRR block format:
    - Byte 0: Header (bit 7: end, bits 6-5: loop, bit 4: end, bits 3-0: range)
    - Bytes 1-9: 16 samples encoded in nibbles

    Returns: (samples_list, end_flag)
    """
    if len(block_data) < 10:
        return [], False

    header = block_data[0]
    end_flag = (header >> 4) & 1
    filter_index = (header >> 5) & 7
    range_val = header & 0x0F

    samples = []
    prev1, prev2 = prev_samples

    for i in range(9):
        byte = block_data[i + 1]
        # High nibble first, then low nibble
        for nibble_idx in range(2):
            if nibble_idx == 0:
                nibble = (byte >> 4) & 0x0F
            else:
                nibble = byte & 0x0F

            # Convert unsigned 4-bit to signed 4-bit
            if nibble >= 8:
                sample = nibble - 16
            else:
                sample = nibble

            # Apply range (shift left)
            if range_val <= 11:
                sample = sample << range_val
            else:
                # Arithmetic shift right for large range
                if sample < 0:
                    sample = (sample >> (range_val - 12)) | (0xF000 << (12 - range_val))
                else:
                    sample = sample >> (range_val - 12)

            # Apply BRR filter
            filter_a, filter_b = BRR_FILTERS[filter_index]
            sample = sample + ((prev1 * filter_a + 128) >> 8) + ((prev2 * filter_b + 128) >> 8)

            # Clamp to 16-bit signed range
            sample = max(-32768, min(32767, sample))

            samples.append(sample)
            prev2 = prev1
            prev1 = sample

    return samples, end_flag == 1


def decode_brr_data(data):
    """
    Decode a sequence of BRR blocks into samples.
    Returns: list of samples
    """
    all_samples = []
    offset = 0
    prev1, prev2 = 0, 0

    while offset + 10 <= len(data):
        block_samples, is_end = decode_brr_block(data[offset:offset + 10], (prev1, prev2))
        all_samples.extend(block_samples)
        offset += 10

        if is_end:
            break

    return all_samples


def is_valid_brr_header(byte):
    """
    Check if a byte looks like a valid BRR header.
    Valid BRR headers have range 0-12 and filter 0-7.
    """
    range_val = byte & 0x0F
    filter_val = (byte >> 5) & 7
    # Range should be 0-12, filter 0-7
    return 0 <= range_val <= 12 and 0 <= filter_val <= 7


def validate_brr_sequence(rom_data, start_offset, max_blocks=50):
    """
    Validate a sequence of BRR blocks starting at start_offset.
    Returns: (is_valid, num_blocks, end_offset)
    """
    offset = start_offset
    block_count = 0

    while offset + 10 <= len(rom_data) and block_count < max_blocks:
        header = rom_data[offset]

        # Check if header is valid
        if not is_valid_brr_header(header):
            break

        # Check if data bytes have reasonable nibble values
        valid_data = True
        for i in range(1, 10):
            byte = rom_data[offset + i]
            high_nibble = (byte >> 4) & 0x0F
            low_nibble = byte & 0x0F
            # Nibbles should be 0-15 (always valid for 4-bit)
            # But we can check for patterns that suggest non-BRR data

        block_count += 1
        offset += 10

        # Stop if end flag is set
        if (header >> 4) & 1:
            break

    return block_count >= 2, block_count, offset


def scan_bank(rom_data, bank_start, bank_end):
    """
    Scan a bank for BRR audio regions.
    Returns: list of (offset, block_count, end_offset)
    """
    regions = []
    i = bank_start

    while i < bank_end - 10:
        if is_valid_brr_header(rom_data[i]):
            is_valid, block_count, end_offset = validate_brr_sequence(rom_data, i, max_blocks=100)

            if is_valid and block_count >= 2:
                regions.append((i, block_count, end_offset))
                i = end_offset  # Skip past this region
            else:
                i += 1
        else:
            i += 1

    return regions


def samples_to_wav(samples, sample_rate=32000, filename="output.wav"):
    """Save decoded samples to WAV file."""
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1)      # Mono
        wav_file.setsampwidth(2)      # 16-bit
        wav_file.setframerate(sample_rate)

        frame_data = b''
        for sample in samples:
            sample = max(-32768, min(32767, sample))
            frame_data += struct.pack('<h', sample)

        wav_file.writeframes(frame_data)


def extract_and_save(rom_data, offset, end_offset, output_path, sample_rate=32000):
    """Extract BRR data from offset to end_offset and save as WAV."""
    data = rom_data[offset:end_offset]
    samples = decode_brr_data(data)

    if samples and len(samples) > 16:  # At least one full block
        filename = output_path / f"sample_{offset:06X}.wav"
        samples_to_wav(samples, sample_rate, str(filename))
        return len(samples), filename
    return 0, None


def main():
    if len(sys.argv) < 2:
        print("Usage: python audio_full_extractor.py <rom_path> [output_dir]")
        sys.exit(1)

    rom_path = sys.argv[1]
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("output/audio")

    if not Path(rom_path).exists():
        print(f"Error: ROM not found: {rom_path}")
        sys.exit(1)

    # Load ROM
    print(f"Loading ROM: {rom_path}")
    with open(rom_path, 'rb') as f:
        rom_data = f.read()

    print(f"ROM loaded: {Path(rom_path).name}")
    print(f"Size: {len(rom_data)} bytes ({len(rom_data) / 1024 / 1024:.2f} MB)")

    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)

    # Scan banks $00-$1F for BRR data
    print("\n" + "=" * 60)
    print("SCANNING ALL BANKS FOR BRR AUDIO DATA")
    print("=" * 60)

    all_regions = []
    bank_size = 0x10000  # 64KB per bank

    for bank in range(0x00, 0x20):
        bank_start = bank * bank_size
        bank_end = min(bank_start + bank_size, len(rom_data))

        if bank_start >= len(rom_data):
            break

        regions = scan_bank(rom_data, bank_start, bank_end)
        if regions:
            print(f"  Bank ${bank:02X} (0x{bank_start:06X}-0x{bank_end:06X}): {len(regions)} regions")
            all_regions.extend(regions)

    print(f"\nTotal BRR regions found: {len(all_regions)}")

    # Remove overlapping regions and sort
    all_regions.sort(key=lambda x: x[0])
    merged_regions = []
    for offset, block_count, end_offset in all_regions:
        if merged_regions and offset <= merged_regions[-1][2]:
            # Merge overlapping regions
            prev = merged_regions[-1]
            merged_regions[-1] = (prev[0], prev[1] + block_count, max(prev[2], end_offset))
        else:
            merged_regions.append((offset, block_count, end_offset))

    print(f"After merging: {len(merged_regions)} unique regions")

    # Extract all samples
    print("\n" + "=" * 60)
    print("EXTRACTING BRR SAMPLES")
    print("=" * 60)

    total_samples = 0
    extracted_files = []
    total_wav_size = 0

    for idx, (offset, block_count, end_offset) in enumerate(merged_regions):
        size = end_offset - offset
        num_samples, filename = extract_and_save(rom_data, offset, end_offset, output_dir)

        if num_samples > 0:
            total_samples += num_samples
            extracted_files.append(filename)
            if filename.exists():
                total_wav_size += filename.stat().st_size

        # Progress update
        if idx % 50 == 0 or idx == len(merged_regions) - 1:
            print(f"  Processing {idx + 1}/{len(merged_regions)}: 0x{offset:06X} ({block_count} blocks, {size} bytes)")

    # Final report
    print("\n" + "=" * 60)
    print("EXTRACTION COMPLETE")
    print("=" * 60)
    print(f"  Total BRR regions: {len(merged_regions)}")
    print(f"  Samples decoded: {total_samples}")
    print(f"  WAV files created: {len(extracted_files)}")
    print(f"  Total WAV size: {total_wav_size / 1024:.2f} KB ({total_wav_size / 1024 / 1024:.2f} MB)")
    print(f"  Output directory: {output_dir}")

    # Save extraction report
    report_path = output_dir / "extraction_report.txt"
    with open(report_path, 'w') as f:
        f.write("BRR Audio Extraction Report\n")
        f.write("=" * 40 + "\n\n")
        f.write(f"ROM: {Path(rom_path).name}\n")
        f.write(f"ROM Size: {len(rom_data)} bytes\n\n")
        f.write(f"BRR Regions Found: {len(merged_regions)}\n")
        f.write(f"Samples Decoded: {total_samples}\n")
        f.write(f"WAV Files Created: {len(extracted_files)}\n")
        f.write(f"Total WAV Size: {total_wav_size} bytes\n\n")
        f.write("Regions:\n")
        for offset, block_count, end_offset in merged_regions:
            size = end_offset - offset
            f.write(f"  0x{offset:06X}-0x{end_offset:06X}: {block_count} blocks ({size} bytes)\n")

    print(f"\nReport saved to: {report_path}")


if __name__ == "__main__":
    main()
