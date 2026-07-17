#ifndef SOUND_H
#define SOUND_H

#include "../types.h"

// ------------------------------------------------------------------
// Sound Command System — Bank $01 + Bank $02 sound dispatch
//
// The SNES Zelda uses the SPC700 audio processor for sound.
// The game communicates with the APU via ports $2140-$2143.
//
// Sound commands are sent through the APU port system:
//   $012C = port 0 (command)
//   $012D = port 1 (data)
//   $012E = port 2 (parameter)
//   $012F = port 3 (extra)
//   $0130 = special command ($F2+)
//
// Sound command types:
//   Port 0 = command byte:
//     $00-$0F: Music commands (play, stop, fade)
//     $10-$1F: SFX commands
//     $20-$2F: Sound effects
//     $30-$3F: Environmental sounds
//     $F0-$FF: Special commands
//
// Common commands from the ROM:
//   $01: $012E = sound effect number
//   $02: $012E = music/song number
//   Bank $09: $012E = room ambient sound
//
// The sound system is called from:
//   - $01:8B74-$01:8B84: enemy death SFX
//   - $01:B0BE-$01:B0CC: sword/hit SFX
//   - $02:8B0C: room ambient sound
//   - $09:81A5: room property sound
// ------------------------------------------------------------------

// Sound command bytes
constexpr uint8 SND_CMD_NONE       = 0x00;
constexpr uint8 SND_CMD_MUSIC      = 0x01;  // Play music
constexpr uint8 SND_CMD_SFX        = 0x02;  // Play SFX
constexpr uint8 SND_CMD_STOP_MUSIC = 0x03;  // Stop music
constexpr uint8 SND_CMD_FADE_IN    = 0x04;  // Fade in music
constexpr uint8 SND_CMD_FADE_OUT   = 0x05;  // Fade out music
constexpr uint8 SND_CMD_SPECIAL    = 0xF2;  // Special command

// Music/Song IDs (partial list)
enum class MusicID : uint8 {
    NONE                = 0x00,
    TITLE_SCREEN        = 0x01,
    OVERWORLD           = 0x02,
    DUNGEON             = 0x03,
    KAKARIKO_VILLAGE    = 0x04,
    DEATH_MOUNTAIN      = 0x05,
    LAKE_HYLIA          = 0x06,
    HYRULE_CASTLE       = 0x07,
    DARK_WORLD          = 0x08,
    SHOP                = 0x09,
    ITEM_GET            = 0x0A,
    CHEST_OPEN          = 0x0B,
    BOSS_INTRO          = 0x0C,
    BOSS_FIGHT          = 0x0D,
    BOSS_DEFEAT         = 0x0E,
    GAME_OVER           = 0x0F,
    MINIGAME            = 0x10,
    FAIRY_FOUNTAIN      = 0x11,
    ZeldaSavedPrincess  = 0x12,
    ending              = 0x13,
    triforce             = 0x14,
    file_select          = 0x15,
};

// Sound Effect IDs (partial list)
enum class SfxID : uint8 {
    NONE                = 0x00,
    SWORD_SWING         = 0x10,
    SWORD_HIT           = 0x11,
    SWORD_CLASH         = 0x12,
    ITEM_GET            = 0x13,
    CHEST_OPEN          = 0x14,
    SWITCH_ACTIVATE     = 0x15,
    DOOR_OPEN           = 0x16,
    DOOR_CLOSE          = 0x17,
    BOMB_EXPLODE        = 0x18,
    ARROW_SHOOT         = 0x19,
    ARROW_HIT           = 0x1A,
    MAGIC_USE           = 0x1B,
    MAGIC_HIT           = 0x1C,
    ENEMY_HURT          = 0x1D,
    ENEMY_DIE           = 0x1E,
    PLAYER_HURT         = 0x1F,
    PLAYER_DIE          = 0x20,
    HEART_RECOVER       = 0x21,
    RUPEE_GET           = 0x22,
    MENU_SELECT         = 0x23,
    MENU_MOVE           = 0x24,
    TEXT_PRINT          = 0x25,
    FAIRY               = 0x26,
    DASH                = 0x27,
    PEGASUS_BOOT        = 0x28,
    HOOKSHOT            = 0x29,
    BOOMERANG           = 0x2A,
    BOMB_DROP           = 0x2B,
    LIGHTNING           = 0x2C,
    WATER_SPLASH        = 0x2D,
    FIREBALL            = 0x2E,
    BLOCK_PUSH          = 0x2F,
    BLOCK_BREAK         = 0x30,
    CRYSTAL             = 0x31,
    SWORD_SPIN          = 0x32,
};

// Ambient/environment sound IDs
enum class AmbientSound : uint8 {
    NONE       = 0x00,
    BIRDS      = 0x30,
    WATERFALL  = 0x31,
    RAIN       = 0x32,
    THUNDER    = 0x33,
    CRICKETS   = 0x34,
    WIND       = 0x35,
    DUNGEON    = 0x36,
    CRYSTAL    = 0x37,
};

// Sound system state
struct SoundState {
    uint8  current_music;     // Currently playing music
    uint8  pending_music;     // Music to play next
    uint8  music_volume;      // Music volume (0-15)
    uint8  sfx_volume;        // SFX volume (0-15)
    uint8  fade_timer;        // Fade in/out timer
    uint8  fade_target;       // Fade target volume
    uint8  ambient_sound;     // Current ambient sound
    bool   music_paused;      // Music pause flag
    uint8  special_cmd;       // Special command ($F2+)
    uint8  port0_sent;        // Last port 0 value sent
    uint8  port1_sent;        // Last port 1 value sent
};

// Global sound state
extern SoundState sound_state;

// Sound System Functions

// Initialize sound system
void sound_init(void);

// Music control
void play_song(MusicID song_id);
void stop_music(void);
void pause_music(void);
void resume_music(void);
void fade_in_music(uint8 speed);
void fade_out_music(uint8 speed);

// SFX control
void play_sfx(uint8 sfx_id);
void play_sfx_extended(uint8 sfx_id, uint8 volume);

// Ambient sound
void set_ambient_sound(AmbientSound sound);
void stop_ambient_sound(void);

// APU communication
void sound_send_command(uint8 port0, uint8 port1);
void sound_send_special(uint8 cmd);
void sound_update(void);

// Sound dispatch — called from Bank $01/Bank $02
// Reads $012E for sound command and sends to APU
void sound_dispatch(void);

// Room-specific sound — called from $09:81A5
void sound_load_room_ambient(uint8 room_type);

#endif // SOUND_H
