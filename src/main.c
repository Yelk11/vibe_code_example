#include "game.h"
#include "map.h"
#include "player.h"
#include "quest.h"
#include "enemy.h"
#include <stdlib.h>
#include <time.h>

int main() {
    // Initialize game systems
    init_game();
    
    // Run game loop
    game_loop();
    
    return 0;
} 