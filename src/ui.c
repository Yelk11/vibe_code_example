#include "ui.h"
#include "game.h"
#include "player.h"
#include "map.h"
#include "quest.h"
#include <stdio.h>
#include <stdlib.h>

// Draw the game screen
void draw() {
    system("clear");  // Clear screen
    Floor* floor = current_floor_ptr();
    
    // Draw map
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Check if tile is in field of view
            if (!floor->visible[y][x]) {
                if (floor->discovered[y][x]) {
                    printf("%c", floor->map[y][x]);  // Show explored but not visible
                } else {
                    printf(" ");  // Not explored
                }
                continue;
            }
            
            // Check for player
            if (x == player.x && y == player.y) {
                printf("@");
                continue;
            }
            
            // Check for enemies
            int enemy_found = 0;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                Enemy* enemy = &floor->enemies[i];
                if (enemy->active && enemy->x == x && enemy->y == y) {
                    printf("%c", enemy->symbol);
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
                    printf("%c", npc->symbol);
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
                    printf("%c", item->symbol);
                    item_found = 1;
                    break;
                }
            }
            if (item_found) continue;
            
            // Draw terrain
            printf("%c", floor->map[y][x]);
        }
        printf("\n");
    }
    
    // Draw status bar
    printf("\nHealth: %d/%d | Mana: %d/%d | Level: %d | Exp: %d/%d | Gold: %d\n",
           player.health, player.max_health,
           player.mana, player.max_mana,
           player.level, player.exp, player.exp_next,
           player.gold);
    
    // Draw messages
    printf("\nMessages:\n");
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (strlen(messages[i]) > 0) {
            printf("%s\n", messages[i]);
        }
    }
    
    // Draw controls
    printf("\nControls: [w/a/s/d] Move | [i]nventory | [q]uests | [v]iew achievements | [t]alk | [Q]uit\n");
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

// Render UI elements
void render_ui() {
    // Draw status effects
    printf("\nStatus Effects:\n");
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        StatusEffect* effect = &player.status[i];
        if (effect->type != STATUS_NONE) {
            printf("%s: %d turns\n", get_status_name(effect->type), effect->duration);
        }
    }
    
    // Draw abilities
    printf("\nAbilities:\n");
    for (int i = 0; i < player.num_abilities; i++) {
        Ability* ability = &player.abilities[i];
        printf("[%c] %s", ability->key, ability->name);
        if (ability->current_cooldown > 0) {
            printf(" (CD: %d)", ability->current_cooldown);
        }
        printf("\n");
    }
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