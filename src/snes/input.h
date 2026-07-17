#ifndef INPUT_H
#define INPUT_H

#include "../types.h"

// SNES Button Masks
constexpr uint16 BTN_B = 0x8000;
constexpr uint16 BTN_Y = 0x4000;
constexpr uint16 BTN_SELECT = 0x2000;
constexpr uint16 BTN_START = 0x1000;
constexpr uint16 BTN_UP = 0x0800;
constexpr uint16 BTN_DOWN = 0x0400;
constexpr uint16 BTN_LEFT = 0x0200;
constexpr uint16 BTN_RIGHT = 0x0100;
constexpr uint16 BTN_A = 0x0080;
constexpr uint16 BTN_X = 0x0040;
constexpr uint16 BTN_L = 0x0020;
constexpr uint16 BTN_R = 0x0010;

// Input State Structure
struct InputState {
    uint16 held;       // Botões pressionados (frame atual)
    uint16 newly;      // Botões recém-pressionados (rising edge)
    uint16 previous;   // Botões do frame anterior
};

// Global input states
extern InputState joypad1;
extern InputState joypad2;

// Input Functions
void read_controllers(void);
bool is_button_pressed(uint16 button_mask);
bool is_button_newly_pressed(uint16 button_mask);

#endif // INPUT_H