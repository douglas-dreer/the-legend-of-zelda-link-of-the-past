#include "maps.h"
#include "../types.h"

// =============================================================================
// Dados de mapas - Zelda: A Link to the Past
// Banks $0A-$1F
// =============================================================================

// =============================================================================
// Grid do mundo overworld (16x8 telas)
// Cada valor indica o tipo/variacao da tela
// =============================================================================
const uint8 OVERWORLD_MAP_GRID[OVERWORLD_WIDTH][OVERWORLD_HEIGHT] = {
    // Coluna 0-7, Linha 0
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07},
    // Linha 1
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
    // Linha 2
    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27},
    // Linha 3
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37},
    // Linha 4
    {0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47},
    // Linha 5
    {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57},
    // Linha 6
    {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67},
    // Linha 7
    {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77},
    // Linha 8
    {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87},
    // Linha 9
    {0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97},
    // Linha 10
    {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7},
    // Linha 11
    {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7},
    // Linha 12
    {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7},
    // Linha 13
    {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7},
    // Linha 14
    {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7},
    // Linha 15
    {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7},
};

// =============================================================================
// Headers de telas do overworld
// Dados reais devem ser extraidos da ROM
// =============================================================================
const OverworldScreenHeader OVERWORLD_SCREEN_HEADERS[OVERWORLD_WIDTH * OVERWORLD_HEIGHT] = {
    // Tela (0,0) - Link's House
    {
        .tileset_index = 0x0000,
        .palette_index = 0x0000,
        .map16_offset = 0x0000,
        .effect_flags = 0x00,
        .music_id = 0x02,       // Overworld theme
        .bg_type = 0x00,
        .exit_count = 2,
        .exit_table_offset = 0x0000,
        .sprite_set = 0x00,
        .property_flags = 0x00,
    },
    // As demais 127 telas serao preenchidas com dados da ROM
    // Placeholder para telas restantes
};

// Tabela de saidas do overworld
const OverworldExit OVERWORLD_EXITS[OVERWORLD_EXIT_COUNT] = {
    // Saida 0: Link's House -> Saida leste
    {
        .direction = 3,      // Direita
        .target_screen = 1,  // Tela (1,0)
        .target_x = 0x0000,
        .target_y = 0x0100,
        .required_item = 0,
        .required_quest = 0,
    },
    // Dados restantes devem ser extraidos da ROM
};

// =============================================================================
// Dados de masmorras
// =============================================================================

// Headers de salas de masmorra
// 13 masmorras x 8 andares x 64 salas = 6656 possiveis salas
const DungeonRoomHeader DUNGEON_ROOMS[DUNGEON_ROOM_COUNT] = {
    // Masmorra 0: Castelo da Agonia, Andar 0, Sala 0
    {
        .layout_offset = 0x0000,
        .collision_offset = 0x0000,
        .tileset_index = 0,
        .palette_index = 11,
        .room_type = 2,        // Sala inicial
        .music_id = 0x03,
        .enemy_set_index = 0,
        .item_index = 0,
        .entrance_count = 1,
        .exit_count = 2,
        .puzzle_flags = 0,
        .warp_flags = 0,
        .entrance_table_offset = 0x0000,
        .exit_table_offset = 0x0000,
        .object_table_offset = 0x0000,
        .layer_offset = 0,
        .animation_flags = 0,
    },
    // Dados restantes devem ser extraidos da ROM
};

// Layouts das salas
const RoomLayout DUNGEON_ROOM_LAYOUTS[DUNGEON_LAYOUT_COUNT] = {
    // Layout vazio (sala inicial)
    {
        {0},  // tiles
        {0},  // collision
    },
};

// Entradas das salas
const DungeonEntrance DUNGEON_ENTRANCES[DUNGEON_ENTRANCE_COUNT] = {
    {
        .direction = 0,     // Cima
        .target_room = 0,
        .target_floor = 0,
        .tile_x = 7,
        .tile_y = 0,
        .lock_type = 0,
        .key_required = 0,
    },
};

// Objetos das salas
const DungeonObject DUNGEON_OBJECTS[DUNGEON_OBJECT_COUNT] = {
    {
        .object_type = OBJ_NONE,
        .id = 0,
        .tile_x = 0,
        .tile_y = 0,
        .state_flags = 0,
        .interaction_flags = 0,
        .data1 = 0,
        .data2 = 0,
    },
};
