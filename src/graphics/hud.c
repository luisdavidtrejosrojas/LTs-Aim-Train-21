#include "hud.h"
#include "../core/window.h"
#include "../game/game_state.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

static void render_char(float x, float y, char c, float scale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);
    
    glBegin(GL_LINES);
    switch(c) {
        case '0':
            glVertex2f(1, 0); glVertex2f(5, 0);
            glVertex2f(0, 1); glVertex2f(0, 7);
            glVertex2f(6, 1); glVertex2f(6, 7);
            glVertex2f(1, 8); glVertex2f(5, 8);
            break;
        case '1':
            glVertex2f(3, 0); glVertex2f(3, 8);
            glVertex2f(2, 1); glVertex2f(3, 0);
            break;
        case '2':
            glVertex2f(0, 1); glVertex2f(0, 0);
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(6, 4);
            glVertex2f(6, 4); glVertex2f(0, 4);
            glVertex2f(0, 4); glVertex2f(0, 8);
            glVertex2f(0, 8); glVertex2f(6, 8);
            break;
        case '3':
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(6, 8);
            glVertex2f(6, 8); glVertex2f(0, 8);
            glVertex2f(0, 4); glVertex2f(4, 4);
            break;
        case '4':
            glVertex2f(0, 0); glVertex2f(0, 4);
            glVertex2f(0, 4); glVertex2f(6, 4);
            glVertex2f(6, 0); glVertex2f(6, 8);
            break;
        case '5':
            glVertex2f(6, 0); glVertex2f(0, 0);
            glVertex2f(0, 0); glVertex2f(0, 4);
            glVertex2f(0, 4); glVertex2f(6, 4);
            glVertex2f(6, 4); glVertex2f(6, 8);
            glVertex2f(6, 8); glVertex2f(0, 8);
            break;
        case '6':
            glVertex2f(6, 0); glVertex2f(0, 0);
            glVertex2f(0, 0); glVertex2f(0, 8);
            glVertex2f(0, 8); glVertex2f(6, 8);
            glVertex2f(6, 8); glVertex2f(6, 4);
            glVertex2f(6, 4); glVertex2f(0, 4);
            break;
        case '7':
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(3, 8);
            break;
        case '8':
            glVertex2f(1, 0); glVertex2f(5, 0);
            glVertex2f(0, 1); glVertex2f(0, 7);
            glVertex2f(6, 1); glVertex2f(6, 7);
            glVertex2f(1, 8); glVertex2f(5, 8);
            glVertex2f(0, 4); glVertex2f(6, 4);
            break;
        case '9':
            glVertex2f(0, 8); glVertex2f(6, 8);
            glVertex2f(6, 8); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(0, 0);
            glVertex2f(0, 0); glVertex2f(0, 4);
            glVertex2f(0, 4); glVertex2f(6, 4);
            break;
        case 'F':
            glVertex2f(0, 0); glVertex2f(0, 8);
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(0, 4); glVertex2f(4, 4);
            break;
        case 'P':
            glVertex2f(0, 0); glVertex2f(0, 8);
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(6, 4);
            glVertex2f(6, 4); glVertex2f(0, 4);
            break;
        case 'S':
            glVertex2f(6, 0); glVertex2f(0, 0);
            glVertex2f(0, 0); glVertex2f(0, 4);
            glVertex2f(0, 4); glVertex2f(6, 4);
            glVertex2f(6, 4); glVertex2f(6, 8);
            glVertex2f(6, 8); glVertex2f(0, 8);
            break;
        case ':':
            glVertex2f(3, 2); glVertex2f(3, 3);
            glVertex2f(3, 5); glVertex2f(3, 6);
            break;
        case ' ':
            break;
    }
    glEnd();
    
    glPopMatrix();
}

void render_text(float x, float y, const char* text, float scale) {
    float char_width = 8.0f * scale;
    float current_x = x;
    
    while (*text) {
        render_char(current_x, y, *text, scale);
        current_x += char_width;
        text++;
    }
}

void draw_hud(void) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    char fps_text[32];
    sprintf(fps_text, "FPS: %d", g_game.current_fps);
    render_text(10, 10, fps_text, 2.0f);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}