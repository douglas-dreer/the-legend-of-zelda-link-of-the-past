// =============================================================================
// test_main.cpp — Test runner for Zelda: A Link to the Past decompilation
//
// Self-contained test suite that validates each subsystem independently.
// Uses real asset data from assets/ and compares against expected SNES behavior.
// Output: colored terminal (OK/FAIL per test, summary at end).
// =============================================================================

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Test framework — minimal, no external deps
// ---------------------------------------------------------------------------

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

struct TestResult {
    const char* name;
    bool passed;
    std::string detail;
};

static std::vector<TestResult> g_results;

// Color codes (ANSI)
static const char* COL_GREEN = "\033[32m";
static const char* COL_RED   = "\033[31m";
static const char* COL_BOLD  = "\033[1m";
static const char* COL_RESET = "\033[0m";

void test_pass(const char* name, const char* detail = "") {
    g_tests_run++;
    g_tests_passed++;
    g_results.push_back({name, true, detail ? detail : ""});
    printf("  %s[PASS]%s %s", COL_GREEN, COL_RESET, name);
    if (detail && detail[0]) printf(" (%s)", detail);
    printf("\n");
}

void test_fail(const char* name, const char* detail = "") {
    g_tests_run++;
    g_tests_failed++;
    g_results.push_back({name, false, detail ? detail : ""});
    printf("  %s[FAIL]%s %s", COL_RED, COL_RESET, name);
    if (detail && detail[0]) printf(" — %s", detail);
    printf("\n");
}

void section_header(const char* title) {
    printf("\n%s=== %s ===%s\n", COL_BOLD, title, COL_RESET);
}

// ---------------------------------------------------------------------------
// Test declarations (defined in separate translation units)
// ---------------------------------------------------------------------------
extern void test_palettes();
extern void test_tiles();
extern void test_audio();
extern void test_game_state();
extern void test_dma();
extern void test_sprites();

// ---------------------------------------------------------------------------
// PPM writer — minimal, no external lib needed
// ---------------------------------------------------------------------------
bool write_ppm(const char* path, int w, int h, const uint8_t* rgb) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    fwrite(rgb, 1, w * h * 3, f);
    fclose(f);
    return true;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    printf("%s╔══════════════════════════════════════════════════╗%s\n", COL_BOLD, COL_RESET);
    printf("%s║  Zelda: A Link to the Past — Decompile Tests   ║%s\n", COL_BOLD, COL_RESET);
    printf("%s╚══════════════════════════════════════════════════╝%s\n", COL_BOLD, COL_RESET);

    test_palettes();
    test_tiles();
    test_audio();
    test_game_state();
    test_dma();
    test_sprites();

    // ---- Summary ----
    printf("\n%s══════════════════════════════════════════════════%s\n", COL_BOLD, COL_RESET);
    printf("%sRESULTS:%s %d/%d passed", COL_BOLD, COL_RESET,
           g_tests_passed, g_tests_run);
    if (g_tests_failed > 0) {
        printf(", %s%d FAILED%s", COL_RED, g_tests_failed, COL_RESET);
    } else {
        printf(" %sALL OK%s", COL_GREEN, COL_RESET);
    }
    printf("\n");

    if (g_tests_failed > 0) {
        printf("\nFailed tests:\n");
        for (auto& r : g_results) {
            if (!r.passed) {
                printf("  %s- %s%s", COL_RED, r.name, COL_RESET);
                if (!r.detail.empty()) printf(": %s", r.detail.c_str());
                printf("\n");
            }
        }
    }

    printf("\n");
    return g_tests_failed > 0 ? 1 : 0;
}
