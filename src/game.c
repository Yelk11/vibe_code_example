#include "game.h"
#include "map.h"
#include "player.h"
#include "common.h"
#include "quest.h"
#include <stdlib.h>
#include <time.h>

// Global game state
GameState game_state;

// Initialize game state
void init_game() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game state
    game_state = GAME_RUNNING;
    current_floor = 0;
    
    // Initialize player
    init_player();
    
    // Initialize first floor
    Floor* floor = &floors[current_floor];
    init_floor(floor);
    
    // Initialize quests
    init_quests();
    
    // Initialize achievements
    init_achievements();
    
    // Add initial message
    add_message("Welcome to the dungeon! Use WASD to move.");
}

// Main game loop
void game_loop() {
    char input;
    while (game_state != GAME_OVER) {
        // Update field of view
        update_fov();
        
        // Draw game state
        draw();
        
        // Get player input
        input = getch();
        
        // Process input
        handle_input(input);
        
        // Check for items at player position
        check_items();
        
        // Update game state
        update_game();
        
        // Check if player is dead
        if (player.health <= 0) {
            add_message("You have died! Game Over.");
            game_state = GAME_OVER;
        }
    }
}

void update_game() {
    // Update enemies
    update_enemies();
    
    // Update quests
    update_quests();
    
    // Update achievements
    update_achievements();
}

void handle_input(char input) {
    switch (input) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            move_player(input);
            break;
        case 'i':
            view_inventory();
            break;
        case 'q':
            view_quests();
            break;
        case 'v':
            view_achievements();
            break;
        case 't': {
            // Find nearby NPC
            for (int i = 0; i < num_npcs; i++) {
                NPC* npc = &npcs[i];
                if (npc->active && npc->floor == current_floor &&
                    abs(npc->x - player.x) <= 1 && abs(npc->y - player.y) <= 1) {
                    handle_npc_interaction(npc);
                    break;
                }
            }
            break;
        }
        case 'Q':
            game_state = GAME_OVER;
            break;
    }
} 