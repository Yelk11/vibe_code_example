#include "globals.h"
#include "common.h"

// Game state
Floor floors[MAX_FLOORS];
int current_floor = 0;
int game_turn = 0;

// Message log
char messages[MAX_MESSAGES][MESSAGE_LENGTH];
MessageLog message_log = {0};

// Player stats
int kill_count = 0;
int gold_collected = 0;

// Global variables
int camera_x = 0;
int camera_y = 0;
Player player; 