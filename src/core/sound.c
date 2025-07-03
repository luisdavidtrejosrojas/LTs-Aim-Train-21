#include "sound.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
#endif

static float g_volume = 0.5f;
static bool g_initialized = false;

// Sound file paths
static const char* HIT_SOUND_PATH = "assets/sounds/hit.wav";
static const char* MISS_SOUND_PATH = "assets/sounds/miss.wav";

bool sound_init(void) {
    g_initialized = true;
    
#ifdef _WIN32
    // Pre-load sounds to avoid first-play delay
    PlaySound(HIT_SOUND_PATH, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    PlaySound(NULL, NULL, 0); // Stop it immediately
#endif
    
    return true;
}

void sound_cleanup(void) {
    g_initialized = false;
}

void sound_play(SoundType type) {
    if (!g_initialized || g_volume == 0.0f) return;
    
#ifdef _WIN32
    const char* sound_file = NULL;
    
    if (type == SOUND_HIT) {
        sound_file = HIT_SOUND_PATH;
    } else if (type == SOUND_MISS) {
        sound_file = MISS_SOUND_PATH;
    }
    
    if (sound_file) {
        // SND_ASYNC plays without blocking
        // SND_NODEFAULT prevents system beep if file not found
        PlaySound(sound_file, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    }
#endif
}

void sound_set_volume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_volume = volume;
    // Note: PlaySound doesn't support volume control
    // The volume is tracked but only affects mute (0 = muted)
}

float sound_get_volume(void) {
    return g_volume;
}

// These are no longer needed
void sound_generate_hit(void) {}
void sound_generate_miss(void) {}