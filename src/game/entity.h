#ifndef ENTITY_H
#define ENTITY_H

#include "../types.h"

// ------------------------------------------------------------------
// Entity System — Bank $01 ($01:A87D entity update dispatch)
//
// Entity data tables in WRAM:
//   $1980-$199F  — Entity type/priority (32 slots × 1 byte)
//   $19A0-$19BF  — Entity state flags (32 slots × 1 byte)
//   $19C0-$19DF  — Entity auxiliary state (32 slots × 1 byte)
//   $0460        — Active entity list pointer (OAM packing index)
//
// Entity update dispatch at $01:A87D:
//   JMP ($20C9,X) — indirect jump through table at $20C9
//   X = entity slot × 2, each entry is a 16-bit function pointer
//
// The entity system processes active entities each frame:
//   1. Load entity index from active list
//   2. Dispatch to type-specific handler via function table
//   3. Handler updates position, animation, state
//   4. Collision检测 results stored in entity tables
//
// Entity type byte layout ($1980+X):
//   Bits 7-4: category (0=none, 1=link, 2=NPC, 3=enemy, ...)
//   Bits 3-0: sub-type within category
// ------------------------------------------------------------------

constexpr int MAX_ENTITIES = 32;
constexpr int ENTITY_SLOT_SIZE = 2;

// Entity type categories (high nibble of type byte)
enum class EntityCategory : uint8 {
    NONE        = 0x00,
    LINK        = 0x10,
    NPC         = 0x20,
    ENEMY       = 0x30,
    PROJECTILE  = 0x40,
    ITEM        = 0x50,
    EFFECT      = 0x60,
    HAZARD      = 0x70,
};

// Entity state flags ($19A0 table)
constexpr uint8 ENTITY_STATE_ACTIVE    = 0x80;
constexpr uint8 ENTITY_STATE_VISIBLE   = 0x40;
constexpr uint8 ENTITY_STATE_INVISIBLE = 0x20;
constexpr uint8 ENTITY_STATE_FROZEN    = 0x10;
constexpr uint8 ENTITY_STATE_HURT      = 0x08;
constexpr uint8 ENTITY_STATE_DYING     = 0x04;
constexpr uint8 ENTITY_STATE_DEAD      = 0x02;
constexpr uint8 ENTITY_STATE_UNUSED    = 0x01;

// ------------------------------------------------------------------
// Entity structure — maps to WRAM entity table layout
//
// Each entity occupies 6 bytes in the OAM packing table ($1980+):
//   +$00: type byte
//   +$01: state flags
//   +$02-$03: X position (16-bit)
//   +$04-$05: Y position (16-bit)
//
// The $0460 pointer tracks the OAM packing index, which advances
// by 6 bytes per entity pair (two entities share a 12-byte block
// in the OAM packing buffer at $19C0).
// ------------------------------------------------------------------

struct Entity {
    uint8  type;           // $1980+X: entity type (category|subtype)
    uint8  state;          // $19A0+X: state flags
    int16  x;              // X position (16-bit, signed)
    int16  y;              // Y position (16-bit, signed)
    uint8  health;         // Hit points (for enemies/items)
    uint8  timer;          // General-purpose timer
    int8   vel_x;          // Horizontal velocity
    int8   vel_y;          // Vertical velocity
    uint8  anim_frame;     // Current animation frame
    uint8  anim_timer;     // Animation frame counter
    uint8  aux_state;      // $19C0+X: auxiliary state data
    uint8  collision_flag; // Collision result bits
};

// ------------------------------------------------------------------
// Entity slot tables — mirror WRAM layout at $1980-$19DF
//
// The original ROM stores entity data in interleaved tables:
//   $1980+X: type byte (X = slot × 2 for paired entries)
//   $19A0+X: state flags
//   $19C0+X: auxiliary state
//
// Active entity list at $0460 is an OAM packing index that
// determines which slots are processed. The index advances
// by 6 per entity pair.
// ------------------------------------------------------------------

struct EntityTables {
    uint8 type[MAX_ENTITIES];       // $1980-$199F
    uint8 state[MAX_ENTITIES];      // $19A0-$19BF
    uint8 aux[MAX_ENTITIES];        // $19C0-$19DF
    uint8 oam_pack_index;           // $0460 — OAM packing pointer
};

// Entity function pointer table ($20C9 indirect jump)
// Each entry is a 16-bit address in bank $01
using EntityUpdateFunc = void (*)(int slot);

// Global entity state
extern EntityTables entity_tables;
extern Entity entities[MAX_ENTITIES];
extern EntityUpdateFunc entity_dispatch_table[];

// Entity System Functions

// Initialize all entity tables and clear active list
void entity_init(void);

// Update all active entities — called each frame from $01:A87D
// Dispatches each entity to its type-specific handler
void entity_update_all(void);

// Dispatch entity update via JMP ($20C9,X) table
// $01:A87D: load entity type, ASL ×2, index into table, JMP indirect
void entity_dispatch(int slot);

// Entity slot management
int  entity_alloc_slot(void);           // $01:A8C8 — find free slot
void entity_free_slot(int slot);        // $01:A8FA — mark slot unused
bool entity_slot_active(int slot);      // check if slot is in use

// Entity spawn — called from room entity setup ($09:813A)
void entity_spawn(uint8 type, int16 x, int16 y, uint8 aux);

// Entity removal
void entity_remove(int slot);
void entity_remove_all(void);

// Entity state helpers
void entity_set_state(int slot, uint8 state);
void entity_set_position(int slot, int16 x, int16 y);
void entity_set_velocity(int slot, int8 vx, int8 vy);

// Collision detection per entity ($01:893C sub_01_893C)
// Reads entity type, checks tile collision properties
// $B2 = solid_h, $B4 = solid_v from collision type lookup
void entity_collision_test(int slot);

// Off-screen culling ($01:89DC sub_01_89DC)
// Checks if entity is off-screen and marks as invisible
// Uses X/Y position comparison against scroll bounds ($E0-$E9)
bool entity_is_offscreen(int slot);

// Entity OAM update — pack entity into OAM buffer
// Uses $0460 OAM packing index to write sprite data
void entity_update_oam(int slot);

#endif // ENTITY_H
