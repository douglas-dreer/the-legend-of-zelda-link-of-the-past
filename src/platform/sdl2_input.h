#ifndef PLATFORM_SDL2_INPUT_H
#define PLATFORM_SDL2_INPUT_H

#ifdef HAS_SDL2

#include <SDL.h>

// SNES button bitmasks (matching input.h)
// These are OR'd together from SDL2 key states
constexpr uint16 SDL2_BTN_B      = 0x8000;
constexpr uint16 SDL2_BTN_Y      = 0x4000;
constexpr uint16 SDL2_BTN_SELECT = 0x2000;
constexpr uint16 SDL2_BTN_START  = 0x1000;
constexpr uint16 SDL2_BTN_UP     = 0x0800;
constexpr uint16 SDL2_BTN_DOWN   = 0x0400;
constexpr uint16 SDL2_BTN_LEFT   = 0x0200;
constexpr uint16 SDL2_BTN_RIGHT  = 0x0100;
constexpr uint16 SDL2_BTN_A      = 0x0080;
constexpr uint16 SDL2_BTN_X      = 0x0040;
constexpr uint16 SDL2_BTN_L      = 0x0020;
constexpr uint16 SDL2_BTN_R      = 0x0010;

// Process SDL2 events (quit, keydown, keyup)
// Returns false if SDL_QUIT was received
bool sdl2_input_process_events(void);

// Get the current SNES joypad state from SDL2 keys
uint16 sdl2_input_get_joypad1_state(void);

// Get the quit flag
bool sdl2_input_quit_requested(void);

#endif // HAS_SDL2
#endif // PLATFORM_SDL2_INPUT_H
