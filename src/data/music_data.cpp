#include "music_data.h"
#include "../types.h"

// =============================================================================
// Dados de audio - Zelda: A Link to the Past
// Banks $0A-$1F
// =============================================================================

// =============================================================================
// Samples BRR
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const BRRSample BRR_SAMPLES[BRR_SAMPLE_COUNT] = {
    // Sample 0: Piano
    {
        .rom_offset = 0x0D0000,
        .block_count = 100,
        .total_size = 1000,
        .loop_offset = 500,
        .pitch = 0x1000,
        .filter = 0,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x07,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x0F,
        .name = "Piano",
    },
    // Sample 1: Harp
    {
        .rom_offset = 0x0D0400,
        .block_count = 80,
        .total_size = 800,
        .loop_offset = 400,
        .pitch = 0x1000,
        .filter = 0,
        .adsr_attack = 0x08,
        .adsr_decay = 0x05,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .name = "Harp",
    },
    // Sample 2: Flute
    {
        .rom_offset = 0x0D0800,
        .block_count = 60,
        .total_size = 600,
        .loop_offset = 300,
        .pitch = 0x1200,
        .filter = 0,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x03,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .name = "Flute",
    },
    // Sample 3: Strings
    {
        .rom_offset = 0x0D0C00,
        .block_count = 120,
        .total_size = 1200,
        .loop_offset = 600,
        .pitch = 0x1000,
        .filter = 0,
        .adsr_attack = 0x04,
        .adsr_decay = 0x03,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .name = "Strings",
    },
    // Sample 4: Organ
    {
        .rom_offset = 0x0D1000,
        .block_count = 90,
        .total_size = 900,
        .loop_offset = 450,
        .pitch = 0x1000,
        .filter = 0,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x07,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x0F,
        .name = "Organ",
    },
    // Samples 5-127: Placeholder - dados reais da ROM
};

// =============================================================================
// Instrumentos
// =============================================================================
const Instrument INSTRUMENTS[INSTRUMENT_COUNT] = {
    // Instrumento 0: Piano
    {
        .sample_index = 0,
        .pitch_base = 0x1000,
        .pitch_range = 0x02,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x07,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x0F,
        .gain = 0x40,
        .harmony_type = 0,
        .effect_type = 0,
        .effect_depth = 0,
        .reserved = 0,
    },
    // Instrumento 1: Harp
    {
        .sample_index = 1,
        .pitch_base = 0x1000,
        .pitch_range = 0x01,
        .adsr_attack = 0x08,
        .adsr_decay = 0x05,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .gain = 0x40,
        .harmony_type = 0,
        .effect_type = 0,
        .effect_depth = 0,
        .reserved = 0,
    },
    // Instrumento 2: Flute
    {
        .sample_index = 2,
        .pitch_base = 0x1200,
        .pitch_range = 0x02,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x03,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .gain = 0x40,
        .harmony_type = 0,
        .effect_type = 0,
        .effect_depth = 0,
        .reserved = 0,
    },
    // Instrumento 3: Strings
    {
        .sample_index = 3,
        .pitch_base = 0x1000,
        .pitch_range = 0x01,
        .adsr_attack = 0x04,
        .adsr_decay = 0x03,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x1F,
        .gain = 0x40,
        .harmony_type = 0,
        .effect_type = 0,
        .effect_depth = 0,
        .reserved = 0,
    },
    // Instrumento 4: Organ
    {
        .sample_index = 4,
        .pitch_base = 0x1000,
        .pitch_range = 0x02,
        .adsr_attack = 0x0F,
        .adsr_decay = 0x07,
        .adsr_sustain = 0x1F,
        .adsr_release = 0x0F,
        .gain = 0x40,
        .harmony_type = 0,
        .effect_type = 0,
        .effect_depth = 0,
        .reserved = 0,
    },
    // Instrumentos 5-63: Placeholder
};

// =============================================================================
// Musicas
// =============================================================================
const MusicTrack MUSIC_TRACKS[MUSIC_COUNT] = {
    // 0: Title Screen
    {
        .name = "Title Screen",
        .sequence_offset = 0x0D0000,
        .sequence_size = 0x1000,
        .channel_count = 5,
        .default_tempo = 0x50,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x80,
    },
    // 1: Overworld Theme
    {
        .name = "Overworld Theme",
        .sequence_offset = 0x0D1000,
        .sequence_size = 0x2000,
        .channel_count = 5,
        .default_tempo = 0x50,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x40,
    },
    // 2: Dark World Theme
    {
        .name = "Dark World Theme",
        .sequence_offset = 0x0D3000,
        .sequence_size = 0x2000,
        .channel_count = 5,
        .default_tempo = 0x48,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x40,
    },
    // 3: Dungeon Theme
    {
        .name = "Dungeon Theme",
        .sequence_offset = 0x0D5000,
        .sequence_size = 0x1800,
        .channel_count = 4,
        .default_tempo = 0x48,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x40,
    },
    // 4: Boss Theme
    {
        .name = "Boss Theme",
        .sequence_offset = 0x0D6800,
        .sequence_size = 0x1800,
        .channel_count = 5,
        .default_tempo = 0x60,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x60,
    },
    // 5: Zelda's Lullaby
    {
        .name = "Zelda's Lullaby",
        .sequence_offset = 0x0D8000,
        .sequence_size = 0x1000,
        .channel_count = 3,
        .default_tempo = 0x40,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x80,
    },
    // 6: Item Get Fanfare
    {
        .name = "Item Get",
        .sequence_offset = 0x0D9000,
        .sequence_size = 0x0800,
        .channel_count = 3,
        .default_tempo = 0x50,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0xC0,
    },
    // 7: Crystal Get
    {
        .name = "Crystal Get",
        .sequence_offset = 0x0D9800,
        .sequence_size = 0x0800,
        .channel_count = 3,
        .default_tempo = 0x50,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0xC0,
    },
    // 8: Game Over
    {
        .name = "Game Over",
        .sequence_offset = 0x0DA000,
        .sequence_size = 0x0800,
        .channel_count = 2,
        .default_tempo = 0x30,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x80,
    },
    // 9: Ending Theme
    {
        .name = "Ending Theme",
        .sequence_offset = 0x0DA800,
        .sequence_size = 0x3000,
        .channel_count = 5,
        .default_tempo = 0x40,
        .default_volume = 0x7F,
        .instrument_bank = 0x1A,
        .instrument_table_offset = 0x0D8000,
        .sfx_channel = 7,
        .priority = 0x40,
    },
    // Musicas 10-64: Placeholder - dados reais da ROM
};

// =============================================================================
// Sound Effects
// =============================================================================
const SFXEntry SFX_TABLE[SFX_COUNT] = {
    // SFX 0: Sword Slash
    {
        .name = "Sword Slash",
        .data_offset = 0x0E4000,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x80,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 1: Sword Hit
    {
        .name = "Sword Hit",
        .data_offset = 0x0E4200,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x80,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 2: Shield Block
    {
        .name = "Shield Block",
        .data_offset = 0x0E4400,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1200,
    },
    // SFX 3: Arrow Shoot
    {
        .name = "Arrow Shoot",
        .data_offset = 0x0E4500,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1400,
    },
    // SFX 4: Bomb Explosion
    {
        .name = "Bomb Explosion",
        .data_offset = 0x0E4600,
        .data_size = 0x0400,
        .channel = 6,
        .priority = 0xA0,
        .repeat_count = 1,
        .pitch = 0x0800,
    },
    // SFX 5: Magic Wand
    {
        .name = "Magic Wand",
        .data_offset = 0x0E4A00,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 6: Heart Pick
    {
        .name = "Heart Pick",
        .data_offset = 0x0E4C00,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x40,
        .repeat_count = 1,
        .pitch = 0x1800,
    },
    // SFX 7: Rupee Get
    {
        .name = "Rupee Get",
        .data_offset = 0x0E4D00,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x40,
        .repeat_count = 1,
        .pitch = 0x1800,
    },
    // SFX 8: Door Open
    {
        .name = "Door Open",
        .data_offset = 0x0E4E00,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 9: Chest Open
    {
        .name = "Chest Open",
        .data_offset = 0x0E5000,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x80,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 10: Item Fanfare
    {
        .name = "Item Fanfare",
        .data_offset = 0x0E5200,
        .data_size = 0x0400,
        .channel = 6,
        .priority = 0xC0,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 11: Link Hurt
    {
        .name = "Link Hurt",
        .data_offset = 0x0E5600,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0xA0,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 12: Enemy Hurt
    {
        .name = "Enemy Hurt",
        .data_offset = 0x0E5800,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 13: Enemy Death
    {
        .name = "Enemy Death",
        .data_offset = 0x0E5900,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 1,
        .pitch = 0x1000,
    },
    // SFX 14: Jump
    {
        .name = "Jump",
        .data_offset = 0x0E5B00,
        .data_size = 0x0100,
        .channel = 6,
        .priority = 0x40,
        .repeat_count = 1,
        .pitch = 0x1400,
    },
    // SFX 15: Pegasus Boot Dash
    {
        .name = "Pegasus Dash",
        .data_offset = 0x0E5C00,
        .data_size = 0x0200,
        .channel = 6,
        .priority = 0x60,
        .repeat_count = 0,  // Repete
        .pitch = 0x1000,
    },
    // SFX 16-95: Placeholder - dados reais da ROM
};
