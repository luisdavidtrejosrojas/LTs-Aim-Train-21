// aim_trainer_optimized.c - Optimized OpenGL 2.1 FPS Aim Trainer
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

// Include GLFW
#include <GLFW/glfw3.h>

// OpenGL headers
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
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

// Game constants
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define MOUSE_SENSITIVITY 0.003f
#define FOV 90.0f
#define SPHERE_SLICES 16  // Reduced from 32
#define SPHERE_STACKS 16  // Reduced from 32
#define MIN_TARGET_RADIUS 0.2f
#define MAX_TARGET_RADIUS 3.0f
#define TARGET_SIZE_STEP 0.1f

// Debug settings
#define DEBUG_LOG 0      // Disabled by default for performance
#define VERBOSE_LOG 0    
#define FPS_UPDATE_INTERVAL 0.25f  // Update FPS display 4 times per second

#if DEBUG_LOG
FILE* debug_file = NULL;
#endif

// Fast inline debug print
static inline void debug_print(const char* format, ...) {
#if DEBUG_LOG
    va_list args;
    va_start(args, format);
    if (debug_file) {
        vfprintf(debug_file, format, args);
        fflush(debug_file);
    }
    va_end(args);
#endif
}

// Vector3 structure
typedef struct {
    float x, y, z;
} Vec3;

// Global state
GLFWwindow* window = NULL;
GLUquadric* sphere_quad = NULL;  // Reuse quadric object

// Display lists for optimized rendering
GLuint floor_display_list = 0;
GLuint crosshair_display_list = 0;

// Cached calculations
float aspect_ratio;
float fov_rad;

// Game state
typedef struct {
    float yaw;
    float pitch;
    double last_mouse_x;
    double last_mouse_y;
    bool first_mouse;
    
    // Target
    Vec3 target_pos;
    float target_radius;
    
    // Stats
    int shots_fired;
    int shots_hit;
    
    // Cached values
    Vec3 cached_ray_dir;
    bool ray_dir_dirty;
    
    // FPS tracking
    double fps_last_time;
    int fps_frames;
    int current_fps;
    
    // Window state
    bool fullscreen;
    int windowed_width;
    int windowed_height;
    int windowed_x;
    int windowed_y;
} GameState;

GameState g_game = {
    .yaw = 0.0f,
    .pitch = 0.0f,
    .first_mouse = true,
    .target_pos = {0.0f, 0.0f, -7.0f},
    .target_radius = 1.0f,
    .shots_fired = 0,
    .shots_hit = 0,
    .ray_dir_dirty = true,
    .fps_last_time = 0.0,
    .fps_frames = 0,
    .current_fps = 0,
    .fullscreen = false,
    .windowed_width = WINDOW_WIDTH,
    .windowed_height = WINDOW_HEIGHT,
    .windowed_x = 0,
    .windowed_y = 0
};

// Fast vector normalization
static inline Vec3 vec3_normalize_fast(Vec3 v) {
    float inv_len = 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x *= inv_len;
    v.y *= inv_len;
    v.z *= inv_len;
    return v;
}

// Get cached ray direction
static inline Vec3 get_ray_direction() {
    if (g_game.ray_dir_dirty) {
        float yaw_rad = -g_game.yaw;
        float pitch_rad = g_game.pitch;
        float cos_pitch = cosf(pitch_rad);
        
        g_game.cached_ray_dir.x = sinf(yaw_rad) * cos_pitch;
        g_game.cached_ray_dir.y = sinf(pitch_rad);
        g_game.cached_ray_dir.z = -cosf(yaw_rad) * cos_pitch;
        
        g_game.ray_dir_dirty = false;
    }
    return g_game.cached_ray_dir;
}

// Optimized ray-sphere intersection
static inline bool ray_sphere_intersect_fast(Vec3 ray_dir, Vec3 sphere_center, float sphere_radius) {
    // Simplified for ray origin at (0,0,0)
    float a = 1.0f;  // Ray direction is normalized
    float b = -2.0f * (ray_dir.x * sphere_center.x + 
                       ray_dir.y * sphere_center.y + 
                       ray_dir.z * sphere_center.z);
    float c = sphere_center.x * sphere_center.x + 
              sphere_center.y * sphere_center.y + 
              sphere_center.z * sphere_center.z - 
              sphere_radius * sphere_radius;
    
    float discriminant = b * b - 4.0f * c;
    if (discriminant < 0) return false;
    
    // Early exit - we just need to know if we hit, not where
    float sqrt_disc = sqrtf(discriminant);
    return ((-b - sqrt_disc) * 0.5f > 0.0f) || ((-b + sqrt_disc) * 0.5f > 0.0f);
}

// Check hit
static inline bool check_hit() {
    Vec3 ray_dir = get_ray_direction();
    return ray_sphere_intersect_fast(ray_dir, g_game.target_pos, g_game.target_radius);
}

// Create floor display list
void create_floor_display_list() {
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
}

// Initialize OpenGL
void init_opengl() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Cache aspect ratio
    aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    fov_rad = FOV * 3.14159f / 180.0f;
    
    // Setup projection matrix once
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspect_ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    
    // Create reusable quadric
    sphere_quad = gluNewQuadric();
    gluQuadricNormals(sphere_quad, GLU_NONE);  // Don't need normals
    
    // Create display lists
    create_floor_display_list();
    
    // Optimize OpenGL state
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_FOG);
    glDisable(GL_DITHER);
    glShadeModel(GL_FLAT);
    
    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Set window title once
    glfwSetWindowTitle(window, "OpenGL 2.1 Aim Trainer");
}

// Draw crosshair (optimized)
void draw_crosshair() {
    // Setup 2D projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    // Use current window size for proper scaling in fullscreen
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Always green crosshair
    glColor3f(0.0f, 1.0f, 0.0f);
    
    glLineWidth(2.0f);
    
    // Draw crosshair at center of current window
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
    
    // Restore state
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Draw target (optimized)
static inline void draw_target() {
    glPushMatrix();
    glTranslatef(g_game.target_pos.x, g_game.target_pos.y, g_game.target_pos.z);
    glColor3f(0.0f, 1.0f, 1.0f);
    gluSphere(sphere_quad, g_game.target_radius, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
}

// Simple bitmap font - using basic ASCII characters
void render_char(float x, float y, char c, float scale) {
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
        case 'i':
            glVertex2f(3, 2); glVertex2f(3, 8);
            break;
        case 'z':
            glVertex2f(0, 0); glVertex2f(6, 0);
            glVertex2f(6, 0); glVertex2f(0, 8);
            glVertex2f(0, 8); glVertex2f(6, 8);
            break;
        case 'e':
            glVertex2f(0, 0); glVertex2f(0, 8);
            glVertex2f(0, 0); glVertex2f(5, 0);
            glVertex2f(0, 4); glVertex2f(4, 4);
            glVertex2f(0, 8); glVertex2f(5, 8);
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
        case ' ':
            // Space - no drawing needed
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

// Draw HUD (optimized)
void draw_hud() {
    // Get current window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Setup 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Draw FPS counter in top-left
    glColor3f(1.0f, 1.0f, 1.0f);  // White text
    glLineWidth(2.0f);
    char fps_text[32];
    sprintf(fps_text, "FPS: %d", g_game.current_fps);
    render_text(10, 10, fps_text, 2.0f);
    
    // Draw accuracy bar if we have shots
    if (g_game.shots_fired > 0) {
        float accuracy = (float)g_game.shots_hit / g_game.shots_fired;
        float bar_fill = 200.0f * accuracy;
        
        // Draw accuracy text
        char acc_text[32];
        sprintf(acc_text, "%d%% ", (int)(accuracy * 100.0f));
        glColor3f(0.0f, 1.0f, 0.0f);
        render_text(10, 45, acc_text, 1.5f);
        
        // Draw accuracy bar
        glBegin(GL_QUADS);
            // Background
            glColor3f(0.2f, 0.2f, 0.2f);
            glVertex2f(60, 50);
            glVertex2f(260, 50);
            glVertex2f(260, 70);
            glVertex2f(60, 70);
            
            // Green portion
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2f(60, 50);
            glVertex2f(60 + bar_fill, 50);
            glVertex2f(60 + bar_fill, 70);
            glVertex2f(60, 70);
            
            // Red portion
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(60 + bar_fill, 50);
            glVertex2f(260, 50);
            glVertex2f(260, 70);
            glVertex2f(60 + bar_fill, 70);
        glEnd();
    }
    
    // Draw target size indicator
    glColor3f(0.8f, 0.8f, 0.8f);  // Light gray
    char size_text[32];
    sprintf(size_text, "Size: %d", (int)(g_game.target_radius * 10));
    render_text(10, 85, size_text, 1.5f);
    
    // Visual size indicator bar
    float size_percent = (g_game.target_radius - MIN_TARGET_RADIUS) / (MAX_TARGET_RADIUS - MIN_TARGET_RADIUS);
    glBegin(GL_QUADS);
        // Background
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex2f(80, 90);
        glVertex2f(180, 90);
        glVertex2f(180, 100);
        glVertex2f(80, 100);
        
        // Size indicator
        glColor3f(0.5f, 0.5f, 1.0f);  // Light blue
        glVertex2f(80, 90);
        glVertex2f(80 + 100 * size_percent, 90);
        glVertex2f(80 + 100 * size_percent, 100);
        glVertex2f(80, 100);
    glEnd();
    
    // Restore state
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}



// Spawn target
void spawn_target() {
    g_game.target_pos.x = (float)(rand() % 7 - 3) * 0.8f;
    g_game.target_pos.y = (float)(rand() % 5 - 2) * 0.8f;
    g_game.target_pos.z = -6.0f - (float)(rand() % 4);
}

// Mouse callback
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
    
    // Clamp pitch
    if (g_game.pitch > 1.5f) g_game.pitch = 1.5f;
    else if (g_game.pitch < -1.5f) g_game.pitch = -1.5f;
    
    g_game.ray_dir_dirty = true;
}

// Mouse button callback
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        g_game.shots_fired++;
        
        if (check_hit()) {
            g_game.shots_hit++;
            spawn_target();
        }
    }
}

// Scroll wheel callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // Adjust target size based on scroll direction
    g_game.target_radius += (float)(yoffset * TARGET_SIZE_STEP);
    
    // Clamp to reasonable limits
    if (g_game.target_radius < MIN_TARGET_RADIUS) {
        g_game.target_radius = MIN_TARGET_RADIUS;
    } else if (g_game.target_radius > MAX_TARGET_RADIUS) {
        g_game.target_radius = MAX_TARGET_RADIUS;
    }
    
    debug_print("Target radius changed to: %.2f\n", g_game.target_radius);
}

// Toggle fullscreen
void toggle_fullscreen() {
    if (!g_game.fullscreen) {
        // Save current window position and size
        glfwGetWindowPos(window, &g_game.windowed_x, &g_game.windowed_y);
        glfwGetWindowSize(window, &g_game.windowed_width, &g_game.windowed_height);
        
        // Get monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        // Switch to fullscreen
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        g_game.fullscreen = true;
    } else {
        // Restore windowed mode
        glfwSetWindowMonitor(window, NULL, g_game.windowed_x, g_game.windowed_y, 
                           g_game.windowed_width, g_game.windowed_height, 0);
        g_game.fullscreen = false;
    }
    
    // Update projection matrix for new window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    aspect_ratio = (float)width / (float)height;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspect_ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Key callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        g_game.shots_fired = 0;
        g_game.shots_hit = 0;
    } else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        toggle_fullscreen();
    }
}

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    aspect_ratio = (float)width / (float)height;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV, aspect_ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// Main render function
static inline void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Apply camera rotation
    glRotatef(-g_game.pitch * 57.29578f, 1.0f, 0.0f, 0.0f);  // 180/PI = 57.29578
    glRotatef(-g_game.yaw * 57.29578f, 0.0f, 1.0f, 0.0f);
    
    // Draw 3D scene
    glCallList(floor_display_list);
    draw_target();
    
    // Draw 2D overlay
    draw_crosshair();
    draw_hud();
}

int main() {
#if DEBUG_LOG
    debug_file = fopen("aim_trainer_debug.log", "w");
    if (debug_file) {
        debug_print("=== Aim Trainer Debug Log ===\n");
        debug_print("Controls:\n");
        debug_print("  Mouse - Look around\n");
        debug_print("  Left Click - Shoot\n");
        debug_print("  Scroll Wheel - Adjust target size\n");
        debug_print("  R - Reset stats\n");
        debug_print("  F11 - Toggle fullscreen\n");
        debug_print("  ESC - Exit\n\n");
    }
#endif
    
    srand((unsigned int)time(NULL));
    srand((unsigned int)time(NULL));
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    // Window hints for performance
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
        "OpenGL 2.1 Aim Trainer (Optimized)", NULL, NULL);
    
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);  // Disable VSync
    
    // Setup callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize
    init_opengl();
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
        render();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    if (sphere_quad) gluDeleteQuadric(sphere_quad);
    if (floor_display_list) glDeleteLists(floor_display_list, 1);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
#if DEBUG_LOG
    if (debug_file) fclose(debug_file);
#endif
    
    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
#endif