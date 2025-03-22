#ifndef GLOBALS_H
#define GLOBALS_H

#include "common.h"

// Global variables
extern int camera_x;
extern int camera_y;
extern char messages[MAX_MESSAGES][MESSAGE_LENGTH];
extern int current_floor;
extern Floor floors[MAX_FLOORS];
extern Player player;
extern int game_turn;
extern MessageLog message_log;

// Player stats
extern int kill_count;
extern int gold_collected;

#endif // GLOBALS_H 