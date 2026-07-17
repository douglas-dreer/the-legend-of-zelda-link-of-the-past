#ifndef DUNGEON_H
#define DUNGEON_H

#include "../types.h"
#include "room.h"

// ------------------------------------------------------------------
// Dungeon System — Bank $02 ($02:8A06-$02:8C4C) + Bank $09
//
// Dungeons are room-based (not screen-scrolling like overworld).
// Each dungeon room has:
//   - Room header (tileset, palette, entity list)
//   - Wall/floor data (tilemap)
//   - Collision properties
//   - Puzzle/switch state
//   - Door/entrance/exit definitions
//
// Bank $02 handles dungeon mode logic:
//   $02:8A06: mode dispatch table (JMP ($8A06,X))
//   $02:8A0B-$02:8A6C: dungeon sub-modes
//   $02:8B0C: room property load ($09AC6B JSL)
//   $02:8B2E: scroll updates for dungeon rooms
//
// Bank $09 handles room-specific logic:
//   $09:813A: room entity setup
//   $09:9E44: room state initialization
//   $09:A200: room transition handler
//   $09:BB67: room property table
//
// Dungeon rooms use fixed scroll positions (no smooth scroll
// between rooms — instead, fade/wipe transitions).
// ------------------------------------------------------------------

// Dungeon constants
constexpr int MAX_DUNGEON_ROOMS = 64;
constexpr int MAX_DOORS = 8;
constexpr int MAX_SWITCHES = 4;

// Dungeon room types
enum class DungeonRoomType : uint8 {
    NORMAL,
    BOSS_ROOM,
    ITEM_ROOM,
    HIDDEN_ROOM,
    ENTRANCE,
    EXIT,
    CHALLENGE_ROOM,
    MINI_BOSS,
};

// Door types
enum class DoorType : uint8 {
    NONE,
    OPEN,           // Always open
    LOCKED,         // Requires key
    BOSS_KEY,       // Requires boss key
    SHUTTER,        // Closes behind player
    BOMBABLE,       // Can be bombed open
    SWITCH_DOOR,    // Opens with switch
    ONE_WAY,        // Can only enter from one side
};

// Switch types
enum class SwitchType : uint8 {
    NONE,
    FLOOR_SWITCH,   // Step on to activate
    TILE_SWITCH,    // Hit with sword/bomb
    CRYSTAL,        // Carry and place
    TORCH,          // Light with fire
};

// Dungeon door structure
struct DungeonDoor {
    uint8  door_type;      // DoorType
    uint8  x;              // Tile X position
    uint8  y;              // Tile Y position
    uint8  direction;      // 0=left, 1=right, 2=up, 3=down
    uint16 target_room;    // Room ID on other side
    uint8  target_x;       // Spawn X in target room
    uint8  target_y;       // Spawn Y in target room
    bool   is_open;        // Current open state
};

// Dungeon switch structure
struct DungeonSwitch {
    uint8  switch_type;    // SwitchType
    uint8  x;              // Tile X position
    uint8  y;              // Tile Y position
    bool   activated;      // Current state
    uint8  linked_door;    // Door index this switch affects
};

// Dungeon room state
struct DungeonRoomState {
    uint8  room_id;        // Current room ID
    uint8  room_type;      // DungeonRoomType
    uint8  floor;          // Floor level (B1=1, 1F=2, etc.)
    uint8  chest_count;    // Number of chests
    uint8  chest_opened;   // Bitmask of opened chests
    bool   cleared;        // Room cleared of enemies
    uint8  puzzle_state;   // Puzzle completion state
    uint8  warp_type;      // Warp/teleport type
};

// Dungeon state
struct DungeonState {
    uint8  current_floor;          // Current floor level
    uint8  current_room;           // Current room index
    uint8  prev_room;              // Previous room (for backtracking)
    uint16 boss_defeated_flags;    // Bitmask of defeated bosses
    uint8  keys_collected;         // Number of small keys
    bool   has_boss_key;           // Boss key flag
    bool   has_map;                // Dungeon map flag
    bool   has_compass;            // Compass flag
    uint8  transition_timer;       // Room transition timer
    uint8  transition_type;        // Transition effect type
    uint8  overlay_state;          // Room overlay/effect state
    uint8  torch_timer;            // Torch light timer
    uint8  dark_room_flag;         // Is room in darkness
    DungeonRoomState room;
};

// Global dungeon state
extern DungeonState dungeon;
extern DungeonDoor dungeon_doors[MAX_DOORS];
extern DungeonSwitch dungeon_switches[MAX_SWITCHES];

// Dungeon Functions

// Initialize dungeon state
void dungeon_init(void);

// Load dungeon room
void dungeon_load_room(uint8 room_id);

// Main dungeon update — called each frame from Bank $02
// $02:8A06: JMP ($8A06,X) — dungeon mode dispatch
void dungeon_update(void);

// Room transition
void dungeon_start_room_transition(uint8 door_index);
void dungeon_update_room_transition(void);

// Door management
void dungeon_open_door(int door_index);
void dungeon_close_door(int door_index);
void dungeon_check_door_collisions(void);

// Switch management
void dungeon_activate_switch(int switch_index);
void dungeon_deactivate_switch(int switch_index);
void dungeon_update_switches(void);

// Chest management
void dungeon_open_chest(int chest_index);
bool dungeon_is_chest_open(int index);

// Room clear check
bool dungeon_is_room_cleared(void);

// Dark room / torch system
void dungeon_update_torches(void);
void dungeon_enter_dark_room(void);
void dungeon_exit_dark_room(void);

// Puzzle state
void dungeon_set_puzzle_state(uint8 state);

// Overlay effects (water, sand, etc.)
void dungeon_update_overlay(void);

#endif // DUNGEON_H
