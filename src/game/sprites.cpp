#include "sprites.h"
#include "../game/state.h"
#include <cstring>

// External references
extern OAMBuffer oam_buffer;
extern SpriteTables sprite_tables;
extern Sprite sprites[MAX_SPRITES];
extern GlobalState shadow;

// ------------------------------------------------------------------
// hide_unused_sprites: $00:841E-$00:8489
//
// Sets Y position of unused sprites to $F0 (off-screen).
// Uses unrolled writes for speed on 65816.
//
// $00:841E: LDX #$60          — start index = $60 (96)
// $00:8420: LDA #$F0          — off-screen Y value
//
// Inner loop (unrolled 8 sprites per iteration):
//   STA $0801,X   (sprite 0 Y)
//   STA $0805,X   (sprite 1 Y)
//   STA $0809,X   (sprite 2 Y)
//   ...
//   STA $081D,X   (sprite 7 Y)
//   STA $0881,X   (sprite 8 Y)
//   ...
//   STA $089D,X   (sprite 15 Y)
//   ... continues for sprites 16-31, 32-47, 48-63, etc.
//
// Each sprite OAM entry is 4 bytes: Y, tile, attr, X
// Y is at offset +1 from the 4-byte-aligned base.
// So $0801 = sprite 0 Y, $0805 = sprite 1 Y, etc.
//
// $00:8482: TXA; SEC; SBC #$20; TAX; BPL $8420
// X decrements by $20 (32) each pass = 8 sprites × 4 bytes
// Loop runs until X wraps below 0 (from $60 down to $40, $20, $00)
// Total: ($60/($20))*8 = 3*8 = 24 sprites cleared? No —
// Actually X starts at $60 and goes $60→$40→$20→$00→wrap
// That's 4 passes × 8 sprites = 32 sprites cleared (sprites 0-31)
//
// But wait — the writes cover $0801-$089D, $0901-$099D, etc.
// Let me count: $0800-$08FF (256 bytes = 64 sprites), $0900-$09FF (64 sprites)
// The loop writes to:
//   $0801+4n for n=0..7 (first block, 8 sprites)
//   $0881+4n for n=0..7 (second block, 8 sprites)
//   $0901+4n for n=0..7 (third block, 8 sprites)
//   $0981+4n for n=0..7 (fourth block, 8 sprites)
// = 32 sprites per X pass
// With 3 passes ($60, $40, $20), that's 96 sprites... but sprites 0-127 = 128 total
//
// Actually reading more carefully:
// $00:8460-$00:8480 writes to $0901-$099D (another 32 sprites)
// $00:8482: TXA; SEC; SBC #$20; TAX; BPL $8420
// X=$60 → $40 → $20 → $00 → $E0 (negative, BPL fails)
// That covers sprites at offsets within the OAM buffer.
// Total cleared: 32 sprites × 4 iterations = 128 sprites (all of them)
// ------------------------------------------------------------------
void hide_unused_sprites(void) {
    const uint8 OFF_SCREEN_Y = 0xF0;

    // Clear Y position of all 128 OAM entries
    // This matches the unrolled loop pattern from $00:841E
    for (int i = 0; i < MAX_SPRITES; i++) {
        oam_buffer.entries[i].y = OFF_SCREEN_Y;
    }
}

// ------------------------------------------------------------------
// update_sprites: Move entities and update OAM
// ------------------------------------------------------------------
void update_sprites(void) {
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (sprites[i].type == 0) continue;

        // Move entity
        sprites[i].x += sprites[i].vel_x;
        sprites[i].y += sprites[i].vel_y;

        // Update OAM entry from sprite data
        update_oam_entry(i, &sprites[i]);
    }
}

// ------------------------------------------------------------------
// init_sprites: Clear all sprite data
// ------------------------------------------------------------------
void init_sprites(void) {
    memset(sprites, 0, sizeof(Sprite) * MAX_SPRITES);
    memset(&oam_buffer, 0, sizeof(OAMBuffer));
    memset(&sprite_tables, 0, sizeof(SpriteTables));
}

// ------------------------------------------------------------------
// set_sprite_position: Set sprite X/Y
// ------------------------------------------------------------------
void set_sprite_position(int index, int16 x, int16 y) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].x = x;
        sprites[index].y = y;
    }
}

// ------------------------------------------------------------------
// set_sprite_tile: Set sprite tile number
// ------------------------------------------------------------------
void set_sprite_tile(int index, uint8 tile) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].tile = tile;
    }
}

// ------------------------------------------------------------------
// set_sprite_attributes: Set sprite attributes
// ------------------------------------------------------------------
void set_sprite_attributes(int index, uint8 attr) {
    if (index >= 0 && index < MAX_SPRITES) {
        sprites[index].attributes = attr;
    }
}

// ------------------------------------------------------------------
// update_oam_entry: Write sprite data to OAM buffer
// ------------------------------------------------------------------
void update_oam_entry(int index, const Sprite* sprite) {
    if (index >= 0 && index < MAX_SPRITES) {
        oam_buffer.entries[index].x = sprite->x & 0xFF;
        oam_buffer.entries[index].y = sprite->y & 0xFF;
        oam_buffer.entries[index].tile = sprite->tile;
        oam_buffer.entries[index].attr = sprite->attributes;

        set_oam_high_entry(index,
                          (sprite->x >> 8) & 0x3,
                          (sprite->y >> 8) & 0x3,
                          sprite_tables.size_table[index]);
    }
}

// ------------------------------------------------------------------
// set_oam_high_entry: Update high byte bits in OAM table
// ------------------------------------------------------------------
void set_oam_high_entry(int index, int x_high, int y_high, int size) {
    if (index >= 0 && index < MAX_SPRITES / 2) {
        oam_buffer.high[index].x_high = x_high & 0x3;
        oam_buffer.high[index].y_high = y_high & 0x3;
        oam_buffer.high[index].size = size & 0x1;
    }
}
