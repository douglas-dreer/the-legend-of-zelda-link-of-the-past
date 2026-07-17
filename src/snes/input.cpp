#include "input.h"

// Read controllers
void read_controllers(void) {
    // In a real implementation, this would read from SNES joypad registers
    // For PC emulation, we would read from keyboard/gamepad
    
    // Simulate SNES joypad registers ($4218-$421B)
    // uint16 hw_joypad1 = (io_read(0x4219) << 8) | io_read(0x4218);
    // uint16 hw_joypad2 = (io_read(0x421B) << 8) | io_read(0x421A);
    
    uint16 hw_joypad1 = 0;  // Placeholder
    uint16 hw_joypad2 = 0;  // Placeholder
    
    // Update joypad 1
    joypad1.previous = joypad1.held;
    joypad1.held = hw_joypad1;
    joypad1.newly = (joypad1.held ^ joypad1.previous) & joypad1.held;
    
    // Update joypad 2
    joypad2.previous = joypad2.held;
    joypad2.held = hw_joypad2;
    joypad2.newly = (joypad2.held ^ joypad2.previous) & joypad2.held;
}

// Check if button is pressed
bool is_button_pressed(uint16 button_mask) {
    return (joypad1.held & button_mask) != 0;
}

// Check if button is newly pressed
bool is_button_newly_pressed(uint16 button_mask) {
    return (joypad1.newly & button_mask) != 0;
}