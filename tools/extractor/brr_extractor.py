#!/usr/bin/env python3
"""
BRR Decoder and Extractor - The Legend of Zelda: A Link to the Past
Decodes BRR (Bit Rate Reduction) audio format used by SNES SPC700
"""

import struct
import wave
import sys
from pathlib import Path


# BRR filter coefficients
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
    - Byte 0: Header (bits 7-4: end flag + filter, bits 3-0: range)
    - Bytes 1-9: 18 nibbles = 16 samples (encoded in 9 bytes)

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
                # Arithmetic shift right for negative values
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


def is_brr_header(byte):
    """Check if a byte looks like a valid BRR header."""
    range_val = byte & 0x0F
    # Range should be 0-12 for valid BRR data
    return 0 <= range_val <= 12


def scan_rom_for_brr(rom_data, bank_start=0x0D0000, bank_end=0x100000):
    """
    Scan ROM for potential BRR sample regions.

    Returns: list of (offset, estimated_length)
    """
    regions = []
    i = bank_start

    while i < bank_end - 10:
        # Check for potential BRR header
        if is_brr_header(rom_data[i]):
            # Look ahead to see if we have sequential BRR blocks
            region_start = i
            block_count = 0
            j = i

            while j + 10 <= bank_end:
                if is_brr_header(rom_data[j]) and block_count < 1000:
                    block_count += 1
                    j += 10
                else:
                    break

            if block_count >= 2:  # At least 2 blocks to be a valid region
                regions.append((region_start, block_count * 10))
                i = j
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


def extract_single_sample(rom_data, offset, output_path, sample_rate=32000):
    """Extract and decode a single BRR sample starting at offset."""
    # Find end of sample (look for end flag or limit)
    end_offset = min(offset + 100000, len(rom_data))  # Max 100KB per sample

    sample_data = rom_data[offset:end_offset]
    samples = decode_brr_data(sample_data)

    if samples and len(samples) > 16:  # At least one full block
        filename = output_path / f"sample_{offset:06X}.wav"
        samples_to_wav(samples, sample_rate, str(filename))
        return len(samples), filename
    return 0, None


def main():
    if len(sys.argv) < 2:
        print("Usage: python brr_extractor.py <rom_path> [output_dir]")
        sys.exit(1)

    rom_path = sys.argv[1]
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("output/audio")

    if not Path(rom_path).exists():
        print(f"Error: ROM not found: {rom_path}")
        sys.exit(1)

    # Load ROM
    with open(rom_path, 'rb') as f:
        rom_data = f.read()

    print(f"ROM loaded: {Path(rom_path).name}")
    print(f"Size: {len(rom_data)} bytes ({len(rom_data) / 1024 / 1024:.2f} MB)")

    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)

    # Scan for BRR regions in SPC700 banks ($1A-$1F)
    print("\nScanning for BRR audio data in SPC700 banks ($0D0000-$0FFFFF)...")
    regions = scan_rom_for_brr(rom_data, 0x0D0000, 0x100000)

    print(f"Found {len(regions)} potential BRR regions")

    # Extract samples
    total_samples = 0
    extracted_files = []

    for idx, (offset, size) in enumerate(regions[:100]):  # Limit to first 100 regions
        num_samples, filename = extract_single_sample(rom_data, offset, output_dir)
        if num_samples > 0:
            total_samples += num_samples
            extracted_files.append(filename)
            if idx % 10 == 0:
                print(f"  Processing region {idx + 1}/{len(regions)}: 0x{offset:06X} ({size} bytes)")

    print(f"\nExtraction complete!")
    print(f"  Samples extracted: {total_samples}")
    print(f"  WAV files created: {len(extracted_files)}")
    print(f"  Output directory: {output_dir}")

    # Report file sizes
    total_size = 0
    for f in extracted_files:
        if f and f.exists():
            total_size += f.stat().st_size
    print(f"  Total WAV size: {total_size / 1024:.2f} KB")


if __name__ == "__main__":
    main()
