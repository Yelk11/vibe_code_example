#include "game.h"
#include "ui.h"
#include "player.h"
#include <locale.h>

int main() {
    // Set up locale for UTF-8 support
    setlocale(LC_ALL, "");
    
    // Initialize game
    init_game();
    
    // Run game loop
    game_loop();
    
    // Clean up
    cleanup_game();
    
    return 0;
} 