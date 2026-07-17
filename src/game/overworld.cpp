#include "overworld.h"
#include "entity.h"
#include "enemy.h"
#include "collision.h"
#include "sound.h"
#include "../game/state.h"
#include <cstdio>

// ------------------------------------------------------------------
// Overworld System Implementation
//
// Converted from Bank $02 ($02:8992) overworld main loop.
//
// The overworld uses a mode-based state machine:
//   Mode 0: Init — load screen data, initialize scroll
//   Mode 1: Running — normal gameplay
//   Mode 2: Transition — scrolling between screens
//   Mode 3+: Special modes (boss, cutscene, etc.)
//
// Key addresses:
//   $02:8992 — JMP ($896C,X): overworld mode dispatch
//   $02:89F0 — scroll update entry point
//   $02:8B2E — scroll register management
//   $02:8995 — STZ $0468: clear init flag
//   $02:8998 — LDA #$07; STA $0690: init transition counter
// ------------------------------------------------------------------

extern GlobalState shadow;
extern EntityTables entity_tables;

OverworldState overworld;

// ------------------------------------------------------------------
// overworld_init: Initialize overworld state
// $02:8995-$02:89B5:
//   STZ $0468       — clear init flag
//   LDA #$07         — initial transition counter
//   STA $0690        — store transition counter
//   LDA $11          — save current mode
//   PHA
//   JSL $01D38F      — call Bank $01 init routine
//   PLA; STA $11     — restore mode
// ------------------------------------------------------------------
void overworld_init(void) {
    overworld.current_screen_x = 0;
    overworld.current_screen_y = 0;
    overworld.init_flag = 0;
    overworld.transition_type = 0;
    overworld.transition_phase = 0;
    overworld.scroll_speed = 1;

    shadow.scroll_bg1_h = 0;
    shadow.scroll_bg1_v = 0;
    shadow.scroll_bg2_h = 0;
    shadow.scroll_bg2_v = 0;

    printf("Overworld initialized\n");
}

// ------------------------------------------------------------------
// overworld_load_screen: Load screen data from ROM
//
// Reads tilemap, entities, and collision data for the specified
// screen coordinates. The ROM stores screen data in a grid
// format with each screen occupying a fixed ROM region.
// ------------------------------------------------------------------
void overworld_load_screen(int screen_x, int screen_y) {
    if (screen_x < 0 || screen_x >= OVERWORLD_SCREEN_W) return;
    if (screen_y < 0 || screen_y >= OVERWORLD_SCREEN_H) return;

    overworld.current_screen_x = screen_x;
    overworld.current_screen_y = screen_y;

    // Compute scroll position for this screen
    overworld.scroll_target_x = screen_x * 256;
    overworld.scroll_target_y = screen_y * 224;

    // Load tilemap, entities, collision for this screen
    // (ROM data access would go here)

    printf("Loaded screen (%d, %d)\n", screen_x, screen_y);
}

// ------------------------------------------------------------------
// overworld_update: $02:8992 main overworld loop
//
// $02:8992: JMP ($896C,X)
// X = mode × 2, dispatches to mode handler
//
// Mode table at $02:896C:
//   Mode 0: Init
//   Mode 1: Normal gameplay
//   Mode 2: Screen transition
//   Mode 3: Scroll update
//   Mode 4: Entity processing
//   Mode 5: Collision check
//   Mode 6: Sound update
//
// $02:89F0: scroll update entry
//   JSR $8D71 — update BG scroll registers
//   LDA $0468 — check init flag
//   BNE $8A05 — skip if already running
//   INC $0468 — set init flag
//   STZ $068E — clear secondary counter
//   STZ $0690 — clear transition counter
//   LDA #$05  — set game mode to 5
//   STA $11   — store in game mode
// ------------------------------------------------------------------
void overworld_update(void) {
    // Update scroll position
    overworld_update_scroll();

    // Check for screen transition
    if (overworld.transition_type != 0) {
        overworld_update_transition();
        return;
    }

    // Spawn/despawn entities based on scroll position
    overworld_despawn_offscreen_entities();
}

// ------------------------------------------------------------------
// overworld_update_scroll: $02:8B2E scroll register updates
//
// $02:8B2E: REP #$10 — 16-bit XY
// $02:8B30: LDX $E2; STX $E0  — save BG1 scroll X
// $02:8B34: LDX $E8; STX $E6  — save BG2 scroll X
// $02:8B38: LDX $A0; CPX #$36 — check screen boundary
//
// The scroll system tracks:
//   $E0/$E1: BG1 horizontal scroll (shadow)
//   $E8/$E9: BG1 vertical scroll (shadow)
//   $E2/$E3: BG2 horizontal scroll (shadow)
//   $EA/$EB: BG2 vertical scroll (shadow)
//
// $02:8B49: LDY #$16 — loop counter for scroll updates
// $02:8B4B: LDA $02894C,X — read scroll data table
// ------------------------------------------------------------------
void overworld_update_scroll(void) {
    // Smooth scroll toward target
    if (shadow.scroll_bg1_h < overworld.scroll_target_x) {
        shadow.scroll_bg1_h += overworld.scroll_speed;
    } else if (shadow.scroll_bg1_h > overworld.scroll_target_x) {
        shadow.scroll_bg1_h -= overworld.scroll_speed;
    }

    if (shadow.scroll_bg1_v < overworld.scroll_target_y) {
        shadow.scroll_bg1_v += overworld.scroll_speed;
    } else if (shadow.scroll_bg1_v > overworld.scroll_target_y) {
        shadow.scroll_bg1_v -= overworld.scroll_speed;
    }

    // Clamp to screen bounds
    uint16 max_x = (OVERWORLD_SCREEN_W - 1) * 256;
    uint16 max_y = (OVERWORLD_SCREEN_H - 1) * 224;

    if (shadow.scroll_bg1_h > max_x) shadow.scroll_bg1_h = max_x;
    if (shadow.scroll_bg1_v > max_y) shadow.scroll_bg1_v = max_y;
}

// ------------------------------------------------------------------
// overworld_start_transition: Begin screen transition
//
// $02:9736: JMP ($9720,X)
// X = transition type × 2
// The transition system scrolls the camera smoothly from one
// screen to the next, loading new tilemap/entity data during
// the scroll.
// ------------------------------------------------------------------
void overworld_start_transition(ScreenTransition type) {
    if (overworld.transition_type != 0) return;  // already transitioning

    overworld.transition_type = static_cast<uint8>(type);
    overworld.transition_phase = 0;

    // Calculate target scroll position
    switch (type) {
        case ScreenTransition::SCROLL_LEFT:
            overworld.transition_scroll_x = overworld.scroll_target_x - 256;
            overworld.transition_scroll_y = overworld.scroll_target_y;
            break;
        case ScreenTransition::SCROLL_RIGHT:
            overworld.transition_scroll_x = overworld.scroll_target_x + 256;
            overworld.transition_scroll_y = overworld.scroll_target_y;
            break;
        case ScreenTransition::SCROLL_UP:
            overworld.transition_scroll_x = overworld.scroll_target_x;
            overworld.transition_scroll_y = overworld.scroll_target_y - 224;
            break;
        case ScreenTransition::SCROLL_DOWN:
            overworld.transition_scroll_x = overworld.scroll_target_x;
            overworld.transition_scroll_y = overworld.scroll_target_y + 224;
            break;
        default:
            break;
    }

    // Update screen coordinates
    switch (type) {
        case ScreenTransition::SCROLL_LEFT:
            if (overworld.current_screen_x > 0) overworld.current_screen_x--;
            break;
        case ScreenTransition::SCROLL_RIGHT:
            if (overworld.current_screen_x < OVERWORLD_SCREEN_W - 1) overworld.current_screen_x++;
            break;
        case ScreenTransition::SCROLL_UP:
            if (overworld.current_screen_y > 0) overworld.current_screen_y--;
            break;
        case ScreenTransition::SCROLL_DOWN:
            if (overworld.current_screen_y < OVERWORLD_SCREEN_H - 1) overworld.current_screen_y++;
            break;
        default:
            break;
    }

    // Load new screen data during transition
    overworld_load_screen(overworld.current_screen_x, overworld.current_screen_y);
}

// ------------------------------------------------------------------
// overworld_update_transition: Animate screen transition
// $02:9736 transition handler
// ------------------------------------------------------------------
void overworld_update_transition(void) {
    if (overworld.transition_type == 0) return;

    // Smooth scroll toward transition target
    bool done = true;

    if (shadow.scroll_bg1_h != overworld.transition_scroll_x) {
        if (shadow.scroll_bg1_h < overworld.transition_scroll_x) {
            shadow.scroll_bg1_h += 2;
        } else {
            shadow.scroll_bg1_h -= 2;
        }
        done = false;
    }

    if (shadow.scroll_bg1_v != overworld.transition_scroll_y) {
        if (shadow.scroll_bg1_v < overworld.transition_scroll_y) {
            shadow.scroll_bg1_v += 2;
        } else {
            shadow.scroll_bg1_v -= 2;
        }
        done = false;
    }

    if (done) {
        // Transition complete
        overworld.scroll_target_x = overworld.transition_scroll_x;
        overworld.scroll_target_y = overworld.transition_scroll_y;
        overworld.transition_type = 0;
        overworld.transition_phase = 0;
    }
}

// ------------------------------------------------------------------
// overworld_spawn_screen_entities: Spawn entities for current screen
// ------------------------------------------------------------------
void overworld_spawn_screen_entities(void) {
    // Read entity list for current screen from ROM
    // Spawn each entity at specified position
}

// ------------------------------------------------------------------
// overworld_despawn_offscreen_entities: Remove entities outside view
// ------------------------------------------------------------------
void overworld_despawn_offscreen_entities(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!entity_slot_active(i)) continue;

        // Check if entity is too far from scroll position
        int16 dx = entities[i].x - shadow.scroll_bg1_h;
        int16 dy = entities[i].y - shadow.scroll_bg1_v;

        if (dx < -64 || dx > 320 || dy < -64 || dy > 288) {
            entity_remove(i);
        }
    }
}

// ------------------------------------------------------------------
// overworld_get_tile_collision: Read collision map for tile
// ------------------------------------------------------------------
uint8 overworld_get_tile_collision(int x, int y) {
    // Read collision byte from WRAM collision map
    // Each tile has 2-bit collision type
    (void)x; (void)y;
    return 0;
}

// ------------------------------------------------------------------
// overworld_update_area_sound: Update music for current area
// ------------------------------------------------------------------
void overworld_update_area_sound(void) {
    // Play area-specific music based on area_id
}
