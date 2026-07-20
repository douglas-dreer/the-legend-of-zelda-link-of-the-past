#ifdef HAS_SDL2

#include "sdl2_input.h"
#include "../snes/input.h"
#include <cstdio>

static bool quit_requested = false;
static uint16 current_keys = 0;

// Keyboard mapping: SDL2 scancode → SNES button
static uint16 map_key_to_snese(SDL_Scancode sc) {
    switch (sc) {
        // D-Pad
        case SDL_SCANCODE_UP:    return SDL2_BTN_UP;
        case SDL_SCANCODE_DOWN:  return SDL2_BTN_DOWN;
        case SDL_SCANCODE_LEFT:  return SDL2_BTN_LEFT;
        case SDL_SCANCODE_RIGHT: return SDL2_BTN_RIGHT;

        // Face buttons
        case SDL_SCANCODE_Z:     return SDL2_BTN_B;
        case SDL_SCANCODE_X:     return SDL2_BTN_A;
        case SDL_SCANCODE_A:     return SDL2_BTN_Y;
        case SDL_SCANCODE_S:     return SDL2_BTN_X;

        // Shoulder buttons
        case SDL_SCANCODE_Q:     return SDL2_BTN_L;
        case SDL_SCANCODE_W:     return SDL2_BTN_R;

        // System buttons
        case SDL_SCANCODE_RETURN:    return SDL2_BTN_START;
        case SDL_SCANCODE_LSHIFT:    return SDL2_BTN_SELECT;
        case SDL_SCANCODE_RSHIFT:    return SDL2_BTN_SELECT;

        default: return 0;
    }
}

bool sdl2_input_process_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit_requested = true;
                return false;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit_requested = true;
                    return false;
                }
                current_keys |= map_key_to_snese(event.key.keysym.scancode);
                break;

            case SDL_KEYUP:
                current_keys &= ~map_key_to_snese(event.key.keysym.scancode);
                break;
        }
    }
    return true;
}

uint16 sdl2_input_get_joypad1_state(void) {
    return current_keys;
}

bool sdl2_input_quit_requested(void) {
    return quit_requested;
}

#endif // HAS_SDL2
