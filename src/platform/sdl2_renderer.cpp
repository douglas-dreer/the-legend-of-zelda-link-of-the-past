#ifdef HAS_SDL2

#include "sdl2_renderer.h"
#include "../snes/ppu.h"
#include <cstring>
#include <cstdio>

extern SNESPalette shadow_palette;
extern uint8 cgram[];

bool sdl2_renderer_init(SDL2Renderer* state) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 video init failed: %s\n", SDL_GetError());
        return false;
    }

    state->window = SDL_CreateWindow(
        "Zelda: A Link to the Past — Decompilation",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!state->window) {
        printf("SDL2 window creation failed: %s\n", SDL_GetError());
        return false;
    }

    state->renderer = SDL_CreateRenderer(
        state->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!state->renderer) {
        // Fallback to software renderer
        state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_SOFTWARE);
        if (!state->renderer) {
            printf("SDL2 renderer creation failed: %s\n", SDL_GetError());
            return false;
        }
    }

    // Create the 256x224 texture that maps to the SNES framebuffer
    state->screen_texture = SDL_CreateTexture(
        state->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );
    if (!state->screen_texture) {
        printf("SDL2 texture creation failed: %s\n", SDL_GetError());
        return false;
    }

    // Allocate pixel buffer (256 * 224 ARGB pixels)
    state->framebuffer = new uint32[SCREEN_WIDTH * SCREEN_HEIGHT];
    memset(state->framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32));

    state->initialized = true;
    printf("SDL2 renderer initialized: %dx%d (window %dx%d)\n",
           SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

void sdl2_renderer_destroy(SDL2Renderer* state) {
    if (state->framebuffer) {
        delete[] state->framebuffer;
        state->framebuffer = nullptr;
    }
    if (state->screen_texture) {
        SDL_DestroyTexture(state->screen_texture);
        state->screen_texture = nullptr;
    }
    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
        state->renderer = nullptr;
    }
    if (state->window) {
        SDL_DestroyWindow(state->window);
        state->window = nullptr;
    }
    SDL_Quit();
    state->initialized = false;
}

void sdl2_renderer_present(SDL2Renderer* state) {
    if (!state->initialized) return;

    // Copy framebuffer to texture
    SDL_UpdateTexture(
        state->screen_texture, nullptr,
        state->framebuffer,
        SCREEN_WIDTH * sizeof(uint32)
    );

    // Clear and render
    SDL_RenderClear(state->renderer);
    SDL_RenderCopy(state->renderer, state->screen_texture, nullptr, nullptr);
    SDL_RenderPresent(state->renderer);
}

uint32* sdl2_renderer_get_framebuffer(SDL2Renderer* state) {
    return state->framebuffer;
}

#endif // HAS_SDL2
