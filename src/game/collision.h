#ifndef COLLISION_H
#define COLLISION_H

#include "../types.h"
#include "sprites.h"

// Tile Collision Structure
struct TileCollision {
    uint8 collision_type;  // 2 bits per tile
    uint8 solid_h;         // $B2: colisão horizontal
    uint8 solid_v;         // $B4: colisão vertical
};

// Collision Constants
constexpr uint8 COLLISION_TYPE_NONE = 0x00;
constexpr uint8 COLLISION_TYPE_SOLID = 0x01;
constexpr uint8 COLLISION_TYPE_WATER = 0x02;
constexpr uint8 COLLISION_TYPE_PIT = 0x03;

// Collision Functions
bool check_collision_aabb(const BoundingBox* a, const BoundingBox* b);
TileCollision get_tile_collision(int x, int y);

bool check_solid_collision_h(int x, int y, int direction);
bool check_solid_collision_v(int x, int y, int direction);

void init_collision_system(void);
void update_collision_map(void);

// Utility Functions
bool is_solid_tile(uint8 tile_data);
bool is_water_tile(uint8 tile_data);
bool is_pit_tile(uint8 tile_data);

#endif // COLLISION_H