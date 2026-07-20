#ifdef HAS_SDL2

#include "sdl2_audio.h"
#include <cstring>
#include <cstdio>

static SDL2Audio* g_audio_state = nullptr;

bool sdl2_audio_init(SDL2Audio* state) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("SDL2 audio init failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired{};
    desired.freq = AUDIO_SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = AUDIO_CHANNELS;
    desired.samples = AUDIO_BUFFER_SIZE;
    desired.callback = sdl2_audio_callback;
    desired.userdata = state;

    SDL_AudioSpec obtained{};
    if (SDL_OpenAudio(&desired, &obtained) < 0) {
        printf("SDL2 audio open failed: %s\n", SDL_GetError());
        return false;
    }

    memset(state->channels, 0, sizeof(state->channels));
    state->initialized = true;
    g_audio_state = state;

    SDL_PauseAudio(0);  // Start audio playback
    printf("SDL2 audio initialized: %d Hz, %d channels\n",
           obtained.freq, obtained.channels);
    return true;
}

void sdl2_audio_destroy(SDL2Audio* state) {
    if (state->initialized) {
        SDL_CloseAudio();
        state->initialized = false;
        g_audio_state = nullptr;
    }
}

int sdl2_audio_play_sample(SDL2Audio* state, const int16* data, uint32 length, uint8 volume) {
    if (!state->initialized) return -1;

    SDL_LockAudio();
    for (int i = 0; i < MAX_AUDIO_CHANNELS; i++) {
        if (!state->channels[i].active) {
            state->channels[i].active = true;
            state->channels[i].data = const_cast<int16*>(data);
            state->channels[i].length = length;
            state->channels[i].position = 0;
            state->channels[i].volume = volume;
            state->channels[i].pan = 128;  // center
            SDL_UnlockAudio();
            return i;
        }
    }
    SDL_UnlockAudio();
    return -1;  // No free channels
}

void sdl2_audio_stop_all(SDL2Audio* state) {
    if (!state->initialized) return;
    SDL_LockAudio();
    memset(state->channels, 0, sizeof(state->channels));
    SDL_UnlockAudio();
}

void sdl2_audio_callback(void* userdata, Uint8* stream, int len) {
    SDL2Audio* state = static_cast<SDL2Audio*>(userdata);
    int16* out = reinterpret_cast<int16*>(stream);
    int samples = len / sizeof(int16);

    // Clear output buffer
    memset(stream, 0, len);

    if (!state) return;

    // Mix all active channels
    for (int ch = 0; ch < MAX_AUDIO_CHANNELS; ch++) {
        if (!state->channels[ch].active) continue;

        AudioChannel& chan = state->channels[ch];
        int16* src = chan.data;
        uint32 remaining = chan.length - chan.position;

        for (int i = 0; i < samples && chan.position < chan.length; i += 2) {
            // Left sample
            int32 left = out[i] + (src[chan.position] * chan.volume / 255);
            out[i] = static_cast<int16>(left < -32768 ? -32768 : (left > 32767 ? 32767 : left));

            // Right sample
            if (chan.position + 1 < chan.length) {
                int32 right = out[i + 1] + (src[chan.position + 1] * chan.volume / 255);
                out[i + 1] = static_cast<int16>(right < -32768 ? -32768 : (right > 32767 ? 32767 : right));
            }
            chan.position += 2;  // Stereo interleaved
        }

        if (chan.position >= chan.length) {
            chan.active = false;
        }
    }
}

#endif // HAS_SDL2
