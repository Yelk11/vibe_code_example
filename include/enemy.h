#ifndef ENEMY_H
#define ENEMY_H

#include "common.h"

// Enemy management functions
void init_enemies(void);
void move_enemies(void);
void check_combat(void);
void handle_combat(Enemy* enemy);

#endif // ENEMY_H 