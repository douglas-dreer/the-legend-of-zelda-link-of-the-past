#include "collision.h"

// Check AABB collision
bool check_collision_aabb(const BoundingBox* a, const BoundingBox* b) {
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

// Get tile collision
TileCollision get_tile_collision(int x, int y) {
    TileCollision col;
    col.collision_type = 0;
    col.solid_h = 0;
    col.solid_v = 0;
    
    // In a real implementation, this would read from collision map
    // and return collision data for the tile at (x, y)
    
    return col;
}

// Check horizontal solid collision
bool check_solid_collision_h(int x, int y, int direction) {
    // Check if movement in horizontal direction is blocked
    TileCollision col = get_tile_collision(x, y);
    return col.solid_h != 0;
}

// Check vertical solid collision
bool check_solid_collision_v(int x, int y, int direction) {
    // Check if movement in vertical direction is blocked
    TileCollision col = get_tile_collision(x, y);
    return col.solid_v != 0;
}

// Initialize collision system
void init_collision_system(void) {
    // Initialize collision map
}

// Update collision map
void update_collision_map(void) {
    // Update collision map for current room
}

// Check if tile is solid
bool is_solid_tile(uint8 tile_data) {
    return (tile_data & 0x03) == COLLISION_TYPE_SOLID;
}

// Check if tile is water
bool is_water_tile(uint8 tile_data) {
    return (tile_data & 0x03) == COLLISION_TYPE_WATER;
}

// Check if tile is pit
bool is_pit_tile(uint8 tile_data) {
    return (tile_data & 0x03) == COLLISION_TYPE_PIT;
}