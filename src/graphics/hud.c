#include "hud.h"
#include "../core/window.h"
#include "../game/game_state.h"
#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
#elif defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

void draw_hud(void) {
    // HUD is now empty - no FPS counter
}