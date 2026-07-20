#ifndef PLATFORM_SDL2_AUDIO_H
#define PLATFORM_SDL2_AUDIO_H

#ifdef HAS_SDL2

#include "../types.h"
#include <SDL.h>

// Audio configuration
constexpr int AUDIO_SAMPLE_RATE = 32000;  // SNES sample rate
constexpr int AUDIO_CHANNELS = 2;         // Stereo output
constexpr int AUDIO_BUFFER_SIZE = 1024;   // Buffer size

// Audio channel for mixing
struct AudioChannel {
    bool active;
    int16* data;
    uint32 length;
    uint32 position;
    uint8 volume;     // 0-255
    uint8 pan;        // 0=left, 128=center, 255=right
};

constexpr int MAX_AUDIO_CHANNELS = 8;

// Audio state
struct SDL2Audio {
    bool initialized;
    AudioChannel channels[MAX_AUDIO_CHANNELS];
};

// Initialize SDL2 audio subsystem
bool sdl2_audio_init(SDL2Audio* state);

// Destroy SDL2 audio subsystem
void sdl2_audio_destroy(SDL2Audio* state);

// Play a WAV sample on a channel
// Returns channel index or -1 on failure
int sdl2_audio_play_sample(SDL2Audio* state, const int16* data, uint32 length, uint8 volume);

// Stop all audio
void sdl2_audio_stop_all(SDL2Audio* state);

// Audio callback (called by SDL2 audio system)
void sdl2_audio_callback(void* userdata, Uint8* stream, int len);

#endif // HAS_SDL2
#endif // PLATFORM_SDL2_AUDIO_H
