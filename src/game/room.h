#ifndef ROOM_H
#define ROOM_H

#include "../types.h"

// Room Header Structure
struct RoomHeader {
    uint16 room_id;
    uint8 map_type;         // 0=overworld, 1=dungeon
    uint32 tileset_addr;
    uint32 tilemap_addr;
    uint32 entity_list_addr;
    uint8 palette_id;
    uint8 bg1_scroll_h;
    uint8 bg1_scroll_v;
    uint8 bg2_scroll_h;
    uint8 bg2_scroll_v;
};

// Map Type Constants
constexpr uint8 MAP_TYPE_OVERWORLD = 0;
constexpr uint8 MAP_TYPE_DUNGEON = 1;

// Room Functions
void load_room(uint16 room_id);
void load_overworld_room(uint16 room_id);
void load_dungeon_room(uint16 room_id);

void decompress_tiles(uint32 addr);
void load_tilemap(uint32 addr);
void init_room_entities(uint32 addr);

RoomHeader* get_room_header(uint16 room_id);

// Room Transition Functions
void start_room_transition(uint16 new_room_id, uint8 direction);
void update_room_transition(void);

#endif // ROOM_H