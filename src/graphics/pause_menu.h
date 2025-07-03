#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <stdbool.h>

// Menu button IDs
typedef enum {
    BUTTON_NONE = -1,
    BUTTON_RESUME = 0,
    BUTTON_SETTINGS,
    BUTTON_QUIT,
    BUTTON_COUNT
} MenuButton;

// Initialize pause menu
void pause_menu_init(void);

// Update pause menu (handle hover states)
void pause_menu_update(double mouseX, double mouseY);

// Handle click on pause menu
MenuButton pause_menu_handle_click(double mouseX, double mouseY);

// Draw the pause menu
void draw_pause_menu(void);

// Get currently hovered button
MenuButton pause_menu_get_hovered(void);

#endif // PAUSE_MENU_H