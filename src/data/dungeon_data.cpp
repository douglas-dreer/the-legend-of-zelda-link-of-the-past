#include "dungeon_data.h"
#include "tiles.h"
#include "../types.h"

// =============================================================================
// Dados de masmorras - Zelda: A Link to the Past
// 13 masmorras no total
// =============================================================================

// =============================================================================
// Informacoes das masmorras
// =============================================================================
const DungeonInfo DUNGEON_INFOS[DUNGEON_COUNT] = {
    // 0: Castelo da Agonia (Eastern Palace)
    {
        .name = "Castelo da Agonia",
        .floor_count = 3,
        .boss_room_index = 12,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x03,
        .tileset_type = 0,
        .palette_group = 11,
        .entrance_type = 0,
    },
    // 1: Caverna do Trovao (Desert Palace)
    {
        .name = "Caverna do Trovao",
        .floor_count = 4,
        .boss_room_index = 20,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x04,
        .tileset_type = 1,
        .palette_group = 12,
        .entrance_type = 0,
    },
    // 2: Mina de Ouro (Tower of Hera)
    {
        .name = "Mina de Ouro",
        .floor_count = 3,
        .boss_room_index = 15,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x05,
        .tileset_type = 2,
        .palette_group = 13,
        .entrance_type = 1,
    },
    // 3: Palacio do Gelo (Ice Palace)
    {
        .name = "Palacio do Gelo",
        .floor_count = 4,
        .boss_room_index = 22,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x06,
        .tileset_type = 3,
        .palette_group = 14,
        .entrance_type = 0,
    },
    // 4: Masmorra do Pity (Mini-dungeon)
    {
        .name = "Masmorra do Pity",
        .floor_count = 2,
        .boss_room_index = 8,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x03,
        .tileset_type = 0,
        .palette_group = 11,
        .entrance_type = 0,
    },
    // 5: Masmorra da Floresta (Mini-dungeon)
    {
        .name = "Masmorra da Floresta",
        .floor_count = 2,
        .boss_room_index = 8,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x03,
        .tileset_type = 0,
        .palette_group = 11,
        .entrance_type = 0,
    },
    // 6: Masmorra da Vila (Mini-dungeon)
    {
        .name = "Masmorra da Vila",
        .floor_count = 2,
        .boss_room_index = 8,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x03,
        .tileset_type = 0,
        .palette_group = 11,
        .entrance_type = 0,
    },
    // 7: Palacio do Prata (Skull Woods)
    {
        .name = "Palacio do Prata",
        .floor_count = 4,
        .boss_room_index = 20,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x07,
        .tileset_type = 4,
        .palette_group = 15,
        .entrance_type = 0,
    },
    // 8: Torre de Hera (Hera Tower)
    {
        .name = "Torre de Hera",
        .floor_count = 6,
        .boss_room_index = 30,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x05,
        .tileset_type = 2,
        .palette_group = 16,
        .entrance_type = 1,
    },
    // 9: Palacio dos Escuridao (Dark Palace)
    {
        .name = "Palacio dos Escuridao",
        .floor_count = 4,
        .boss_room_index = 25,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x08,
        .tileset_type = 5,
        .palette_group = 17,
        .entrance_type = 0,
    },
    // 10: Masmorra da Morte (Gargoyle's Domain)
    {
        .name = "Masmorra da Morte",
        .floor_count = 3,
        .boss_room_index = 15,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x03,
        .tileset_type = 0,
        .palette_group = 11,
        .entrance_type = 0,
    },
    // 11: Palacio do Forte (Ganon's Tower)
    {
        .name = "Palacio do Forte",
        .floor_count = 7,
        .boss_room_index = 40,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x09,
        .tileset_type = 6,
        .palette_group = 18,
        .entrance_type = 2,
    },
    // 12: Caverna Escura (Mini-dungeon final)
    {
        .name = "Caverna Escura",
        .floor_count = 2,
        .boss_room_index = 8,
        .start_room_floor = 0,
        .start_room_index = 0,
        .music_id = 0x09,
        .tileset_type = 6,
        .palette_group = 18,
        .entrance_type = 0,
    },
};

// =============================================================================
// Tilesets das masmorras
// Nota: Ponteiros para dados de tiles que serao carregados da ROM
// =============================================================================
const DungeonTileset DUNGEON_TILESETS[DUNGEON_COUNT] = {
    // Castelo da Agonia
    {
        .wall_tiles = TILES_DUNGEON_1,
        .floor_tiles = TILES_DUNGEON_1 + 0x1000,
        .door_tiles = TILES_DUNGEON_1 + 0x2000,
        .switch_tiles = TILES_DUNGEON_1 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_1 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Caverna do Trovao
    {
        .wall_tiles = TILES_DUNGEON_2,
        .floor_tiles = TILES_DUNGEON_2 + 0x1000,
        .door_tiles = TILES_DUNGEON_2 + 0x2000,
        .switch_tiles = TILES_DUNGEON_2 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_2 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Mina de Ouro
    {
        .wall_tiles = TILES_DUNGEON_3,
        .floor_tiles = TILES_DUNGEON_3 + 0x1000,
        .door_tiles = TILES_DUNGEON_3 + 0x2000,
        .switch_tiles = TILES_DUNGEON_3 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_3 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Palacio do Gelo
    {
        .wall_tiles = TILES_DUNGEON_4,
        .floor_tiles = TILES_DUNGEON_4 + 0x1000,
        .door_tiles = TILES_DUNGEON_4 + 0x2000,
        .switch_tiles = TILES_DUNGEON_4 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_4 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Masmorra do Pity
    {
        .wall_tiles = TILES_DUNGEON_GENERIC,
        .floor_tiles = TILES_DUNGEON_GENERIC + 0x1000,
        .door_tiles = TILES_DUNGEON_GENERIC + 0x2000,
        .switch_tiles = TILES_DUNGEON_GENERIC + 0x2800,
        .decoration_tiles = TILES_DUNGEON_GENERIC + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Masmorra da Floresta
    {
        .wall_tiles = TILES_DUNGEON_GENERIC,
        .floor_tiles = TILES_DUNGEON_GENERIC + 0x1000,
        .door_tiles = TILES_DUNGEON_GENERIC + 0x2000,
        .switch_tiles = TILES_DUNGEON_GENERIC + 0x2800,
        .decoration_tiles = TILES_DUNGEON_GENERIC + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Masmorra da Vila
    {
        .wall_tiles = TILES_DUNGEON_GENERIC,
        .floor_tiles = TILES_DUNGEON_GENERIC + 0x1000,
        .door_tiles = TILES_DUNGEON_GENERIC + 0x2000,
        .switch_tiles = TILES_DUNGEON_GENERIC + 0x2800,
        .decoration_tiles = TILES_DUNGEON_GENERIC + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Palacio do Prata (Skull Woods)
    {
        .wall_tiles = TILES_DUNGEON_5,
        .floor_tiles = TILES_DUNGEON_5 + 0x1000,
        .door_tiles = TILES_DUNGEON_5 + 0x2000,
        .switch_tiles = TILES_DUNGEON_5 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_5 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Torre de Hera
    {
        .wall_tiles = TILES_DUNGEON_6,
        .floor_tiles = TILES_DUNGEON_6 + 0x1000,
        .door_tiles = TILES_DUNGEON_6 + 0x2000,
        .switch_tiles = TILES_DUNGEON_6 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_6 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Palacio dos Escuridao
    {
        .wall_tiles = TILES_DUNGEON_7,
        .floor_tiles = TILES_DUNGEON_7 + 0x1000,
        .door_tiles = TILES_DUNGEON_7 + 0x2000,
        .switch_tiles = TILES_DUNGEON_7 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_7 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Masmorra da Morte
    {
        .wall_tiles = TILES_DUNGEON_GENERIC,
        .floor_tiles = TILES_DUNGEON_GENERIC + 0x1000,
        .door_tiles = TILES_DUNGEON_GENERIC + 0x2000,
        .switch_tiles = TILES_DUNGEON_GENERIC + 0x2800,
        .decoration_tiles = TILES_DUNGEON_GENERIC + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Palacio do Forte
    {
        .wall_tiles = TILES_DUNGEON_8,
        .floor_tiles = TILES_DUNGEON_8 + 0x1000,
        .door_tiles = TILES_DUNGEON_8 + 0x2000,
        .switch_tiles = TILES_DUNGEON_8 + 0x2800,
        .decoration_tiles = TILES_DUNGEON_8 + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
    // Caverna Escura
    {
        .wall_tiles = TILES_DUNGEON_GENERIC,
        .floor_tiles = TILES_DUNGEON_GENERIC + 0x1000,
        .door_tiles = TILES_DUNGEON_GENERIC + 0x2000,
        .switch_tiles = TILES_DUNGEON_GENERIC + 0x2800,
        .decoration_tiles = TILES_DUNGEON_GENERIC + 0x3000,
        .wall_tile_count = 64,
        .floor_tile_count = 32,
        .door_tile_count = 16,
        .switch_tile_count = 8,
        .decoration_tile_count = 16,
    },
};

// =============================================================================
// Portas das masmorras
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const DoorData DUNGEON_DOORS[DUNGEON_COUNT][MAX_ROOMS_PER_FLOOR][MAX_DOORS_PER_ROOM] = {};

// =============================================================================
// Inimigos das masmorras
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const EnemySpawn DUNGEON_ENEMY_SPAWNS[DUNGEON_COUNT][MAX_ROOMS_PER_FLOOR][MAX_ENEMIES_PER_ROOM] = {};

// =============================================================================
// Baus das masmorras
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const ChestData DUNGEON_CHESTS[DUNGEON_COUNT][MAX_ROOMS_PER_FLOOR][MAX_CHESTS_PER_ROOM] = {};

// =============================================================================
// Itens das masmorras
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const RoomItem DUNGEON_ITEMS[DUNGEON_COUNT][MAX_ROOMS_PER_FLOOR][MAX_ITEMS_PER_ROOM] = {};

// =============================================================================
// Puzzles das masmorras
// Dados placeholder - devem ser extraidos da ROM
// =============================================================================
const PuzzleData DUNGEON_PUZZLES[DUNGEON_COUNT][MAX_ROOMS_PER_FLOOR][MAX_PUZZLES_PER_ROOM] = {};

// =============================================================================
// Bosses das masmorras
// =============================================================================
const BossData DUNGEON_BOSSES[DUNGEON_COUNT] = {
    // 0: Castelo da Agonia - Armos Knights
    {
        .boss_id = 0x01,
        .health = 48,
        .phases = 1,
        .weakness_item = 0x01,  // Espada
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x09,
        .drop_item = 0x06,      // Heart Container
        .drop_count = 4,
    },
    // 1: Caverna do Trovao - Lanmolas
    {
        .boss_id = 0x02,
        .health = 48,
        .phases = 1,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0A,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 2: Mina de Ouro - Moldorm
    {
        .boss_id = 0x03,
        .health = 32,
        .phases = 1,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0B,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 3: Palacio do Gelo - Kholdstare
    {
        .boss_id = 0x04,
        .health = 64,
        .phases = 1,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0C,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 4-6: Mini-dungeons (sem boss)
    {
        .boss_id = 0x00,
        .health = 0,
        .phases = 0,
        .weakness_item = 0x00,
        .spawn_x = 0,
        .spawn_y = 0,
        .death_flag = 0,
        .drop_item = 0,
        .drop_count = 0,
    },
    {
        .boss_id = 0x00,
        .health = 0,
        .phases = 0,
        .weakness_item = 0x00,
        .spawn_x = 0,
        .spawn_y = 0,
        .death_flag = 0,
        .drop_item = 0,
        .drop_count = 0,
    },
    {
        .boss_id = 0x00,
        .health = 0,
        .phases = 0,
        .weakness_item = 0x00,
        .spawn_x = 0,
        .spawn_y = 0,
        .death_flag = 0,
        .drop_item = 0,
        .drop_count = 0,
    },
    // 7: Palacio do Prata - Mothula
    {
        .boss_id = 0x05,
        .health = 48,
        .phases = 1,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0D,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 8: Torre de Hera - Helmasaur
    {
        .boss_id = 0x06,
        .health = 80,
        .phases = 2,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0E,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 9: Palacio dos Escuridao - Arrghus
    {
        .boss_id = 0x07,
        .health = 64,
        .phases = 2,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x0F,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 10: Masmorra da Morte - Blind
    {
        .boss_id = 0x08,
        .health = 48,
        .phases = 1,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x10,
        .drop_item = 0x06,
        .drop_count = 4,
    },
    // 11: Palacio do Forte - Agahnim
    {
        .boss_id = 0x09,
        .health = 96,
        .phases = 3,
        .weakness_item = 0x01,
        .spawn_x = 0x0078,
        .spawn_y = 0x0040,
        .death_flag = 0x11,
        .drop_item = 0x00,
        .drop_count = 0,
    },
    // 12: Caverna Escura - Ganon
    {
        .boss_id = 0x0A,
        .health = 128,
        .phases = 4,
        .weakness_item = 0x03,  // Silver Arrow
        .spawn_x = 0x0078,
        .spawn_y = 0x0070,
        .death_flag = 0x12,
        .drop_item = 0x00,
        .drop_count = 0,
    },
};

// =============================================================================
// Offsets ROM das masmorras
// =============================================================================
const DungeonROMOffset DUNGEON_ROM_OFFSETS[DUNGEON_COUNT] = {
    // 0: Castelo da Agonia
    {
        .room_data_offset = 0x050000,
        .layout_data_offset = 0x054000,
        .tileset_data_offset = 0x058000,
        .enemy_data_offset = 0x05C000,
        .item_data_offset = 0x05E000,
        .music_data_offset = 0x05F000,
        .room_data_size = 0x2000,
        .layout_data_size = 0x2000,
    },
    // 1-12: Dados placeholder - devem ser extraidos da ROM
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};
