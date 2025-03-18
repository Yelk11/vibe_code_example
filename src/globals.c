#include "common.h"
#include "game.h"

// Global variables
int camera_x = 0;
int camera_y = 0;
char messages[MAX_MESSAGES][MESSAGE_LENGTH];
int current_floor = 0;
Floor floors[MAX_FLOORS];
Player player;
int game_turn = 0;
MessageLog message_log = {0};
GameState game_state;

// Quest system globals
Quest quests[MAX_QUESTS];
int num_quests = 0;
NPC npcs[MAX_NPCS];
int num_npcs = 0;
Achievement achievements[MAX_ACHIEVEMENTS];
int num_achievements = 0; 