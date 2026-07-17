#ifndef SPRITES_H
#define SPRITES_H

#include "../types.h"

// Sprite Constants
constexpr int MAX_SPRITES = 128;
constexpr uint8 SPRITE_OFFSCREEN_Y = 0xF0;

// OAM Entry Structure (4 bytes)
struct OAMEntry {
    uint8 y;         // Posição Y (byte baixo)
    uint8 tile;      // Número do tile
    uint8 attr;      // Atributos (paleta, prioridade, flip)
    uint8 x;         // Posição X (byte baixo)
};

// OAM High Entry (2 bits per sprite)
struct OAMHighEntry {
    uint8 x_high : 2;  // Bit 8 e 9 de X
    uint8 y_high : 2;  // Bit 8 e 9 de Y
    uint8 size : 1;    // 0=small, 1=large
};

// OAM Attributes
constexpr uint8 OAM_ATTR_PALETTE(uint8 n) { return (n << 1); }       // Bits 1-3: paleta (0-7)
constexpr uint8 OAM_ATTR_PRIORITY(uint8 n) { return (n << 4); }     // Bits 4-5: prioridade
constexpr uint8 OAM_ATTR_FLIP_H = 0x40;                              // Bit 6: flip horizontal
constexpr uint8 OAM_ATTR_FLIP_V = 0x80;                              // Bit 7: flip vertical

// Complete OAM Buffer Structure
struct OAMBuffer {
    OAMEntry entries[MAX_SPRITES];    // 128 sprites × 4 bytes = 512 bytes
    OAMHighEntry high[MAX_SPRITES / 2];  // High table = 64 bytes (128 entries × 2 bits)
};

// Sprite Tables (Shadow RAM $0A00-$0A3F)
struct SpriteTables {
    uint8 size_table[MAX_SPRITES];      // 1=sprite grande, 0=pequeno
    uint8 priority_table[MAX_SPRITES];  // Prioridade de desenho
};

// Bounding Box for Collision
struct BoundingBox {
    int16 x;
    int16 y;
    uint8 width;
    uint8 height;
};

// Sprite Structure
struct Sprite {
    int16 x;
    int16 y;
    uint8 tile;
    uint8 attributes;
    uint8 type;
    uint8 state;
    uint8 health;
    uint8 speed;
    int8 vel_x;
    int8 vel_y;
    BoundingBox hitbox;
};

// Global sprite data
extern OAMBuffer oam_buffer;
extern SpriteTables sprite_tables;
extern Sprite sprites[MAX_SPRITES];

// Sprite Functions
void update_sprites(void);
void hide_unused_sprites(void);
void init_sprites(void);
void set_sprite_position(int index, int16 x, int16 y);
void set_sprite_tile(int index, uint8 tile);
void set_sprite_attributes(int index, uint8 attr);

// OAM Functions
void update_oam_entry(int index, const Sprite* sprite);
void set_oam_high_entry(int index, int x_high, int y_high, int size);

#endif // SPRITES_H