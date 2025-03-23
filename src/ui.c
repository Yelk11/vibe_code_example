#include "ui.h"
#include "game.h"
#include "player.h"
#include "enemy.h"
#include "message.h"
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
            
            // Check for stairs
            char tile = floor->map[y][x];
            if (tile == '<' || tile == '>' || tile == '%') {
                attron(COLOR_PAIR(3));  // Yellow for stairs
                mvaddch(y - start_y, x - start_x, tile);
                attroff(COLOR_PAIR(3));
                continue;
            }
            
            // Draw terrain
            mvaddch(y - start_y, x - start_x, tile);
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
            if (msg_index < message_log.num_messages && strlen(message_log.messages[message_log.num_messages - 1 - msg_index]) > 0) {
                attron(COLOR_PAIR(7));
                mvprintw(y - start_y, map_width + 2, "%s", message_log.messages[message_log.num_messages - 1 - msg_index]);
                attroff(COLOR_PAIR(7));
            }
        }
    }
    
    // Draw controls at the bottom
    attron(COLOR_PAIR(7));
    mvprintw(term_height - 1, 0, 
             "Controls: [w/a/s/d] Move | [i]nventory | [Q]uit");
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
    
    while (1) {
        // Clear screen
        clear();
        
        // Calculate center position
        int center_x = term_width / 2;
        int center_y = term_height / 2;
        
        // Draw inventory header
        attron(COLOR_PAIR(7));
        mvprintw(center_y - 10, center_x - 15, "=== Inventory (%d/%d) ===", player.num_items, MAX_INVENTORY);
        mvhline(center_y - 9, center_x - 20, '-', 40);  // Draw separator line
        attroff(COLOR_PAIR(7));
        
        // Draw equipment section first
        int y = center_y - 8;
        attron(COLOR_PAIR(7));
        mvprintw(y++, center_x - 18, "Equipped:");
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
            if (item) {
                if (item->type == ITEM_WEAPON || item->type == ITEM_ARMOR) {
                    mvprintw(y++, center_x - 16, "%s: %s (Power: %d, Value: %d)", 
                            slot_name, item->name, item->power, item->value);
                } else {
                    mvprintw(y++, center_x - 16, "%s: %s", slot_name, item->name);
                }
            } else {
                mvprintw(y++, center_x - 16, "%s: None", slot_name);
            }
        }
        attroff(COLOR_PAIR(4));
        
        // Draw inventory items
        y += 2;  // Add spacing between sections
        attron(COLOR_PAIR(7));
        mvprintw(y++, center_x - 18, "Items:");
        attroff(COLOR_PAIR(7));
        
        int item_count = 0;
        for (int i = 0; i < MAX_INVENTORY; i++) {
            Item* item = &player.inventory[i];
            if (item->type != ITEM_NONE && item->active) {  // Only show active items
                attron(COLOR_PAIR(3));  // Yellow for items
                if (item->type == ITEM_WEAPON || item->type == ITEM_ARMOR) {
                    mvprintw(y++, center_x - 16, "%d. %s (Power: %d, Value: %d)", 
                            ++item_count, item->name, item->power, item->value);
                } else if (item->type == ITEM_POTION) {
                    mvprintw(y++, center_x - 16, "%d. %s (Heals: %d, Value: %d)", 
                            ++item_count, item->name, item->power, item->value);
                } else {
                    mvprintw(y++, center_x - 16, "%d. %s (Value: %d)", 
                            ++item_count, item->name, item->value);
                }
                attroff(COLOR_PAIR(3));
            }
        }
        
        // Draw controls at the bottom
        y = center_y + 8;
        attron(COLOR_PAIR(7));
        mvprintw(y, center_x - 20, "Commands: (u)se item, (d)rop item, (e)quip item, (q)uit inventory");
        mvprintw(y + 1, center_x - 20, "Enter command: ");
        attroff(COLOR_PAIR(7));
        
        refresh();
        
        // Get input
        char cmd = getch();
        
        if (cmd == 'q') break;
        
        if (cmd == 'u' || cmd == 'd' || cmd == 'e') {
            // Get item number
            mvprintw(y + 1, center_x - 20, "Enter item number (1-%d): ", item_count);
            refresh();
            
            char num_str[16] = {0};
            int num_pos = 0;
            while (1) {
                char c = getch();
                if (c == '\n') break;
                if (c == 27) { // ESC key
                    num_pos = 0;
                    break;
                }
                if (num_pos < 15 && c >= '0' && c <= '9') {
                    num_str[num_pos++] = c;
                    mvprintw(y + 1, center_x - 20 + 20 + num_pos - 1, "%c", c);
                    refresh();
                }
            }
            
            if (num_pos > 0) {
                int index = atoi(num_str) - 1;
                if (index >= 0 && index < item_count) {
                    // Find the actual item index
                    int actual_index = -1;
                    int current_count = -1;
                    for (int i = 0; i < MAX_INVENTORY; i++) {
                        if (player.inventory[i].type != ITEM_NONE && player.inventory[i].active) {
                            current_count++;
                            if (current_count == index) {
                                actual_index = i;
                                break;
                            }
                        }
                    }
                    
                    if (actual_index >= 0) {
                        switch (cmd) {
                            case 'u':
                                use_item(&player.inventory[actual_index]);
                                // Item will be deactivated by use_item if consumed
                                break;
                            case 'd':
                                drop_item(actual_index);
                                // Item will be removed by drop_item
                                break;
                            case 'e':
                                equip_item(actual_index);
                                // Item will be removed by equip_item
                                break;
                        }
                        // Brief pause to show the result message
                        refresh();
                        napms(500);  // 500ms delay to show the message
                    }
                }
            }
        }
    }
}

// Render map
void render_map() {
    Floor* floor = current_floor_ptr();
    
    // Clear screen
    clear();
    
    // Draw map
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (floor->visible[y][x]) {
                char tile = floor->map[y][x];
                
                // Apply colors based on terrain type and special tiles
                if (tile == '<' || tile == '>' || tile == '%') {
                    attron(COLOR_PAIR(3));  // Yellow for stairs
                } else {
                    switch (floor->terrain[y][x]) {
                        case TERRAIN_WATER:
                            attron(COLOR_PAIR(4));
                            break;
                        case TERRAIN_LAVA:
                            attron(COLOR_PAIR(1));
                            break;
                        case TERRAIN_GRASS:
                            attron(COLOR_PAIR(2));
                            break;
                        case TERRAIN_TRAP:
                            attron(COLOR_PAIR(5));
                            break;
                        default:
                            attron(COLOR_PAIR(7));
                            break;
                    }
                }
                
                mvaddch(y, x, tile);
                attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(7));
            } else if (floor->discovered[y][x]) {
                mvaddch(y, x, floor->map[y][x] | A_DIM);
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }
}

// Render player
void render_player() {
    attron(COLOR_PAIR(2));
    mvaddch(player.y, player.x, '@');
    attroff(COLOR_PAIR(2));
}

// Render messages
void render_messages() {
    int start_y = MAP_HEIGHT + 1;
    
    // Draw message box border
    attron(COLOR_PAIR(6));
    mvhline(start_y - 1, 0, '-', MAP_WIDTH);
    attroff(COLOR_PAIR(6));
    
    // Draw messages
    attron(COLOR_PAIR(7));
    for (int i = 0; i < message_log.num_messages && i < 5; i++) {
        mvprintw(start_y + i, 1, "%s", message_log.messages[i]);
    }
    attroff(COLOR_PAIR(7));
}

// Render status
void render_status() {
    int start_x = MAP_WIDTH + 2;
    int start_y = 1;
    
    // Draw status box border
    attron(COLOR_PAIR(6));
    mvvline(0, MAP_WIDTH + 1, '|', MAP_HEIGHT);
    attroff(COLOR_PAIR(6));
    
    // Draw player stats
    attron(COLOR_PAIR(7));
    mvprintw(start_y++, start_x, "Level: %d", player.level);
    mvprintw(start_y++, start_x, "HP: %d/%d", player.health, player.max_health);
    mvprintw(start_y++, start_x, "MP: %d/%d", player.mana, player.max_mana);
    mvprintw(start_y++, start_x, "XP: %d/%d", player.exp, player.exp_next);
    mvprintw(start_y++, start_x, "Power: %d", player.power);
    mvprintw(start_y++, start_x, "Defense: %d", player.defense);
    mvprintw(start_y++, start_x, "Gold: %d", player.gold);
    
    // Draw floor info
    start_y++;
    mvprintw(start_y++, start_x, "Floor: %d", current_floor + 1);
    
    // Draw status effects
    start_y++;
    mvprintw(start_y++, start_x, "Status:");
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        if (player.status[i].type != STATUS_NONE) {
            mvprintw(start_y++, start_x, "%d: %d turns", 
                    player.status[i].type, player.status[i].duration);
        }
    }
    
    // Draw abilities
    start_y++;
    mvprintw(start_y++, start_x, "Abilities:");
    for (int i = 0; i < player.num_abilities; i++) {
        if (player.abilities[i].current_cooldown > 0) {
            mvprintw(start_y++, start_x, "%c) %s (%d)",
                    player.abilities[i].key,
                    player.abilities[i].name,
                    player.abilities[i].current_cooldown);
        } else {
            mvprintw(start_y++, start_x, "%c) %s",
                    player.abilities[i].key,
                    player.abilities[i].name);
        }
    }
    attroff(COLOR_PAIR(7));
}

// Refresh screen
void refresh_screen() {
    refresh();
}

// Save game state
void save_game(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        add_message("Failed to save game");
        return;
    }
    
    // Save game state
    fwrite(&current_floor, sizeof(int), 1, file);
    fwrite(&game_turn, sizeof(int), 1, file);
    
    // Save floors
    fwrite(floors, sizeof(Floor), MAX_FLOORS, file);
    
    // Save player
    fwrite(&player, sizeof(Player), 1, file);
    
    // Save message log
    fwrite(&message_log, sizeof(MessageLog), 1, file);
    
    fclose(file);
    add_message("Game saved successfully");
}

// Load game state
void load_game(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        add_message("Failed to load game");
        return;
    }
    
    // Load game state
    fread(&current_floor, sizeof(int), 1, file);
    fread(&game_turn, sizeof(int), 1, file);
    
    // Load floors
    fread(floors, sizeof(Floor), MAX_FLOORS, file);
    
    // Load player
    fread(&player, sizeof(Player), 1, file);
    
    // Load message log
    fread(&message_log, sizeof(MessageLog), 1, file);
    
    fclose(file);
    add_message("Game loaded successfully");
} 