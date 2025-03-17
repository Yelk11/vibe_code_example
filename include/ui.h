#ifndef UI_H
#define UI_H

#include "common.h"

// UI initialization and cleanup
void init_ui(void);
void cleanup_ui(void);

// Drawing functions
void draw(void);
void update_camera(void);
void render_ui(void);

// Input handling
char get_input(void);

// Window management
void handle_resize(void);
void get_terminal_size(int* width, int* height);

// Message handling
void add_message(const char* fmt, ...);
void clear_messages(void);

#endif // UI_H 