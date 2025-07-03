// LT's Aim Train 21 - Main Entry Point
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "core/ltat21.h"
#include "core/window.h"
#include "core/renderer.h"
#include "game/game_state.h"
#include "utils/debug.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

int main() {
    debug_init();
    srand((unsigned int)time(NULL));
    
    // Initialize game state
    game_state_init();
    
    // Initialize window
    if (!window_init()) {
        return -1;
    }
    
    // Initialize renderer
    if (!renderer_init()) {
        window_cleanup();
        return -1;
    }
    
    // Initial target spawn
    spawn_target();
    
    // Main loop
    g_game.fps_last_time = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        // Update FPS counter
        double current_time = glfwGetTime();
        g_game.fps_frames++;
        
        if (current_time - g_game.fps_last_time >= FPS_UPDATE_INTERVAL) {
            g_game.current_fps = (int)(g_game.fps_frames / (current_time - g_game.fps_last_time));
            g_game.fps_frames = 0;
            g_game.fps_last_time = current_time;
        }
        
        // Render
        render_frame();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    renderer_cleanup();
    window_cleanup();
    debug_cleanup();
    
    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
#endif