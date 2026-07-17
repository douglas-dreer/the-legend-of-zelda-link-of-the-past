#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "../types.h"

// =============================================================================
// Tabelas de compressao LZ - Zelda: A Link to the Past
// Banks $0A-$1F contem dados comprimidos via formato LZ proprietario
// =============================================================================

// =============================================================================
// Formato LZ do Zelda
// =============================================================================

// Opcodes do formato LZ proprietario
enum LZOpcode : uint8 {
    LZ_END = 0x00,              // Fim dos dados
    LZ_LITERAL = 0x10,          // Copia bytes literais
    LZ_BYTE_FILL = 0x11,        // Preenche com byte repetido
    LZ_WORD_FILL = 0x12,        // Preenche com word (16-bit)
    LZ_INC_FILL = 0x13,         // Preenche com incremento
    LZ_BACK_REF_LONG = 0x14,    // Referencia para traz (offset 16-bit)
    LZ_BACK_REF_SHORT = 0x15,   // Referencia para traz (offset 8-bit)
    LZ_VRAM_COPY = 0x16,        // Copia para VRAM
    LZ_SPECIAL = 0x17,          // Operacao especial
    LZ_END_MARKER = 0x18,       // Fim dos dados (marcador)
};

// =============================================================================
// Tipos de dados comprimidos
// =============================================================================

enum CompressedDataType : uint8 {
    COMP_TYPE_TILES_BG = 0,      // Tiles de fundo
    COMP_TYPE_TILES_SPRITE = 1,  // Tiles de sprites
    COMP_TYPE_PALETTE = 2,       // Paleta
    COMP_TYPE_MAP = 3,           // Mapa/tela
    COMP_TYPE_DUNGEON = 4,       // Dados de masmorra
    COMP_TYPE_AUDIO = 5,         // Dados de audio
    COMP_TYPE_MUSIC_SEQ = 6,     // Sequencia musical
    COMP_TYPE_SAMPLES = 7,       // Samples BRR
    COMP_TYPE_TEXT = 8,          // Texto
    COMP_TYPE_AI = 9,            // Dados de IA
    COMP_TYPE_OTHER = 10,        // Outros
};

// =============================================================================
// Estrutura para bloco de dados comprimidos
// =============================================================================

struct CompressedDataBlock {
    uint32 rom_offset;          // Offset na ROM (LoROM)
    uint16 uncompressed_size;   // Tamanho apos descompressao
    uint16 compressed_size;     // Tamanho comprimido (estimativa)
    uint8  type;                // Tipo de dados
    uint8  target_bank;         // Banco de destino
    uint16 target_addr;         // Endereco de destino
    const char* description;    // Descricao dos dados
};

// =============================================================================
// Tabela principal de dados comprimidos
// =============================================================================

// Numero total de blocos comprimidos conhecidos
constexpr int COMPRESSED_DATA_TABLE_SIZE = 256;

// Tabela de blocos comprimidos
extern const CompressedDataBlock COMPRESSED_DATA_TABLE[COMPRESSED_DATA_TABLE_SIZE];

// =============================================================================
// Dados de tiles comprimidos
// =============================================================================

constexpr int LZ_TILE_BLOCK_COUNT = 64;
extern const CompressedDataBlock LZ_TILE_BLOCKS[LZ_TILE_BLOCK_COUNT];

// =============================================================================
// Dados de paletas comprimidas
// =============================================================================

constexpr int LZ_PALETTE_BLOCK_COUNT = 32;
extern const CompressedDataBlock LZ_PALETTE_BLOCKS[LZ_PALETTE_BLOCK_COUNT];

// =============================================================================
// Dados de mapa comprimidos
// =============================================================================

constexpr int LZ_MAP_BLOCK_COUNT = 32;
extern const CompressedDataBlock LZ_MAP_BLOCKS[LZ_MAP_BLOCK_COUNT];

// =============================================================================
// Dados de masmorra comprimidos
// =============================================================================

constexpr int LZ_DUNGEON_BLOCK_COUNT = 128;
extern const CompressedDataBlock LZ_DUNGEON_BLOCKS[LZ_DUNGEON_BLOCK_COUNT];

// =============================================================================
// Dados de audio comprimidos
// =============================================================================

constexpr int LZ_AUDIO_BLOCK_COUNT = 64;
extern const CompressedDataBlock LZ_AUDIO_BLOCKS[LZ_AUDIO_BLOCK_COUNT];

// =============================================================================
// Tabela de descompressao para uso em runtime
// =============================================================================

// Entrada na tabela de descompressao (usada pelo decompressor em runtime)
struct DecompressionEntry {
    uint32 src_offset;          // Offset fonte na ROM
    uint32 dst_address;         // Endereco destino
    uint16 size;                // Tamanho descomprimido
    uint8  decompress;          // 1=descomprimir, 0=copiar direto
    uint8  padding[3];          // Padding para alinhamento
};

// Tabela de descompressao completa
constexpr int DECOMPRESSION_TABLE_SIZE = 512;
extern const DecompressionEntry DECOMPRESSION_TABLE[DECOMPRESSION_TABLE_SIZE];

// =============================================================================
// Offsets conocidos na ROM para dados comprimidos
// Banks $0A-$1F (LoROM offsets: $050000 - $0FFFFF)
// =============================================================================

// Banks de dados (nao codigo)
// $0A-$0F: Tiles e paletas comprimidos
// $10-$14: Dados de overworld
// $15-$1C: Dados de masmorras
// $1D-$1F: Audio e dados misc

// Estrutura com informacoes de um bank de dados
struct DataBankInfo {
    uint8  bank;                // Numero do bank ($0A-$1F)
    uint32 rom_start;           // Offset inicial na ROM
    uint32 rom_end;             // Offset final na ROM
    uint8  primary_type;        // Tipo principal de dados
    const char* description;    // Descricao do bank
};

// Tabela de informacoes dos banks de dados
constexpr int DATA_BANK_COUNT = 22;  // $0A ate $1F
extern const DataBankInfo DATA_BANKS[DATA_BANK_COUNT];

// =============================================================================
// Funcoes auxiliares
// =============================================================================

// Converte endereço SNES para offset LoROM
inline uint32 snes_to_lorom(uint8 bank, uint16 addr) {
    if (addr < 0x8000) {
        return 0xFFFFFFFF;  // Nao esta na ROM
    }
    return static_cast<uint32>(bank) * 0x8000 + (addr - 0x8000);
}

// Converte offset LoROM para endereço SNES
inline void lorom_to_snes(uint32 offset, uint8* bank, uint16* addr) {
    *bank = offset / 0x8000;
    *addr = (offset % 0x8000) + 0x8000;
}

// Busca um bloco comprimido por offset
inline const CompressedDataBlock* find_compressed_block(uint32 rom_offset) {
    for (int i = 0; i < COMPRESSED_DATA_TABLE_SIZE; i++) {
        if (COMPRESSED_DATA_TABLE[i].rom_offset == rom_offset) {
            return &COMPRESSED_DATA_TABLE[i];
        }
    }
    return nullptr;
}

// Busca blocos por tipo
inline int find_compressed_blocks_by_type(CompressedDataType type,
                                          const CompressedDataBlock** results,
                                          int max_results) {
    int count = 0;
    for (int i = 0; i < COMPRESSED_DATA_TABLE_SIZE && count < max_results; i++) {
        if (COMPRESSED_DATA_TABLE[i].type == type) {
            results[count++] = &COMPRESSED_DATA_TABLE[i];
        }
    }
    return count;
}

// Verifica se um offset esta dentro de um bank de dados
inline bool is_data_bank(uint32 rom_offset) {
    uint8 bank = rom_offset / 0x8000;
    return (bank >= 0x0A && bank <= 0x1F);
}

// Obtem informacoes de um bank
inline const DataBankInfo* get_data_bank_info(uint8 bank) {
    if (bank >= 0x0A && bank <= 0x1F) {
        return &DATA_BANKS[bank - 0x0A];
    }
    return nullptr;
}

#endif // COMPRESSION_H
