#include "input_processor.h"
#include "../snes/input.h"
#include "../game/state.h"
#include <cstring>

#ifdef HAS_SDL2
#include "../platform/sdl2_input.h"
#endif

extern InputState joypad1;
extern InputState joypad2;
extern GlobalState shadow;

// Previous frame's held state for edge detection
static uint16 prev_held = 0;

void input_processor_init(void) {
    prev_held = 0;
    memset(&joypad1, 0, sizeof(InputState));
    memset(&joypad2, 0, sizeof(InputState));
}

void input_processor_update(void) {
    uint16 current_held = 0;

#ifdef HAS_SDL2
    current_held = sdl2_input_get_joypad1_state();
#endif

    // Edge detection: newly pressed = current & ~previous
    uint16 newly_pressed = current_held & ~prev_held;

    // Update shadow RAM ($F0-$FB)
    // $F0 = joypad1_held_high, $F2 = joypad1_held_low
    // $F4 = joypad1_new_high, $F6 = joypad1_new_low
    // $F8 = joypad1_prev_high, $FA = joypad1_prev_low
    shadow.joypad1_held = current_held;
    shadow.joypad1_new = newly_pressed;
    shadow.joypad1_prev = prev_held;

    // Update InputState struct for compatibility with is_button_pressed()
    joypad1.held = current_held;
    joypad1.newly = newly_pressed;
    joypad1.previous = prev_held;

    // Store for next frame
    prev_held = current_held;
}

uint16 input_processor_get_held(void) {
    return shadow.joypad1_held;
}

uint16 input_processor_get_newly(void) {
    return shadow.joypad1_new;
}
