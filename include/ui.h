#ifndef UI_H
#define UI_H

#include "common.h"

// UI and display functions
void draw(void);
void update_camera(void);
void render_ui(void);
void save_game(const char* filename);
void load_game(const char* filename);

#endif // UI_H 