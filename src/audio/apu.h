#ifndef APU_H
#define APU_H

#include "../types.h"

// APU State Structure
struct APUState {
    uint8 port0_send;   // $012C
    uint8 port1_send;   // $012D
    uint8 port2_send;   // $012E
    uint8 port3_send;   // $012F
    uint8 port0_recv;   // $0133
    uint8 port1_recv;   // $0131
    uint8 special_cmd;  // $0130
};

// APU Commands
constexpr uint8 APU_CMD_BOOT = 0x00;
constexpr uint8 APU_CMD_SONG = 0x01;
constexpr uint8 APU_CMD_SFX = 0x02;
constexpr uint8 APU_CMD_STOP = 0x03;
constexpr uint8 APU_CMD_SPECIAL = 0xF2;

// Global APU state
extern APUState apu;

// APU Functions
void apu_handshake(void);
void apu_send_command(uint8 cmd, uint8 data);
void apu_receive_response(void);

// SPC700 Boot/Upload Functions
void spc700_boot(void);
void spc700_block_transfer(const uint8* data, uint16 addr, uint16 size);
void spc700_wait_handshake(void);

// Sound Command Functions
void play_song(uint8 song_id);
void play_sfx(uint8 sfx_id);
void stop_sound(void);

#endif // APU_H