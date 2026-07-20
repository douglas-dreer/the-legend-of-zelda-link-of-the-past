#ifndef ENGINE_GAME_LOOP_HEADLESS_H
#define ENGINE_GAME_LOOP_HEADLESS_H

#include "../types.h"

// Headless game loop: runs N frames without SDL2, saving PPM output.
// Uses fixed timing (no real-time clock), simulated input, and
// a PPM framebuffer renderer.

struct HeadlessGameLoopConfig {
    uint32 num_frames;          // Number of frames to run (0 = unlimited)
    const char* output_dir;     // Output directory for PPM frames
    const char* input_script;   // Input script file (NULL = no input)
    bool save_state_after;      // Save game state JSON after run
    bool verbose;               // Print per-frame info
};

struct HeadlessGameLoopResult {
    uint32 frames_run;          // Frames actually executed
    bool state_changed;         // Game state changed from INIT
    bool vram_has_data;         // VRAM has non-zero data
    bool cgram_has_palettes;    // CGRAM has palette data
    bool oam_has_sprites;       // OAM has sprite entries
    bool frames_rendered;       // At least 1 frame was saved
    uint32 total_frames_saved;  // Number of PPM files written
};

// Run the game loop headlessly for the specified number of frames.
// Returns results for integration testing.
HeadlessGameLoopResult game_loop_headless_run(const HeadlessGameLoopConfig* config);

// Initialize a default config (60 frames, output/frames/, no input)
void headless_config_default(HeadlessGameLoopConfig* config);

// Composite frame: render VRAM/CGRAM/OAM to the ARGB framebuffer
// (shared between headless and SDL2 paths)
void headless_composite_frame(uint32* framebuffer);

#endif // ENGINE_GAME_LOOP_HEADLESS_H
