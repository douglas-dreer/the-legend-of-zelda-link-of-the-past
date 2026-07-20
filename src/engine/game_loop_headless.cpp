#include "game_loop_headless.h"
#include "game_loop.h"
#include "input_processor.h"
#include "nmi.h"
#include "main_loop.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../snes/input.h"
#include "../game/state.h"
#include "../game/sprites.h"
#include "../render/tile_renderer.h"
#include "../render/sprite_renderer.h"
#include "../platform/headless_renderer.h"
#include <cstdio>
#include <cstring>

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
extern StateMachine state_machine;

// ------------------------------------------------------------------
// headless_config_default: Initialize config with sane defaults
// ------------------------------------------------------------------
void headless_config_default(HeadlessGameLoopConfig* config) {
    memset(config, 0, sizeof(HeadlessGameLoopConfig));
    config->num_frames = 60;
    config->output_dir = "output";
    config->input_script = nullptr;
    config->save_state_after = false;
    config->verbose = false;
}

// ------------------------------------------------------------------
// headless_composite_frame: Full-screen compositing from PPU state
//
// This is the same composite logic used by the SDL2 path, but
// extracted here so the headless renderer can use it too.
// ------------------------------------------------------------------
void headless_composite_frame(uint32* framebuffer) {
    // Clear to black
    memset(framebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32));

    // Check force blank
    if (ppu.inidisp & 0x80) return;

    uint8 brightness = ppu.inidisp & 0x0F;
    uint8 main_screen = shadow.main_screen;

    // Render BG layers back-to-front
    if (main_screen & 0x04) {  // BG3
        render_bg_layer(3, vram, VRAM_BG3_MAP, VRAM_BG3_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg3hofs, ppu.bg3vofs, framebuffer);
    }
    if (main_screen & 0x02) {  // BG2
        render_bg_layer(2, vram, VRAM_BG2_MAP, VRAM_BG2_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg2hofs, ppu.bg2vofs, framebuffer);
    }
    if (main_screen & 0x01) {  // BG1
        render_bg_layer(1, vram, VRAM_BG1_MAP, VRAM_BG1_TILES,
                       shadow_palette.bg_palette,
                       ppu.bg1hofs, ppu.bg1vofs, framebuffer);
    }
    if (main_screen & 0x10) {  // OBJ (sprites)
        render_sprites(&oam_buffer, vram,
                      shadow_palette.sprite_palette,
                      ppu.objsel, framebuffer);
    }

    // Apply brightness
    if (brightness < 15) {
        float b = brightness / 15.0f;
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            uint32 px = framebuffer[i];
            uint8 r = (px >> 16) & 0xFF;
            uint8 g = (px >> 8) & 0xFF;
            uint8 bv = px & 0xFF;
            r = (uint8)(r * b);
            g = (uint8)(g * b);
            bv = (uint8)(bv * b);
            framebuffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | bv;
        }
    }
}

// ------------------------------------------------------------------
// read_input_from_script: Parse input script file for simulated input
//
// Script format (one command per line):
//   HOLD <button_mask>     — hold button for N frames
//   PRESS <button_mask>    — press and release button
//   WAIT <frames>          — wait N frames with no input
//   END                    — stop reading
//
// Button masks: see snes/input.h (BTN_UP, BTN_DOWN, etc.)
// ------------------------------------------------------------------
struct InputScriptEntry {
    enum Type { HOLD, PRESS, WAIT, END } type;
    uint16 button_mask;
    uint16 frames;
};

struct InputScript {
    InputScriptEntry entries[256];
    int count;
    int current;
    uint16 hold_frames_remaining;
    uint16 current_held;
};

static bool input_script_load(InputScript* script, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return false;

    script->count = 0;
    script->current = 0;
    script->hold_frames_remaining = 0;
    script->current_held = 0;

    char line[128];
    while (fgets(line, sizeof(line), f) && script->count < 256) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;

        uint16 val1, val2;
        if (sscanf(line, "HOLD %hx %hx", &val1, &val2) == 2) {
            script->entries[script->count].type = InputScriptEntry::HOLD;
            script->entries[script->count].button_mask = val1;
            script->entries[script->count].frames = val2;
            script->count++;
        } else if (sscanf(line, "PRESS %hx", &val1) == 1) {
            script->entries[script->count].type = InputScriptEntry::PRESS;
            script->entries[script->count].button_mask = val1;
            script->entries[script->count].frames = 1;
            script->count++;
        } else if (sscanf(line, "WAIT %hx", &val1) == 1) {
            script->entries[script->count].type = InputScriptEntry::WAIT;
            script->entries[script->count].frames = val1;
            script->count++;
        } else if (strncmp(line, "END", 3) == 0) {
            script->entries[script->count].type = InputScriptEntry::END;
            script->count++;
            break;
        }
    }

    fclose(f);
    return script->count > 0;
}

static uint16 input_script_poll(InputScript* script) {
    if (script->hold_frames_remaining > 0) {
        script->hold_frames_remaining--;
        return script->current_held;
    }

    while (script->current < script->count) {
        auto& entry = script->entries[script->current];
        switch (entry.type) {
            case InputScriptEntry::HOLD:
                script->current_held = entry.button_mask;
                script->hold_frames_remaining = entry.frames - 1;
                script->current++;
                return entry.button_mask;
            case InputScriptEntry::PRESS:
                script->current++;
                return entry.button_mask;
            case InputScriptEntry::WAIT:
                script->hold_frames_remaining = entry.frames - 1;
                script->current_held = 0;
                script->current++;
                return 0;
            case InputScriptEntry::END:
                script->current++;
                return 0;
        }
    }
    return 0;
}

// ------------------------------------------------------------------
// game_loop_headless_run: Run N frames headlessly
// ------------------------------------------------------------------
HeadlessGameLoopResult game_loop_headless_run(const HeadlessGameLoopConfig* config) {
    HeadlessGameLoopResult result;
    memset(&result, 0, sizeof(HeadlessGameLoopResult));

    printf("Headless game loop: %u frames to %s\n",
           config->num_frames, config->output_dir);

    // Initialize headless renderer
    HeadlessRenderer renderer;
    if (!headless_renderer_init(&renderer, config->output_dir)) {
        fprintf(stderr, "Failed to initialize headless renderer\n");
        return result;
    }

    // Load input script if provided
    InputScript input_script;
    bool has_input_script = false;
    if (config->input_script) {
        has_input_script = input_script_load(&input_script, config->input_script);
        if (!has_input_script) {
            fprintf(stderr, "Warning: Could not load input script: %s\n",
                    config->input_script);
        }
    }

    // Initialize game loop
    GameLoopState loop_state;
    game_loop_init(&loop_state);

    // Record initial game state for comparison
    GameState initial_state = state_machine.current_state;

    // Run frames
    uint32 max_frames = config->num_frames > 0 ? config->num_frames : 10000;

    for (uint32 frame = 0; frame < max_frames && loop_state.running; frame++) {
        // 1. Simulate input (from script or empty)
        uint16 input_held = 0;
        if (has_input_script) {
            input_held = input_script_poll(&input_script);
        }
        shadow.joypad1_held = input_held;

        // 2. Trigger NMI handler (VBlank)
        nmi_handler_asm();

        // 3. Frame logic
        frame_logic();

        // 4. Composite and save frame
        headless_composite_frame(renderer.framebuffer);
        headless_renderer_save_frame(&renderer);

        // 5. Update loop state
        loop_state.frame_count++;
        loop_state.delta_time = FRAME_TIME_MS / 1000.0;

        if (config->verbose && (frame % 10 == 0 || frame == max_frames - 1)) {
            printf("  Frame %u: state=%d game_mode=%d scroll=(%u,%u)\n",
                   frame, (int)state_machine.current_state,
                   shadow.game_mode,
                   shadow.scroll_bg1_h, shadow.scroll_bg1_v);
        }
    }

    // Cleanup renderer
    headless_renderer_destroy(&renderer);

    // Gather results
    result.frames_run = loop_state.frame_count;
    result.total_frames_saved = renderer.frames_written;
    result.frames_rendered = renderer.frames_written > 0;
    result.state_changed = (state_machine.current_state != initial_state);

    // Check VRAM
    bool vram_nonzero = false;
    for (uint32 i = 0; i < VRAM_SIZE && !vram_nonzero; i++) {
        if (vram[i] != 0) vram_nonzero = true;
    }
    result.vram_has_data = vram_nonzero;

    // Check CGRAM
    bool cgram_nonzero = false;
    for (uint32 i = 0; i < CGRAM_SIZE && !cgram_nonzero; i++) {
        if (cgram[i] != 0) cgram_nonzero = true;
    }
    result.cgram_has_palettes = cgram_nonzero;

    // Check OAM
    bool oam_nonzero = false;
    for (int i = 0; i < MAX_SPRITES && !oam_nonzero; i++) {
        if (oam_buffer.entries[i].y != 0 || oam_buffer.entries[i].tile != 0 ||
            oam_buffer.entries[i].attr != 0 || oam_buffer.entries[i].x != 0) {
            oam_nonzero = true;
        }
    }
    result.oam_has_sprites = oam_nonzero;

    printf("Headless run complete: %u frames, %u saved, state_changed=%s\n",
           result.frames_run, result.total_frames_saved,
           result.state_changed ? "yes" : "no");

    return result;
}
