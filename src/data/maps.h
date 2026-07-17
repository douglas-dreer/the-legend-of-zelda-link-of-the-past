#ifndef MAPS_H
#define MAPS_H

#include "../types.h"

// =============================================================================
// Dados de mapas - Zelda: A Link to the Past
// Banks $0A-$1F contem dados de overworld e dungeon
// =============================================================================

// Dimensoes do mundo
constexpr int OVERWORLD_WIDTH = 16;   // 16 telas de largura
constexpr int OVERWORLD_HEIGHT = 8;   // 8 telas de altura
constexpr int OVERWORLD_SCREEN_WIDTH = 64;   // 64 tiles por tela (largura)
constexpr int OVERWORLD_SCREEN_HEIGHT = 64;  // 64 tiles por tela (altura)
constexpr int TILE_SIZE = 16;                // 16x16 pixels por tile no SNES

// Dimensoes de masmorra
constexpr int DUNGEON_ROOM_WIDTH = 16;    // 16 tiles de largura
constexpr int DUNGEON_ROOM_HEIGHT = 11;   // 11 tiles de altura
constexpr int DUNGEON_FLOORS = 8;         // 8 andares por masmorra
constexpr int DUNGEON_ROOMS_PER_FLOOR = 64; // Max salas por andar

// =============================================================================
// Estrutura de uma tela do overworld
// =============================================================================

// Header de uma tela do overworld (16 bytes)
struct OverworldScreenHeader {
    uint16 tileset_index;        // Indice do tileset
    uint16 palette_index;        // Indice da paleta
    uint16 map16_offset;         // Offset dos tiles 16x16
    uint8  effect_flags;         // Flags de efeito (agua, areia, etc)
    uint8  music_id;             // ID da musica
    uint8  bg_type;              // Tipo de fundo
    uint8  exit_count;           // Numero de saidas
    uint16 exit_table_offset;    // Offset da tabela de saidas
    uint8  sprite_set;           // Conjunto de sprites
    uint8  property_flags;       // Flags de propriedades
};

// Saida de uma tela do overworld
struct OverworldExit {
    uint8  direction;            // 0=cima, 1=baixo, 2=esquerda, 3=direita
    uint8  target_screen;        // Tela destino (indice no grid)
    uint16 target_x;             // Posicao X de entrada na tela destino
    uint16 target_y;             // Posicao Y de entrada na tela destino
    uint8  required_item;        // Item necessario (0=nenhum)
    uint8  required_quest;       // Missao necessaria (0=nenhuma)
};

// =============================================================================
// Estrutura de uma sala de masmorra
// =============================================================================

// Layout de uma sala (16x11 tiles = 176 bytes)
struct RoomLayout {
    uint8  tiles[DUNGEON_ROOM_WIDTH * DUNGEON_ROOM_HEIGHT];
    uint8  collision[DUNGEON_ROOM_WIDTH * DUNGEON_ROOM_HEIGHT];
};

// Header de uma sala de masmorra (32 bytes)
struct DungeonRoomHeader {
    uint16 layout_offset;        // Offset do layout na ROM
    uint16 collision_offset;     // Offset dos dados de colisao
    uint8  tileset_index;        // Indice do tileset
    uint8  palette_index;        // Indice da paleta
    uint8  room_type;            // Tipo: 0=normal, 1=boss, 2=inicial, 3=secret
    uint8  music_id;             // ID da musica
    uint8  enemy_set_index;      // Indice do conjunto de inimigos
    uint8  item_index;           // Indice dos itens/baus
    uint8  entrance_count;       // Numero de entradas
    uint8  exit_count;           // Numero de saidas
    uint8  puzzle_flags;         // Flags de puzzles
    uint8  warp_flags;           // Flags de teleporte
    uint16 entrance_table_offset; // Offset da tabela de entradas
    uint16 exit_table_offset;    // Offset da tabela de saidas
    uint16 object_table_offset;  // Offset dos objetos
    uint8  layer_offset;         // Offset de layers extras
    uint8  animation_flags;      // Flags de animacao
};

// Entrada de uma sala (porta)
struct DungeonEntrance {
    uint8  direction;            // 0=cima, 1=baixo, 2=esquerda, 3=direita
    uint8  target_room;          // Sala destino
    uint8  target_floor;         // Andar destino
    uint16 tile_x;               // Posicao X da entrada
    uint16 tile_y;               // Posicao Y da entrada
    uint8  lock_type;            // Tipo de tranca (0=nenhuma)
    uint8  key_required;         // Chave necessaria
};

// Objeto em uma sala (inimigo, bau, switch, etc)
struct DungeonObject {
    uint8  object_type;          // Tipo do objeto
    uint8  id;                   // ID especifico
    uint16 tile_x;               // Posicao X em tiles
    uint16 tile_y;               // Posicao Y em tiles
    uint8  state_flags;          // Flags de estado
    uint8  interaction_flags;    // Flags de interacao
    uint16 data1;                // Dado adicional 1
    uint16 data2;                // Dado adicional 2
};

// Tipos de objetos de masmorra
enum DungeonObjectType : uint8 {
    OBJ_NONE = 0,
    OBJ_ENEMY = 1,
    OBJ_CHEST = 2,
    OBJ_SWITCH = 3,
    OBJ_DOOR = 4,
    OBJ_LOCKED_DOOR = 5,
    OBJ_BOSS_DOOR = 6,
    OBJ_STAIRS = 7,
    OBJ_WARP = 8,
    OBJ_POT = 9,
    OBJ_STATUE = 10,
    OBJ_CRYSTAL = 11,
    OBJ_HEART_CONTAINER = 12,
    OBJ_ITEM_GET = 13,
    OBJ_NPC = 14,
    OBJ_BOSS = 15,
};

// =============================================================================
// Grid do mundo overworld (16x8 telas)
// =============================================================================

// Mapa do mundo aberto - indices de tela
// Cada valor aponta para os dados de uma tela
extern const uint8 OVERWORLD_MAP_GRID[OVERWORLD_WIDTH][OVERWORLD_HEIGHT];

// Headers de todas as telas do overworld (128 telas)
extern const OverworldScreenHeader OVERWORLD_SCREEN_HEADERS[OVERWORLD_WIDTH * OVERWORLD_HEIGHT];

// Tabela de saidas do overworld
extern const OverworldExit OVERWORLD_EXITS[];
constexpr int OVERWORLD_EXIT_COUNT = 256;

// =============================================================================
// Dados de masmorras
// =============================================================================

// Headers de todas as salas de masmorra
// 8 masmorras x 8 andares x 64 salas = 4096 salas maximo
extern const DungeonRoomHeader DUNGEON_ROOMS[];
constexpr int DUNGEON_ROOM_COUNT = 4096;

// Layouts das salas (descompactados da ROM)
extern const RoomLayout DUNGEON_ROOM_LAYOUTS[];
constexpr int DUNGEON_LAYOUT_COUNT = 2048;

// Entradas das salas
extern const DungeonEntrance DUNGEON_ENTRANCES[];
constexpr int DUNGEON_ENTRANCE_COUNT = 1024;

// Objetos das salas
extern const DungeonObject DUNGEON_OBJECTS[];
constexpr int DUNGEON_OBJECT_COUNT = 4096;

// =============================================================================
// Mapas de colisao
// =============================================================================

// Mapa de colisao global (1 byte por tile)
// Bit 0: solido
// Bit 1: agua
// Bit 2: escalavel
// Bit 4-7: tipo de terreno
constexpr uint8 COLLISION_SOLID = 0x01;
constexpr uint8 COLLISION_WATER = 0x02;
constexpr uint8 COLLISION_CLIMBABLE = 0x04;
constexpr uint8 COLLISION_BUSH = 0x08;
constexpr uint8 COLLISION_GRASS = 0x10;

// =============================================================================
// Funcoes auxiliares
// =============================================================================

// Converte coordenadas do mundo para indice de tela
inline int overworld_get_screen_index(int x, int y) {
    return y * OVERWORLD_WIDTH + x;
}

// Obtem header de uma tela do overworld
inline const OverworldScreenHeader* overworld_get_screen(int x, int y) {
    int idx = overworld_get_screen_index(x, y);
    if (idx < OVERWORLD_WIDTH * OVERWORLD_HEIGHT) {
        return &OVERWORLD_SCREEN_HEADERS[idx];
    }
    return nullptr;
}

// Converte coordenadas de tiles para sala de masmorra
inline int dungeon_get_room_index(int floor, int room) {
    return floor * DUNGEON_ROOMS_PER_FLOOR + room;
}

// Verifica se uma posicao e solida no mapa de colisao
inline bool is_tile_solid(uint8 collision_byte) {
    return (collision_byte & COLLISION_SOLID) != 0;
}

// Verifica se uma posicao e agua
inline bool is_tile_water(uint8 collision_byte) {
    return (collision_byte & COLLISION_WATER) != 0;
}

#endif // MAPS_H
