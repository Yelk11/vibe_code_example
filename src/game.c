#include "game.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

// Initialize game state
void init_game() {
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize UI
    init_ui();
    
    // Initialize game state
    current_floor = 0;
    game_turn = 0;
    
    // Initialize message log
    message_log.num_messages = 0;
    
    // Initialize player
    init_player();
    
    // Initialize first floor
    init_floor(current_floor);
    
    // Add welcome message
    add_message("Welcome to the dungeon!");
}

// Clean up game resources
void cleanup_game() {
    cleanup_ui();
}

// Main game loop
void game_loop() {
    int input;
    
    while (1) {
        // Update field of view
        update_fov();
        
        // Render game state
        render_game();
        
        // Get player input
        input = getch();
        
        // Handle input
        handle_input(input);
        
        // Update game state
        update_game();
        
        // Check if player is dead
        if (player.health <= 0) {
            show_death_screen();
            break;
        }
    }
}

// Handle player input
void handle_input(int input) {
    int dx = 0, dy = 0;
    
    switch (input) {
        case 'a': dx = -1; break;  // Move left
        case 'd': dx = 1; break;   // Move right
        case 'w': dy = -1; break;  // Move up
        case 's': dy = 1; break;   // Move down
        case 'q': dx = -1; dy = -1; break;  // Move diagonally up-left
        case 'e': dx = 1; dy = -1; break;   // Move diagonally up-right
        case 'z': dx = -1; dy = 1; break;   // Move diagonally down-left
        case 'c': dx = 1; dy = 1; break;    // Move diagonally down-right
        case 'i': handle_inventory(); break; // Open inventory
        case '.': break;  // Wait one turn
        case 'Q': cleanup_game(); exit(0); break;  // Quit game
    }
    
    if (dx != 0 || dy != 0) {
        move_player(dx, dy);
    }
}

// Update game state
void update_game() {
    // Update enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &current_floor_ptr()->enemies[i];
        if (enemy->active) {
            update_enemy(enemy);
        }
    }
    
    // Update status effects
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        if (player.status[i].type != STATUS_NONE) {
            if (--player.status[i].duration <= 0) {
                player.status[i].type = STATUS_NONE;
            }
        }
    }
    
    // Update ability cooldowns
    for (int i = 0; i < player.num_abilities; i++) {
        if (player.abilities[i].current_cooldown > 0) {
            player.abilities[i].current_cooldown--;
        }
    }
    
    // Regenerate mana
    if (player.mana < player.max_mana) {
        player.mana += player.mana_regen;
        if (player.mana > player.max_mana) {
            player.mana = player.max_mana;
        }
    }
    
    game_turn++;
}

// Render game state
void render_game() {
    // First render the map
    render_map();
    
    // Then render items, enemies, and player
    Floor* floor = current_floor_ptr();
    
    // Render items
    for (int i = 0; i < MAX_ITEMS; i++) {
        Item* item = &floor->items[i];
        if (item->active && floor->visible[item->y][item->x]) {
            attron(COLOR_PAIR(3));  // Yellow for items
            mvaddch(item->y, item->x, item->symbol);
            attroff(COLOR_PAIR(3));
        }
    }
    
    // Render enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &floor->enemies[i];
        if (enemy->active && floor->visible[enemy->y][enemy->x]) {
            attron(COLOR_PAIR(1));  // Red for enemies
            mvaddch(enemy->y, enemy->x, enemy->symbol);
            attroff(COLOR_PAIR(1));
        }
    }
    
    // Finally render the player
    attron(COLOR_PAIR(2));  // Green for player
    mvaddch(player.y, player.x, '@');
    attroff(COLOR_PAIR(2));
    
    // Render UI elements
    render_messages();
    render_status();
    refresh_screen();
}

// Show death screen and handle retry option
void show_death_screen() {
    clear();
    mvprintw(SCREEN_HEIGHT/2 - 2, SCREEN_WIDTH/2 - 5, "YOU DIED!");
    mvprintw(SCREEN_HEIGHT/2, SCREEN_WIDTH/2 - 12, "Press any key to exit");
    refresh();
    getch();
} 