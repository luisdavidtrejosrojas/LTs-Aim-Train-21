#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include "../utils/vec3.h"

typedef struct {
    float yaw;
    float pitch;
    double last_mouse_x;
    double last_mouse_y;
    bool first_mouse;
    
    // Target
    Vec3 target_pos;
    float target_radius;
    
    // Hit animation
    bool hit_animating;
    double hit_animation_start;
    
    // Volume display
    bool show_volume;
    double volume_display_start;
    
    // Cached values
    Vec3 cached_ray_dir;
    bool ray_dir_dirty;
    
    // Window state
    bool fullscreen;
    int windowed_width;
    int windowed_height;
    int windowed_x;
    int windowed_y;
} GameState;

extern GameState g_game;

void game_state_init(void);
Vec3 get_ray_direction(void);
bool check_hit(void);
void spawn_target(void);

#endif // GAME_STATE_H