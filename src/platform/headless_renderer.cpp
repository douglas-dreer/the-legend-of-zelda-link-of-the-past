#include "headless_renderer.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <errno.h>

// ------------------------------------------------------------------
// headless_renderer_init: Set up output directory
// ------------------------------------------------------------------
bool headless_renderer_init(HeadlessRenderer* r, const char* output_dir) {
    if (!r || !output_dir) return false;

    memset(r->framebuffer, 0, sizeof(r->framebuffer));
    strncpy(r->output_dir, output_dir, sizeof(r->output_dir) - 1);
    r->output_dir[sizeof(r->output_dir) - 1] = '\0';
    r->frames_written = 0;
    r->initialized = true;

    // Create output directory if it doesn't exist
    mkdir(r->output_dir, 0755);

    // Create subdirectories for frame groups (every 1000 frames)
    char subdir[300];
    snprintf(subdir, sizeof(subdir), "%s/frames", r->output_dir);
    mkdir(subdir, 0755);

    printf("Headless renderer initialized: output to %s/frames/\n", r->output_dir);
    return true;
}

// ------------------------------------------------------------------
// headless_renderer_get_framebuffer: Return ARGB framebuffer pointer
// ------------------------------------------------------------------
uint32* headless_renderer_get_framebuffer(HeadlessRenderer* r) {
    return r->framebuffer;
}

// ------------------------------------------------------------------
// headless_renderer_to_rgb: Convert ARGB framebuffer to RGB byte array
// ------------------------------------------------------------------
void headless_renderer_to_rgb(const uint32* argb, uint8* rgb, int width, int height) {
    for (int i = 0; i < width * height; i++) {
        uint32 pixel = argb[i];
        // ARGB format: bits 23-16 = R, 15-8 = G, 7-0 = B
        rgb[i * 3 + 0] = (pixel >> 16) & 0xFF;  // R
        rgb[i * 3 + 1] = (pixel >> 8) & 0xFF;   // G
        rgb[i * 3 + 2] = pixel & 0xFF;           // B
    }
}

// ------------------------------------------------------------------
// headless_renderer_save_frame: Write framebuffer as PPM P6
// ------------------------------------------------------------------
bool headless_renderer_save_frame(HeadlessRenderer* r) {
    if (!r || !r->initialized) return false;

    char filename[300];
    snprintf(filename, sizeof(filename), "%s/frames/frame_%06u.ppm",
             r->output_dir, r->frames_written);

    bool ok = headless_renderer_save_frame_as(r, filename);
    if (ok) {
        r->frames_written++;
    }
    return ok;
}

// ------------------------------------------------------------------
// headless_renderer_save_frame_as: Write to specific PPM filename
// ------------------------------------------------------------------
bool headless_renderer_save_frame_as(HeadlessRenderer* r, const char* filename) {
    if (!r || !filename) return false;

    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Could not write frame to %s: %s\n",
                filename, strerror(errno));
        return false;
    }

    // PPM P6 header
    fprintf(f, "P6\n%d %d\n255\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    // Convert ARGB → RGB and write
    uint8 rgb[SCREEN_WIDTH * SCREEN_HEIGHT * 3];
    headless_renderer_to_rgb(r->framebuffer, rgb, SCREEN_WIDTH, SCREEN_HEIGHT);
    fwrite(rgb, 1, sizeof(rgb), f);

    fclose(f);
    return true;
}

// ------------------------------------------------------------------
// headless_renderer_destroy: Cleanup
// ------------------------------------------------------------------
void headless_renderer_destroy(HeadlessRenderer* r) {
    if (r) {
        printf("Headless renderer: %u frames written to %s/frames/\n",
               r->frames_written, r->output_dir);
        r->initialized = false;
    }
}
