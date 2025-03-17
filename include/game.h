#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "ui.h"

// Game initialization and main loop
void init_game(void);
int game_loop(void);

#endif // GAME_H 