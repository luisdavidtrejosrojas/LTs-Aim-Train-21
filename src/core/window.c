#include "window.h"
#include "ltat21.h"
#include "../game/game_state.h"
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
        if (check_hit()) {
            spawn_target();
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
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        toggle_fullscreen();
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    update_projection();
}