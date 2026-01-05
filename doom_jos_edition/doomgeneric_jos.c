// doomgeneric port for JOS

#include <inc/lib.h>
#include <inc/kbdreg.h>
#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

Window_ptr window = NULL;
Renderer_ptr renderer = NULL;
Texture_ptr texture = NULL;

#define KEYQUEUE_SIZE 256
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static uint64_t tsc_hz = 0;
static uint64_t tsc_base = 0;
static uint32_t ms_base = 0;

static inline uint64_t
rdtsc64(void) {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void
calibrate_tsc(void) {
    int t0, t1;
    uint64_t c0, c1;

    t0 = sys_gettime();
    while ((t1 = sys_gettime()) == t0) {
        sys_yield();
    }
    t0 = t1;
    c0 = rdtsc64();
    while ((t1 = sys_gettime()) == t0) {
        sys_yield();
    }
    c1 = rdtsc64();

    if (c1 > c0) {
        tsc_hz = c1 - c0; // cycles per second
        tsc_base = c1;
        ms_base = (uint32_t)t1 * 1000;
    } else {
        tsc_hz = 1000000000ULL;
        tsc_base = rdtsc64();
        ms_base = (uint32_t)sys_gettime() * 1000;
    }
}

// Track "held" keys using console key-repeat (sys_cgetc()) as a proxy for release.
static unsigned char s_KeyDown[256];
static unsigned int s_KeyLastSeenFrame[256];
static unsigned int s_FrameCounter = 0;

// If a key stops repeating for this many frames, we synthesize a key-up.
#define KEY_RELEASE_FRAMES 6

// Helper function for tolower
static unsigned char
to_lower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

// Map JOS key codes to DOOM keys
static unsigned char
convertToDoomKey(unsigned char key) {
    // Basic ASCII mapping
    if (key >= 'a' && key <= 'z') {
        if (key == 'f' || key == 'x') return KEY_FIRE;
        return key;
    }
    if (key >= 'A' && key <= 'Z') {
        unsigned char k = to_lower(key);
        if (k == 'f' || k == 'x') return KEY_FIRE;
        return k;
    }
    if (key >= '0' && key <= '9') {
        return key;
    }
    
    // Special keys mapping
    switch (key) {
        // JOS console special keys (inc/kbdreg.h):
        case KEY_UP: return KEY_UPARROW;
        case KEY_DN: return KEY_DOWNARROW;
        case KEY_LF: return KEY_LEFTARROW;
        case KEY_RT: return KEY_RIGHTARROW;
        case '\r':
        case '\n':
            return KEY_ENTER;
        case 27: // ESC
            return KEY_ESCAPE;
        case '\t':
            return KEY_TAB;
        case '\b':
        case 127: // Backspace
            return KEY_BACKSPACE;
        case ' ':
            return KEY_USE;
        case '-':
            return KEY_MINUS;
        case '=':
            return KEY_EQUALS;
        default:
            return key;
    }
}

static void
addKeyToQueue(int pressed, unsigned char keyCode) {
    unsigned char key = convertToDoomKey(keyCode);
    unsigned short keyData = (pressed << 8) | key;

    // Overwrite-oldest behavior to avoid deadlock if producer outruns consumer.
    unsigned int next = (s_KeyQueueWriteIndex + 1) % KEYQUEUE_SIZE;
    if (next == s_KeyQueueReadIndex) {
        s_KeyQueueReadIndex = (s_KeyQueueReadIndex + 1) % KEYQUEUE_SIZE;
    }
    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex = next;
}

static void
handleKeyInput() {
    int c;
    // Try to get keys - sys_cgetc returns 0 if no key available
    // We call it multiple times to drain the buffer
    int max_keys = 10; // Limit to avoid blocking too long
    s_FrameCounter++;

    while (max_keys-- > 0) {
        c = sys_cgetc();
        if (c <= 0) {
            break; // No more keys available
        }

        // Convert + track hold state:
        unsigned char doomKey = convertToDoomKey((unsigned char)c);
        if (doomKey == 0) continue;

        if (!s_KeyDown[doomKey]) {
            s_KeyDown[doomKey] = 1;
            addKeyToQueue(1, doomKey);
        }

        s_KeyLastSeenFrame[doomKey] = s_FrameCounter;
    }

    // Synthesize releases for keys that stopped repeating.
    for (int k = 0; k < 256; k++) {
        if (!s_KeyDown[k]) continue;
        if (s_FrameCounter - s_KeyLastSeenFrame[k] > KEY_RELEASE_FRAMES) {
            s_KeyDown[k] = 0;
            addKeyToQueue(0, (unsigned char)k);
        }
    }
}

void
DG_Init() {
    calibrate_tsc();

    // Default to 2x scaled output (320x200 -> 640x400).
    vga_window_mode mode = VGA_WINDOW_MODE_SCALED_2X;
    for (int i = 1; i < myargc; i++) {
        if (!myargv[i]) continue;
        if (strcmp(myargv[i], "-1x") == 0) mode = VGA_WINDOW_MODE_CENTERED;
        if (strcmp(myargv[i], "-2x") == 0) mode = VGA_WINDOW_MODE_SCALED_2X;
    }

    window = vga_window_create(DOOMGENERIC_RESX, DOOMGENERIC_RESY, mode);
    if (!window) {
        panic("Failed to create window");
    }
    
    // Create renderer
    renderer = vga_renderer_create(window);
    if (!renderer) {
        panic("Failed to create renderer");
    }
    
    // Create texture with mapped buffer
    texture = vga_texture_create(DOOMGENERIC_RESX, DOOMGENERIC_RESY, 1, &DG_ScreenBuffer);
    if (!texture) {
        panic("Failed to create texture");
    }
    
    // Enable VGA display
    sys_display_change_vga_state(1);
    
    // Initial display
    vga_display(renderer);
}

void
DG_DrawFrame() {
    // Copy texture to renderer
    vga_texture_copy(renderer, texture);
    
    // Display the frame
    vga_display(renderer);
    
    // Handle keyboard input
    handleKeyInput();
}

void
DG_SleepMs(uint32_t ms) {
    uint32_t target = DG_GetTicksMs() + ms;
    while (DG_GetTicksMs() < target) {
        sys_yield();
    }
}

uint32_t
DG_GetTicksMs() {
    uint64_t now = rdtsc64();
    uint64_t delta = now - tsc_base;
    if (!tsc_hz) return (uint32_t)sys_gettime() * 1000;
    uint32_t add_ms = (uint32_t)((delta * 1000ULL) / tsc_hz);
    return ms_base + add_ms;
}

int
DG_GetKey(int* pressed, unsigned char* doomKey) {
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
        // Key queue is empty
        return 0;
    } else {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;
        
        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;
        
        return 1;
    }
}

void
DG_SetWindowTitle(const char* title) {
    // JOS doesn't support window titles, so we just ignore this
    (void)title;
}

