#include "common.h"

// Global game state variables
Floor floors[MAX_FLOORS];
int current_floor = 0;
Player player;
int camera_x = 0;
int camera_y = 0;
int game_turn = 0;
MessageLog message_log = {0}; 