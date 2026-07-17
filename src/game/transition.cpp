#include "transition.h"
#include "overworld.h"
#include "dungeon.h"
#include "../game/state.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include <cstdio>

// ------------------------------------------------------------------
// Screen Transition System Implementation
//
// Converted from Bank $02 ($02:9736) transition dispatch.
//
// The transition system manages visual effects between screens:
//   1. Start effect (iris, fade, wipe, scroll)
//   2. Load new screen data during effect
//   3. Animate the visual transition
//   4. Complete and resume normal gameplay
//
// HDMA effects are used for smooth animation:
//   - Iris: window position HDMA
//   - Fades: color math HDMA
//   - Wipes: scroll/window HDMA
// ------------------------------------------------------------------

extern GlobalState shadow;

TransitionState transition;

// Forward declarations for internal animation functions
static void transition_update_fade_black(void);
static void transition_update_fade_white(void);
static void transition_update_iris(void);
static void transition_update_wipe(void);

// ------------------------------------------------------------------
// transition_init: Initialize transition state
// ------------------------------------------------------------------
void transition_init(void) {
    transition = {};
    transition.type = TransitionType::NONE;
    transition.phase = TransitionPhase::NONE;
}

// ------------------------------------------------------------------
// transition_start: Begin transition effect
//
// $02:9736: JMP ($9720,X)
// X = transition type × 2
// Each type has its own HDMA setup and animation logic.
// ------------------------------------------------------------------
void transition_start(TransitionType type) {
    transition.type = type;
    transition.phase = TransitionPhase::START;
    transition.timer = 0;
    transition.screen_loaded = false;

    // Store starting scroll position
    transition.source_scroll_x = shadow.scroll_bg1_h;
    transition.source_scroll_y = shadow.scroll_bg1_v;

    switch (type) {
        case TransitionType::SCROLL_H:
            transition.duration = 64;  // frames
            transition.speed = 4;
            break;
        case TransitionType::SCROLL_V:
            transition.duration = 56;
            transition.speed = 4;
            break;
        case TransitionType::IRIS:
            transition.duration = 48;
            transition.color_step = 0;
            break;
        case TransitionType::FADE_BLACK:
        case TransitionType::FADE_WHITE:
            transition.duration = 32;
            transition.color_step = 0;
            break;
        case TransitionType::WIPE_H:
        case TransitionType::WIPE_V:
            transition.duration = 40;
            transition.speed = 6;
            break;
        default:
            transition.duration = 1;
            break;
    }

    printf("Transition started: type=%d duration=%d\n",
           static_cast<int>(type), transition.duration);
}

// ------------------------------------------------------------------
// transition_update: Animate transition each frame
//
// Called each frame while transition is active.
// Handles scroll, fade, iris, and wipe effects.
// ------------------------------------------------------------------
void transition_update(void) {
    if (transition.type == TransitionType::NONE) return;
    if (transition.phase == TransitionPhase::NONE) return;

    transition.timer++;

    switch (transition.phase) {
        case TransitionPhase::START:
            transition.phase = TransitionPhase::ANIMATING;
            break;

        case TransitionPhase::LOADING:
            // Screen data loaded, start animation
            transition.phase = TransitionPhase::ANIMATING;
            break;

        case TransitionPhase::ANIMATING:
            // Animate the effect
            switch (transition.type) {
                case TransitionType::SCROLL_H:
                case TransitionType::SCROLL_V:
                    // Scroll animation handled by overworld
                    break;
                case TransitionType::FADE_BLACK:
                    transition_update_fade_black();
                    break;
                case TransitionType::FADE_WHITE:
                    transition_update_fade_white();
                    break;
                case TransitionType::IRIS:
                    transition_update_iris();
                    break;
                case TransitionType::WIPE_H:
                case TransitionType::WIPE_V:
                    transition_update_wipe();
                    break;
                default:
                    break;
            }

            // Check if animation complete
            if (transition.timer >= transition.duration) {
                transition.phase = TransitionPhase::COMPLETE;
            }
            break;

        case TransitionPhase::COMPLETE:
            // Clean up and resume normal gameplay
            transition.type = TransitionType::NONE;
            transition.phase = TransitionPhase::NONE;
            transition_complete();
            break;

        default:
            break;
    }
}

// ------------------------------------------------------------------
// transition_is_active: Check if transition is in progress
// ------------------------------------------------------------------
bool transition_is_active(void) {
    return transition.type != TransitionType::NONE;
}

// ------------------------------------------------------------------
// transition_complete: Finalize transition
// ------------------------------------------------------------------
void transition_complete(void) {
    // Clear HDMA channels used for transition
    // Restore normal scroll registers

    printf("Transition complete\n");
}

// ------------------------------------------------------------------
// Scroll transition helpers
// ------------------------------------------------------------------

void transition_start_scroll_h(int direction) {
    transition.direction = direction;

    if (direction == 0) {  // left
        transition.target_scroll_x = transition.source_scroll_x - 256;
        transition.target_scroll_y = transition.source_scroll_y;
    } else {  // right
        transition.target_scroll_x = transition.source_scroll_x + 256;
        transition.target_scroll_y = transition.source_scroll_y;
    }

    transition_start(TransitionType::SCROLL_H);
}

void transition_start_scroll_v(int direction) {
    transition.direction = direction;

    if (direction == 0) {  // up
        transition.target_scroll_x = transition.source_scroll_x;
        transition.target_scroll_y = transition.source_scroll_y - 224;
    } else {  // down
        transition.target_scroll_x = transition.source_scroll_x;
        transition.target_scroll_y = transition.source_scroll_y + 224;
    }

    transition_start(TransitionType::SCROLL_V);
}

// ------------------------------------------------------------------
// Iris transition helpers
// ------------------------------------------------------------------

void transition_start_iris_open(void) {
    transition_start(TransitionType::IRIS);
}

void transition_start_iris_close(void) {
    transition_start(TransitionType::IRIS);
}

// ------------------------------------------------------------------
// Color fade helpers
// ------------------------------------------------------------------

void transition_start_fade_black(void) {
    transition_start(TransitionType::FADE_BLACK);
}

void transition_start_fade_white(void) {
    transition_start(TransitionType::FADE_WHITE);
}

// ------------------------------------------------------------------
// Wipe transition helpers
// ------------------------------------------------------------------

void transition_start_wipe_h(int direction) {
    transition.direction = direction;
    transition_start(TransitionType::WIPE_H);
}

void transition_start_wipe_v(int direction) {
    transition.direction = direction;
    transition_start(TransitionType::WIPE_V);
}

// ------------------------------------------------------------------
// Internal animation update functions
// ------------------------------------------------------------------

// Fade to black: interpolate screen brightness to 0
static void transition_update_fade_black(void) {
    // $02:9736 fade handler
    // Gradually reduce screen brightness via INIDISP register
    // Each step reduces brightness by 1 (from 15 to 0)
    int progress = (transition.timer * 16) / transition.duration;
    if (progress > 15) progress = 15;

    uint8 brightness = 15 - progress;
    // Write brightness to shadow register
    // shadow.screen_display = brightness;
    (void)brightness;
}

// Fade to white: interpolate toward fixed color
static void transition_update_fade_white(void) {
    int progress = (transition.timer * 16) / transition.duration;
    if (progress > 15) progress = 15;

    // Use color math to blend toward white
    // Set CGADSUB to add white fixed color
    (void)progress;
}

// Iris: window position HDMA for circular reveal
static void transition_update_iris(void) {
    // $02:9736 iris handler
    // Use window position registers with HDMA
    // to create circular open/close effect
    int progress = (transition.timer * 100) / transition.duration;
    if (progress > 100) progress = 100;
    (void)progress;
}

// Wipe: horizontal or vertical screen wipe
static void transition_update_wipe(void) {
    // $02:9736 wipe handler
    // Use window position registers to create wipe effect
    int progress = (transition.timer * 256) / transition.duration;
    if (progress > 256) progress = 256;
    (void)progress;
}
