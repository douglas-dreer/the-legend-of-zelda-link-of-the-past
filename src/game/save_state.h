#ifndef GAME_SAVE_STATE_H
#define GAME_SAVE_STATE_H

#include "../types.h"
#include <cstdio>

// Save/load game state to/from a simple text format.
// No JSON library dependency — uses a minimal key=value text format
// that is both human-readable and trivially parseable.
//
// Format:
//   [SECTION]
//   key=value
//   ...
//
// Sections: hardware, ppu, vram_summary, cgram_summary, oam_summary, shadow, state_machine

// Save current game state to a file
bool save_state_to_file(const char* filename);

// Load game state from a file (partial restore — memory arrays only)
bool load_state_from_file(const char* filename);

// Save a PPM screenshot alongside the state
bool save_state_with_screenshot(const char* state_file, const char* ppm_file);

// Print state summary to stdout
void print_state_summary(void);

#endif // GAME_SAVE_STATE_H
