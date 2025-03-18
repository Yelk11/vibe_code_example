#ifndef GLOBALS_H
#define GLOBALS_H

#include "common.h"

// Game state
extern Floor floors[MAX_FLOORS];
extern int current_floor;
extern int game_turn;

// Message log
extern MessageLog message_log;

// Player stats
extern int kill_count;
extern int gold_collected;

#endif // GLOBALS_H 