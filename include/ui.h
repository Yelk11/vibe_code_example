#ifndef UI_H
#define UI_H

#include "common.h"

// UI initialization and cleanup
void init_ui(void);
void cleanup_ui(void);

// Rendering functions
void render_map(void);
void render_enemies(void);
void render_messages(void);
void render_status(void);
void refresh_screen(void);

// Menu functions
void show_inventory(void);
void show_equipment(void);
void show_character_screen(void);
void show_help_screen(void);
void show_death_screen(void);

// Drawing functions

void update_camera(void);




// Window management
void get_terminal_size(int* width, int* height);

// Message handling
void add_message(const char* fmt, ...);
void clear_messages(void);

#endif // UI_H 