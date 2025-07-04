#include "window.h"
#include "ltat21.h"
#include "sound.h"
#include "../game/game_state.h"
#include "../graphics/pause_menu.h"
#include "../utils/debug.h"
#include <stdio.h>

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

GLFWwindow* window = NULL;
extern float aspect_ratio; // From renderer.c

bool window_init(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, LTAT21_NAME, NULL, NULL);
    
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable VSync
    
    // Set minimum window size to 1024x768
    glfwSetWindowSizeLimits(window, 1024, 768, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    // Setup callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return true;
}

void window_cleanup(void) {
    if (window) {
        glfwDestroyWindow(window);
        window = NULL;
    }
    glfwTerminate();
}

void update_projection(void) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    aspect_ratio = (float)width / (float)height;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspect_ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void toggle_fullscreen(void) {
    if (!g_game.fullscreen) {
        glfwGetWindowPos(window, &g_game.windowed_x, &g_game.windowed_y);
        glfwGetWindowSize(window, &g_game.windowed_width, &g_game.windowed_height);
        
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        g_game.fullscreen = true;
    } else {
        glfwSetWindowMonitor(window, NULL, g_game.windowed_x, g_game.windowed_y, 
                           g_game.windowed_width, g_game.windowed_height, 0);
        g_game.fullscreen = false;
    }
    
    update_projection();
}

// Callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_game.paused) {
        // Update pause menu hover state
        pause_menu_update(xpos, ypos);
        return;
    }
    
    if (g_game.first_mouse) {
        g_game.last_mouse_x = xpos;
        g_game.last_mouse_y = ypos;
        g_game.first_mouse = false;
        return;
    }
    
    double dx = xpos - g_game.last_mouse_x;
    double dy = g_game.last_mouse_y - ypos;
    
    g_game.last_mouse_x = xpos;
    g_game.last_mouse_y = ypos;
    
    g_game.yaw -= (float)(dx * MOUSE_SENSITIVITY);
    g_game.pitch += (float)(dy * MOUSE_SENSITIVITY);
    
    if (g_game.pitch > 1.5f) g_game.pitch = 1.5f;
    else if (g_game.pitch < -1.5f) g_game.pitch = -1.5f;
    
    g_game.ray_dir_dirty = true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (g_game.paused) {
            // Handle menu click
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            MenuButton clicked = pause_menu_handle_click(xpos, ypos);
            
            switch (clicked) {
                case BUTTON_RESUME:
                    g_game.paused = false;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    g_game.first_mouse = true;
                    break;
                case BUTTON_QUIT:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                case BUTTON_SETTINGS:
                    // Placeholder - just play a sound for now
                    sound_play(SOUND_HIT);
                    break;
                default:
                    break;
            }
            return;
        }
        
        if (check_hit() && !g_game.hit_animating) {
            g_game.hit_animating = true;
            g_game.hit_animation_start = glfwGetTime();
            sound_play(SOUND_HIT);
        } else if (!g_game.hit_animating) {
            sound_play(SOUND_MISS);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    g_game.target_radius += (float)(yoffset * TARGET_SIZE_STEP);
    
    if (g_game.target_radius < MIN_TARGET_RADIUS) {
        g_game.target_radius = MIN_TARGET_RADIUS;
    } else if (g_game.target_radius > MAX_TARGET_RADIUS) {
        g_game.target_radius = MAX_TARGET_RADIUS;
    }
    
    debug_print("Target radius changed to: %.2f\n", g_game.target_radius);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (!g_game.paused) {
            // Pause the game
            g_game.paused = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwGetCursorPos(window, &g_game.pause_mouse_x, &g_game.pause_mouse_y);
        } else {
            // Unpause
            g_game.paused = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            g_game.first_mouse = true;
        }
    } else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        toggle_fullscreen();
    } else if (key == GLFW_KEY_MINUS && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        float volume = sound_get_volume() - 0.1f;
        sound_set_volume(volume);
        g_game.show_volume = true;
        g_game.volume_display_start = glfwGetTime();
        debug_print("Volume: %.0f%%\n", sound_get_volume() * 100.0f);
    } else if (key == GLFW_KEY_EQUAL && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        float volume = sound_get_volume() + 0.1f;
        sound_set_volume(volume);
        g_game.show_volume = true;
        g_game.volume_display_start = glfwGetTime();
        debug_print("Volume: %.0f%%\n", sound_get_volume() * 100.0f);
    } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        // Toggle mute
        static float saved_volume = 0.5f;
        float current = sound_get_volume();
        if (current > 0.0f) {
            saved_volume = current;
            sound_set_volume(0.0f);
        } else {
            sound_set_volume(saved_volume);
        }
        g_game.show_volume = true;
        g_game.volume_display_start = glfwGetTime();
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    update_projection();
}