#ifndef ENGINE_GAME_LOOP_H
#define ENGINE_GAME_LOOP_H

#include "../types.h"

// Frame timing constants (NTSC 60fps)
constexpr double FRAME_TIME_MS = 16.666667;  // 1000/60
constexpr int TARGET_FPS = 60;

// Game loop state
struct GameLoopState {
    bool running;
    uint32 frame_count;
    double delta_time;         // Time since last frame in seconds
    uint32 last_frame_time;    // SDL_GetTicks() of last frame
    bool paused;
};

// Initialize game loop state
void game_loop_init(GameLoopState* state);

// Run the main SDL2 game loop
// This replaces main_loop_run() when using SDL2
void game_loop_run(void);

// Stop the game loop
void game_loop_stop(void);

// Get the current game loop state
GameLoopState* game_loop_get_state(void);

#endif // ENGINE_GAME_LOOP_H
