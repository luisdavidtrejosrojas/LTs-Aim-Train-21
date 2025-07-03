#include "sound.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
#endif

// For now, we'll use simple beeps on Windows
// Cross-platform sound requires external libraries

static float g_volume = 0.5f;
static bool g_initialized = false;

bool sound_init(void) {
    g_initialized = true;
    return true;
}

void sound_cleanup(void) {
    g_initialized = false;
}

void sound_play(SoundType type) {
    if (!g_initialized) return;
    
#ifdef _WIN32
    // Simple Windows beeps for now
    if (type == SOUND_HIT) {
        // Higher pitch for hit (880 Hz)
        Beep(880, 100);
    } else if (type == SOUND_MISS) {
        // Lower pitch for miss (220 Hz)
        Beep(220, 50);
    }
#else
    // On other platforms, we'd need a proper audio library
    // For now, just print to debug
    if (type == SOUND_HIT) {
        // Could use printf("\a") for terminal beep, but it's annoying
    }
#endif
}

void sound_set_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_volume = volume;
}

float sound_get_volume(void) {
    return g_volume;
}

// These are no longer needed with the simple implementation
void sound_generate_hit(void) {}
void sound_generate_miss(void) {}