#ifndef TRANSITION_H
#define TRANSITION_H

#include "../types.h"

// ------------------------------------------------------------------
// Screen Transition System — Bank $02 ($02:9736)
//
// Transitions handle moving between screens (overworld) or
// rooms (dungeon). The ROM uses a mode-based transition system:
//
// $02:9736: JMP ($9720,X)
// X = transition type × 2, dispatches to effect handler
//
// Transition types:
//   0: None / instant
//   1: Horizontal scroll (left/right)
//   2: Vertical scroll (up/down)
//   3: Iris open/close
//   4: Fade to black
//   5: Fade to white
//   6: Horizontal wipe
//   7: Vertical wipe
//
// The transition system uses HDMA effects for smooth animation:
//   - Window position HDMA for iris effects
//   - Color math HDMA for fade effects
//   - Scroll HDMA for wipe effects
//
// Key addresses:
//   $02:9736 — transition type dispatch
//   $02:8B0C — room property load during transition
//   $02:8B2E — scroll register updates
//   $01:B8B4 — transition effect processor
// ------------------------------------------------------------------

// Transition types
enum class TransitionType : uint8 {
    NONE,
    SCROLL_H,         // Horizontal scroll
    SCROLL_V,         // Vertical scroll
    IRIS,             // Iris open/close
    FADE_BLACK,       // Fade to black
    FADE_WHITE,       // Fade to white
    WIPE_H,           // Horizontal wipe
    WIPE_V,           // Vertical wipe
};

// Transition phases
enum class TransitionPhase : uint8 {
    NONE,
    START,            // Begin transition
    LOADING,          // Loading new screen data
    ANIMATING,        // Playing transition effect
    COMPLETE,         // Transition done
};

// Transition state
struct TransitionState {
    TransitionType type;        // Current transition type
    TransitionPhase phase;      // Current phase
    uint8  timer;               // Frame counter
    uint8  duration;            // Total frames for effect
    uint8  speed;               // Scroll/wipe speed
    int16  source_scroll_x;     // Starting scroll X
    int16  source_scroll_y;     // Starting scroll Y
    int16  target_scroll_x;     // Ending scroll X
    int16  target_scroll_y;     // Ending scroll Y
    uint8  hdma_channel;        // HDMA channel for effect
    uint8  color_step;          // Color math step for fades
    uint8  direction;           // Direction (0=left, 1=right, etc.)
    bool   screen_loaded;       // Flag: new screen loaded
};

// Global transition state
extern TransitionState transition;

// Transition Functions

// Initialize transition system
void transition_init(void);

// Start a transition effect
void transition_start(TransitionType type);

// Update transition each frame
void transition_update(void);

// Check if transition is in progress
bool transition_is_active(void);

// Force complete transition (skip animation)
void transition_complete(void);

// Scroll transition helpers
void transition_start_scroll_h(int direction);  // left/right
void transition_start_scroll_v(int direction);  // up/down

// Iris transition helpers
void transition_start_iris_open(void);
void transition_start_iris_close(void);

// Color fade helpers
void transition_start_fade_black(void);
void transition_start_fade_white(void);

// Wipe transition helpers
void transition_start_wipe_h(int direction);
void transition_start_wipe_v(int direction);

#endif // TRANSITION_H
