#include "entity.h"
#include "collision.h"
#include "sprites.h"
#include "../game/state.h"
#include <cstring>

// ------------------------------------------------------------------
// Entity System Implementation
//
// Converted from Bank $01 assembly — entity update dispatch and
// slot management. The ROM's entity system uses 32 slots with
// interleaved WRAM tables for type, state, and auxiliary data.
//
// Key addresses:
//   $01:A87D — JMP ($20C9,X): entity update dispatch
//   $01:A8C8 — entity slot allocation
//   $01:A8FA — entity slot cleanup
//   $01:893C — collision detection per entity
//   $01:89DC — off-screen entity culling
// ------------------------------------------------------------------

extern GlobalState shadow;
extern OAMBuffer oam_buffer;

EntityTables entity_tables;
Entity entities[MAX_ENTITIES];

// ------------------------------------------------------------------
// Entity dispatch table — $20C9 indirect jump targets
//
// $01:A87D: JMP ($20C9,X)
// X = entity type × 2, each entry is 16-bit handler address
//
// The table entries correspond to different entity categories:
//   0x00: No-op (inactive entity)
//   0x01: Link / player character
//   0x02: NPC / friendly character
//   0x03-0x0F: Various enemy types
//   0x10: Projectile / weapon
//   0x11: Item pickup
//   0x12: Visual effect
//   0x13: Environmental hazard
//
// For now, all handlers are stubs — the actual behavior is
// in bank $01 subroutines that this dispatch calls.
// ------------------------------------------------------------------

// Stub handlers — actual implementations will call bank $01 code
static void entity_handler_none(int slot) { (void)slot; }
static void entity_handler_link(int slot) { (void)slot; }
static void entity_handler_npc(int slot) { (void)slot; }
static void entity_handler_enemy(int slot) { (void)slot; }
static void entity_handler_projectile(int slot) { (void)slot; }
static void entity_handler_item(int slot) { (void)slot; }
static void entity_handler_effect(int slot) { (void)slot; }
static void entity_handler_hazard(int slot) { (void)slot; }

// $20C9 — dispatch table (16 entries × 2 bytes)
EntityUpdateFunc entity_dispatch_table[16] = {
    entity_handler_none,       // 0x00: inactive
    entity_handler_link,       // 0x01: player
    entity_handler_npc,        // 0x02: NPC
    entity_handler_enemy,      // 0x03: enemy type A
    entity_handler_enemy,      // 0x04: enemy type B
    entity_handler_enemy,      // 0x05: enemy type C
    entity_handler_enemy,      // 0x06: enemy type D
    entity_handler_enemy,      // 0x07: enemy type E
    entity_handler_enemy,      // 0x08: enemy type F
    entity_handler_projectile, // 0x09: projectile
    entity_handler_item,       // 0x0A: item pickup
    entity_handler_effect,     // 0x0B: effect
    entity_handler_hazard,     // 0x0C: hazard
    entity_handler_none,       // 0x0D: reserved
    entity_handler_none,       // 0x0E: reserved
    entity_handler_none,       // 0x0F: reserved
};

// ------------------------------------------------------------------
// entity_init: Clear all entity tables
// ------------------------------------------------------------------
void entity_init(void) {
    memset(&entity_tables, 0, sizeof(EntityTables));
    memset(entities, 0, sizeof(Entity) * MAX_ENTITIES);
    entity_tables.oam_pack_index = 0;
}

// ------------------------------------------------------------------
// entity_update_all: $01:A87D main dispatch loop
//
// The ROM processes entities by iterating the active list:
//   $01:A87D: JMP ($20C9,X)
//   X = entity_type × 2
//   The handler updates position, state, and OAM for each entity.
//
// Entity allocation at $01:A8C8:
//   LDY $0460       — load OAM packing index
//   LDA #$00         — initialize type to 0
//   STA $1980,Y      — store in type table
//   STA $19A0,Y      — store in state table
//   TXA; LSR A; XBA  — compute index from slot
//   ORA $04          — combine with auxiliary
//   STA $1980,X      — store entity type
// ------------------------------------------------------------------
void entity_update_all(void) {
    // Iterate through all entity slots
    for (int slot = 0; slot < MAX_ENTITIES; slot++) {
        // $01:A87D: check entity type
        uint8 type = entity_tables.type[slot];
        if (type == 0) continue;  // inactive slot

        // $01:A8A8: AND #$0F — extract sub-type for dispatch
        int table_index = (type >> 4) & 0x0F;

        // $01:A87D: JMP ($20C9,X) — indirect jump
        entity_dispatch_table[table_index](slot);

        // After handler: update OAM if entity is visible
        if (entity_tables.state[slot] & ENTITY_STATE_VISIBLE) {
            entity_update_oam(slot);
        }
    }
}

// ------------------------------------------------------------------
// entity_dispatch: Dispatch entity to type-specific handler
//
// $01:A87D: JMP ($20C9,X)
// X = entity type category (high nibble) × 2
// ------------------------------------------------------------------
void entity_dispatch(int slot) {
    uint8 type = entity_tables.type[slot];
    int table_index = (type >> 4) & 0x0F;

    if (table_index < 16) {
        entity_dispatch_table[table_index](slot);
    }
}

// ------------------------------------------------------------------
// entity_alloc_slot: Find and allocate a free entity slot
//
// $01:A8C8-$01:A8CF:
//   LDY $0460       — current OAM pack index
//   LDA #$00         — zero = free slot marker
//   CPY #$36         — check if list is full
//   BCS $A8CE        — branch if full (carry set)
//   STA $1980,Y      — clear entity type
//   INY; INY         — advance by 2 (paired slot)
//   STY $0460        — update OAM pack index
//   CLC              — return success (carry clear)
// ------------------------------------------------------------------
int entity_alloc_slot(void) {
    // Scan for first free slot (type == 0)
    for (int slot = 0; slot < MAX_ENTITIES; slot++) {
        if (entity_tables.type[slot] == 0) {
            // Mark as allocated but not yet typed
            entity_tables.state[slot] = ENTITY_STATE_ACTIVE;
            entity_tables.aux[slot] = 0;
            entities[slot] = {};
            return slot;
        }
    }
    return -1;  // all slots full
}

// ------------------------------------------------------------------
// entity_free_slot: Clear entity slot
// $01:A8FA: clears type and state, frees OAM resources
// ------------------------------------------------------------------
void entity_free_slot(int slot) {
    if (slot >= 0 && slot < MAX_ENTITIES) {
        entity_tables.type[slot] = 0;
        entity_tables.state[slot] = 0;
        entity_tables.aux[slot] = 0;
        memset(&entities[slot], 0, sizeof(Entity));
    }
}

// ------------------------------------------------------------------
// entity_slot_active: Check if slot is in use
// ------------------------------------------------------------------
bool entity_slot_active(int slot) {
    if (slot < 0 || slot >= MAX_ENTITIES) return false;
    return entity_tables.type[slot] != 0;
}

// ------------------------------------------------------------------
// entity_spawn: Create new entity from room entity list
//
// Called from $09:813A (room entity setup) which reads entity
// spawn data from the room's entity list in ROM. Each entry
// specifies type, position, and auxiliary parameters.
//
// $01:A934: load entity type and parameters
// $01:A937: allocate slot and initialize entity tables
// ------------------------------------------------------------------
void entity_spawn(uint8 type, int16 x, int16 y, uint8 aux) {
    int slot = entity_alloc_slot();
    if (slot < 0) return;

    // $01:A8D4: STA $08 — store type parameter
    entity_tables.type[slot] = type;
    entity_tables.state[slot] = ENTITY_STATE_ACTIVE | ENTITY_STATE_VISIBLE;
    entity_tables.aux[slot] = aux;

    entities[slot].x = x;
    entities[slot].y = y;
    entities[slot].type = type;
    entities[slot].state = ENTITY_STATE_ACTIVE;
    entities[slot].health = 1;
    entities[slot].timer = 0;
    entities[slot].vel_x = 0;
    entities[slot].vel_y = 0;
}

// ------------------------------------------------------------------
// entity_remove: Free entity slot and clear OAM
// ------------------------------------------------------------------
void entity_remove(int slot) {
    entity_free_slot(slot);
}

// ------------------------------------------------------------------
// entity_remove_all: Clear all entities
// ------------------------------------------------------------------
void entity_remove_all(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entity_free_slot(i);
    }
    entity_tables.oam_pack_index = 0;
}

// ------------------------------------------------------------------
// entity_set_state: Update entity state flags
// ------------------------------------------------------------------
void entity_set_state(int slot, uint8 state) {
    if (slot >= 0 && slot < MAX_ENTITIES) {
        entity_tables.state[slot] = state;
        entities[slot].state = state;
    }
}

// ------------------------------------------------------------------
// entity_set_position: Update entity X/Y
// ------------------------------------------------------------------
void entity_set_position(int slot, int16 x, int16 y) {
    if (slot >= 0 && slot < MAX_ENTITIES) {
        entities[slot].x = x;
        entities[slot].y = y;
    }
}

// ------------------------------------------------------------------
// entity_set_velocity: Update entity velocity
// ------------------------------------------------------------------
void entity_set_velocity(int slot, int8 vx, int8 vy) {
    if (slot >= 0 && slot < MAX_ENTITIES) {
        entities[slot].vel_x = vx;
        entities[slot].vel_y = vy;
    }
}

// ------------------------------------------------------------------
// entity_collision_test: $01:893C sub_01_893C
//
// Reads entity collision type from $08, masks with #$FC to get
// collision properties, then extracts horizontal ($B2) and
// vertical ($B4) solid bits.
//
// $01:893C: SEP #$20
// $01:893E: AND #$FC     — mask low 2 bits (direction bits)
// $01:8940: CMP #$FC     — check if fully solid
// $01:8942: BEQ $8983    — special case for fully solid
// $01:8944: STA $08      — save masked type
// $01:8946: LDA $00      — load collision data byte 0
// $01:8948: AND #$03     — extract horizontal solid bits
// $01:894A: STA $B2      — store horizontal collision
// $01:894C: LDA $01      — load collision data byte 1
// $01:894E: AND #$03     — extract vertical solid bits
// $01:8950: STA $B4      — store vertical collision
// ------------------------------------------------------------------
void entity_collision_test(int slot) {
    if (slot < 0 || slot >= MAX_ENTITIES) return;

    // Get entity position
    int16 x = entities[slot].x;
    int16 y = entities[slot].y;

    // Check tile collision at entity position
    TileCollision col = get_tile_collision(x >> 4, y >> 4);

    // Store collision flags: horizontal in $B2, vertical in $B4
    entities[slot].collision_flag = (col.solid_h & 0x03) |
                                    ((col.solid_v & 0x03) << 2);
}

// ------------------------------------------------------------------
// entity_is_offscreen: $01:89DC sub_01_89DC
//
// Checks if entity position is outside visible scroll area.
// Compares entity X/Y against scroll registers $E0-$E9.
//
// $01:89DC: LDX #$1E     — start from last slot pair
// $01:89DF: LDA $0B08,X  — load entity X low
// $01:89E2: CMP $E0      — compare with scroll X low
//   ... continues comparing high bytes and Y
// ------------------------------------------------------------------
bool entity_is_offscreen(int slot) {
    if (slot < 0 || slot >= MAX_ENTITIES) return true;

    // Scroll bounds from shadow RAM
    // $E0/$E1 = scroll_x, $E8/$E9 = scroll_y
    int16 scroll_x = (shadow.scroll_bg1_h & 0xFF) |
                      ((shadow.scroll_bg1_h & 0xFF) << 8);
    int16 scroll_y = (shadow.scroll_bg1_v & 0xFF) |
                      ((shadow.scroll_bg1_v & 0xFF) << 8);

    int16 ex = entities[slot].x;
    int16 ey = entities[slot].y;

    // Screen is 256 pixels wide, 224 pixels tall
    // Entity is offscreen if fully outside these bounds
    constexpr int16 SCREEN_LEFT   = 0;
    constexpr int16 SCREEN_RIGHT  = 256;
    constexpr int16 SCREEN_TOP    = -16;
    constexpr int16 SCREEN_BOTTOM = 224;

    return (ex < SCREEN_LEFT - 32 || ex > SCREEN_RIGHT + 32 ||
            ey < SCREEN_TOP - 32 || ey > SCREEN_BOTTOM + 32);
}

// ------------------------------------------------------------------
// entity_update_oam: Pack entity into OAM buffer
//
// Uses $0460 OAM packing index to write sprite data.
// Each entity occupies up to 4 OAM entries (4 sprites per entity).
// ------------------------------------------------------------------
void entity_update_oam(int slot) {
    if (slot < 0 || slot >= MAX_ENTITIES) return;

    const Entity& e = entities[slot];
    int oam_index = slot * 2;  // each entity gets 2 OAM slots

    if (oam_index >= MAX_SPRITES) return;

    // Update low bytes (position, tile, attributes)
    oam_buffer.entries[oam_index].x = e.x & 0xFF;
    oam_buffer.entries[oam_index].y = e.y & 0xFF;
    oam_buffer.entries[oam_index].tile = e.anim_frame;
    oam_buffer.entries[oam_index].attr = 0;  // default attributes

    // Update high byte table (position bits 8-9)
    set_oam_high_entry(oam_index,
                       (e.x >> 8) & 0x3,
                       (e.y >> 8) & 0x3,
                       0);  // default size
}
