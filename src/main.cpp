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

// ROM Data (loaded from file)
uint8 rom[ROM_SIZE];

// RAM Arrays
uint8 wram_low[WRAM_LOW_SIZE];
uint8 wram_extended[WRAM_EXTENDED_SIZE];
uint8 vram[VRAM_SIZE];
uint8 cgram[CGRAM_SIZE];

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

// Initialize hardware (simulates SNES reset)
void init_hardware(void) {
    // Disable interrupts
    cpu.flag_i = true;
    
    // Clear control registers
    ppu.nmitimen = 0;
    ppu.hdma_enable = 0;
    // ppu.dmaen = 0;
    
    // Clear APU ports
    apu.port0_send = 0;
    apu.port1_send = 0;
    apu.port2_send = 0;
    apu.port3_send = 0;
    
    // Force blank
    ppu.inidisp = 0x80;
    
    // Transition to native mode (65816)
    cpu.flag_c = false;
    cpu.emulation = false;
    
    // Clear decimal mode and A=16bit
    cpu.flag_d = false;
    cpu.flag_m = false;
    
    // Initialize stack pointer
    cpu.sp = 0x01FF;
    
    // 8-bit mode
    cpu.flag_m = true;
    cpu.flag_x = true;
}

// Initialize RAM
void init_ram(void) {
    // Clear low WRAM
    memset(wram_low, 0, WRAM_LOW_SIZE);
    
    // Clear extended WRAM
    memset(wram_extended, 0, WRAM_EXTENDED_SIZE);
    
    // Clear VRAM
    memset(vram, 0, VRAM_SIZE);
    
    // Clear CGRAM
    memset(cgram, 0, CGRAM_SIZE);
    
    // Clear OAM buffer
    memset(&oam_buffer, 0, sizeof(OAMBuffer));
    
    // Clear sprite tables
    memset(&sprite_tables, 0, sizeof(SpriteTables));
    
    // Clear sprites
    memset(sprites, 0, sizeof(Sprite) * MAX_SPRITES);
    
    // Clear global state
    memset(&shadow, 0, sizeof(GlobalState));
}

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

// Reset handler (simulates SNES reset vector)
void reset(void) {
    printf("Initializing SNES hardware...\n");
    init_hardware();
    
    printf("Clearing RAM...\n");
    init_ram();
    
    printf("Booting SPC700...\n");
    spc700_boot();
    
    printf("Hardware initialization complete.\n");
}

// NMI handler (simulates VBlank interrupt)
void nmi_handler(void) {
    // Acknowledge NMI
    // io_read(0x4210);
    
    // APU handshake
    apu_handshake();
    
    // Force blank (prepare for DMA)
    // io_write(0x2100, 0x80);
    
    // Disable HDMA
    // io_write(0x420C, 0x00);
    
    // First frame: init
    if (shadow.frame_counter == 0) {
        // initial_dma_setup();
        read_controllers();
    }
    
    // If NMI skip flag, use fast path
    if (shadow.nmi_skip != 0) {
        // nmi_fast_path();
        return;
    }
    
    // Write PPU registers
    // io_write(0x2123, shadow.w12sel);
    // io_write(0x212C, shadow.main_screen);
    // io_write(0x212D, shadow.sub_screen);
    
    // Scroll registers
    // io_write(0x210D, shadow.scroll_bg1_h & 0xFF);
    // io_write(0x210D, shadow.scroll_bg1_h >> 8);
    // ... BG1V, BG2H, BG2V, BG3H, BG3V
    
    // BG Mode and Mosaic
    // io_write(0x2105, shadow.bg_mode);
    // io_write(0x2106, shadow.mosaic);
    
    // Update sprites
    hide_unused_sprites();
    
    // Increment frame counter
    shadow.frame_counter++;
}

// Main game logic
void main_game_logic(void) {
    // Game mode dispatch
    game_mode_dispatch(shadow.game_mode);
    
    // Update sprites
    update_sprites();
    
    // Update collision
    // update_collision_map();
}

// Wait for NMI (VBlank)
void wait_nmi(void) {
    // In real SNES, this would wait for VBlank
    // For PC emulation, we simulate this
    while (shadow.nmi_flag == 0) {
        // busy-wait (can use SDL_Delay in PC)
    }
    
    // Clear NMI flag
    shadow.nmi_flag = 0;
}

// Main loop
void main_loop(void) {
    printf("Entering main loop...\n");
    
    while (true) {
        // Wait for NMI (VBlank)
        wait_nmi();
        
        // Enable IRQs
        cpu.flag_i = false;
        
        // Increment frame counter
        shadow.frame_counter++;
        
        // Hide unused sprites
        hide_unused_sprites();
        
        // Main game logic
        main_game_logic();
        
        // Prepare DMA parameters
        // prepare_dma_params();
        
        // Clear NMI flag (wait for next frame)
        shadow.nmi_flag = 0;
    }
}

// Main entry point
int main(int argc, char* argv[]) {
    printf("The Legend of Zelda: A Link to the Past - PC Port\n");
    printf("Decompilation Project\n\n");
    
    // Load ROM
    const char* rom_path = "data/zelda.sfc";
    if (argc > 1) {
        rom_path = argv[1];
    }
    
    if (!load_rom(rom_path)) {
        printf("Failed to load ROM. Exiting.\n");
        return 1;
    }
    
    // Reset SNES
    reset();
    
    // Enter main loop
    main_loop();
    
    return 0;
}