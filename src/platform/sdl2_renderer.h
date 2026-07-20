#ifndef PLATFORM_SDL2_RENDERER_H
#define PLATFORM_SDL2_RENDERER_H

#ifdef HAS_SDL2

#include "../types.h"
#include <SDL.h>

// Window dimensions (2x SNES native)
constexpr int WINDOW_WIDTH = SCREEN_WIDTH * WINDOW_SCALE;
constexpr int WINDOW_HEIGHT = SCREEN_HEIGHT * WINDOW_SCALE;

// Renderer state
struct SDL2Renderer {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;  // 256x224 framebuffer
    uint32* framebuffer;         // ARGB pixel buffer
    bool initialized;
};

// Initialize SDL2 renderer: create window, renderer, texture
bool sdl2_renderer_init(SDL2Renderer* state);

// Destroy SDL2 renderer and free resources
void sdl2_renderer_destroy(SDL2Renderer* state);

// Present the framebuffer to the screen
void sdl2_renderer_present(SDL2Renderer* state);

// Get a writable pointer to the framebuffer
uint32* sdl2_renderer_get_framebuffer(SDL2Renderer* state);

#endif // HAS_SDL2
#endif // PLATFORM_SDL2_RENDERER_H
