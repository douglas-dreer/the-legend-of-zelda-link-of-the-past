#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <cstddef>

// Basic types for SNES emulation
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

// SNES address types
using SNESAddr = uint16;
using SNESBank = uint8;
using SNESAddress24 = uint32;  // bank:addr (24-bit)

// ROM constants
constexpr uint32 ROM_SIZE = 0x100000;  // 1 MB LoROM
constexpr uint32 WRAM_LOW_SIZE = 0x2000;  // 8 KB
constexpr uint32 WRAM_EXTENDED_SIZE = 0x10000;  // 64 KB
constexpr uint32 VRAM_SIZE = 0x10000;  // 64 KB
constexpr uint32 OAM_SIZE = 0x220;  // 544 bytes
constexpr uint32 CGRAM_SIZE = 0x200;  // 512 bytes

// Utility functions
constexpr uint16 read16_le(const uint8* data) {
    return data[0] | (data[1] << 8);
}

constexpr SNESAddress24 snes_address(SNESBank bank, SNESAddr addr) {
    return (static_cast<uint32>(bank) << 16) | addr;
}

// LoROM address translation
constexpr uint32 snes_to_rom_offset(SNESBank bank, SNESAddr addr) {
    if (addr < 0x8000) {
        return 0xFFFFFFFF;  // Not in ROM
    }
    return static_cast<uint32>(bank) * 0x8000 + (addr - 0x8000);
}

constexpr void rom_offset_to_snes(uint32 offset, SNESBank* bank, SNESAddr* addr) {
    *bank = offset / 0x8000;
    *addr = (offset % 0x8000) + 0x8000;
}

#endif // TYPES_H