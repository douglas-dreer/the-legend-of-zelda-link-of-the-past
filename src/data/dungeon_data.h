#ifndef DUNGEON_DATA_H
#define DUNGEON_DATA_H

#include "../types.h"

// =============================================================================
// Dados de masmorras - Zelda: A Link to the Past
// Banks $0A-$1F contem dados detalhados de todas as 13 masmorras
// =============================================================================

// Numero total de masmorras
constexpr int DUNGEON_COUNT = 13;

// Dimensoes de sala
constexpr int ROOM_WIDTH_TILES = 16;
constexpr int ROOM_HEIGHT_TILES = 11;

// Numero maximo de andares por masmorra
constexpr int MAX_FLOORS = 8;

// Numero maximo de salas por andar
constexpr int MAX_ROOMS_PER_FLOOR = 64;

// =============================================================================
// Informacoes de cada masmorra
// =============================================================================

// Estrutura com informacoes gerais de uma masmorra
struct DungeonInfo {
    const char* name;                // Nome da masmorra
    uint8  floor_count;              // Numero de andares
    uint8  boss_room_index;          // Index da sala do boss
    uint8  start_room_floor;         // Andar inicial
    uint8  start_room_index;         // Sala inicial
    uint8  music_id;                 // ID da musica
    uint8  tileset_type;             // Tipo de tileset
    uint8  palette_group;            // Grupo de paletas
    uint8  entrance_type;            // Tipo de entrada (0=porta, 1=escada, 2=warp)
};

// Tabela de informacoes de todas as masmorras
extern const DungeonInfo DUNGEON_INFOS[DUNGEON_COUNT];

// =============================================================================
// Layouts de tiles de masmorra
// =============================================================================

// Cada masmorra tem seus proprios tiles para parede/chao
// Tiles organizados por tipo de bloco
struct DungeonTileset {
    const uint8* wall_tiles;         // Tiles de parede
    const uint8* floor_tiles;        // Tiles de chao
    const uint8* door_tiles;         // Tiles de portas
    const uint8* switch_tiles;       // Tiles de interruptores
    const uint8* decoration_tiles;   // Tiles de decoracao
    int wall_tile_count;
    int floor_tile_count;
    int door_tile_count;
    int switch_tile_count;
    int decoration_tile_count;
};

extern const DungeonTileset DUNGEON_TILESETS[DUNGEON_COUNT];

// =============================================================================
// Portas e conexoes entre salas
// =============================================================================

// Dados de uma porta
struct DoorData {
    uint8  direction;                // Direcao: 0=cima, 1=baixo, 2=esquerda, 3=direita
    uint16 tile_x;                   // Posicao X em tiles
    uint16 tile_y;                   // Posicao Y em tiles
    uint8  door_type;                // Tipo: 0=simples, 1=tranca, 2=boss, 3=secret
    uint8  key_required;             // Numero de chaves necessarias (0=nenhuma)
    uint8  item_required;            // Item necessario (0=nenhum, 0xFF=boss key)
    uint8  state_flag;               // Flag de estado (aberta/fechada)
};

// Tabela de portas por sala
// Index: [andar][sala][porta]
constexpr int MAX_DOORS_PER_ROOM = 4;
extern const DoorData DUNGEON_DOORS[][MAX_ROOMS_PER_FLOOR][MAX_DOORS_PER_ROOM];

// =============================================================================
// Posicoes de inimigos
// =============================================================================

// Posicao e tipo de um inimigo em uma sala
struct EnemySpawn {
    uint8  enemy_id;                 // ID do inimigo
    uint8  variant;                  // Variante
    uint16 spawn_x;                  // Posicao X de spawn (pixels)
    uint16 spawn_y;                  // Posicao Y de spawn (pixels)
    uint8  spawn_flag;               // Flag de spawn (0=sempre, 1=primeira vez, etc)
    uint8  behavior_flags;           // Flags de comportamento
    uint8  health;                   // Pontos de vida
    uint8  damage;                   // Dano causado
};

// Tabela de spawns de inimigos por sala
constexpr int MAX_ENEMIES_PER_ROOM = 10;
extern const EnemySpawn DUNGEON_ENEMY_SPAWNS[][MAX_ROOMS_PER_FLOOR][MAX_ENEMIES_PER_ROOM];

// =============================================================================
// Posicoes de itens e baus
// =============================================================================

// Conteudo de um bau
struct ChestData {
    uint16 tile_x;                   // Posicao X em tiles
    uint16 tile_y;                   // Posicao Y em tiles
    uint8  item_id;                  // ID do item
    uint8  item_type;                // Tipo: 0=consumivel, 1=chave, 2=equipamento
    uint8  chest_flag;               // Flag de bau aberto
    uint8  room_flag;                // Flag da sala associada
};

// Itens soltos em uma sala (nao em baus)
struct RoomItem {
    uint16 tile_x;
    uint16 tile_y;
    uint8  item_id;
    uint8  item_type;
    uint8  respawn_flag;             // Flag de respawn
};

// Tabela de baus por sala
constexpr int MAX_CHESTS_PER_ROOM = 3;
extern const ChestData DUNGEON_CHESTS[][MAX_ROOMS_PER_FLOOR][MAX_CHESTS_PER_ROOM];

// Tabela de itens por sala
constexpr int MAX_ITEMS_PER_ROOM = 5;
extern const RoomItem DUNGEON_ITEMS[][MAX_ROOMS_PER_FLOOR][MAX_ITEMS_PER_ROOM];

// =============================================================================
// Switches e puzzles
// =============================================================================

// Tipos de puzzles
enum PuzzleType : uint8 {
    PUZZLE_NONE = 0,
    PUZZLE_FLOOR_SWITCH = 1,        // Switch no chao
    PUZZLE_WALL_SWITCH = 2,         // Switch na parede
    PUZZLE_BLOCK_PUSH = 3,          // Empurrar bloco
    PUZZLE_TORCH = 4,               // Acender tochas
    PUZZLE_CRYSTAL = 5,             // Mover cristal
    PUZZLE_CHEST_PATTERN = 6,       // Abrir baus em ordem
    PUZZLE_POT_SMASH = 7,           // Quebrar vasos
    PUZZLE_STALFOS = 8,             // Derrotar Stalfos
    PUZZLE_MAZE = 9,                // Labirinto
};

// Dados de um puzzle
struct PuzzleData {
    uint8  puzzle_type;              // Tipo do puzzle
    uint8  solution_count;           // Numero de passos
    uint16 trigger_x;                // Posicao X do gatilho
    uint16 trigger_y;                // Posicao Y do gatilho
    uint8  result_type;              // Tipo do resultado
    uint16 result_data;              // Dados do resultado
    uint8  solved_flag;              // Flag de puzzle resolvido
};

// Tabela de puzzles por sala
constexpr int MAX_PUZZLES_PER_ROOM = 3;
extern const PuzzleData DUNGEON_PUZZLES[][MAX_ROOMS_PER_FLOOR][MAX_PUZZLES_PER_ROOM];

// =============================================================================
// Dados de boss
// =============================================================================

// Informacoes de um boss
struct BossData {
    uint8  boss_id;                  // ID do boss
    uint8  health;                   // Pontos de vida
    uint8  phases;                   // Numero de fases
    uint8  weakness_item;            // Item fraqueza
    uint16 spawn_x;                  // Posicao de spawn X
    uint16 spawn_y;                  // Posicao de spawn Y
    uint8  death_flag;               // Flag de morte
    uint8  drop_item;                // Item dropado ao morrer
    uint8  drop_count;               // Quantidade de coracoes
};

// Tabela de bosses
extern const BossData DUNGEON_BOSSES[DUNGEON_COUNT];

// =============================================================================
// Enderecos de dados na ROM
// =============================================================================

// Offsets para dados de masmorra na ROM (LoROM)
// Estes apontam para onde os dados estao comprimidos na ROM
struct DungeonROMOffset {
    uint32 room_data_offset;         // Offset dos dados de salas
    uint32 layout_data_offset;       // Offset dos layouts
    uint32 tileset_data_offset;      // Offset dos tilesets
    uint32 enemy_data_offset;        // Offset dos dados de inimigos
    uint32 item_data_offset;         // Offset dos dados de itens
    uint32 music_data_offset;        // Offset dos dados de musica
    uint16 room_data_size;           // Tamanho dos dados de salas
    uint16 layout_data_size;         // Tamanho dos layouts
};

// Tabela de offsets ROM por masmorra
extern const DungeonROMOffset DUNGEON_ROM_OFFSETS[DUNGEON_COUNT];

// =============================================================================
// Constantes de tipos de tile de masmorra
// =============================================================================

// Tipos de tiles de masmorra
constexpr uint8 DUNGEON_TILE_FLOOR = 0x00;
constexpr uint8 DUNGEON_TILE_WALL = 0x01;
constexpr uint8 DUNGEON_TILE_DOOR = 0x02;
constexpr uint8 DUNGEON_TILE_LOCKED_DOOR = 0x03;
constexpr uint8 DUNGEON_TILE_BOSS_DOOR = 0x04;
constexpr uint8 DUNGEON_TILE_SECRET_DOOR = 0x05;
constexpr uint8 DUNGEON_TILE_STAIRS_UP = 0x06;
constexpr uint8 DUNGEON_TILE_STAIRS_DOWN = 0x07;
constexpr uint8 DUNGEON_TILE_PIT = 0x08;
constexpr uint8 DUNGEON_TILE_SWITCH = 0x09;
constexpr uint8 DUNGEON_TILE_CHEST = 0x0A;
constexpr uint8 DUNGEON_TILE_POT = 0x0B;
constexpr uint8 DUNGEON_TILE_STATUE = 0x0C;
constexpr uint8 DUNGEON_TILE_CRYSTAL = 0x0D;
constexpr uint8 DUNGEON_TILE_BOSS_SPAWN = 0x0E;
constexpr uint8 DUNGEON_TILE_WARP = 0x0F;

// =============================================================================
// Funcoes auxiliares
// =============================================================================

// Obtem informacoes de uma masmorra
inline const DungeonInfo* dungeon_get_info(int dungeon_id) {
    if (dungeon_id >= 0 && dungeon_id < DUNGEON_COUNT) {
        return &DUNGEON_INFOS[dungeon_id];
    }
    return nullptr;
}

// Verifica se uma sala e a sala de boss
inline bool dungeon_is_boss_room(int dungeon_id, int room_index) {
    if (dungeon_id >= 0 && dungeon_id < DUNGEON_COUNT) {
        return DUNGEON_INFOS[dungeon_id].boss_room_index == room_index;
    }
    return false;
}

// Obtem o tileset de uma masmorra
inline const DungeonTileset* dungeon_get_tileset(int dungeon_id) {
    if (dungeon_id >= 0 && dungeon_id < DUNGEON_COUNT) {
        return &DUNGEON_TILESETS[dungeon_id];
    }
    return nullptr;
}

#endif // DUNGEON_DATA_H
