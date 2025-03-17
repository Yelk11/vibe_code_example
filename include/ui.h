#ifndef UI_H
#define UI_H

#include "common.h"

// Function declarations
void draw(void);
void update_camera(void);
void render_ui(void);
void save_game(const char* filename);
void load_game(const char* filename);
void get_terminal_size(int* width, int* height);
void handle_resize(void);

#endif // UI_H 