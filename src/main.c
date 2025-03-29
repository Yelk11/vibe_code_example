#include "../include/game.h"
#include "../include/ui.h"
#include "../include/player.h"
#include <locale.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    long seed;
    if(argc < 1){
        seed = time(NULL);
    }else{
        seed = atol(argv[1]);
    }
    

    // Set up locale for UTF-8 support
    setlocale(LC_ALL, "");
    
    // Initialize game
    init_game(seed);
    
    // Run game loop
    game_loop();
    
    // Clean up
    cleanup_game();
    
    return 0;
} 