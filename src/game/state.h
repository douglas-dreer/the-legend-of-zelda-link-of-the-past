#ifndef STATE_H
#define STATE_H

#include "../types.h"

// Game State Enum
enum class GameState : uint8 {
    BOOT,
    TITLE_SCREEN,
    OVERWORLD,
    DUNGEON,
    CUTSCENE,
    MENU,
    GAME_OVER,
    SAVING,
    TRANSITION,
};

// Game Mode Constants (for dispatch)
constexpr uint8 GAME_MODE_INIT = 0x00;
constexpr uint8 GAME_MODE_SECONDARY = 0x01;
constexpr uint8 GAME_MODE_DMA_VRAM = 0x02;
constexpr uint8 GAME_MODE_TILEMAP = 0x03;
constexpr uint8 GAME_MODE_FULL_DMA = 0x04;
constexpr uint8 GAME_MODE_VRAM_UPDATE = 0x05;
constexpr uint8 GAME_MODE_IDLE = 0x06;
constexpr uint8 GAME_MODE_VRAM_STREAM = 0x07;
constexpr uint8 GAME_MODE_BG_UPDATE = 0x08;
constexpr uint8 GAME_MODE_SPRITE_UPDATE = 0x09;
constexpr uint8 GAME_MODE_OAM_UPDATE = 0x0A;
constexpr uint8 GAME_MODE_SCROLL_UPDATE = 0x0B;
constexpr uint8 GAME_MODE_DMA_PARTIAL = 0x0C;
constexpr uint8 GAME_MODE_DMA_ALT = 0x0D;
constexpr uint8 GAME_MODE_VRAM_WRITE = 0x0E;
constexpr uint8 GAME_MODE_MULTI_DMA = 0x0F;

// State Machine Structure
struct StateMachine {
    GameState current_state;
    GameState next_state;
    uint8 sub_state;
    uint8 transition_counter;
};

// Global State Structure (Shadow RAM)
struct GlobalState {
    // --- Game State ($00-$1F) ---
    uint8 temp_vars[7];         // $00-$06: variáveis temporárias
    uint8 nmi_flag;             // $12: frame sync flag
    uint8 palette_dirty;        // $15: flag de paleta suja
    uint8 bg_mode_dirty;        // $16: flag de BG mode sujo
    uint8 game_mode;            // $17: game mode index
    uint8 frame_counter;        // $1A: contador de frames
    uint8 main_screen;          // $1C: main screen designation
    uint8 sub_screen;           // $1D: sub screen designation
    uint8 window_mask1;         // $1E
    uint8 window_mask2;         // $1F
    
    // --- Scroll Registers ($94-$EB) ---
    uint8 bg_mode;              // $94: BG mode shadow
    uint8 mosaic;               // $95: mosaic shadow
    uint8 w12sel;               // $96
    uint8 w34sel;               // $97
    uint8 wobjsel;              // $98
    uint8 cgsel;                // $99
    uint8 cgadsub;              // $9A
    uint8 hdma_enable;          // $9B
    uint8 coldata_green;        // $9C
    uint8 coldata_red;          // $9D
    uint8 coldata_blue;         // $9E
    uint8 bg3hofs_low;          // $E4
    uint8 bg3hofs_high;         // $E5
    uint8 bg3vofs_low;          // $EA
    uint8 bg3vofs_high;         // $EB
    uint8 transition_counter;   // $B0: transição de tela
    
    // --- Extended Scroll ($011E-$0125) ---
    uint16 scroll_bg2_h;        // $011E-$011F
    uint16 scroll_bg1_h;        // $0120-$0121
    uint16 scroll_bg2_v;        // $0122-$0123
    uint16 scroll_bg1_v;        // $0124-$0125
    
    // --- IRQ Config ($0128-$012A) ---
    uint8 irq_config;           // $0128
    uint8 nmi_skip;             // $012A
    
    // --- APU Communication ($012C-$0133) ---
    uint8 apu_cmd_to_send;      // $012C: port 0 tx
    uint8 apu_data_to_send;     // $012D: port 1 tx
    uint8 apu_param2;           // $012E: port 2 tx
    uint8 apu_param3;           // $012F: port 3 tx
    uint8 apu_special_cmd;      // $0130: special $F2+
    uint8 apu_last_sent_p1;     // $0131
    uint8 apu_last_sent_p0;     // $0133
    
    // --- Input ($F0-$FB) ---
    uint16 joypad1_held;        // $F0-$F2 (high, low)
    uint16 joypad1_new;         // $F4-$F6 (high, low)
    uint16 joypad1_prev;        // $F8-$FA (high, low)
    uint16 joypad2_held;        // $F1-$F3
    uint16 joypad2_new;         // $F5-$F7
    uint16 joypad2_prev;        // $F9-$FB
    
    // --- BG3 Scroll (HUD) ($0630-$0631) ---
    uint16 irq_scroll_bg3;      // $0630-$0631
    
    // --- DMA Parameters ($0AC0-$0AF8) ---
    uint32 dma_sources[5];      // $0AC0, $0AC4, $0AC8, $0ADC, $0AF4
};

// Global state instance
extern GlobalState shadow;
extern StateMachine state_machine;

// Game State Functions
void init_game_state(void);
void run_secondary_mode(void);
void game_mode_dispatch(uint8 mode);

// Game Mode Functions
void game_mode_init(void);
void game_mode_secondary(void);
void game_mode_dma_vram(void);
void game_mode_tilemap(void);
void game_mode_full_dma(void);

// Transition Functions
void start_transition(GameState next_state);
void update_transition(void);

#endif // STATE_H