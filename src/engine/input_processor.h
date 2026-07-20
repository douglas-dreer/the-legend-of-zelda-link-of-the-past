#ifndef ENGINE_INPUT_PROCESSOR_H
#define ENGINE_INPUT_PROCESSOR_H

#include "../types.h"

// Initialize the input processor
void input_processor_init(void);

// Process a frame of input:
// 1. Poll SDL2 events
// 2. Map keyboard to SNES joypad
// 3. Update shadow RAM ($F0-$FB) with edge detection
void input_processor_update(void);

// Get the current joypad 1 state (held buttons as SNES bitfield)
uint16 input_processor_get_held(void);

// Get newly pressed buttons this frame
uint16 input_processor_get_newly(void);

#endif // ENGINE_INPUT_PROCESSOR_H
