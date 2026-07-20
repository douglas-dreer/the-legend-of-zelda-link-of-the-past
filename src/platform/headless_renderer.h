#ifndef PLATFORM_HEADLESS_RENDERER_H
#define PLATFORM_HEADLESS_RENDERER_H

#include "../types.h"
#include <cstdio>

// Headless renderer that generates PPM frames without any window/display.
// Zero external dependencies — uses only C++ stdlib.
// Output: binary PPM P6 images at output/frames/frame_NNNNNN.ppm

struct HeadlessRenderer {
    uint32 framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    char output_dir[256];
    uint32 frames_written;
    bool initialized;
};

// Initialize headless renderer with output directory
bool headless_renderer_init(HeadlessRenderer* r, const char* output_dir);

// Get pointer to the ARGB framebuffer for compositing
uint32* headless_renderer_get_framebuffer(HeadlessRenderer* r);

// Write current framebuffer to a PPM file, increment frame counter
bool headless_renderer_save_frame(HeadlessRenderer* r);

// Write framebuffer to a specific filename
bool headless_renderer_save_frame_as(HeadlessRenderer* r, const char* filename);

// Convert ARGB framebuffer to RGB bytes for PPM output
void headless_renderer_to_rgb(const uint32* argb, uint8* rgb, int width, int height);

// Cleanup (no-op for headless, but included for API consistency)
void headless_renderer_destroy(HeadlessRenderer* r);

#endif // PLATFORM_HEADLESS_RENDERER_H
