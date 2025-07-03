// sound.c - WaveOut-based audio engine for LT's Aim Train 21
#include "sound.h"
#include "../utils/debug.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
    #include <windows.h>
    #include <mmsystem.h>
#endif

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define BITS_PER_SAMPLE 16
#define MAX_VOICES 8

// Wave header structure for in-memory WAV
typedef struct {
    // RIFF header
    char riff[4];          // "RIFF"
    DWORD fileSize;
    char wave[4];          // "WAVE"
    
    // Format chunk
    char fmt[4];           // "fmt "
    DWORD fmtSize;
    WORD format;           // 1 = PCM
    WORD channels;
    DWORD sampleRate;
    DWORD byteRate;
    WORD blockAlign;
    WORD bitsPerSample;
    
    // Data chunk
    char data[4];          // "data"
    DWORD dataSize;
} WaveHeader;

// Sound data structure
typedef struct {
    BYTE* data;           // Complete WAV file in memory
    DWORD size;          // Total size including header
    short* samples;      // Pointer to actual audio data
    DWORD sampleCount;   // Number of samples
} SoundData;

// Voice structure for playing sounds
typedef struct {
    HWAVEOUT hWaveOut;
    WAVEHDR waveHdr;
    bool playing;
    bool initialized;
    SoundType currentSound;
} Voice;

// Global audio state
static struct {
    Voice voices[MAX_VOICES];
    SoundData sounds[SOUND_COUNT];
    float volume;
    bool initialized;
    CRITICAL_SECTION cs;
} g_audio = { .volume = 0.5f };

// Forward declarations
static void generate_hit_sound(SoundData* sound);
static void generate_miss_sound(SoundData* sound);
static Voice* get_free_voice(void);
static void create_wav_data(SoundData* sound, short* samples, int sampleCount);
static void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, 
                                  DWORD_PTR dwParam1, DWORD_PTR dwParam2);

bool sound_init(void) {
#ifdef _WIN32
    // Initialize critical section for thread safety
    InitializeCriticalSection(&g_audio.cs);
    
    // Generate sounds
    generate_hit_sound(&g_audio.sounds[SOUND_HIT]);
    generate_miss_sound(&g_audio.sounds[SOUND_MISS]);
    
    // Initialize voice pool
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;
    
    int successCount = 0;
    for (int i = 0; i < MAX_VOICES; i++) {
        MMRESULT result = waveOutOpen(&g_audio.voices[i].hWaveOut, WAVE_MAPPER, 
                                     &wfx, (DWORD_PTR)wave_out_proc, 
                                     (DWORD_PTR)&g_audio.voices[i], CALLBACK_FUNCTION);
        if (result == MMSYSERR_NOERROR) {
            g_audio.voices[i].initialized = true;
            g_audio.voices[i].playing = false;
            successCount++;
        } else {
            debug_print("Failed to create voice %d: %d\n", i, result);
            g_audio.voices[i].initialized = false;
        }
    }
    
    if (successCount == 0) {
        debug_print("Failed to create any audio voices\n");
        DeleteCriticalSection(&g_audio.cs);
        return false;
    }
    
    g_audio.initialized = true;
    debug_print("WaveOut initialized with %d voices\n", successCount);
    return true;
#else
    return false;
#endif
}

void sound_cleanup(void) {
#ifdef _WIN32
    if (!g_audio.initialized) return;
    
    // Stop and close all voices
    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_audio.voices[i].initialized) {
            waveOutReset(g_audio.voices[i].hWaveOut);
            waveOutClose(g_audio.voices[i].hWaveOut);
        }
    }
    
    // Free sound data
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (g_audio.sounds[i].data) {
            free(g_audio.sounds[i].data);
            g_audio.sounds[i].data = NULL;
        }
    }
    
    DeleteCriticalSection(&g_audio.cs);
    g_audio.initialized = false;
#endif
}

void sound_play(SoundType type) {
#ifdef _WIN32
    if (!g_audio.initialized || g_audio.volume == 0.0f || type >= SOUND_COUNT) return;
    
    EnterCriticalSection(&g_audio.cs);
    
    Voice* voice = get_free_voice();
    if (!voice || !voice->initialized) {
        LeaveCriticalSection(&g_audio.cs);
        return;
    }
    
    // Reset the device
    waveOutReset(voice->hWaveOut);
    
    // Set volume (0-65535 range for each channel)
    DWORD volume = (DWORD)(g_audio.volume * 65535);
    DWORD stereoVolume = (volume << 16) | volume;
    waveOutSetVolume(voice->hWaveOut, stereoVolume);
    
    // Prepare header
    memset(&voice->waveHdr, 0, sizeof(WAVEHDR));
    voice->waveHdr.lpData = (LPSTR)g_audio.sounds[type].data;
    voice->waveHdr.dwBufferLength = g_audio.sounds[type].size;
    voice->waveHdr.dwFlags = 0;
    
    MMRESULT result = waveOutPrepareHeader(voice->hWaveOut, &voice->waveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        LeaveCriticalSection(&g_audio.cs);
        debug_print("Failed to prepare header: %d\n", result);
        return;
    }
    
    // Play the sound
    result = waveOutWrite(voice->hWaveOut, &voice->waveHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(voice->hWaveOut, &voice->waveHdr, sizeof(WAVEHDR));
        LeaveCriticalSection(&g_audio.cs);
        debug_print("Failed to play sound: %d\n", result);
        return;
    }
    
    voice->playing = true;
    voice->currentSound = type;
    
    LeaveCriticalSection(&g_audio.cs);
#endif
}

void sound_set_volume(float volume) {
    g_audio.volume = fmaxf(0.0f, fminf(1.0f, volume));
}

float sound_get_volume(void) {
    return g_audio.volume;
}

static void CALLBACK wave_out_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, 
                                  DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    if (uMsg == WOM_DONE) {
        Voice* voice = (Voice*)dwInstance;
        EnterCriticalSection(&g_audio.cs);
        
        // Unprepare the header
        waveOutUnprepareHeader(voice->hWaveOut, &voice->waveHdr, sizeof(WAVEHDR));
        voice->playing = false;
        
        LeaveCriticalSection(&g_audio.cs);
    }
}

static Voice* get_free_voice(void) {
    // First, try to find a voice that's not playing
    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_audio.voices[i].initialized && !g_audio.voices[i].playing) {
            return &g_audio.voices[i];
        }
    }
    
    // If all voices are playing, steal the first one
    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_audio.voices[i].initialized) {
            return &g_audio.voices[i];
        }
    }
    
    return NULL;
}

static void create_wav_data(SoundData* sound, short* samples, int sampleCount) {
    int dataSize = sampleCount * sizeof(short);
    int headerSize = sizeof(WaveHeader);
    
    sound->size = headerSize + dataSize;
    sound->data = (BYTE*)malloc(sound->size);
    sound->sampleCount = sampleCount;
    
    WaveHeader* header = (WaveHeader*)sound->data;
    
    // Fill RIFF header
    memcpy(header->riff, "RIFF", 4);
    header->fileSize = sound->size - 8;
    memcpy(header->wave, "WAVE", 4);
    
    // Fill format chunk
    memcpy(header->fmt, "fmt ", 4);
    header->fmtSize = 16;
    header->format = 1;  // PCM
    header->channels = CHANNELS;
    header->sampleRate = SAMPLE_RATE;
    header->bitsPerSample = BITS_PER_SAMPLE;
    header->blockAlign = header->channels * header->bitsPerSample / 8;
    header->byteRate = header->sampleRate * header->blockAlign;
    
    // Fill data chunk
    memcpy(header->data, "data", 4);
    header->dataSize = dataSize;
    
    // Copy audio data
    sound->samples = (short*)(sound->data + headerSize);
    memcpy(sound->samples, samples, dataSize);
}

// Generate a crisp hit sound (high-pitched beep with fast attack)
static void generate_hit_sound(SoundData* sound) {
    float duration = 0.15f;  // 150ms
    int num_samples = (int)(duration * SAMPLE_RATE);
    int total_samples = num_samples * CHANNELS;
    
    short* samples = (short*)malloc(total_samples * sizeof(short));
    
    // Generate a 800Hz tone with envelope
    float frequency = 800.0f;
    
    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        
        // Sine wave
        float sample = sinf(2.0f * 3.14159f * frequency * t);
        
        // Apply envelope: fast attack, medium decay
        float envelope = 1.0f;
        if (t < 0.01f) {
            // Attack: 10ms
            envelope = t / 0.01f;
        } else if (t > 0.05f) {
            // Decay after 50ms
            envelope = expf(-(t - 0.05f) * 8.0f);
        }
        
        // Add slight harmonics for richness
        sample += 0.3f * sinf(4.0f * 3.14159f * frequency * t);
        sample += 0.1f * sinf(6.0f * 3.14159f * frequency * t);
        
        // Apply envelope and convert to 16-bit
        short value = (short)(sample * envelope * 16000.0f);
        
        // Stereo: same in both channels
        samples[i * 2] = value;
        samples[i * 2 + 1] = value;
    }
    
    create_wav_data(sound, samples, total_samples);
    free(samples);
}

// Generate a punchy miss sound (low thud)
static void generate_miss_sound(SoundData* sound) {
    float duration = 0.2f;  // 200ms
    int num_samples = (int)(duration * SAMPLE_RATE);
    int total_samples = num_samples * CHANNELS;
    
    short* samples = (short*)malloc(total_samples * sizeof(short));
    
    // Generate a low frequency thud with noise
    float base_freq = 150.0f;
    
    for (int i = 0; i < num_samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        
        // Low sine wave
        float sample = sinf(2.0f * 3.14159f * base_freq * t);
        
        // Add some noise for impact
        float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.3f;
        
        // Pitch bend down
        float pitch_bend = 1.0f - (t * 2.0f);  // Drops to half frequency
        if (pitch_bend < 0.5f) pitch_bend = 0.5f;
        sample = sinf(2.0f * 3.14159f * base_freq * pitch_bend * t);
        
        // Mix with noise
        sample = sample * 0.7f + noise * 0.3f;
        
        // Apply envelope: very fast attack, fast decay
        float envelope = expf(-t * 15.0f);
        
        // Apply envelope and convert to 16-bit
        short value = (short)(sample * envelope * 20000.0f);
        
        // Stereo: same in both channels
        samples[i * 2] = value;
        samples[i * 2 + 1] = value;
    }
    
    create_wav_data(sound, samples, total_samples);
    free(samples);
}

// These are no longer needed but kept for compatibility
void sound_generate_hit(void) {}
void sound_generate_miss(void) {}