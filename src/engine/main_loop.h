#ifndef ENGINE_MAIN_LOOP_H
#define ENGINE_MAIN_LOOP_H

#include "../types.h"

// Wait for NMI flag ($12): $00:8034
// Busy-loops until NMI handler sets flag
void wait_for_nmi(void);

// Main game loop: $00:8034-$00:805F
// Waits for NMI, runs sprite clear, game logic, DMA prep, clears flag
void main_loop_run(void);

// Frame logic (post-NMI, pre-next-wait): $00:8038-$00:805A
// CLI, debug hotkeys check, sprite clear, game update, update routines
void frame_logic(void);

// Game Logic Update: $00:80B5 (JSL $0080B5)
// Calls the game's main update routine
void game_logic_update(void);

// Game Logic Sub ($85FC): $00:85FC
// Secondary update routine called each frame
void game_logic_sub_update(void);

#endif // ENGINE_MAIN_LOOP_H
