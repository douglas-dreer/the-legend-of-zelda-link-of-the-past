#include "tiles.h"
#include "../types.h"

// =============================================================================
// Tiles 4bpp - Dados extraidos da ROM
// Banks $0A-$1F
// Formato: 32 bytes por tile (8x8 pixels, 4bpp)
// =============================================================================

// Nota: Estes arrays serao preenchidos com dados reais da ROM
// usando o extractor. Os tamanhos sao estimativas baseadas na
// analise da ROM.

// =============================================================================
// Tiles de Interface (HUD)
// =============================================================================
const uint8 TILES_UI[TILES_UI_COUNT * TILE_SIZE_4BPP] = {
    // Dados serao extraidos da ROM offset $0013E0 - $012640
    // 512 tiles x 32 bytes = 16384 bytes
    0x00, // Placeholder - dados reais devem ser carregados da ROM
};

// =============================================================================
// Tiles de Masmorra
// =============================================================================
const uint8 TILES_DUNGEON_GENERIC[TILES_DUNGEON_GENERIC_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Placeholder
};

const uint8 TILES_DUNGEON_1[TILES_DUNGEON_1_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Castelo da Agonia - dados do bloco comprimido 5
};

const uint8 TILES_DUNGEON_2[TILES_DUNGEON_2_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Caverna do Trovao - dados do bloco comprimido 6
};

const uint8 TILES_DUNGEON_3[TILES_DUNGEON_3_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Palacio do Prata
};

const uint8 TILES_DUNGEON_4[TILES_DUNGEON_4_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Mina de Ouro
};

const uint8 TILES_DUNGEON_5[TILES_DUNGEON_5_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Palacio do Gelo
};

const uint8 TILES_DUNGEON_6[TILES_DUNGEON_6_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Torre de Hera
};

const uint8 TILES_DUNGEON_7[TILES_DUNGEON_7_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Palacio dos Escuridao
};

const uint8 TILES_DUNGEON_8[TILES_DUNGEON_8_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Palacio do Forte
};

// =============================================================================
// Tiles de Mundo Aberto
// =============================================================================
const uint8 TILES_OVERWORLD_BASE[TILES_OVERWORLD_BASE_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles de terreno base
};

const uint8 TILES_OVERWORLD_LIGHT[TILES_OVERWORLD_LIGHT_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Hyperlink (Light World)
};

const uint8 TILES_OVERWORLD_DARK[TILES_OVERWORLD_DARK_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Dark World
};

const uint8 TILES_WATER[TILES_WATER_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles de agua e animacoes
};

// =============================================================================
// Tiles de Sprites
// =============================================================================
const uint8 TILES_LINK[TILES_LINK_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles do Link (todas as animacoes)
};

const uint8 TILES_ENEMIES_1[TILES_ENEMIES_1_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles de inimigos tipo 1
};

const uint8 TILES_ENEMIES_2[TILES_ENEMIES_2_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles de inimigos tipo 2
};

const uint8 TILES_ITEMS[TILES_ITEMS_COUNT * TILE_SIZE_4BPP] = {
    0x00, // Tiles de itens e objetos
};
