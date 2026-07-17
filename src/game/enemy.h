#ifndef ENEMY_H
#define ENEMY_H

#include "../types.h"
#include "entity.h"

// ------------------------------------------------------------------
// Enemy System — Bank $01 entity handlers + Bank $02 dispatchers
//
// Enemy behavior is dispatched via the entity system ($01:A87D).
// Each enemy type has its own handler in the entity dispatch table.
// Enemy AI runs per-frame and handles:
//   - Movement patterns (chase, patrol, random)
//   - Attack patterns (projectile, melee, charge)
//   - Damage/death handling
//   - Sprite animation states
//
// Enemy spawn data comes from room entity lists ($09:813A),
// which specify enemy type, position, and initial parameters.
//
// Enemy type byte layout:
//   Bits 7-4: 0x3 (enemy category)
//   Bits 3-0: sub-type (0x0-0xF specific enemy)
//
// Sub-types (partial list from ROM data):
//   0x30: Green Eyegore
//   0x31: Red Eyegore
//   0x32: Stalfos Knight
//   0x33: Hinox
//   0x34: Moblin
//   0x35: Darknut
//   0x36: Wizzrobe
//   0x37: Like Like
//   0x38: Keese
//   0x39: Gel
//   0x3A: Zol
//   0x3B: Moldorm
//   0x3C: Armos Knight
//   0x3D: Lanmola
//   0x3E: Moldorm (boss)
//   0x3F: Agahnim
// ------------------------------------------------------------------

constexpr int MAX_ENEMIES = 16;

// Enemy sub-types
enum class EnemyType : uint8 {
    NONE            = 0x00,
    GREEN_EYEGORE   = 0x30,
    RED_EYEGORE     = 0x31,
    STALFOS_KNIGHT  = 0x32,
    HINOX           = 0x33,
    MOBLIN          = 0x34,
    DARKNUT         = 0x35,
    WIZZROBE        = 0x36,
    LIKE_LIKE       = 0x37,
    KEESE           = 0x38,
    GEL             = 0x39,
    ZOL             = 0x3A,
    MOLDORM         = 0x3B,
    ARMOS_KNIGHT    = 0x3C,
    LANMOLA         = 0x3D,
    MOLDORM_BOSS    = 0x3E,
    AGAHNIM         = 0x3F,
};

// Enemy AI states
enum class EnemyAIState : uint8 {
    IDLE,
    PATROL,
    CHASE,
    ATTACK,
    HURT,
    DYING,
    DEAD,
};

// Enemy movement patterns
enum class EnemyMovement : uint8 {
    NONE,
    CHASE_PLAYER,     // Move toward Link
    PATROL_H,         // Horizontal patrol
    PATROL_V,         // Vertical patrol
    RANDOM,           // Random direction changes
    CIRCLE,           // Circular movement
    BOUNCE,           // Bounce off walls
    CHARGE,           // Charge at player
};

// Enemy structure — extends Entity with enemy-specific data
struct Enemy {
    uint8  enemy_type;       // Sub-type (lower nibble of entity type)
    EnemyAIState ai_state;   // Current AI state
    EnemyMovement movement;  // Movement pattern
    uint8  damage;           // Contact damage value
    uint8  knockback_timer;  // Frames of knockback
    int8   knockback_dir;    // Knockback direction
    uint8  invincibility;    // Invincibility frames after hit
    uint8  stun_timer;       // Stun duration
    uint8  death_timer;      // Death animation timer
    uint8  drop_item;        // Item drop type (0=none)
    int16  patrol_x_min;     // Patrol bounds X min
    int16  patrol_x_max;     // Patrol bounds X max
    int16  patrol_y_min;     // Patrol bounds Y min
    int16  patrol_y_max;     // Patrol bounds Y max
};

// Global enemy data
extern Enemy enemies[MAX_ENEMIES];
extern int   active_enemy_count;

// Enemy System Functions

// Initialize enemy system
void enemy_init(void);

// Spawn enemy at position (called from room entity setup)
int  enemy_spawn(EnemyType type, int16 x, int16 y);

// Update all active enemies (called from entity dispatch)
void enemy_update_all(void);

// Per-enemy AI update — dispatches to type-specific behavior
void enemy_update_ai(int slot);

// Movement functions
void enemy_move_chase(int slot, int target_x, int target_y);
void enemy_move_patrol(int slot);
void enemy_move_random(int slot);
void enemy_apply_velocity(int slot);

// Combat functions
void enemy_take_damage(int slot, uint8 damage);
void enemy_apply_knockback(int slot, int direction);
void enemy_check_death(int slot);

// Contact damage — called when player touches enemy
uint8 enemy_get_contact_damage(int slot);

// Item drops — called on enemy death
void enemy_drop_item(int slot);

// Enemy type-specific behavior stubs
void enemy_behavior_eyegore(int slot);
void enemy_behavior_stalfos(int slot);
void enemy_behavior_hinox(int slot);
void enemy_behavior_moblin(int slot);
void enemy_behavior_darknut(int slot);
void enemy_behavior_wizzrobe(int slot);
void enemy_behavior_keese(int slot);
void enemy_behavior_gel(int slot);
void enemy_behavior_boss(int slot);

// Utility
void enemy_remove(int slot);
bool enemy_is_alive(int slot);
bool enemy_is_invincible(int slot);
int  enemy_get_slot_for_entity(int entity_slot);

#endif // ENEMY_H
