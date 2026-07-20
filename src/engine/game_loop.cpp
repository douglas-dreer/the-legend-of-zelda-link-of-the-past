#include "game_loop.h"
#include "input_processor.h"
#include "nmi.h"
#include "main_loop.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../game/state.h"
#include "../game/sprites.h"
#include "../render/tile_renderer.h"
#include "../render/sprite_renderer.h"

#ifdef HAS_SDL2
#include "../platform/sdl2_renderer.h"
#include "../platform/sdl2_input.h"
#include "../platform/sdl2_audio.h"
#include <SDL.h>
#endif

#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

// External references
extern PPURegisters ppu;
extern GlobalState shadow;
extern SNESPalette shadow_palette;
extern OAMBuffer oam_buffer;
extern uint8 vram[];
extern uint8 cgram[];
extern uint8 rom[];
extern CPU65816 cpu;
extern DMAChannel dma_channels[5];

static GameLoopState g_loop_state;
static bool g_loop_initialized = false;

// ---- Render subsystem (platform-independent framebuffer compositing) ----

#ifdef HAS_SDL2
static SDL2Renderer g_renderer;
static SDL2Audio g_audio;

// Full-screen composite: reads VRAM, CGRAM, OAM and writes to the framebuffer
static void composite_frame(uint32* framebuffer) {
    // Clear framebuffer to black
    memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32));

    // Check if display is enabled (force blank = bit 7 of INIDISP)
    if (ppu.inidisp & 0x80) return;

    uint8 brightness = ppu.inidisp & 0x0F;

    // BG layer rendering order: BG3 (lowest) → BG2 → BG1 (highest)
    // But for compositing, we render back-to-front
    uint8 main_screen = shadow.main_screen;

    if (main_screen & 0x04) {  // BG3 enabled
        render_bg_layer(3, vram, VRAM_BG3_MAP, VRAM_BG3_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg3hofs, ppu.bg3vofs, framebuffer);
    }
    if (main_screen & 0x02) {  // BG2 enabled
        render_bg_layer(2, vram, VRAM_BG2_MAP, VRAM_BG2_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg2hofs, ppu.bg2vofs, framebuffer);
    }
    if (main_screen & 0x01) {  // BG1 enabled
        render_bg_layer(1, vram, VRAM_BG1_MAP, VRAM_BG1_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg1hofs, ppu.bg1vofs, framebuffer);
    }
    if (main_screen & 0x10) {  // OBJ (sprites) enabled
        render_sprites(&oam_buffer, vram,
                      shadow_palette.sprite_palette,
                      ppu.objsel, framebuffer);
    }

    // Apply brightness (multiply RGB by brightness/15)
    if (brightness < 15) {
        float b = brightness / 15.0f;
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            uint32 px = framebuffer[i];
            uint8 a = (px >> 24) & 0xFF;
            uint8 r = (px >> 16) & 0xFF;
            uint8 g = (px >> 8) & 0xFF;
            uint8 b_val = px & 0xFF;
            r = (uint8)(r * b);
            g = (uint8)(g * b);
            b_val = (uint8)(b_val * b);
            framebuffer[i] = (a << 24) | (r << 16) | (g << 8) | b_val;
        }
    }
}
#endif

void game_loop_init(GameLoopState* state) {
    memset(state, 0, sizeof(GameLoopState));
    state->running = true;
    state->paused = false;
    state->frame_count = 0;
    state->delta_time = FRAME_TIME_MS / 1000.0;
    g_loop_initialized = true;
}

GameLoopState* game_loop_get_state() {
    return &g_loop_state;
}

void game_loop_stop(void) {
    g_loop_state.running = false;
}

void game_loop_run(void) {
    printf("Starting SDL2 game loop...\n");

    game_loop_init(&g_loop_state);

#ifdef HAS_SDL2
    // Initialize SDL2 subsystems
    if (!sdl2_renderer_init(&g_renderer)) {
        printf("Failed to initialize SDL2 renderer, exiting.\n");
        return;
    }

    sdl2_audio_init(&g_audio);
    input_processor_init();

    g_loop_state.last_frame_time = SDL_GetTicks();

    while (g_loop_state.running) {
        uint32 frame_start = SDL_GetTicks();

        // 1. Process input
        if (!sdl2_input_process_events()) {
            g_loop_state.running = false;
            break;
        }
        input_processor_update();

        // 2. Trigger NMI handler (simulates VBlank interrupt)
        // This calls the NMI handler which does PPU register writes,
        // APU handshake, DMA engine run, controller read
        nmi_handler_asm();

        // 3. Frame logic (game update)
        // Simulates $00:8038-$00:805F: CLI, debug hotkeys, sprite clear,
        // game logic update, secondary update, clear NMI flag
        frame_logic();

        // 4. Render
        uint32* fb = sdl2_renderer_get_framebuffer(&g_renderer);
        composite_frame(fb);
        sdl2_renderer_present(&g_renderer);

        // 5. Frame timing — maintain 60fps NTSC
        uint32 frame_end = SDL_GetTicks();
        uint32 elapsed = frame_end - frame_start;
        if (elapsed < (uint32)FRAME_TIME_MS) {
            SDL_Delay((uint32)FRAME_TIME_MS - elapsed);
        }

        g_loop_state.delta_time = (SDL_GetTicks() - frame_start) / 1000.0;
        g_loop_state.frame_count++;
    }

    // Cleanup
    sdl2_audio_destroy(&g_audio);
    sdl2_renderer_destroy(&g_renderer);
    printf("Game loop ended after %u frames.\n", g_loop_state.frame_count);

#else
    // Fallback: no SDL2, run the original main_loop_run (spins on NMI flag)
    printf("SDL2 not available, falling back to main_loop_run.\n");
    main_loop_run();
#endif
}
