#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "ui.h"

// Game state functions
void init_game(long seed);
void cleanup_game(void);
void game_loop(void);
void handle_input(int input);
void update_game(void);
void render_game(void);

#endif // GAME_H 