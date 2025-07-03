#include "debug.h"
#include <stdarg.h>

#if DEBUG_LOG
FILE* debug_file = NULL;
#endif

void debug_init(void) {
#if DEBUG_LOG
    debug_file = fopen("ltat21_debug.log", "w");
    if (debug_file) {
        debug_print("=== LT's Aim Train 21 Debug Log ===\n");
        debug_print("Controls:\n");
        debug_print("  Mouse - Look around\n");
        debug_print("  Left Click - Shoot\n");
        debug_print("  Scroll Wheel - Adjust target size\n");
        debug_print("  R - Reset stats\n");
        debug_print("  F11 - Toggle fullscreen\n");
        debug_print("  ESC - Exit\n\n");
    }
#endif
}

void debug_cleanup(void) {
#if DEBUG_LOG
    if (debug_file) {
        fclose(debug_file);
        debug_file = NULL;
    }
#endif
}

void debug_print(const char* format, ...) {
#if DEBUG_LOG
    if (debug_file) {
        va_list args;
        va_start(args, format);
        vfprintf(debug_file, format, args);
        fflush(debug_file);
        va_end(args);
    }
#endif
}