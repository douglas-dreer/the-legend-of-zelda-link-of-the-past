#include "sound.h"
#include "../game/state.h"
#include "../audio/apu.h"
#include <cstdio>

// ------------------------------------------------------------------
// Sound Command System Implementation
//
// Converted from Bank $01 sound dispatch and Bank $02/Bank $09
// sound command generation.
//
// The APU communication uses the 4-port system:
//   Port 0 ($2140): Command byte
//   Port 1 ($2141): Data byte
//   Port 2 ($2142): Parameter
//   Port 3 ($2143): Extra
//
// Handshake protocol:
//   1. Write command to port 0
//   2. Wait for APU to read (port 0 changes value)
//   3. APU writes response to port 0
//   4. CPU reads response
//
// The game queues sound commands in shadow RAM:
//   $012C: port 0 tx
//   $012D: port 1 tx
//   $012E: port 2 tx (also used for $012E = room sound)
//   $012F: port 3 tx
//   $0130: special command ($F2+)
// ------------------------------------------------------------------

extern GlobalState shadow;

SoundState sound_state;

// ------------------------------------------------------------------
// sound_init: Initialize sound system
// ------------------------------------------------------------------
void sound_init(void) {
    sound_state = {};
    sound_state.current_music = 0;
    sound_state.music_volume = 15;
    sound_state.sfx_volume = 15;
    sound_state.music_paused = false;
}

// ------------------------------------------------------------------
// play_song: Start playing music track
//
// Sends music command through APU port system.
// The APU interprets the command and starts the music engine.
// ------------------------------------------------------------------
void play_song(MusicID song_id) {
    uint8 id = static_cast<uint8>(song_id);

    if (sound_state.current_music == id) return;

    sound_state.pending_music = id;
    sound_send_command(SND_CMD_MUSIC, id);
    sound_state.current_music = id;

    printf("Playing music: %d\n", id);
}

// ------------------------------------------------------------------
// stop_music: Stop current music
// ------------------------------------------------------------------
void stop_music(void) {
    sound_send_command(SND_CMD_STOP_MUSIC, 0);
    sound_state.current_music = 0;
}

// ------------------------------------------------------------------
// pause_music: Pause music playback
// ------------------------------------------------------------------
void pause_music(void) {
    sound_state.music_paused = true;
    // Send pause command to APU
}

// ------------------------------------------------------------------
// resume_music: Resume paused music
// ------------------------------------------------------------------
void resume_music(void) {
    sound_state.music_paused = false;
    // Send resume command to APU
}

// ------------------------------------------------------------------
// fade_in_music: Gradually increase music volume
// ------------------------------------------------------------------
void fade_in_music(uint8 speed) {
    sound_state.fade_timer = 0;
    sound_state.fade_target = 15;
    (void)speed;
}

// ------------------------------------------------------------------
// fade_out_music: Gradually decrease music volume
// ------------------------------------------------------------------
void fade_out_music(uint8 speed) {
    sound_state.fade_timer = 0;
    sound_state.fade_target = 0;
    (void)speed;
}

// ------------------------------------------------------------------
// play_sfx: Play sound effect
//
// $01:8B74-$01:8B84: enemy death SFX
//   JSR $B0BE — load SFX data
//   LDA #$00  — SFX channel
//   JSR $B220 — send to APU
//
// $01:B0BE: SFX lookup
//   Reads SFX type from entity data
//   Maps to APU command byte
// ------------------------------------------------------------------
void play_sfx(uint8 sfx_id) {
    sound_send_command(SND_CMD_SFX, sfx_id);
}

// ------------------------------------------------------------------
// play_sfx_extended: Play SFX with volume control
// ------------------------------------------------------------------
void play_sfx_extended(uint8 sfx_id, uint8 volume) {
    sound_send_command(SND_CMD_SFX, sfx_id);
    (void)volume;
}

// ------------------------------------------------------------------
// set_ambient_sound: Set ambient background sound
// ------------------------------------------------------------------
void set_ambient_sound(AmbientSound sound) {
    sound_state.ambient_sound = static_cast<uint8>(sound);
    // Send ambient sound command to APU
}

// ------------------------------------------------------------------
// stop_ambient_sound: Stop ambient sound
// ------------------------------------------------------------------
void stop_ambient_sound(void) {
    sound_state.ambient_sound = 0;
}

// ------------------------------------------------------------------
// sound_send_command: Send command to APU via port system
//
// The port system uses a handshake protocol:
//   1. Write port0 = command
//   2. Write port1 = data
//   3. Wait for APU to acknowledge (port0 value changes)
// ------------------------------------------------------------------
void sound_send_command(uint8 port0, uint8 port1) {
    // Store in shadow RAM for NMI handler to send
    shadow.apu_cmd_to_send = port0;
    shadow.apu_data_to_send = port1;
    sound_state.port0_sent = port0;
    sound_state.port1_sent = port1;
}

// ------------------------------------------------------------------
// sound_send_special: Send special command ($F2+)
//
// $01:8B77: special command handling
// $012E = special command number
// ------------------------------------------------------------------
void sound_send_special(uint8 cmd) {
    shadow.apu_special_cmd = cmd;
    sound_state.special_cmd = cmd;
}

// ------------------------------------------------------------------
// sound_update: Update sound system each frame
//
// Handles fade in/out, ambient sound updates, and queued commands.
// ------------------------------------------------------------------
void sound_update(void) {
    // Update fade
    if (sound_state.fade_timer < 255) {
        sound_state.fade_timer++;
    }

    // Check if pending music should start
    if (sound_state.pending_music != 0 &&
        sound_state.pending_music != sound_state.current_music) {
        sound_send_command(SND_CMD_MUSIC, sound_state.pending_music);
        sound_state.current_music = sound_state.pending_music;
        sound_state.pending_music = 0;
    }
}

// ------------------------------------------------------------------
// sound_dispatch: Process queued sound commands
//
// Called from Bank $01/Bank $02 after game logic.
// Reads $012E for sound number and dispatches to APU.
// ------------------------------------------------------------------
void sound_dispatch(void) {
    // Read sound command from shadow RAM
    uint8 cmd = shadow.apu_cmd_to_send;
    uint8 data = shadow.apu_data_to_send;

    if (cmd != 0) {
        // Send to APU
        // In a real implementation, this would write to $2140-$2143
        // and wait for the handshake protocol

        // Clear shadow RAM after sending
        shadow.apu_cmd_to_send = 0;
        shadow.apu_data_to_send = 0;
    }

    // Handle special commands ($F2+)
    if (shadow.apu_special_cmd >= 0xF2) {
        // Process special command
        shadow.apu_special_cmd = 0;
    }
}

// ------------------------------------------------------------------
// sound_load_room_ambient: Load ambient sound for room type
//
// $09:81A5: room property sound
//   ORA #$0B — set sound parameter
//   STA $012E — store in port 2 for APU
//
// Different room types have different ambient sounds:
//   - Overworld: birds, wind
//   - Dungeon: echo, drips
//   - Water area: splashing
//   - Forest: birds, rustling
// ------------------------------------------------------------------
void sound_load_room_ambient(uint8 room_type) {
    uint8 ambient = 0;

    switch (room_type) {
        case 0x00:  // Overworld field
            ambient = static_cast<uint8>(AmbientSound::BIRDS);
            break;
        case 0x01:  // Forest
            ambient = static_cast<uint8>(AmbientSound::BIRDS);
            break;
        case 0x02:  // Water area
            ambient = static_cast<uint8>(AmbientSound::WATERFALL);
            break;
        case 0x03:  // Dungeon
            ambient = static_cast<uint8>(AmbientSound::DUNGEON);
            break;
        case 0x04:  // Dark world
            ambient = static_cast<uint8>(AmbientSound::WIND);
            break;
        case 0x05:  // Cave
            ambient = static_cast<uint8>(AmbientSound::DUNGEON);
            break;
        default:
            ambient = 0;
            break;
    }

    if (ambient != 0) {
        set_ambient_sound(static_cast<AmbientSound>(ambient));

        // $09:81A5: store in port 2
        // ORA #$0B — combine with sound parameter
        // STA $012E — send to APU
        shadow.apu_param2 = ambient | 0x0B;
    }
}
