#include "dungeon.h"
#include "entity.h"
#include "enemy.h"
#include "collision.h"
#include "sound.h"
#include "../game/state.h"
#include <cstdio>
#include <cstring>

// ------------------------------------------------------------------
// Dungeon System Implementation
//
// Converted from Bank $02 dungeon modes and Bank $09 room logic.
//
// Bank $02 dungeon mode dispatch ($02:8A06):
//   JMP ($8A06,X) — mode table indexed by $B0
//   Mode 0: Init dungeon, load first room
//   Mode 1: Normal dungeon gameplay
//   Mode 2: Room transition (fade/wipe)
//   Mode 3: Boss intro cutscene
//   Mode 4: Post-boss room
//   Mode 5: Dark room update
//   Mode 6: Switch/door puzzle
//   Mode 7: Item get cutscene
//
// Bank $09 room logic:
//   $09:813A: entity setup — reads room entity list from ROM
//   $09:9E44: state init — initializes room-specific state
//   $09:A200: transition handler — manages room-to-room movement
//   $09:BB67: property table — collision/puzzle data per room
// ------------------------------------------------------------------

extern GlobalState shadow;
extern EntityTables entity_tables;

DungeonState dungeon;
DungeonDoor dungeon_doors[MAX_DOORS];
DungeonSwitch dungeon_switches[MAX_SWITCHES];

// ------------------------------------------------------------------
// dungeon_init: Initialize dungeon state
// ------------------------------------------------------------------
void dungeon_init(void) {
    memset(&dungeon, 0, sizeof(DungeonState));
    memset(dungeon_doors, 0, sizeof(DungeonDoor) * MAX_DOORS);
    memset(dungeon_switches, 0, sizeof(DungeonSwitch) * MAX_SWITCHES);
    dungeon.current_floor = 2;  // 1F default
    printf("Dungeon initialized\n");
}

// ------------------------------------------------------------------
// dungeon_load_room: Load room data from ROM
//
// $09:813A room entity setup:
//   JSL $0DDB7F  — load room header from ROM
//   PLX
//   STZ $03EA,X  — clear entity timer
//   STZ $0C54,X  — clear entity state
//   STZ $0C5E,X  — clear entity aux
//   STZ $03C2,X  — clear entity collision
//   STZ $0385,X  — clear entity type
//   LDA $9543     — load room type
//   STA $039F,X   — store room type
//   LDA #$07      — max entities
//   STA $03C0,X
//
// The room loading process:
//   1. Read room header (tileset, palette, entities)
//   2. Load tileset graphics to VRAM
//   3. Load tilemap data
//   4. Load collision map
//   5. Spawn room entities
//   6. Initialize switches/doors
// ------------------------------------------------------------------
void dungeon_load_room(uint8 room_id) {
    dungeon.room.room_id = room_id;
    dungeon.room.cleared = false;
    dungeon.room.puzzle_state = 0;
    dungeon.room.chest_opened = 0;

    dungeon.current_room = room_id;

    // Read room header from ROM
    // (ROM data access would go here)

    // Clear existing entities
    entity_remove_all();
    enemy_init();

    // Load tileset and tilemap
    // (PPU/DMA operations would go here)

    // Spawn room entities from entity list
    // $09:813A reads entity data and calls entity_spawn for each

    printf("Loaded dungeon room %d\n", room_id);
}

// ------------------------------------------------------------------
// dungeon_update: $02:8A06 main dungeon loop
//
// $02:8A06: JMP ($8A06,X)
// X = $B0 × 2 (dungeon sub-mode)
//
// Sub-modes:
//   0: Init — load room, set up scroll, spawn entities
//   1: Normal — run entity AI, check doors, update switches
//   2: Transition — room-to-room transition animation
//   3: Boss intro — special cutscene for boss rooms
//   4: Item get — player receiving item animation
//   5: Dark room — torch/darkness system
//   6: Puzzle — switch/door puzzle logic
//   7: Overlay — water/sand overlay effect
//
// $02:8AD5: check room clear
//   LDA $0200 — load room state
//   CMP #$05  — check if cleared
//   BNE $8B2D — skip if not
// ------------------------------------------------------------------
void dungeon_update(void) {
    // Update room-specific logic
    dungeon_update_switches();
    dungeon_check_door_collisions();

    // Update dark room / torch system
    if (dungeon.dark_room_flag) {
        dungeon_update_torches();
    }

    // Update overlay effects
    dungeon_update_overlay();
}

// ------------------------------------------------------------------
// dungeon_start_room_transition: Begin fade/wipe to new room
//
// $09:A200: JSL $099E90 — room transition handler
// The transition system fades or wipes the screen, then loads
// the new room data and spawns entities.
// ------------------------------------------------------------------
void dungeon_start_room_transition(uint8 door_index) {
    if (door_index >= MAX_DOORS) return;
    if (!dungeon_doors[door_index].is_open) return;

    dungeon.prev_room = dungeon.current_room;
    dungeon.room.room_id = dungeon_doors[door_index].target_room;
    dungeon.transition_timer = 30;  // frames for transition
    dungeon.transition_type = 1;    // fade transition
}

// ------------------------------------------------------------------
// dungeon_update_room_transition: Animate room transition
// ------------------------------------------------------------------
void dungeon_update_room_transition(void) {
    if (dungeon.transition_timer == 0) return;

    dungeon.transition_timer--;

    if (dungeon.transition_timer == 0) {
        // Load new room
        dungeon_load_room(dungeon.room.room_id);

        // Set player spawn position from door data
        // (would read from door target_x/target_y)
    }
}

// ------------------------------------------------------------------
// dungeon_open_door: Open a door
// ------------------------------------------------------------------
void dungeon_open_door(int door_index) {
    if (door_index < 0 || door_index >= MAX_DOORS) return;
    dungeon_doors[door_index].is_open = true;
    play_sfx(0x12);  // door open sound
}

// ------------------------------------------------------------------
// dungeon_close_door: Close a door
// ------------------------------------------------------------------
void dungeon_close_door(int door_index) {
    if (door_index < 0 || door_index >= MAX_DOORS) return;
    dungeon_doors[door_index].is_open = false;
}

// ------------------------------------------------------------------
// dungeon_check_door_collisions: Check if player is near a door
// ------------------------------------------------------------------
void dungeon_check_door_collisions(void) {
    for (int i = 0; i < MAX_DOORS; i++) {
        if (!dungeon_doors[i].is_open) continue;
        if (dungeon_doors[i].door_type == static_cast<uint8>(DoorType::SHUTTER)) {
            // Check if player passed through
            // Auto-close shutter doors behind player
        }
    }
}

// ------------------------------------------------------------------
// dungeon_activate_switch: Activate a switch
// ------------------------------------------------------------------
void dungeon_activate_switch(int switch_index) {
    if (switch_index < 0 || switch_index >= MAX_SWITCHES) return;
    dungeon_switches[switch_index].activated = true;

    // Open linked door
    int linked = dungeon_switches[switch_index].linked_door;
    if (linked >= 0 && linked < MAX_DOORS) {
        dungeon_open_door(linked);
    }

    play_sfx(0x15);  // switch activation sound
}

// ------------------------------------------------------------------
// dungeon_deactivate_switch: Deactivate a switch
// ------------------------------------------------------------------
void dungeon_deactivate_switch(int switch_index) {
    if (switch_index < 0 || switch_index >= MAX_SWITCHES) return;
    dungeon_switches[switch_index].activated = false;

    // Close linked door
    int linked = dungeon_switches[switch_index].linked_door;
    if (linked >= 0 && linked < MAX_DOORS) {
        dungeon_close_door(linked);
    }
}

// ------------------------------------------------------------------
// dungeon_update_switches: Check switch states
// ------------------------------------------------------------------
void dungeon_update_switches(void) {
    for (int i = 0; i < MAX_SWITCHES; i++) {
        if (dungeon_switches[i].switch_type == 0) continue;

        // Check if player is on floor switch
        if (dungeon_switches[i].switch_type ==
            static_cast<uint8>(SwitchType::FLOOR_SWITCH)) {
            // Check player position against switch position
            // Activate if player is standing on it
        }
    }
}

// ------------------------------------------------------------------
// dungeon_open_chest: Mark chest as opened
// ------------------------------------------------------------------
void dungeon_open_chest(int chest_index) {
    if (chest_index < 0 || chest_index > 7) return;
    dungeon.room.chest_opened |= (1 << chest_index);
    play_sfx(0x13);  // chest open sound
}

// ------------------------------------------------------------------
// dungeon_is_chest_open: Check if chest has been opened
// ------------------------------------------------------------------
bool dungeon_is_chest_open(int index) {
    if (index < 0 || index > 7) return false;
    return (dungeon.room.chest_opened & (1 << index)) != 0;
}

// ------------------------------------------------------------------
// dungeon_is_room_cleared: Check if all enemies are defeated
// ------------------------------------------------------------------
bool dungeon_is_room_cleared(void) {
    return dungeon.room.cleared;
}

// ------------------------------------------------------------------
// dungeon_update_torches: Torch/darkness system
// $09:BB67 room property system
// ------------------------------------------------------------------
void dungeon_update_torches(void) {
    if (dungeon.torch_timer > 0) {
        dungeon.torch_timer--;
        if (dungeon.torch_timer == 0) {
            // Torches go out — room becomes dark
            // Player takes damage in darkness
        }
    }
}

// ------------------------------------------------------------------
// dungeon_enter_dark_room: Enable darkness effect
// ------------------------------------------------------------------
void dungeon_enter_dark_room(void) {
    dungeon.dark_room_flag = 1;
    dungeon.torch_timer = 0;
    // Apply darkness overlay to BG layers
}

// ------------------------------------------------------------------
// dungeon_exit_dark_room: Disable darkness effect
// ------------------------------------------------------------------
void dungeon_exit_dark_room(void) {
    dungeon.dark_room_flag = 0;
    // Remove darkness overlay
}

// ------------------------------------------------------------------
// dungeon_set_puzzle_state: Update puzzle completion state
// ------------------------------------------------------------------
void dungeon_set_puzzle_state(uint8 state) {
    dungeon.room.puzzle_state = state;
}

// ------------------------------------------------------------------
// dungeon_update_overlay: Water/sand overlay effects
// ------------------------------------------------------------------
void dungeon_update_overlay(void) {
    // Some rooms have water or sand overlays that affect
    // player movement speed and entity behavior
    if (dungeon.overlay_state == 0) return;

    // Apply overlay effect based on state
}
