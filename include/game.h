#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "item.h"
#include "ui.h"
#include "quest.h"

// Game states
typedef enum {
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

// Global game state
extern GameState game_state;

// Game initialization and main loop
void init_game(void);
void game_loop(void);

// Game systems initialization
void init_enemies(void);
void init_quests(void);
void init_npcs(void);
void init_dialogue(void);
void init_achievements(void);

// Game systems update
void update_enemies(void);
void view_quests(void);

// Functions
void handle_input(char input);
void update_game(void);
void draw(void);
void add_message(const char* fmt, ...);

#endif // GAME_H 