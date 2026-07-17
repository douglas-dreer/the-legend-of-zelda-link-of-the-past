#include "enemy.h"
#include "entity.h"
#include "collision.h"
#include "../game/state.h"
#include <cstring>
#include <cstdlib>

// ------------------------------------------------------------------
// Enemy System Implementation
//
// Converted from Bank $01 entity handlers and Bank $02 enemy
// behavior dispatchers. The ROM's enemy system uses the entity
// dispatch table ($20C9) to route enemy updates.
//
// Enemy behavior is type-specific: each enemy has movement
// patterns, attack patterns, and damage handling that varies
// by sub-type. The entity system calls into these handlers
// each frame.
// ------------------------------------------------------------------

extern EntityTables entity_tables;
extern Entity entities[MAX_ENTITIES];
extern GlobalState shadow;

Enemy enemies[MAX_ENEMIES];
int active_enemy_count = 0;

// ------------------------------------------------------------------
// enemy_init: Clear all enemy data
// ------------------------------------------------------------------
void enemy_init(void) {
    memset(enemies, 0, sizeof(Enemy) * MAX_ENEMIES);
    active_enemy_count = 0;
}

// ------------------------------------------------------------------
// enemy_spawn: Create new enemy at position
//
// Called from $09:813A (room entity setup) which reads enemy
// spawn data from ROM and calls entity_spawn. The enemy system
// hooks into the entity system to track enemy-specific state.
// ------------------------------------------------------------------
int enemy_spawn(EnemyType type, int16 x, int16 y) {
    if (active_enemy_count >= MAX_ENEMIES) return -1;

    // Find free enemy slot
    int slot = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].enemy_type == 0) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;

    // Initialize enemy
    enemies[slot].enemy_type = static_cast<uint8>(type);
    enemies[slot].ai_state = EnemyAIState::IDLE;
    enemies[slot].movement = EnemyMovement::CHASE_PLAYER;
    enemies[slot].damage = 1;
    enemies[slot].knockback_timer = 0;
    enemies[slot].invincibility = 0;
    enemies[slot].stun_timer = 0;
    enemies[slot].death_timer = 0;
    enemies[slot].drop_item = 0;

    active_enemy_count++;
    return slot;
}

// ------------------------------------------------------------------
// enemy_update_all: Update all active enemies
//
// Called from entity dispatch ($01:A87D) when entity category
// is ENEMY (0x30). Each enemy is updated with its type-specific
// AI behavior.
// ------------------------------------------------------------------
void enemy_update_all(void) {
    for (int slot = 0; slot < MAX_ENEMIES; slot++) {
        if (enemies[slot].enemy_type == 0) continue;

        // Update invincibility timer
        if (enemies[slot].invincibility > 0) {
            enemies[slot].invincibility--;
        }

        // Update knockback
        if (enemies[slot].knockback_timer > 0) {
            enemies[slot].knockback_timer--;
            enemy_apply_velocity(slot);
            continue;
        }

        // Update stun timer
        if (enemies[slot].stun_timer > 0) {
            enemies[slot].stun_timer--;
            continue;
        }

        // Update death timer
        if (enemies[slot].death_timer > 0) {
            enemies[slot].death_timer--;
            if (enemies[slot].death_timer == 0) {
                enemy_drop_item(slot);
                enemy_remove(slot);
            }
            continue;
        }

        // Run AI
        enemy_update_ai(slot);

        // Apply movement
        enemy_apply_velocity(slot);
    }
}

// ------------------------------------------------------------------
// enemy_update_ai: Dispatch to type-specific AI behavior
//
// $01:A87D dispatches through the entity table. For enemies
// (category 0x30), the handler calls into type-specific code
// based on the lower nibble of the type byte.
// ------------------------------------------------------------------
void enemy_update_ai(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    uint8 subtype = enemies[slot].enemy_type & 0x0F;

    switch (subtype) {
        case 0x00: enemy_behavior_eyegore(slot); break;
        case 0x01: enemy_behavior_eyegore(slot); break;
        case 0x02: enemy_behavior_stalfos(slot); break;
        case 0x03: enemy_behavior_hinox(slot); break;
        case 0x04: enemy_behavior_moblin(slot); break;
        case 0x05: enemy_behavior_darknut(slot); break;
        case 0x06: enemy_behavior_wizzrobe(slot); break;
        case 0x08: enemy_behavior_keese(slot); break;
        case 0x09: enemy_behavior_gel(slot); break;
        case 0x0A: enemy_behavior_gel(slot); break;
        case 0x0B: enemy_behavior_boss(slot); break;
        case 0x0C: enemy_behavior_boss(slot); break;
        case 0x0D: enemy_behavior_boss(slot); break;
        case 0x0E: enemy_behavior_boss(slot); break;
        case 0x0F: enemy_behavior_boss(slot); break;
        default:   enemy_behavior_moblin(slot); break;
    }
}

// ------------------------------------------------------------------
// Movement functions
// ------------------------------------------------------------------

// enemy_move_chase: Move toward target position
// $01:8A89-$01:8AFC: enemy chase behavior
void enemy_move_chase(int slot, int target_x, int target_y) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    int16 dx = target_x - entities[slot].x;
    int16 dy = target_y - entities[slot].y;

    // Normalize to unit direction
    if (dx > 0) entities[slot].vel_x = 1;
    else if (dx < 0) entities[slot].vel_x = -1;
    else entities[slot].vel_x = 0;

    if (dy > 0) entities[slot].vel_y = 1;
    else if (dy < 0) entities[slot].vel_y = -1;
    else entities[slot].vel_y = 0;
}

// enemy_move_patrol: Move within patrol bounds
void enemy_move_patrol(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    // Simple horizontal patrol
    entities[slot].vel_x = (entities[slot].x < enemies[slot].patrol_x_max) ? 1 : -1;
}

// enemy_move_random: Random direction changes
void enemy_move_random(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    // Random velocity every few frames
    if (entities[slot].anim_timer == 0) {
        entities[slot].vel_x = (rand() % 3) - 1;  // -1, 0, or 1
        entities[slot].vel_y = (rand() % 3) - 1;
        entities[slot].anim_timer = 30 + (rand() % 60);
    }
    entities[slot].anim_timer--;
}

// enemy_apply_velocity: Move entity by velocity
void enemy_apply_velocity(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    if (enemies[slot].knockback_timer > 0) {
        // Apply knockback velocity
        entities[slot].x += enemies[slot].knockback_dir * 2;
    } else {
        // Apply normal velocity
        entities[slot].x += entities[slot].vel_x;
        entities[slot].y += entities[slot].vel_y;
    }
}

// ------------------------------------------------------------------
// Combat functions
// ------------------------------------------------------------------

// enemy_take_damage: Reduce enemy health
void enemy_take_damage(int slot, uint8 damage) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    if (enemies[slot].invincibility > 0) return;

    if (entities[slot].health > damage) {
        entities[slot].health -= damage;
        enemies[slot].invincibility = 16;  // brief invincibility
        enemies[slot].ai_state = EnemyAIState::HURT;
    } else {
        entities[slot].health = 0;
        enemies[slot].ai_state = EnemyAIState::DYING;
        enemies[slot].death_timer = 30;  // death animation
    }
}

// enemy_apply_knockback: Push enemy in direction
void enemy_apply_knockback(int slot, int direction) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    enemies[slot].knockback_timer = 8;
    enemies[slot].knockback_dir = direction;
}

// enemy_check_death: Check if enemy should die
void enemy_check_death(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    if (entities[slot].health == 0 &&
        enemies[slot].ai_state == EnemyAIState::DYING) {
        // Already dying, let death timer run
    }
}

// enemy_get_contact_damage: Get damage dealt on contact
uint8 enemy_get_contact_damage(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return 0;
    return enemies[slot].damage;
}

// enemy_drop_item: Spawn item on enemy death
void enemy_drop_item(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    if (enemies[slot].drop_item != 0) {
        // Spawn dropped item at enemy position
        // entity_spawn(enemies[slot].drop_item, entities[slot].x, entities[slot].y, 0);
    }
}

// ------------------------------------------------------------------
// enemy_remove: Remove enemy from active list
// ------------------------------------------------------------------
void enemy_remove(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    memset(&enemies[slot], 0, sizeof(Enemy));
    if (active_enemy_count > 0) active_enemy_count--;
}

// ------------------------------------------------------------------
// Enemy type-specific behavior stubs
// Each implements the AI for a specific enemy type.
// ------------------------------------------------------------------

void enemy_behavior_eyegore(int slot) {
    // Eyegore: stares at player, charges when looked at
    // $01:8A89-$01:8AFC
    if (slot < 0 || slot >= MAX_ENEMIES) return;

    switch (enemies[slot].ai_state) {
        case EnemyAIState::IDLE:
            // Face player, wait for eye contact
            enemies[slot].ai_state = EnemyAIState::CHASE;
            break;
        case EnemyAIState::CHASE:
            enemy_move_chase(slot, shadow.scroll_bg1_h, shadow.scroll_bg1_v);
            break;
        default:
            break;
    }
}

void enemy_behavior_stalfos(int slot) {
    // Stalfos: teleports and throws bones
    // $01:8B74
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_random(slot);
}

void enemy_behavior_hinox(int slot) {
    // Hinox: charges and throws bombs
    // $01:8B79
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_chase(slot, shadow.scroll_bg1_h, shadow.scroll_bg1_v);
}

void enemy_behavior_moblin(int slot) {
    // Moblin: walks in patterns, throws spears
    // $01:8A89
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_patrol(slot);
}

void enemy_behavior_darknut(int slot) {
    // Darknut: blocks attacks, charges with sword
    // $01:8AC9
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_chase(slot, shadow.scroll_bg1_h, shadow.scroll_bg1_v);
}

void enemy_behavior_wizzrobe(int slot) {
    // Wizzrobe: teleports and shoots beams
    // $01:8AD7
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    // Teleport logic
}

void enemy_behavior_keese(int slot) {
    // Keese: erratic bat movement
    // $01:8AF9
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_random(slot);
}

void enemy_behavior_gel(int slot) {
    // Gel/Zol: hop toward player
    // $01:8B03
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_chase(slot, shadow.scroll_bg1_h, shadow.scroll_bg1_v);
}

void enemy_behavior_boss(int slot) {
    // Boss enemies: complex multi-phase patterns
    // $01:8B74-$01:8B7E
    if (slot < 0 || slot >= MAX_ENEMIES) return;
    enemy_move_chase(slot, shadow.scroll_bg1_h, shadow.scroll_bg1_v);
}

// ------------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------------

bool enemy_is_alive(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return false;
    return enemies[slot].enemy_type != 0 &&
           enemies[slot].ai_state != EnemyAIState::DEAD;
}

bool enemy_is_invincible(int slot) {
    if (slot < 0 || slot >= MAX_ENEMIES) return false;
    return enemies[slot].invincibility > 0;
}

int enemy_get_slot_for_entity(int entity_slot) {
    // Map entity slot to enemy slot
    // In the ROM, entity slots and enemy slots are separate arrays
    // This is a placeholder mapping
    if (entity_slot >= 0 && entity_slot < MAX_ENEMIES) {
        return entity_slot;
    }
    return -1;
}
