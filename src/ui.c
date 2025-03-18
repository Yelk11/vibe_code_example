#include "ui.h"
#include "game.h"
#include "player.h"
#include "map.h"
#include "quest.h"
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <locale.h>

// Initialize ncurses
void init_ui() {
    // Set locale for UTF-8 support
    setlocale(LC_ALL, "");
    
    // Initialize ncurses
    initscr();
    
    // Enable keypad for special keys
    keypad(stdscr, TRUE);
    
    // Don't echo input
    noecho();
    
    // Don't wait for input
    cbreak();
    
    // Hide cursor
    curs_set(0);
    
    // Enable colors if available
    if (has_colors()) {
        start_color();
        // Define color pairs
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);
    }
    
    // Clear screen
    clear();
    refresh();
}

// Cleanup ncurses
void cleanup_ui() {
    endwin();
}

// Get terminal dimensions
void get_terminal_size(int* width, int* height) {
    getmaxyx(stdscr, *height, *width);
}

// Draw the game screen
void draw() {
    Floor* floor = current_floor_ptr();
    
    // Get terminal dimensions
    int term_width, term_height;
    get_terminal_size(&term_width, &term_height);
    
    // Calculate map scaling
    int map_width = min(MAP_WIDTH, term_width - 40);  // Leave space for messages
    int map_height = min(MAP_HEIGHT, term_height - 8);  // Leave space for status bar
    
    // Calculate starting position to center the map
    int start_x = max(0, player.x - map_width / 2);
    int start_y = max(0, player.y - map_height / 2);
    
    // Ensure we don't go past map boundaries
    if (start_x + map_width > MAP_WIDTH) start_x = MAP_WIDTH - map_width;
    if (start_y + map_height > MAP_HEIGHT) start_y = MAP_HEIGHT - map_height;
    
    // Clear screen
    clear();
    
    // Draw map and messages side by side
    for (int y = start_y; y < start_y + map_height; y++) {
        // Draw map row
        for (int x = start_x; x < start_x + map_width; x++) {
            // Check if tile is in field of view
            if (!floor->visible[y][x]) {
                if (floor->discovered[y][x]) {
                    mvaddch(y - start_y, x - start_x, floor->map[y][x]);
                }
                continue;
            }
            
            // Check for player
            if (x == player.x && y == player.y) {
                attron(COLOR_PAIR(2));  // Green for player
                mvaddch(y - start_y, x - start_x, '@');
                attroff(COLOR_PAIR(2));
                continue;
            }
            
            // Check for enemies
            int enemy_found = 0;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                Enemy* enemy = &floor->enemies[i];
                if (enemy->active && enemy->x == x && enemy->y == y) {
                    attron(COLOR_PAIR(1));  // Red for enemies
                    mvaddch(y - start_y, x - start_x, enemy->symbol);
                    attroff(COLOR_PAIR(1));
                    enemy_found = 1;
                    break;
                }
            }
            if (enemy_found) continue;
            
            // Check for NPCs
            int npc_found = 0;
            for (int i = 0; i < num_npcs; i++) {
                NPC* npc = &npcs[i];
                if (npc->active && npc->floor == current_floor && 
                    npc->x == x && npc->y == y) {
                    attron(COLOR_PAIR(4));  // Blue for NPCs
                    mvaddch(y - start_y, x - start_x, npc->symbol);
                    attroff(COLOR_PAIR(4));
                    npc_found = 1;
                    break;
                }
            }
            if (npc_found) continue;
            
            // Check for items
            int item_found = 0;
            for (int i = 0; i < MAX_ITEMS; i++) {
                Item* item = &floor->items[i];
                if (item->active && item->x == x && item->y == y) {
                    attron(COLOR_PAIR(3));  // Yellow for items
                    mvaddch(y - start_y, x - start_x, item->symbol);
                    attroff(COLOR_PAIR(3));
                    item_found = 1;
                    break;
                }
            }
            if (item_found) continue;
            
            // Draw terrain
            mvaddch(y - start_y, x - start_x, floor->map[y][x]);
        }
        
        // Draw status and messages in the right margin
        if (y == start_y) {
            attron(COLOR_PAIR(7));
            mvprintw(y - start_y, map_width + 2, "Health: %d/%d", 
                    player.health, player.max_health);
            attroff(COLOR_PAIR(7));
        } else if (y == start_y + 1) {
            attron(COLOR_PAIR(6));
            mvprintw(y - start_y, map_width + 2, "Mana: %d/%d", 
                    player.mana, player.max_mana);
            attroff(COLOR_PAIR(6));
        } else if (y == start_y + 2) {
            attron(COLOR_PAIR(3));
            mvprintw(y - start_y, map_width + 2, "Level: %d", player.level);
            attroff(COLOR_PAIR(3));
        } else if (y == start_y + 3) {
            attron(COLOR_PAIR(5));
            mvprintw(y - start_y, map_width + 2, "Exp: %d/%d", 
                    player.exp, player.exp_next);
            attroff(COLOR_PAIR(5));
        } else if (y == start_y + 4) {
            attron(COLOR_PAIR(3));
            mvprintw(y - start_y, map_width + 2, "Gold: %d", player.gold);
            attroff(COLOR_PAIR(3));
        } else if (y == start_y + 6) {
            attron(COLOR_PAIR(7));
            mvprintw(y - start_y, map_width + 2, "Messages:");
            attroff(COLOR_PAIR(7));
        } else if (y > start_y + 6) {
            int msg_index = y - (start_y + 7);
            if (msg_index < MAX_MESSAGES && strlen(messages[msg_index]) > 0) {
                attron(COLOR_PAIR(7));
                mvprintw(y - start_y, map_width + 2, "%s", messages[msg_index]);
                attroff(COLOR_PAIR(7));
            }
        }
    }
    
    // Draw controls at the bottom
    attron(COLOR_PAIR(7));
    mvprintw(term_height - 1, 0, 
             "Controls: [w/a/s/d] Move | [i]nventory | [q]uests | [v]iew achievements | [t]alk | [Q]uit");
    attroff(COLOR_PAIR(7));
    
    // Refresh the screen
    refresh();
}

// Update camera position
void update_camera() {
    // Center camera on player
    camera_x = player.x - SCREEN_WIDTH / 2;
    camera_y = player.y - SCREEN_HEIGHT / 2;
    
    // Keep camera within map bounds
    camera_x = max(0, min(camera_x, MAP_WIDTH - SCREEN_WIDTH));
    camera_y = max(0, min(camera_y, MAP_HEIGHT - SCREEN_HEIGHT));
}

// Handle window resize
void handle_resize() {
    // Clear and redraw
    clear();
    draw();
}

// Get input from user
char get_input() {
    return getch();
}

// Add a message to the message log
void add_message(const char* fmt, ...) {
    // Shift messages up
    for (int i = 0; i < MAX_MESSAGES - 1; i++) {
        strcpy(messages[i], messages[i + 1]);
    }
    
    // Format and add new message
    va_list args;
    va_start(args, fmt);
    vsnprintf(messages[MAX_MESSAGES - 1], MESSAGE_LENGTH, fmt, args);
    va_end(args);
}

// Clear all messages
void clear_messages() {
    memset(messages, 0, sizeof(messages));
}

// Render UI elements
void render_ui() {
    int term_width, term_height;
    get_terminal_size(&term_width, &term_height);
    
    // Draw status effects
    attron(COLOR_PAIR(7));
    mvprintw(term_height - 8, 0, "Status Effects:");
    attroff(COLOR_PAIR(7));
    
    int y = term_height - 7;
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        StatusEffect* effect = &player.status[i];
        if (effect->type != STATUS_NONE) {
            attron(COLOR_PAIR(3));  // Yellow for status effects
            mvprintw(y++, 2, "%s: %d turns", get_status_name(effect->type), effect->duration);
            attroff(COLOR_PAIR(3));
        }
    }
    
    // Draw abilities
    y = term_height - 4;
    attron(COLOR_PAIR(7));
    mvprintw(y++, 0, "Abilities:");
    attroff(COLOR_PAIR(7));
    
    for (int i = 0; i < player.num_abilities; i++) {
        Ability* ability = &player.abilities[i];
        attron(COLOR_PAIR(4));  // Blue for abilities
        mvprintw(y++, 2, "%s (Power: %d, Cooldown: %d)", 
                ability->name, ability->power, ability->current_cooldown);
        attroff(COLOR_PAIR(4));
    }
}

// View inventory
void view_inventory() {
    int term_width, term_height;
    get_terminal_size(&term_width, &term_height);
    
    // Clear screen
    clear();
    
    // Draw inventory header
    attron(COLOR_PAIR(7));
    mvprintw(0, term_width/2 - 10, "INVENTORY");
    attroff(COLOR_PAIR(7));
    
    // Draw inventory items
    int y = 2;
    for (int i = 0; i < MAX_INVENTORY; i++) {
        Item* item = &player.inventory[i];
        if (item->type != ITEM_NONE) {
            attron(COLOR_PAIR(3));  // Yellow for items
            mvprintw(y++, 2, "%d. %s", i + 1, item->name);
            attroff(COLOR_PAIR(3));
        }
    }
    
    // Draw equipment
    y += 2;
    attron(COLOR_PAIR(7));
    mvprintw(y++, 2, "EQUIPMENT");
    attroff(COLOR_PAIR(7));
    
    attron(COLOR_PAIR(4));  // Blue for equipment
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        Item* item = player.equipment[i];
        const char* slot_name;
        switch (i) {
            case SLOT_WEAPON: slot_name = "Weapon"; break;
            case SLOT_ARMOR: slot_name = "Armor"; break;
            case SLOT_RING: slot_name = "Ring"; break;
            case SLOT_AMULET: slot_name = "Amulet"; break;
            default: slot_name = "Unknown"; break;
        }
        mvprintw(y++, 4, "%s: %s", slot_name, item ? item->name : "None");
    }
    attroff(COLOR_PAIR(4));
    
    // Draw controls
    y = term_height - 2;
    attron(COLOR_PAIR(7));
    mvprintw(y, 2, "Controls: [1-%d] Use/Equip | [d] Drop | [e] Equip | [u] Unequip | [q] Back", MAX_INVENTORY);
    attroff(COLOR_PAIR(7));
    
    refresh();
}

// Save game state to file
void save_game(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        add_message("Failed to save game!");
        return;
    }
    
    // Save game data
    fwrite(&player, sizeof(Player), 1, file);
    fwrite(&current_floor, sizeof(int), 1, file);
    fwrite(floors, sizeof(Floor), MAX_FLOORS, file);
    fwrite(quests, sizeof(Quest), MAX_QUESTS, file);
    fwrite(&num_quests, sizeof(int), 1, file);
    fwrite(achievements, sizeof(Achievement), MAX_ACHIEVEMENTS, file);
    fwrite(&num_achievements, sizeof(int), 1, file);
    
    fclose(file);
    add_message("Game saved successfully!");
}

// Load game state from file
void load_game(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        add_message("Failed to load game!");
        return;
    }
    
    // Load game data
    fread(&player, sizeof(Player), 1, file);
    fread(&current_floor, sizeof(int), 1, file);
    fread(floors, sizeof(Floor), MAX_FLOORS, file);
    fread(quests, sizeof(Quest), MAX_QUESTS, file);
    fread(&num_quests, sizeof(int), 1, file);
    fread(achievements, sizeof(Achievement), MAX_ACHIEVEMENTS, file);
    fread(&num_achievements, sizeof(int), 1, file);
    
    fclose(file);
    add_message("Game loaded successfully!");
} 