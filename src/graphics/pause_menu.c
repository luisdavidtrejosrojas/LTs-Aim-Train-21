#include "pause_menu.h"
#include "../core/window.h"
#include <GLFW/glfw3.h>
#include <string.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

// Button structure
typedef struct {
    const char* text;
    float x, y;
    float width, height;
    bool hovered;
} Button;

// Menu state
static struct {
    Button buttons[BUTTON_COUNT];
    MenuButton hoveredButton;
} menu;

void pause_menu_init(void) {
    // Initialize buttons with positions (these will be updated based on window size)
    menu.buttons[BUTTON_RESUME].text = "RESUME";
    menu.buttons[BUTTON_SETTINGS].text = "SETTINGS";
    menu.buttons[BUTTON_QUIT].text = "QUIT";
    
    menu.hoveredButton = BUTTON_NONE;
}

static void update_button_positions(int windowWidth, int windowHeight) {
    float centerX = windowWidth * 0.5f;
    float centerY = windowHeight * 0.5f;
    float buttonWidth = 200.0f;
    float buttonHeight = 50.0f;
    float buttonSpacing = 20.0f;
    
    // Calculate starting Y position to center all buttons
    float totalHeight = BUTTON_COUNT * buttonHeight + (BUTTON_COUNT - 1) * buttonSpacing;
    float startY = centerY - totalHeight * 0.5f;
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        menu.buttons[i].x = centerX - buttonWidth * 0.5f;
        menu.buttons[i].y = startY + i * (buttonHeight + buttonSpacing);
        menu.buttons[i].width = buttonWidth;
        menu.buttons[i].height = buttonHeight;
    }
}

void pause_menu_update(double mouseX, double mouseY) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_button_positions(width, height);
    
    menu.hoveredButton = BUTTON_NONE;
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button* btn = &menu.buttons[i];
        btn->hovered = false;
        
        if (mouseX >= btn->x && mouseX <= btn->x + btn->width &&
            mouseY >= btn->y && mouseY <= btn->y + btn->height) {
            btn->hovered = true;
            menu.hoveredButton = i;
        }
    }
}

MenuButton pause_menu_handle_click(double mouseX, double mouseY) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button* btn = &menu.buttons[i];
        if (mouseX >= btn->x && mouseX <= btn->x + btn->width &&
            mouseY >= btn->y && mouseY <= btn->y + btn->height) {
            return i;
        }
    }
    return BUTTON_NONE;
}

MenuButton pause_menu_get_hovered(void) {
    return menu.hoveredButton;
}

static void render_text_centered(float x, float y, const char* text, float scale) {
    // Calculate text width (assuming 8 pixels per character at scale 1)
    float charWidth = 8.0f * scale;
    float textWidth = strlen(text) * charWidth;
    float startX = x - textWidth * 0.5f;
    
    glPushMatrix();
    glTranslatef(startX, y - 4.0f * scale, 0);  // Center vertically too
    
    // Draw each character
    for (const char* c = text; *c; c++) {
        glBegin(GL_LINES);
        
        switch (*c) {
            case 'R':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(5, 0);
                glVertex2f(5, 0); glVertex2f(5, 4);
                glVertex2f(0, 4); glVertex2f(5, 4);
                glVertex2f(0, 4); glVertex2f(6, 8);
                break;
            case 'E':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(6, 0);
                glVertex2f(0, 4); glVertex2f(5, 4);
                glVertex2f(0, 8); glVertex2f(6, 8);
                break;
            case 'S':
                glVertex2f(6, 0); glVertex2f(0, 0);
                glVertex2f(0, 0); glVertex2f(0, 4);
                glVertex2f(0, 4); glVertex2f(6, 4);
                glVertex2f(6, 4); glVertex2f(6, 8);
                glVertex2f(6, 8); glVertex2f(0, 8);
                break;
            case 'U':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 8); glVertex2f(6, 8);
                glVertex2f(6, 0); glVertex2f(6, 8);
                break;
            case 'M':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(3, 4);
                glVertex2f(3, 4); glVertex2f(6, 0);
                glVertex2f(6, 0); glVertex2f(6, 8);
                break;
            case 'T':
                glVertex2f(0, 0); glVertex2f(6, 0);
                glVertex2f(3, 0); glVertex2f(3, 8);
                break;
            case 'I':
                glVertex2f(3, 0); glVertex2f(3, 8);
                glVertex2f(1, 0); glVertex2f(5, 0);
                glVertex2f(1, 8); glVertex2f(5, 8);
                break;
            case 'N':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(6, 8);
                glVertex2f(6, 0); glVertex2f(6, 8);
                break;
            case 'G':
                glVertex2f(6, 0); glVertex2f(0, 0);
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 8); glVertex2f(6, 8);
                glVertex2f(6, 8); glVertex2f(6, 4);
                glVertex2f(6, 4); glVertex2f(3, 4);
                break;
            case 'Q':
                glVertex2f(1, 0); glVertex2f(5, 0);
                glVertex2f(0, 1); glVertex2f(0, 7);
                glVertex2f(6, 1); glVertex2f(6, 7);
                glVertex2f(1, 8); glVertex2f(5, 8);
                glVertex2f(4, 6); glVertex2f(7, 9);
                break;
            case ' ':
                // Space - no lines to draw
                break;
            case 'P':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(5, 0);
                glVertex2f(5, 0); glVertex2f(5, 4);
                glVertex2f(0, 4); glVertex2f(5, 4);
                break;
            case 'A':
                glVertex2f(0, 8); glVertex2f(3, 0);
                glVertex2f(3, 0); glVertex2f(6, 8);
                glVertex2f(1, 5); glVertex2f(5, 5);
                break;
            case 'D':
                glVertex2f(0, 0); glVertex2f(0, 8);
                glVertex2f(0, 0); glVertex2f(4, 0);
                glVertex2f(4, 0); glVertex2f(6, 2);
                glVertex2f(6, 2); glVertex2f(6, 6);
                glVertex2f(6, 6); glVertex2f(4, 8);
                glVertex2f(4, 8); glVertex2f(0, 8);
                break;
        }
        
        glEnd();
        glTranslatef(charWidth, 0, 0);
    }
    
    glPopMatrix();
}

void draw_pause_menu(void) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Set up 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw dark overlay
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
    glEnd();
    
    // Draw menu background
    float menuWidth = 300.0f;
    float menuHeight = 300.0f;
    float menuX = (width - menuWidth) * 0.5f;
    float menuY = (height - menuHeight) * 0.5f;
    
    glColor4f(0.1f, 0.1f, 0.1f, 0.9f);
    glBegin(GL_QUADS);
        glVertex2f(menuX, menuY);
        glVertex2f(menuX + menuWidth, menuY);
        glVertex2f(menuX + menuWidth, menuY + menuHeight);
        glVertex2f(menuX, menuY + menuHeight);
    glEnd();
    
    // Draw menu border
    glLineWidth(2.0f);
    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(menuX, menuY);
        glVertex2f(menuX + menuWidth, menuY);
        glVertex2f(menuX + menuWidth, menuY + menuHeight);
        glVertex2f(menuX, menuY + menuHeight);
    glEnd();
    
    // Draw title
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    float titleScale = 2.0f;
    float titleY = menuY + 40;
    render_text_centered(width * 0.5f, titleY, "GAME", titleScale);
    render_text_centered(width * 0.5f, titleY + 25, "PAUSED", titleScale);
    
    // Draw buttons
    glLineWidth(2.0f);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button* btn = &menu.buttons[i];
        
        // Button background
        if (btn->hovered) {
            glColor4f(0.3f, 0.3f, 0.3f, 0.9f);
        } else {
            glColor4f(0.15f, 0.15f, 0.15f, 0.9f);
        }
        
        glBegin(GL_QUADS);
            glVertex2f(btn->x, btn->y);
            glVertex2f(btn->x + btn->width, btn->y);
            glVertex2f(btn->x + btn->width, btn->y + btn->height);
            glVertex2f(btn->x, btn->y + btn->height);
        glEnd();
        
        // Button border
        if (btn->hovered) {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        }
        
        glBegin(GL_LINE_LOOP);
            glVertex2f(btn->x, btn->y);
            glVertex2f(btn->x + btn->width, btn->y);
            glVertex2f(btn->x + btn->width, btn->y + btn->height);
            glVertex2f(btn->x, btn->y + btn->height);
        glEnd();
        
        // Button text
        if (btn->hovered) {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
        }
        
        render_text_centered(btn->x + btn->width * 0.5f, 
                           btn->y + btn->height * 0.5f, 
                           btn->text, 1.5f);
    }
    
    // Restore OpenGL state
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}