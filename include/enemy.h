#ifndef ENEMY_H
#define ENEMY_H

#include "common.h"

// Enemy functions
void init_enemies(void);
void update_enemy(Enemy* enemy);
void spawn_enemy(int x, int y, EnemyType type);
void spawn_floor_enemies(void);
void kill_enemy(Enemy* enemy);
void move_enemy(Enemy* enemy, int dx, int dy);
void enemy_attack(Enemy* enemy, int target_x, int target_y);

#endif // ENEMY_H 