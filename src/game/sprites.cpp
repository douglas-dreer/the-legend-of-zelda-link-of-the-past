#include "sprites.h"
#include <cstring>

// Update sprites
void update_sprites(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].type == 0) continue;  // empty
        
        // Move entity
        sprites[i].x += sprites[i].vel_x;
        sprites[i].y += sprites[i].vel_y;
        
        // Update OAM
        update_oam_entry(i, &sprites[i]);
    }
}

// Hide unused sprites
void hide_unused_sprites(void) {
    // $00:841E — hide unused sprites by setting Y = $F0
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].type == 0) {
            oam_buffer.entries[i].y = SPRITE_OFFSCREEN_Y;
        }
    }
}

// Initialize sprites
void init_sprites(void) {
    memset(sprites, 0, sizeof(Sprite) * MAX_SPRITES);
    memset(&oam_buffer, 0, sizeof(OAMBuffer));
    memset(&sprite_tables, 0, sizeof(SpriteTables));
}

// Set sprite position
void set_sprite_position(int index, int16 x, int16 y) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].x = x;
        sprites[index].y = y;
    }
}

// Set sprite tile
void set_sprite_tile(int index, uint8 tile) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].tile = tile;
    }
}

// Set sprite attributes
void set_sprite_attributes(int index, uint8 attr) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].attributes = attr;
    }
}

// Update OAM entry
void update_oam_entry(int index, const Sprite* sprite) {
    if (index >= 0 && index < MAX_SPRITES) {
        oam_buffer.entries[index].x = sprite->x & 0xFF;
        oam_buffer.entries[index].y = sprite->y & 0xFF;
        oam_buffer.entries[index].tile = sprite->tile;
        oam_buffer.entries[index].attr = sprite->attributes;
        
        // Update high table
        set_oam_high_entry(index, 
                          (sprite->x >> 8) & 0x3,
                          (sprite->y >> 8) & 0x3,
                          sprite_tables.size_table[index]);
    }
}

// Set OAM high entry
void set_oam_high_entry(int index, int x_high, int y_high, int size) {
    if (index >= 0 && index < MAX_SPRITES / 2) {
        oam_buffer.high[index].x_high = x_high & 0x3;
        oam_buffer.high[index].y_high = y_high & 0x3;
        oam_buffer.high[index].size = size & 0x1;
    }
}