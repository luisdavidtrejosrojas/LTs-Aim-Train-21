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
    
    glColor3f(0.2f, 0.2f, 0.2f); // Dark gray grid
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
    float outline_scale = 1.08f; // 8% larger for outline
    bool flash_red = false;
    
    if (g_game.hit_animating) {
        double current_time = glfwGetTime();
        double elapsed = current_time - g_game.hit_animation_start;
        const double ANIMATION_DURATION = 0.2; // 200ms
        
        if (elapsed < ANIMATION_DURATION) {
            // Calculate animation progress (0 to 1)
            float progress = (float)(elapsed / ANIMATION_DURATION);
            
            // Scale up then down
            float anim_factor = 1.0f + 0.5f * sinf(progress * 3.14159f);
            scale *= anim_factor;
            outline_scale *= anim_factor;
            
            // Flash red for first half of animation
            if (progress < 0.5f) {
                flash_red = true;
            }
        } else {
            // Animation complete
            g_game.hit_animating = false;
            spawn_target();
        }
    }
    
    // First pass: Draw outline (scaled up, front face culling)
    glCullFace(GL_FRONT); // Show back faces only
    glPushMatrix();
    glScalef(outline_scale, outline_scale, outline_scale);
    if (flash_red) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red flash on hit
    } else {
        glColor3f(1.0f, 1.0f, 1.0f); // White outline normally
    }
    gluSphere(sphere_quad, g_game.target_radius, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
    
    // Second pass: Draw black sphere normally (back face culling)
    glCullFace(GL_BACK); // Normal culling
    glPushMatrix();
    glScalef(scale, scale, scale);
    glColor3f(0.0f, 0.0f, 0.0f); // Black sphere
    gluSphere(sphere_quad, g_game.target_radius, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
    
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
    
    const float cx = width * 0.5f;
    const float cy = height * 0.5f;
    
    // Draw black border (6x6 total)
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(cx - 3, cy - 3);
        glVertex2f(cx + 3, cy - 3);
        glVertex2f(cx + 3, cy + 3);
        glVertex2f(cx - 3, cy + 3);
    glEnd();
    
    // Draw white center (2x2)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(cx - 1, cy - 1);
        glVertex2f(cx + 1, cy - 1);
        glVertex2f(cx + 1, cy + 1);
        glVertex2f(cx - 1, cy + 1);
    glEnd();
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}