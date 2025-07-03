#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

// Sound types
typedef enum {
    SOUND_HIT,
    SOUND_MISS,
    SOUND_COUNT
} SoundType;

// Initialize sound system
bool sound_init(void);

// Cleanup sound system
void sound_cleanup(void);

// Play a sound
void sound_play(SoundType type);

// Volume control (0.0 to 1.0)
void sound_set_volume(float volume);
float sound_get_volume(void);

// Generate sounds programmatically (no external files needed)
void sound_generate_hit(void);
void sound_generate_miss(void);

#endif // SOUND_H