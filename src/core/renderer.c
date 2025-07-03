#include "renderer.h"
#include "window.h"
#include "ltat21.h"
#include "../game/game_state.h"
#include "../graphics/primitives.h"
#include "../graphics/hud.h"
#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

float aspect_ratio;
float fov_rad;

bool renderer_init(void) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    fov_rad = FOV * 3.14159f / 180.0f;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspect_ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    
    // Optimize OpenGL state
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_FOG);
    glDisable(GL_DITHER);
    glShadeModel(GL_FLAT);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Initialize graphics subsystems
    if (!primitives_init()) return false;
    
    return true;
}

void renderer_cleanup(void) {
    primitives_cleanup();
}

void render_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Apply camera rotation
    glRotatef(-g_game.pitch * 57.29578f, 1.0f, 0.0f, 0.0f);
    glRotatef(-g_game.yaw * 57.29578f, 0.0f, 1.0f, 0.0f);
    
    // Draw 3D scene
    draw_floor();
    draw_target();
    
    // Draw 2D overlay
    draw_crosshair();
    draw_hud();
}