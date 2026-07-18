#include <cstdio>
#include <cstring>
#include "types.h"
#include "snes/hardware.h"
#include "snes/ppu.h"
#include "snes/dma.h"
#include "snes/input.h"
#include "game/state.h"
#include "game/sprites.h"
#include "game/collision.h"
#include "game/room.h"
#include "audio/apu.h"
#include "engine/reset.h"
#include "engine/nmi.h"
#include "engine/main_loop.h"

// ROM Data (loaded from file)
uint8 rom[ROM_SIZE];

// Asset paths (relative to executable)
const char* ASSETS_DIR = "assets";

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

// Load ROM from file
bool load_rom(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open ROM file: %s\n", filename);
        return false;
    }

    size_t bytes_read = fread(rom, 1, ROM_SIZE, file);
    fclose(file);

    if (bytes_read != ROM_SIZE) {
        printf("Warning: ROM file size mismatch. Expected %d bytes, got %zu bytes\n",
               ROM_SIZE, bytes_read);
    }

    printf("ROM loaded successfully: %s (%zu bytes)\n", filename, bytes_read);
    return true;
}

// Initialize RAM — called by engine_reset
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
}

// Main entry point
int main(int argc, char* argv[]) {
    printf("The Legend of Zelda: A Link to the Past - PC Port\n");
    printf("Decompilation Project\n\n");

    // Load assets from assets/ directory (no ROM dependency)
    printf("Loading assets from %s/\n", ASSETS_DIR);

    // Load palettes
    printf("  Palettes: %s/palettes/palettes.json\n", ASSETS_DIR);
    // TODO: Parse palettes.json and populate palette arrays

    // Load tiles
    printf("  Tiles: %s/tiles/\n", ASSETS_DIR);
    // TODO: Load tile data from assets/tiles/

    // Load maps
    printf("  Maps: %s/maps/\n", ASSETS_DIR);
    // TODO: Load map data from assets/maps/

    // Load audio
    printf("  Audio: %s/audio.tar.gz\n", ASSETS_DIR);
    // TODO: Extract and load BRR samples from assets/audio.tar.gz

    // Load text
    printf("  Texts: %s/texts.tar.gz\n", ASSETS_DIR);
    // TODO: Extract and load text data from assets/texts.tar.gz

    printf("Assets loaded successfully.\n\n");

    // Reset SNES — full hardware init sequence
    // Implements $00:8000-$00:8031 (init_hardware + boot_spc700 + clear_ram + enable_nmi)
    engine_reset();

    // Initialize game state
    init_game_state();

    // Enter main loop — implements $00:8034-$00:805F
    // wait_nmi → frame_logic → wait_nmi → ...
    main_loop_run();

    return 0;
}
