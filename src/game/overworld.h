#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "../types.h"
#include "room.h"

// ------------------------------------------------------------------
// Overworld System — Bank $02 ($02:8992 overworld main loop)
//
// The overworld is a grid of screens (128×112 tiles per screen,
// 16 screens wide × 8 screens tall). Each screen has:
//   - BG1 tilemap (terrain, floor)
//   - BG2 tilemap (objects, trees, buildings)
//   - Entity list (NPCs, enemies, items)
//   - Collision map (2 bits per tile)
//
// Main loop at $02:8992:
//   JMP ($896C,X) — indirect jump through overworld mode table
//   $0468 = mode flag (0=init, 1=running)
//   $0690 = transition counter
//   $0646 = scroll target X
//   $0642 = scroll target Y
//
// The overworld manages:
//   - Screen scroll and camera ($E0-$E9 shadow registers)
//   - Screen transitions (horizontal/vertical scrolling)
//   - Entity spawn/despawn per screen
//   - Music/sound changes per area
// ------------------------------------------------------------------

// Overworld constants
constexpr int OVERWORLD_SCREEN_W = 16;   // screens wide
constexpr int OVERWORLD_SCREEN_H = 8;    // screens tall
constexpr int OVERWORLD_TILE_SIZE = 16;  // pixels per tile

// Overworld area IDs (partial list)
enum class OverworldArea : uint8 {
    HYRULE_CASTLE_FIELD = 0x00,
    LOST_WOODS          = 0x03,
    KAKARIKO_VILLAGE    = 0x04,
    DEATH_MOUNTAIN      = 0x05,
    LAKE_HYLIA          = 0x08,
    MOUNTAIN_VILLAGE    = 0x0A,
    GRAVEYARD           = 0x0B,
    VILLAGE_OF_OUTCAST  = 0x0C,
    MIRROW_POND         = 0x0D,
    HYRULE_CASTLE       = 0x1B,
};

// Screen transition types
enum class ScreenTransition : uint8 {
    NONE,
    SCROLL_LEFT,
    SCROLL_RIGHT,
    SCROLL_UP,
    SCROLL_DOWN,
    FADE_BLACK,
    FADE_WHITE,
    WIPE,
};

// Overworld state
struct OverworldState {
    uint8  current_screen_x;    // Current screen X index
    uint8  current_screen_y;    // Current screen Y index
    uint16 screen_tilemap_addr; // ROM address of current tilemap
    uint8  area_id;             // Area ID for music/sound
    uint8  transition_type;     // Current transition type
    uint8  transition_phase;    // Transition animation phase
    uint16 transition_scroll_x; // Target scroll X during transition
    uint16 transition_scroll_y; // Target scroll Y during transition
    uint8  scroll_speed;        // Scroll speed during transition
    uint8  init_flag;           // $0468: 0=init, 1=running
    uint16 scroll_target_x;     // $0646: scroll target X
    uint16 scroll_target_y;     // $0642: scroll target Y
    uint16 bg2_hofs;            // BG2 horizontal scroll
    uint16 bg2_vofs;            // BG2 vertical scroll
};

// Global overworld state
extern OverworldState overworld;

// Overworld Functions

// Initialize overworld for new game or load
void overworld_init(void);

// Load overworld screen at (screen_x, screen_y)
void overworld_load_screen(int screen_x, int screen_y);

// Main overworld update — called each frame from Bank $02
// $02:8992: JMP ($896C,X) — mode table dispatch
void overworld_update(void);

// Scroll handling — update camera position
// $02:8B2E: scroll register updates
void overworld_update_scroll(void);

// Screen transition — animate between screens
// $02:9736: JMP ($9720,X) — transition type dispatch
void overworld_start_transition(ScreenTransition type);
void overworld_update_transition(void);

// Entity management per screen
void overworld_spawn_screen_entities(void);
void overworld_despawn_offscreen_entities(void);

// Collision
uint8 overworld_get_tile_collision(int x, int y);

// Music/sound management
void overworld_update_area_sound(void);

#endif // OVERWORLD_H
