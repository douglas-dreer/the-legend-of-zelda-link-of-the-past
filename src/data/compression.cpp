#include "compression.h"
#include "../types.h"

// =============================================================================
// Tabelas de compressao LZ - Zelda: A Link to the Past
// Banks $0A-$1F
// =============================================================================

// =============================================================================
// Tabela principal de dados comprimidos
// =============================================================================
const CompressedDataBlock COMPRESSED_DATA_TABLE[COMPRESSED_DATA_TABLE_SIZE] = {
    // Bloco 0: Tiles UI
    {
        .rom_offset = 0x057BE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0A,
        .target_addr = 0x8000,
        .description = "Tiles UI/Fontes",
    },
    // Bloco 1: Tiles BG overworld
    {
        .rom_offset = 0x05FBE0,
        .uncompressed_size = 0x8000,
        .compressed_size = 0x4000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0B,
        .target_addr = 0x8000,
        .description = "Tiles BG Overworld",
    },
    // Bloco 2: Tiles Link
    {
        .rom_offset = 0x067BE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_SPRITE,
        .target_bank = 0x0C,
        .target_addr = 0x8000,
        .description = "Tiles Link (todas animacoes)",
    },
    // Bloco 3: Tiles inimigos 1
    {
        .rom_offset = 0x06FBE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_SPRITE,
        .target_bank = 0x0D,
        .target_addr = 0x8000,
        .description = "Tiles Inimigos Tipo 1",
    },
    // Bloco 4: Tiles inimigos 2
    {
        .rom_offset = 0x077BE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_SPRITE,
        .target_bank = 0x0D,
        .target_addr = 0xC000,
        .description = "Tiles Inimigos Tipo 2",
    },
    // Bloco 5: Tiles masmorra 1
    {
        .rom_offset = 0x07FBE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0E,
        .target_addr = 0x8000,
        .description = "Tiles Castelo da Agonia",
    },
    // Bloco 6: Tiles masmorra 2
    {
        .rom_offset = 0x087BE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0E,
        .target_addr = 0xC000,
        .description = "Tiles Caverna do Trovao",
    },
    // Bloco 7: Tiles masmorra 3
    {
        .rom_offset = 0x08FBE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0F,
        .target_addr = 0x8000,
        .description = "Tiles Mina de Ouro",
    },
    // Bloco 8: Tiles masmorra 4
    {
        .rom_offset = 0x097BE0,
        .uncompressed_size = 0x4000,
        .compressed_size = 0x2000,
        .type = COMP_TYPE_TILES_BG,
        .target_bank = 0x0F,
        .target_addr = 0xC000,
        .description = "Tiles Palacio do Gelo",
    },
    // Bloco 9: Tiles itens
    {
        .rom_offset = 0x09FBE0,
        .uncompressed_size = 0x2000,
        .compressed_size = 0x1000,
        .type = COMP_TYPE_TILES_SPRITE,
        .target_bank = 0x10,
        .target_addr = 0x8000,
        .description = "Tiles Itens/Objetos",
    },
    // Blocos 10-63: Placeholder - devem ser mapeados pela analise da ROM
};

// Blocos de tiles
const CompressedDataBlock LZ_TILE_BLOCKS[LZ_TILE_BLOCK_COUNT] = {
    // Mesmos blocos da tabela principal (0-9)
    {0x057BE0, 0x4000, 0x2000, COMP_TYPE_TILES_BG, 0x0A, 0x8000, "Tiles UI"},
    {0x05FBE0, 0x8000, 0x4000, COMP_TYPE_TILES_BG, 0x0B, 0x8000, "Tiles BG Overworld"},
    {0x067BE0, 0x4000, 0x2000, COMP_TYPE_TILES_SPRITE, 0x0C, 0x8000, "Tiles Link"},
    {0x06FBE0, 0x4000, 0x2000, COMP_TYPE_TILES_SPRITE, 0x0D, 0x8000, "Tiles Inimigos 1"},
    {0x077BE0, 0x4000, 0x2000, COMP_TYPE_TILES_SPRITE, 0x0D, 0xC000, "Tiles Inimigos 2"},
    {0x07FBE0, 0x4000, 0x2000, COMP_TYPE_TILES_BG, 0x0E, 0x8000, "Tiles Dungeon 1"},
    {0x087BE0, 0x4000, 0x2000, COMP_TYPE_TILES_BG, 0x0E, 0xC000, "Tiles Dungeon 2"},
    {0x08FBE0, 0x4000, 0x2000, COMP_TYPE_TILES_BG, 0x0F, 0x8000, "Tiles Dungeon 3"},
    {0x097BE0, 0x4000, 0x2000, COMP_TYPE_TILES_BG, 0x0F, 0xC000, "Tiles Dungeon 4"},
    {0x09FBE0, 0x2000, 0x1000, COMP_TYPE_TILES_SPRITE, 0x10, 0x8000, "Tiles Items"},
    // Blocos 10-63: Placeholder
};

// Blocos de paletas comprimidas
const CompressedDataBlock LZ_PALETTE_BLOCKS[LZ_PALETTE_BLOCK_COUNT] = {
    {0x01900, 0x0200, 0x0200, COMP_TYPE_PALETTE, 0x00, 0x1900, "Paletas BG/Sprites"},
    {0x01B00, 0x0200, 0x0200, COMP_TYPE_PALETTE, 0x00, 0x1B00, "Paletas UI/Title"},
    {0x01D00, 0x0400, 0x0400, COMP_TYPE_PALETTE, 0x00, 0x1D00, "Paletas Link"},
    {0x02100, 0x0200, 0x0200, COMP_TYPE_PALETTE, 0x00, 0x2100, "Paletas Inimigos"},
    {0x02300, 0x0200, 0x0200, COMP_TYPE_PALETTE, 0x00, 0x2300, "Paletas Effects"},
    {0x02500, 0x1000, 0x1000, COMP_TYPE_PALETTE, 0x00, 0x2500, "Paletas Dungeons"},
    {0x03500, 0x0400, 0x0400, COMP_TYPE_PALETTE, 0x00, 0x3500, "Paletas Overworld"},
    // Blocos 7-31: Placeholder
};

// Blocos de mapa comprimidos
const CompressedDataBlock LZ_MAP_BLOCKS[LZ_MAP_BLOCK_COUNT] = {
    // Blocos de tiles 16x16 do overworld
    {0x0A0000, 0x8000, 0x4000, COMP_TYPE_MAP, 0x10, 0x8000, "Map16 Overworld Light"},
    {0x0A8000, 0x8000, 0x4000, COMP_TYPE_MAP, 0x11, 0x8000, "Map16 Overworld Dark"},
    // Blocos 2-31: Placeholder
};

// Blocos de dados de masmorra comprimidos
const CompressedDataBlock LZ_DUNGEON_BLOCKS[LZ_DUNGEON_BLOCK_COUNT] = {
    // Castelo da Agonia
    {0x0B0000, 0x4000, 0x2000, COMP_TYPE_DUNGEON, 0x12, 0x8000, "Dungeon 0 Layouts"},
    {0x0B4000, 0x2000, 0x1000, COMP_TYPE_DUNGEON, 0x12, 0xC000, "Dungeon 0 Enemies"},
    // Caverna do Trovao
    {0x0B6000, 0x4000, 0x2000, COMP_TYPE_DUNGEON, 0x13, 0x8000, "Dungeon 1 Layouts"},
    {0x0BA000, 0x2000, 0x1000, COMP_TYPE_DUNGEON, 0x13, 0xC000, "Dungeon 1 Enemies"},
    // Mina de Ouro
    {0x0BC000, 0x4000, 0x2000, COMP_TYPE_DUNGEON, 0x14, 0x8000, "Dungeon 2 Layouts"},
    {0x0C0000, 0x2000, 0x1000, COMP_TYPE_DUNGEON, 0x14, 0xC000, "Dungeon 2 Enemies"},
    // Blocos 6-127: Placeholder
};

// Blocos de audio comprimidos
const CompressedDataBlock LZ_AUDIO_BLOCKS[LZ_AUDIO_BLOCK_COUNT] = {
    // Samples BRR
    {0x0D0000, 0x8000, 0x8000, COMP_TYPE_SAMPLES, 0x18, 0x8000, "BRR Samples Bank 1"},
    {0x0D8000, 0x8000, 0x8000, COMP_TYPE_SAMPLES, 0x19, 0x8000, "BRR Samples Bank 2"},
    // Sequencias musicais
    {0x0E0000, 0x4000, 0x4000, COMP_TYPE_MUSIC_SEQ, 0x1A, 0x8000, "Music Sequences"},
    // Sound effects
    {0x0E4000, 0x4000, 0x4000, COMP_TYPE_AUDIO, 0x1A, 0xC000, "Sound Effects"},
    // Blocos 4-63: Placeholder
};

// =============================================================================
// Tabela de descompressao para uso em runtime
// =============================================================================
const DecompressionEntry DECOMPRESSION_TABLE[DECOMPRESSION_TABLE_SIZE] = {
    // Entradas de tiles
    {0x057BE0, 0x7E2000, 0x4000, 1, {0, 0, 0}},  // Tiles UI -> WRAM
    {0x05FBE0, 0x7E6000, 0x8000, 1, {0, 0, 0}},  // Tiles BG -> WRAM
    {0x067BE0, 0x7EA000, 0x4000, 1, {0, 0, 0}},  // Tiles Link -> WRAM
    // Entradas restantes: Placeholder
};

// =============================================================================
// Informacoes dos banks de dados
// =============================================================================
const DataBankInfo DATA_BANKS[DATA_BANK_COUNT] = {
    // $0A: Tiles UI e fontes
    {
        .bank = 0x0A,
        .rom_start = 0x050000,
        .rom_end = 0x057FFF,
        .primary_type = COMP_TYPE_TILES_BG,
        .description = "Tiles UI/Fontes",
    },
    // $0B: Tiles BG overworld
    {
        .bank = 0x0B,
        .rom_start = 0x058000,
        .rom_end = 0x05FFFF,
        .primary_type = COMP_TYPE_TILES_BG,
        .description = "Tiles BG Overworld",
    },
    // $0C: Tiles sprites Link
    {
        .bank = 0x0C,
        .rom_start = 0x060000,
        .rom_end = 0x067FFF,
        .primary_type = COMP_TYPE_TILES_SPRITE,
        .description = "Tiles Sprites Link",
    },
    // $0D: Tiles inimigos
    {
        .bank = 0x0D,
        .rom_start = 0x068000,
        .rom_end = 0x06FFFF,
        .primary_type = COMP_TYPE_TILES_SPRITE,
        .description = "Tiles Inimigos",
    },
    // $0E: Tiles masmorras 1-2
    {
        .bank = 0x0E,
        .rom_start = 0x070000,
        .rom_end = 0x077FFF,
        .primary_type = COMP_TYPE_TILES_BG,
        .description = "Tiles Dungeons 1-2",
    },
    // $0F: Tiles masmorras 3-4
    {
        .bank = 0x0F,
        .rom_start = 0x078000,
        .rom_end = 0x07FFFF,
        .primary_type = COMP_TYPE_TILES_BG,
        .description = "Tiles Dungeons 3-4",
    },
    // $10: Tiles itens e objetos
    {
        .bank = 0x10,
        .rom_start = 0x080000,
        .rom_end = 0x087FFF,
        .primary_type = COMP_TYPE_TILES_SPRITE,
        .description = "Tiles Itens/Objetos",
    },
    // $11: Map16 overworld
    {
        .bank = 0x11,
        .rom_start = 0x088000,
        .rom_end = 0x08FFFF,
        .primary_type = COMP_TYPE_MAP,
        .description = "Map16 Overworld Light",
    },
    // $12: Map16 overworld dark
    {
        .bank = 0x12,
        .rom_start = 0x090000,
        .rom_end = 0x097FFF,
        .primary_type = COMP_TYPE_MAP,
        .description = "Map16 Overworld Dark",
    },
    // $13: Dados masmorras 1
    {
        .bank = 0x13,
        .rom_start = 0x098000,
        .rom_end = 0x09FFFF,
        .primary_type = COMP_TYPE_DUNGEON,
        .description = "Dungeon Data 1",
    },
    // $14: Dados masmorras 2
    {
        .bank = 0x14,
        .rom_start = 0x0A0000,
        .rom_end = 0x0A7FFF,
        .primary_type = COMP_TYPE_DUNGEON,
        .description = "Dungeon Data 2",
    },
    // $15: Dados masmorras 3
    {
        .bank = 0x15,
        .rom_start = 0x0A8000,
        .rom_end = 0x0AFFFF,
        .primary_type = COMP_TYPE_DUNGEON,
        .description = "Dungeon Data 3",
    },
    // $16: Dados masmorras 4
    {
        .bank = 0x16,
        .rom_start = 0x0B0000,
        .rom_end = 0x0B7FFF,
        .primary_type = COMP_TYPE_DUNGEON,
        .description = "Dungeon Data 4",
    },
    // $17: Dados masmorras 5
    {
        .bank = 0x17,
        .rom_start = 0x0B8000,
        .rom_end = 0x0BFFFF,
        .primary_type = COMP_TYPE_DUNGEON,
        .description = "Dungeon Data 5",
    },
    // $18: Samples BRR 1
    {
        .bank = 0x18,
        .rom_start = 0x0C0000,
        .rom_end = 0x0C7FFF,
        .primary_type = COMP_TYPE_SAMPLES,
        .description = "BRR Samples Bank 1",
    },
    // $19: Samples BRR 2
    {
        .bank = 0x19,
        .rom_start = 0x0C8000,
        .rom_end = 0x0CFFFF,
        .primary_type = COMP_TYPE_SAMPLES,
        .description = "BRR Samples Bank 2",
    },
    // $1A: Music sequences
    {
        .bank = 0x1A,
        .rom_start = 0x0D0000,
        .rom_end = 0x0D7FFF,
        .primary_type = COMP_TYPE_MUSIC_SEQ,
        .description = "Music Sequences",
    },
    // $1B: Sound effects
    {
        .bank = 0x1B,
        .rom_start = 0x0D8000,
        .rom_end = 0x0DFFFF,
        .primary_type = COMP_TYPE_AUDIO,
        .description = "Sound Effects",
    },
    // $1C: Text data
    {
        .bank = 0x1C,
        .rom_start = 0x0E0000,
        .rom_end = 0x0E7FFF,
        .primary_type = COMP_TYPE_TEXT,
        .description = "Text Data",
    },
    // $1D: AI data
    {
        .bank = 0x1D,
        .rom_start = 0x0E8000,
        .rom_end = 0x0EFFFF,
        .primary_type = COMP_TYPE_AI,
        .description = "AI/Enemy Data",
    },
    // $1E: Misc data 1
    {
        .bank = 0x1E,
        .rom_start = 0x0F0000,
        .rom_end = 0x0F7FFF,
        .primary_type = COMP_TYPE_OTHER,
        .description = "Misc Data 1",
    },
    // $1F: Misc data 2
    {
        .bank = 0x1F,
        .rom_start = 0x0F8000,
        .rom_end = 0x0FFFFF,
        .primary_type = COMP_TYPE_OTHER,
        .description = "Misc Data 2",
    },
};
