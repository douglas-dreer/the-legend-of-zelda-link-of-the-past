// =============================================================================
// test_globals.cpp — Provides global symbols for integration test linking.
//
// Mirrors the globals defined in main.cpp, but without main() itself.
// This avoids needing to link against main.o (which has its own main).
// =============================================================================

#include "../types.h"
#include "../snes/hardware.h"
#include "../snes/ppu.h"
#include "../snes/dma.h"
#include "../snes/input.h"
#include "../game/state.h"
#include "../game/sprites.h"
#include "../game/overworld.h"
#include "../game/dungeon.h"
#include "../game/entity.h"
#include "../game/collision.h"
#include "../game/room.h"
#include "../game/save_state.h"
#include "../game/sound.h"
#include "../audio/apu.h"
#include <cstdio>
#include <cstring>

// ROM Data (loaded from file)
uint8 rom[ROM_SIZE];

// RAM Arrays
uint8 wram_low[WRAM_LOW_SIZE];
uint8 wram_extended[WRAM_EXTENDED_SIZE];
uint8 vram[VRAM_SIZE];
uint8 cgram[CGRAM_SIZE];

// Raw OAM buffer ($0800-$09FF in WRAM)
uint8 oam_buffer_raw[OAM_SIZE];

// PPU Registers Shadow State
PPURegisters ppu;

// CPU State
CPU65816 cpu;

// DMA Channels
DMAChannel dma_channels[5];

// DMA Queue
DMATransfer dma_queue[DMA_QUEUE_SIZE];

// Input State
InputState joypad1;
InputState joypad2;

// OAM Buffer
OAMBuffer oam_buffer;
SpriteTables sprite_tables;
Sprite sprites[MAX_SPRITES];

// Palette Buffer
SNESPalette shadow_palette;

// Global State
GlobalState shadow;
StateMachine state_machine;

// APU State
APUState apu;

// Sound State
SoundState sound_state;

// Dungeon state
DungeonState dungeon;
DungeonDoor dungeon_doors[MAX_DOORS];
DungeonSwitch dungeon_switches[MAX_SWITCHES];

// Collision map (simple implementation for tests)
static uint8 collision_map[256 * 256];

// Forward declarations
static void init_collision_map(void);

// Initialize RAM — called by test code
void init_ram(void) {
    memset(wram_low, 0, WRAM_LOW_SIZE);
    memset(wram_extended, 0, WRAM_EXTENDED_SIZE);
    memset(vram, 0, VRAM_SIZE);
    memset(cgram, 0, CGRAM_SIZE);
    memset(oam_buffer_raw, 0, OAM_SIZE);
    memset(&oam_buffer, 0, sizeof(OAMBuffer));
    memset(&sprite_tables, 0, sizeof(SpriteTables));
    memset(sprites, 0, sizeof(Sprite) * MAX_SPRITES);
    memset(&shadow, 0, sizeof(GlobalState));
    memset(&ppu, 0, sizeof(PPURegisters));
    memset(&cpu, 0, sizeof(CPU65816));
    memset(dma_channels, 0, sizeof(dma_channels));
    memset(dma_queue, 0, sizeof(dma_queue));
    memset(&joypad1, 0, sizeof(InputState));
    memset(&joypad2, 0, sizeof(InputState));
    memset(&sound_state, 0, sizeof(SoundState));
    memset(&dungeon, 0, sizeof(DungeonState));
    memset(dungeon_doors, 0, sizeof(dungeon_doors));
    memset(dungeon_switches, 0, sizeof(dungeon_switches));
    init_collision_map();
}

// Collision map initialization
static void init_collision_map(void) {
    memset(collision_map, 0, sizeof(collision_map));
}

// Stub implementations for collision system used by entity.cpp
bool check_collision_aabb(const BoundingBox* a, const BoundingBox* b) {
    if (!a || !b) return false;
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

TileCollision get_tile_collision(int x, int y) {
    TileCollision col;
    memset(&col, 0, sizeof(col));
    if (x >= 0 && x < 256 && y >= 0 && y < 256) {
        col.collision_type = collision_map[y * 256 + x];
    }
    return col;
}

bool check_solid_collision_h(int x, int y, int direction) {
    TileCollision col = get_tile_collision(x, y);
    return col.solid_h != 0;
}

bool check_solid_collision_v(int x, int y, int direction) {
    TileCollision col = get_tile_collision(x, y);
    return col.solid_v != 0;
}

void init_collision_system(void) {
    init_collision_map();
}

void update_collision_map(void) {
    // No-op in test mode
}

bool is_solid_tile(uint8 tile_data) {
    return (tile_data & 0x01) != 0;
}

bool is_water_tile(uint8 tile_data) {
    return (tile_data & 0x02) != 0;
}

bool is_pit_tile(uint8 tile_data) {
    return (tile_data & 0x03) == 0x03;
}

// Stubs for transition system used by overworld/dungeon
void start_transition(GameState next_state) {
    state_machine.next_state = next_state;
    state_machine.transition_counter = 0;
}

void update_transition(void) {
    if (state_machine.current_state != state_machine.next_state) {
        state_machine.transition_counter++;
    }
}
