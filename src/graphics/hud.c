#include "hud.h"
#include "../core/window.h"
#include "../core/sound.h"
#include "../game/game_state.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

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
        case '%':
            glVertex2f(0, 0); glVertex2f(0, 3);
            glVertex2f(0, 0); glVertex2f(1, 0);
            glVertex2f(1, 0); glVertex2f(1, 3);
            glVertex2f(0, 3); glVertex2f(1, 3);
            glVertex2f(1, 3); glVertex2f(5, 5);
            glVertex2f(5, 5); glVertex2f(5, 8);
            glVertex2f(5, 5); glVertex2f(6, 5);
            glVertex2f(6, 5); glVertex2f(6, 8);
            glVertex2f(5, 8); glVertex2f(6, 8);
            break;
        case 'V':
            glVertex2f(0, 0); glVertex2f(3, 8);
            glVertex2f(3, 8); glVertex2f(6, 0);
            break;
        case 'o':
            glVertex2f(1, 3); glVertex2f(5, 3);
            glVertex2f(0, 4); glVertex2f(0, 7);
            glVertex2f(6, 4); glVertex2f(6, 7);
            glVertex2f(1, 8); glVertex2f(5, 8);
            break;
        case 'l':
            glVertex2f(3, 0); glVertex2f(3, 8);
            break;
        case 'u':
            glVertex2f(0, 3); glVertex2f(0, 8);
            glVertex2f(0, 8); glVertex2f(6, 8);
            glVertex2f(6, 3); glVertex2f(6, 8);
            break;
        case 'm':
            glVertex2f(0, 3); glVertex2f(0, 8);
            glVertex2f(0, 3); glVertex2f(3, 3);
            glVertex2f(3, 3); glVertex2f(3, 8);
            glVertex2f(3, 3); glVertex2f(6, 3);
            glVertex2f(6, 3); glVertex2f(6, 8);
            break;
        case 'e':
            glVertex2f(0, 3); glVertex2f(0, 8);
            glVertex2f(0, 3); glVertex2f(6, 3);
            glVertex2f(0, 5); glVertex2f(5, 5);
            glVertex2f(0, 8); glVertex2f(6, 8);
            glVertex2f(6, 5); glVertex2f(6, 8);
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

static void render_text(float x, float y, const char* text, float scale) {
    float char_width = 8.0f * scale;
    float current_x = x;
    
    while (*text) {
        render_char(current_x, y, *text, scale);
        current_x += char_width;
        text++;
    }
}

void draw_hud(void) {
    // Check if we should show volume
    if (g_game.show_volume) {
        double current_time = glfwGetTime();
        double elapsed = current_time - g_game.volume_display_start;
        const double VOLUME_DISPLAY_DURATION = 2.0; // Show for 2 seconds
        
        if (elapsed < VOLUME_DISPLAY_DURATION) {
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
            
            // Calculate fade out in last 0.5 seconds
            float alpha = 1.0f;
            if (elapsed > (VOLUME_DISPLAY_DURATION - 0.5)) {
                alpha = (float)((VOLUME_DISPLAY_DURATION - elapsed) / 0.5);
            }
            
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            // Draw volume text centered
            char volume_text[32];
            int volume_percent = (int)(sound_get_volume() * 100.0f);
            sprintf(volume_text, "Volume: %d%%", volume_percent);
            
            float text_width = strlen(volume_text) * 8.0f * 2.0f; // scale of 2
            float text_x = (width - text_width) * 0.5f;
            float text_y = height * 0.5f;
            
            // Draw background box
            glColor4f(0.0f, 0.0f, 0.0f, alpha * 0.7f);
            glBegin(GL_QUADS);
                glVertex2f(text_x - 20, text_y - 10);
                glVertex2f(text_x + text_width + 20, text_y - 10);
                glVertex2f(text_x + text_width + 20, text_y + 30);
                glVertex2f(text_x - 20, text_y + 30);
            glEnd();
            
            // Draw text
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glLineWidth(2.0f);
            render_text(text_x, text_y, volume_text, 2.0f);
            
            // Draw volume bar
            float bar_x = width * 0.5f - 100;
            float bar_y = text_y + 40;
            float bar_width = 200;
            float bar_height = 10;
            float bar_fill = bar_width * sound_get_volume();
            
            // Background
            glColor4f(0.3f, 0.3f, 0.3f, alpha);
            glBegin(GL_QUADS);
                glVertex2f(bar_x, bar_y);
                glVertex2f(bar_x + bar_width, bar_y);
                glVertex2f(bar_x + bar_width, bar_y + bar_height);
                glVertex2f(bar_x, bar_y + bar_height);
            glEnd();
            
            // Fill
            glColor4f(0.0f, 1.0f, 0.0f, alpha);
            glBegin(GL_QUADS);
                glVertex2f(bar_x, bar_y);
                glVertex2f(bar_x + bar_fill, bar_y);
                glVertex2f(bar_x + bar_fill, bar_y + bar_height);
                glVertex2f(bar_x, bar_y + bar_height);
            glEnd();
            
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        } else {
            g_game.show_volume = false;
        }
    }
}