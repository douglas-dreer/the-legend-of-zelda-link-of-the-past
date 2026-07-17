#ifndef TILES_H
#define TILES_H

#include "../types.h"

// =============================================================================
// Tiles 4bpp (32 bytes cada) - Banks $0A-$1F
// Formato SNES: 8x8 pixels, 4 bits por pixel
// Planos interleaved: planos 0-1 primeiros 16 bytes, planos 2-3 últimos 16 bytes
// =============================================================================

// Tamanho de um tile 4bpp em bytes
constexpr int TILE_SIZE_4BPP = 32;

// Tamanho de um tile em pixels
constexpr int TILE_PIXEL_SIZE = 8;

// =============================================================================
// Tiles de Interface (HUD)
// Fontes, numeros, barras de vida, icones de itens
// =============================================================================

// Tiles da fonte e HUD (512 tiles = 16384 bytes)
// Localizados na ROM: offset $0013E0 - $012640
extern const uint8 TILES_UI[];
constexpr int TILES_UI_COUNT = 512;

// =============================================================================
// Tiles de Masmorra
// Parede, chao, portas, armadilhas, interruptores
// =============================================================================

// Tiles de masmorra generica (256 tiles = 8192 bytes)
extern const uint8 TILES_DUNGEON_GENERIC[];
constexpr int TILES_DUNGEON_GENERIC_COUNT = 256;

// Tiles de masmorra 1 - Castelo da Agonia
extern const uint8 TILES_DUNGEON_1[];
constexpr int TILES_DUNGEON_1_COUNT = 256;

// Tiles de masmorra 2 - Caverna do Trovao
extern const uint8 TILES_DUNGEON_2[];
constexpr int TILES_DUNGEON_2_COUNT = 256;

// Tiles de masmorra 3 - Palacio do Prata
extern const uint8 TILES_DUNGEON_3[];
constexpr int TILES_DUNGEON_3_COUNT = 256;

// Tiles de masmorra 4 - Mina de Ouro
extern const uint8 TILES_DUNGEON_4[];
constexpr int TILES_DUNGEON_4_COUNT = 256;

// Tiles de masmorra 5 - Palacio do Gelo
extern const uint8 TILES_DUNGEON_5[];
constexpr int TILES_DUNGEON_5_COUNT = 256;

// Tiles de masmorra 6 - Torre de Hera
extern const uint8 TILES_DUNGEON_6[];
constexpr int TILES_DUNGEON_6_COUNT = 256;

// Tiles de masmorra 7 - Palacio dos Escuridao
extern const uint8 TILES_DUNGEON_7[];
constexpr int TILES_DUNGEON_7_COUNT = 256;

// Tiles de masmorra 8 - Palacio do Forte
extern const uint8 TILES_DUNGEON_8[];
constexpr int TILES_DUNGEON_8_COUNT = 256;

// =============================================================================
// Tiles de Mundo Aberto (Overworld)
// Terreno, agua, arvores, construcoes
// =============================================================================

// Tiles de terreno base
extern const uint8 TILES_OVERWORLD_BASE[];
constexpr int TILES_OVERWORLD_BASE_COUNT = 512;

// Tiles de Hyperlink (Light World)
extern const uint8 TILES_OVERWORLD_LIGHT[];
constexpr int TILES_OVERWORLD_LIGHT_COUNT = 256;

// Tiles de Dark World
extern const uint8 TILES_OVERWORLD_DARK[];
constexpr int TILES_OVERWORLD_DARK_COUNT = 256;

// Tiles de agua e animacoes
extern const uint8 TILES_WATER[];
constexpr int TILES_WATER_COUNT = 128;

// =============================================================================
// Tiles de Sprites
// Inimigos, NPCs, efeitos
// =============================================================================

// Tiles de Link (todas as animacoes)
extern const uint8 TILES_LINK[];
constexpr int TILES_LINK_COUNT = 1024;

// Tiles de inimigos (blocos comprimidos)
extern const uint8 TILES_ENEMIES_1[];
constexpr int TILES_ENEMIES_1_COUNT = 512;

extern const uint8 TILES_ENEMIES_2[];
constexpr int TILES_ENEMIES_2_COUNT = 512;

// Tiles de itens e objetos
extern const uint8 TILES_ITEMS[];
constexpr int TILES_ITEMS_COUNT = 256;

// =============================================================================
// Blocos de tiles comprimidos via LZ
// Offsets na ROM para dados comprimidos
// =============================================================================

// Estrutura para mapear tiles comprimidos
struct CompressedTileBlock {
    uint32 rom_offset;          // Offset na ROM (LoROM)
    uint16 uncompressed_size;   // Tamanho apos descompressao
    uint16 tile_count;          // Numero de tiles no bloco
    uint8  palette_index;       // Indice da paleta a usar
    uint8  type;                // Tipo: 0=BG, 1=Sprite, 2=UI
};

// Tabela de blocos comprimidos (10 blocos encontrados pelo extractor)
constexpr int COMPRESSED_TILE_BLOCK_COUNT = 10;
extern const CompressedTileBlock COMPRESSED_TILE_BLOCKS[COMPRESSED_TILE_BLOCK_COUNT];

// =============================================================================
// Funcoes auxiliares para tiles
// =============================================================================

// Decodifica um tile 4bpp para indices de pixel
// pixels deve ter espaco para 64 bytes (8x8)
inline void decode_tile_4bpp(const uint8* tile_data, uint8* pixels) {
    for (int y = 0; y < 8; y++) {
        uint8 plane0 = tile_data[y * 2];
        uint8 plane1 = tile_data[y * 2 + 1];
        uint8 plane2 = tile_data[16 + y * 2];
        uint8 plane3 = tile_data[16 + y * 2 + 1];

        for (int x = 0; x < 8; x++) {
            int bit = 7 - x;
            pixels[y * 8 + x] = ((plane0 >> bit) & 1) |
                                (((plane1 >> bit) & 1) << 1) |
                                (((plane2 >> bit) & 1) << 2) |
                                (((plane3 >> bit) & 1) << 3);
        }
    }
}

// Converte tile 4bpp para indices usando paleta SNES
// output deve ter 64 entries (8x8 pixels)
inline void tile_to_palette_indices(const uint8* tile_data, uint8* output) {
    decode_tile_4bpp(tile_data, output);
}

// Obtem pointer para um tile especifico em um array de tiles
inline const uint8* get_tile_ptr(const uint8* tile_array, int tile_index) {
    return &tile_array[tile_index * TILE_SIZE_4BPP];
}

// Verifica se um tile esta vazio (todos os pixels sao 0)
inline bool is_tile_empty(const uint8* tile_data) {
    for (int i = 0; i < TILE_SIZE_4BPP; i++) {
        if (tile_data[i] != 0) return false;
    }
    return true;
}

#endif // TILES_H
