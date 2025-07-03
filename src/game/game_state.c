#include "game_state.h"
#include "../core/ltat21.h"
#include "../utils/debug.h"
#include <math.h>
#include <stdlib.h>

GameState g_game;

void game_state_init(void) {
    g_game.yaw = 0.0f;
    g_game.pitch = 0.0f;
    g_game.first_mouse = true;
    g_game.target_pos.x = 0.0f;
    g_game.target_pos.y = 0.0f;
    g_game.target_pos.z = -7.0f;
    g_game.target_radius = 1.0f;
    g_game.ray_dir_dirty = true;
    g_game.fps_last_time = 0.0;
    g_game.fps_frames = 0;
    g_game.current_fps = 0;
    g_game.fullscreen = false;
    g_game.windowed_width = WINDOW_WIDTH;
    g_game.windowed_height = WINDOW_HEIGHT;
    g_game.windowed_x = 0;
    g_game.windowed_y = 0;
}

Vec3 get_ray_direction(void) {
    if (g_game.ray_dir_dirty) {
        float yaw_rad = -g_game.yaw;
        float pitch_rad = g_game.pitch;
        float cos_pitch = cosf(pitch_rad);
        
        g_game.cached_ray_dir.x = sinf(yaw_rad) * cos_pitch;
        g_game.cached_ray_dir.y = sinf(pitch_rad);
        g_game.cached_ray_dir.z = -cosf(yaw_rad) * cos_pitch;
        
        g_game.ray_dir_dirty = false;
    }
    return g_game.cached_ray_dir;
}

// Optimized ray-sphere intersection
static bool ray_sphere_intersect_fast(Vec3 ray_dir, Vec3 sphere_center, float sphere_radius) {
    float b = -2.0f * (ray_dir.x * sphere_center.x + 
                       ray_dir.y * sphere_center.y + 
                       ray_dir.z * sphere_center.z);
    float c = sphere_center.x * sphere_center.x + 
              sphere_center.y * sphere_center.y + 
              sphere_center.z * sphere_center.z - 
              sphere_radius * sphere_radius;
    
    float discriminant = b * b - 4.0f * c;
    if (discriminant < 0) return false;
    
    float sqrt_disc = sqrtf(discriminant);
    return ((-b - sqrt_disc) * 0.5f > 0.0f) || ((-b + sqrt_disc) * 0.5f > 0.0f);
}

bool check_hit(void) {
    Vec3 ray_dir = get_ray_direction();
    return ray_sphere_intersect_fast(ray_dir, g_game.target_pos, g_game.target_radius);
}

void spawn_target(void) {
    Vec3 new_pos;
    int attempts = 0;
    const float min_distance = 2.0f;
    
    do {
        new_pos.x = (float)(rand() % 7 - 3) * 0.8f;
        new_pos.y = (float)(rand() % 5 - 2) * 0.8f;
        new_pos.z = -6.0f - (float)(rand() % 4);
        
        float dx = new_pos.x - g_game.target_pos.x;
        float dy = new_pos.y - g_game.target_pos.y;
        float dz = new_pos.z - g_game.target_pos.z;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);
        
        if (distance >= min_distance || attempts++ > 10) {
            g_game.target_pos = new_pos;
            break;
        }
    } while (1);
}