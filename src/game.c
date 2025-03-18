#include "game.h"
#include "map.h"
#include "player.h"
#include "common.h"
#include "quest.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

// Initialize game state
void init_game() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game state
    game_state = GAME_RUNNING;
    current_floor = 0;
    
    // Initialize UI
    init_ui();
    
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

// Show death screen and handle retry option
void show_death_screen() {
    int term_width, term_height;
    get_terminal_size(&term_width, &term_height);
    
    // Clear screen
    clear();
    
    // Draw death message
    attron(COLOR_PAIR(1));  // Red color for death message
    mvprintw(term_height/2 - 2, term_width/2 - 15, "YOU HAVE DIED!");
    attroff(COLOR_PAIR(1));
    
    // Draw stats
    attron(COLOR_PAIR(7));  // White color for stats
    mvprintw(term_height/2, term_width/2 - 20, "Level: %d", player.level);
    mvprintw(term_height/2 + 1, term_width/2 - 20, "Gold: %d", player.gold);
    mvprintw(term_height/2 + 2, term_width/2 - 20, "Experience: %d", player.exp);
    attroff(COLOR_PAIR(7));
    
    // Draw options
    attron(COLOR_PAIR(3));  // Yellow color for options
    mvprintw(term_height/2 + 4, term_width/2 - 20, "Press [R] to try again");
    mvprintw(term_height/2 + 5, term_width/2 - 20, "Press [Q] to quit");
    attroff(COLOR_PAIR(3));
    
    refresh();
    
    // Wait for input
    char input;
    while (1) {
        input = get_input();
        if (input == 'r' || input == 'R') {
            // Reset game state
            game_state = GAME_RUNNING;
            init_game();  // Reinitialize the game
            break;
        } else if (input == 'q' || input == 'Q') {
            game_state = GAME_OVER;
            break;
        }
    }
}

// Main game loop
void game_loop() {
    char input;
    int last_width = 0, last_height = 0;
    int current_width, current_height;
    
    while (game_state != GAME_OVER) {
        // Check for window resize
        get_terminal_size(&current_width, &current_height);
        if (current_width != last_width || current_height != last_height) {
            handle_resize();
            last_width = current_width;
            last_height = current_height;
        }
        
        // Update field of view
        update_fov();
        
        // Draw game state
        draw();
        
        // Get player input
        input = get_input();
        
        // Process input
        handle_input(input);
        
        // Check for items at player position
        check_items();
        
        // Update game state
        update_game();
        
        // Check if player is dead
        if (player.health <= 0) {
            show_death_screen();
        }
    }
    
    // Cleanup UI before exit
    cleanup_ui();
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