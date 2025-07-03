#include "primitives.h"
#include "../core/window.h"
#include "../core/ltat21.h"
#include "../game/game_state.h"
#include <GLFW/glfw3.h>
#include <math.h>

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

static GLUquadric* sphere_quad = NULL;
static GLuint floor_display_list = 0;

bool primitives_init(void) {
    // Create reusable quadric
    sphere_quad = gluNewQuadric();
    if (!sphere_quad) return false;
    gluQuadricNormals(sphere_quad, GLU_NONE);
    
    // Create floor display list
    floor_display_list = glGenLists(1);
    glNewList(floor_display_list, GL_COMPILE);
    
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; i++) {
        glVertex3f(-10.0f, -2.0f, (float)i);
        glVertex3f(10.0f, -2.0f, (float)i);
        glVertex3f((float)i, -2.0f, -10.0f);
        glVertex3f((float)i, -2.0f, 10.0f);
    }
    glEnd();
    
    glEndList();
    
    return true;
}

void primitives_cleanup(void) {
    if (sphere_quad) {
        gluDeleteQuadric(sphere_quad);
        sphere_quad = NULL;
    }
    if (floor_display_list) {
        glDeleteLists(floor_display_list, 1);
        floor_display_list = 0;
    }
}

void draw_floor(void) {
    glCallList(floor_display_list);
}

void draw_target(void) {
    glPushMatrix();
    glTranslatef(g_game.target_pos.x, g_game.target_pos.y, g_game.target_pos.z);
    
    float scale = 1.0f;
    float r = 0.0f, g = 1.0f, b = 1.0f; // Default cyan color
    
    if (g_game.hit_animating) {
        double current_time = glfwGetTime();
        double elapsed = current_time - g_game.hit_animation_start;
        const double ANIMATION_DURATION = 0.2; // 200ms
        
        if (elapsed < ANIMATION_DURATION) {
            // Calculate animation progress (0 to 1)
            float progress = (float)(elapsed / ANIMATION_DURATION);
            
            // Scale up then down
            scale = 1.0f + 0.5f * sinf(progress * 3.14159f);
            
            // Flash to white then fade
            float flash = 1.0f - progress;
            r = flash;
            g = 1.0f;
            b = flash + (1.0f - flash) * 1.0f; // Fade from white to cyan
        } else {
            // Animation complete
            g_game.hit_animating = false;
            spawn_target();
        }
    }
    
    glScalef(scale, scale, scale);
    glColor3f(r, g, b);
    gluSphere(sphere_quad, g_game.target_radius, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
}

void draw_crosshair(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    
    const float cx = width * 0.5f;
    const float cy = height * 0.5f;
    
    glBegin(GL_LINES);
        glVertex2f(cx - 20, cy);
        glVertex2f(cx - 5, cy);
        glVertex2f(cx + 5, cy);
        glVertex2f(cx + 20, cy);
        glVertex2f(cx, cy - 20);
        glVertex2f(cx, cy - 5);
        glVertex2f(cx, cy + 5);
        glVertex2f(cx, cy + 20);
    glEnd();
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}