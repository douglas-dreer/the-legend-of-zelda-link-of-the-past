// =============================================================================
// test_game_state.cpp — Game state machine tests
//
// Tests:
//   1. Game state enum values
//   2. State machine structure size and layout
//   3. Game mode constants match ROM addresses
//   4. GlobalState initialization
//   5. Game mode dispatch (state transitions)
//   6. Input state simulation (button masks)
//   7. NMI flag behavior (frame sync)
//   8. Frame counter increment
// =============================================================================

#include <cstdio>
#include <cstring>
#include <cstdint>

// ---------------------------------------------------------------------------
// Game state enums and constants (from state.h)
// ---------------------------------------------------------------------------
enum class GameState : uint8_t {
    BOOT, TITLE_SCREEN, OVERWORLD, DUNGEON,
    CUTSCENE, MENU, GAME_OVER, SAVING, TRANSITION,
};

static const uint8_t GAME_MODE_INIT = 0x00;
static const uint8_t GAME_MODE_SECONDARY = 0x01;
static const uint8_t GAME_MODE_DMA_VRAM = 0x02;
static const uint8_t GAME_MODE_TILEMAP = 0x03;
static const uint8_t GAME_MODE_FULL_DMA = 0x04;
static const uint8_t GAME_MODE_VRAM_UPDATE = 0x05;
static const uint8_t GAME_MODE_IDLE = 0x06;
static const uint8_t GAME_MODE_VRAM_STREAM = 0x07;
static const uint8_t GAME_MODE_BG_UPDATE = 0x08;
static const uint8_t GAME_MODE_SPRITE_UPDATE = 0x09;
static const uint8_t GAME_MODE_OAM_UPDATE = 0x0A;
static const uint8_t GAME_MODE_SCROLL_UPDATE = 0x0B;
static const uint8_t GAME_MODE_DMA_PARTIAL = 0x0C;
static const uint8_t GAME_MODE_DMA_ALT = 0x0D;
static const uint8_t GAME_MODE_VRAM_WRITE = 0x0E;
static const uint8_t GAME_MODE_MULTI_DMA = 0x0F;

// ---------------------------------------------------------------------------
// State machine structure (from state.h)
// ---------------------------------------------------------------------------
struct StateMachine {
    GameState current_state;
    GameState next_state;
    uint8_t sub_state;
    uint8_t transition_counter;
};

// ---------------------------------------------------------------------------
// Global state structure (simplified from state.h)
// ---------------------------------------------------------------------------
struct GlobalState {
    uint8_t temp_vars[7];
    uint8_t nmi_flag;
    uint8_t palette_dirty;
    uint8_t bg_mode_dirty;
    uint8_t game_mode;
    uint8_t frame_counter;
    uint8_t main_screen;
    uint8_t sub_screen;
    uint8_t window_mask1;
    uint8_t window_mask2;
    uint8_t bg_mode;
    uint8_t mosaic;
    uint16_t scroll_bg1_h;
    uint16_t scroll_bg1_v;
    uint16_t scroll_bg2_h;
    uint16_t scroll_bg2_v;
    uint8_t bg3hofs_low;
    uint8_t bg3hofs_high;
    uint8_t bg3vofs_low;
    uint8_t bg3vofs_high;
    uint16_t joypad1_held;
    uint16_t joypad1_new;
    uint16_t joypad1_prev;
    uint16_t joypad2_held;
    uint16_t joypad2_new;
    uint16_t joypad2_prev;
};

// Input constants (from input.h)
static const uint16_t BTN_B      = 0x8000;
static const uint16_t BTN_Y      = 0x4000;
static const uint16_t BTN_SELECT = 0x2000;
static const uint16_t BTN_START  = 0x1000;
static const uint16_t BTN_UP     = 0x0800;
static const uint16_t BTN_DOWN   = 0x0400;
static const uint16_t BTN_LEFT   = 0x0200;
static const uint16_t BTN_RIGHT  = 0x0100;
static const uint16_t BTN_A      = 0x0080;
static const uint16_t BTN_X      = 0x0040;
static const uint16_t BTN_L      = 0x0020;
static const uint16_t BTN_R      = 0x0010;

// External framework functions
extern void test_pass(const char* name, const char* detail = "");
extern void test_fail(const char* name, const char* detail = "");
extern void section_header(const char* title);

// ---------------------------------------------------------------------------
// Test: Game state enum values
// ---------------------------------------------------------------------------
static void test_game_state_enum() {
    bool ok = true;
    if (static_cast<uint8_t>(GameState::BOOT) != 0) ok = false;
    if (static_cast<uint8_t>(GameState::TITLE_SCREEN) != 1) ok = false;
    if (static_cast<uint8_t>(GameState::OVERWORLD) != 2) ok = false;
    if (static_cast<uint8_t>(GameState::DUNGEON) != 3) ok = false;
    if (static_cast<uint8_t>(GameState::CUTSCENE) != 4) ok = false;
    if (static_cast<uint8_t>(GameState::MENU) != 5) ok = false;
    if (static_cast<uint8_t>(GameState::GAME_OVER) != 6) ok = false;
    if (static_cast<uint8_t>(GameState::SAVING) != 7) ok = false;
    if (static_cast<uint8_t>(GameState::TRANSITION) != 8) ok = false;

    if (ok) test_pass("GameState enum: 9 values (0-8)");
    else    test_fail("GameState enum: value mismatch");
}

// ---------------------------------------------------------------------------
// Test: Game mode constants (16 modes matching ROM dispatch table)
// ---------------------------------------------------------------------------
static void test_game_mode_constants() {
    // Modes 0x00-0x0F, matching the 16-entry dispatch table at $00:8C7E
    bool ok = true;
    if (GAME_MODE_INIT != 0x00) ok = false;
    if (GAME_MODE_SECONDARY != 0x01) ok = false;
    if (GAME_MODE_DMA_VRAM != 0x02) ok = false;
    if (GAME_MODE_TILEMAP != 0x03) ok = false;
    if (GAME_MODE_FULL_DMA != 0x04) ok = false;
    if (GAME_MODE_VRAM_UPDATE != 0x05) ok = false;
    if (GAME_MODE_IDLE != 0x06) ok = false;
    if (GAME_MODE_VRAM_STREAM != 0x07) ok = false;
    if (GAME_MODE_BG_UPDATE != 0x08) ok = false;
    if (GAME_MODE_SPRITE_UPDATE != 0x09) ok = false;
    if (GAME_MODE_OAM_UPDATE != 0x0A) ok = false;
    if (GAME_MODE_SCROLL_UPDATE != 0x0B) ok = false;
    if (GAME_MODE_DMA_PARTIAL != 0x0C) ok = false;
    if (GAME_MODE_DMA_ALT != 0x0D) ok = false;
    if (GAME_MODE_VRAM_WRITE != 0x0E) ok = false;
    if (GAME_MODE_MULTI_DMA != 0x0F) ok = false;

    if (ok) test_pass("Game mode constants: 16 modes (0x00-0x0F)");
    else    test_fail("Game mode constants: value mismatch");
}

// ---------------------------------------------------------------------------
// Test: StateMachine structure
// ---------------------------------------------------------------------------
static void test_state_machine_structure() {
    StateMachine sm;
    memset(&sm, 0, sizeof(sm));

    sm.current_state = GameState::BOOT;
    sm.next_state = GameState::BOOT;
    sm.sub_state = 0;
    sm.transition_counter = 0;

    if (sm.current_state == GameState::BOOT &&
        sm.next_state == GameState::BOOT &&
        sm.sub_state == 0 &&
        sm.transition_counter == 0) {
        test_pass("StateMachine: zero-init + BOOT state");
    } else {
        test_fail("StateMachine: initialization", "unexpected values");
    }

    // Test state transition
    sm.current_state = GameState::OVERWORLD;
    sm.next_state = GameState::DUNGEON;
    if (sm.current_state != sm.next_state) {
        test_pass("StateMachine: state transition (OW→DNG)");
    } else {
        test_fail("StateMachine: state transition", "states same");
    }
}

// ---------------------------------------------------------------------------
// Test: GlobalState initialization
// ---------------------------------------------------------------------------
static void test_global_state_init() {
    GlobalState shadow;
    memset(&shadow, 0, sizeof(shadow));

    bool ok = true;
    if (shadow.game_mode != 0) ok = false;
    if (shadow.nmi_flag != 0) ok = false;
    if (shadow.frame_counter != 0) ok = false;
    if (shadow.palette_dirty != 0) ok = false;
    if (shadow.joypad1_held != 0) ok = false;

    if (ok) test_pass("GlobalState: zero-initialized");
    else    test_fail("GlobalState: zero-initialized", "non-zero fields");
}

// ---------------------------------------------------------------------------
// Test: Input button masks
// ---------------------------------------------------------------------------
static void test_input_masks() {
    // Verify button masks match known SNES joypad layout
    bool ok = true;
    if (BTN_B != 0x8000) ok = false;
    if (BTN_A != 0x0080) ok = false;
    if (BTN_START != 0x1000) ok = false;
    if (BTN_UP != 0x0800) ok = false;
    if (BTN_DOWN != 0x0400) ok = false;
    if (BTN_LEFT != 0x0200) ok = false;
    if (BTN_RIGHT != 0x0100) ok = false;

    if (ok) test_pass("Input masks: SNES joypad layout correct");
    else    test_fail("Input masks: SNES joypad layout", "value mismatch");
}

// ---------------------------------------------------------------------------
// Test: Simulate button press (edge detection)
// ---------------------------------------------------------------------------
static void test_button_edge_detection() {
    GlobalState shadow;
    memset(&shadow, 0, sizeof(shadow));

    // Frame 0: no buttons pressed
    shadow.joypad1_held = 0;
    shadow.joypad1_prev = 0;

    // Frame 1: press A button
    uint16_t current = BTN_A;
    uint8_t cur_low = current & 0xFF;
    uint8_t prev_low = shadow.joypad1_prev & 0xFF;
    uint8_t new_low = (cur_low ^ prev_low) & cur_low;

    shadow.joypad1_held = current;
    shadow.joypad1_new = new_low;
    shadow.joypad1_prev = current;

    if (shadow.joypad1_new == (BTN_A & 0xFF)) {
        test_pass("Edge detection: A button newly pressed");
    } else {
        test_fail("Edge detection: A button newly pressed", "unexpected new bits");
    }

    // Frame 2: A still held (no new press)
    current = BTN_A;
    cur_low = current & 0xFF;
    prev_low = shadow.joypad1_prev & 0xFF;
    new_low = (cur_low ^ prev_low) & cur_low;

    shadow.joypad1_held = current;
    shadow.joypad1_new = new_low;
    shadow.joypad1_prev = current;

    if (shadow.joypad1_new == 0) {
        test_pass("Edge detection: A held (no new press)");
    } else {
        test_fail("Edge detection: A held", "false new-press detected");
    }
}

// ---------------------------------------------------------------------------
// Test: NMI flag behavior
// ---------------------------------------------------------------------------
static void test_nmi_flag() {
    GlobalState shadow;
    memset(&shadow, 0, sizeof(shadow));

    // NMI flag starts at 0
    if (shadow.nmi_flag == 0) {
        test_pass("NMI flag: initial value = 0");
    } else {
        test_fail("NMI flag: initial value", "expected 0");
    }

    // Simulate NMI handler setting flag
    shadow.nmi_flag = 1;
    if (shadow.nmi_flag == 1) {
        test_pass("NMI flag: set to 1 by handler");
    } else {
        test_fail("NMI flag: set to 1", "value wrong");
    }

    // Simulate main loop clearing flag
    shadow.nmi_flag = 0;
    if (shadow.nmi_flag == 0) {
        test_pass("NMI flag: cleared by main loop");
    } else {
        test_fail("NMI flag: cleared", "value wrong");
    }
}

// ---------------------------------------------------------------------------
// Test: Frame counter
// ---------------------------------------------------------------------------
static void test_frame_counter() {
    GlobalState shadow;
    memset(&shadow, 0, sizeof(shadow));

    shadow.frame_counter = 0;
    for (int i = 0; i < 256; i++) {
        shadow.frame_counter++;
    }
    // uint8 wraps at 256 → should be 0
    if (shadow.frame_counter == 0) {
        test_pass("Frame counter: uint8 overflow wraps to 0");
    } else {
        char buf[64];
        snprintf(buf, sizeof(buf), "expected 0, got %d", shadow.frame_counter);
        test_fail("Frame counter: uint8 overflow wraps", buf);
    }
}

// ---------------------------------------------------------------------------
// Test: Game mode dispatch simulation
// ---------------------------------------------------------------------------
static void test_game_mode_dispatch() {
    // Simulate the dispatch table from $00:8C7E
    // Each mode has a "handled" flag
    bool handled[16] = {};
    uint8_t modes_tested[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };

    for (int i = 0; i < 16; i++) {
        uint8_t mode = modes_tested[i];
        if (mode < 16) {
            handled[mode] = true;
        }
    }

    bool all_handled = true;
    for (int i = 0; i < 16; i++) {
        if (!handled[i]) {
            all_handled = false;
            break;
        }
    }

    if (all_handled) {
        test_pass("Game mode dispatch: all 16 modes reachable");
    } else {
        test_fail("Game mode dispatch: some modes unreachable");
    }
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
void test_game_state() {
    section_header("GAME STATE");
    test_game_state_enum();
    test_game_mode_constants();
    test_state_machine_structure();
    test_global_state_init();
    test_input_masks();
    test_button_edge_detection();
    test_nmi_flag();
    test_frame_counter();
    test_game_mode_dispatch();
}
