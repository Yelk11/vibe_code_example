#include "enemy.h"
#include "item.h"
#include "player.h"

// Initialize enemies
void init_enemies() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        // Pick a random room (not the first one where player starts)
        int room_idx = random_range(1, floor->num_rooms - 1);
        Room* room = &floor->rooms[room_idx];
        
        // Place enemy randomly within the room
        floor->enemies[i].x = random_range(room->x + 1, room->x + room->width - 2);
        floor->enemies[i].y = random_range(room->y + 1, room->y + room->height - 2);
        
        // Set enemy type and stats based on floor level
        EnemyType type = (EnemyType)(rand() % (current_floor == MAX_FLOORS - 1 ? MAX_ENEMY_TYPES : MAX_ENEMY_TYPES - 1));
        floor->enemies[i].type = type;
        
        switch(type) {
            case ENEMY_BASIC:
                strcpy(floor->enemies[i].name, "Goblin");
                floor->enemies[i].symbol = 'g';
                floor->enemies[i].health = 30 + current_floor * 10;
                floor->enemies[i].max_health = floor->enemies[i].health;
                floor->enemies[i].power = 5 + current_floor * 2;
                floor->enemies[i].defense = 2 + current_floor;
                floor->enemies[i].speed = 1;
                floor->enemies[i].range = 1;
                floor->enemies[i].exp_value = 20 + current_floor * 5;
                break;
                
            case ENEMY_FAST:
                strcpy(floor->enemies[i].name, "Wolf");
                floor->enemies[i].symbol = 'w';
                floor->enemies[i].health = 20 + current_floor * 8;
                floor->enemies[i].max_health = floor->enemies[i].health;
                floor->enemies[i].power = 4 + current_floor * 2;
                floor->enemies[i].defense = 1 + current_floor;
                floor->enemies[i].speed = 2;
                floor->enemies[i].range = 1;
                floor->enemies[i].exp_value = 25 + current_floor * 6;
                break;
                
            case ENEMY_RANGED:
                strcpy(floor->enemies[i].name, "Archer");
                floor->enemies[i].symbol = 'a';
                floor->enemies[i].health = 25 + current_floor * 7;
                floor->enemies[i].max_health = floor->enemies[i].health;
                floor->enemies[i].power = 6 + current_floor * 2;
                floor->enemies[i].defense = 1 + current_floor;
                floor->enemies[i].speed = 1;
                floor->enemies[i].range = 3;
                floor->enemies[i].exp_value = 30 + current_floor * 7;
                break;
                
            case ENEMY_BOSS:
                strcpy(floor->enemies[i].name, "Dragon");
                floor->enemies[i].symbol = 'D';
                floor->enemies[i].health = 100 + current_floor * 20;
                floor->enemies[i].max_health = floor->enemies[i].health;
                floor->enemies[i].power = 15 + current_floor * 3;
                floor->enemies[i].defense = 5 + current_floor * 2;
                floor->enemies[i].speed = 2;
                floor->enemies[i].range = 2;
                floor->enemies[i].exp_value = 100 + current_floor * 20;
                break;
        }
        
        floor->enemies[i].active = 1;
    }
}

// Move enemies based on their type and speed
void move_enemies() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!floor->enemies[i].active) continue;
        
        Enemy* enemy = &floor->enemies[i];
        int moves = enemy->speed;  // Fast enemies move multiple times
        
        while (moves > 0) {
            int new_x = enemy->x;
            int new_y = enemy->y;
            
            // Ranged enemies try to maintain distance
            if (enemy->type == ENEMY_RANGED) {
                int dx = player.x - enemy->x;
                int dy = player.y - enemy->y;
                int dist = (int)sqrt(dx*dx + dy*dy);
                
                if (dist < enemy->range) {
                    // Move away from player
                    new_x = enemy->x - (dx > 0 ? 1 : -1);
                    new_y = enemy->y - (dy > 0 ? 1 : -1);
                } else if (dist > enemy->range + 1) {
                    // Move toward player
                    new_x = enemy->x + (dx > 0 ? 1 : -1);
                    new_y = enemy->y + (dy > 0 ? 1 : -1);
                }
            } else {
                // Other enemies move toward player
                int dx = player.x - enemy->x;
                int dy = player.y - enemy->y;
                
                if (abs(dx) > abs(dy)) {
                    new_x += (dx > 0) ? 1 : -1;
                } else {
                    new_y += (dy > 0) ? 1 : -1;
                }
            }
            
            // Check if new position is valid
            if (new_x > 0 && new_x < MAP_WIDTH-1 && 
                new_y > 0 && new_y < MAP_HEIGHT-1 && 
                floor->map[new_y][new_x] != '#' && 
                !(new_x == player.x && new_y == player.y)) {
                
                // Check for collision with other enemies
                int collision = 0;
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (j != i && floor->enemies[j].active &&
                        new_x == floor->enemies[j].x && 
                        new_y == floor->enemies[j].y) {
                        collision = 1;
                        break;
                    }
                }
                
                if (!collision) {
                    enemy->x = new_x;
                    enemy->y = new_y;
                }
            }
            
            moves--;
        }
    }
}

// Check for combat and handle different enemy types
void check_combat() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!floor->enemies[i].active) continue;
        
        Enemy* enemy = &floor->enemies[i];
        int dx = player.x - enemy->x;
        int dy = player.y - enemy->y;
        int dist = (int)sqrt(dx*dx + dy*dy);
        
        if (dist <= enemy->range) {
            // Check for dodge
            if (rand() % 100 < player.dodge_chance) {
                add_message("You dodged %s's attack!", enemy->name);
                continue;
            }
            
            int player_damage = max(1, player.power - enemy->defense);
            int enemy_damage = max(1, enemy->power - player.defense);
            
            // Check for critical hit
            if (rand() % 100 < player.critical_chance) {
                player_damage *= 2;
                add_message("Critical hit!");
            }
            
            if (enemy->type == ENEMY_RANGED && dist <= 1) {
                enemy_damage = enemy_damage / 2;
            }
            
            if (enemy->type == ENEMY_BOSS && game_turn % 3 == 0) {
                enemy_damage *= 2;
                add_message("The %s uses a special attack!", enemy->name);
                
                // Boss can apply status effects
                if (rand() % 100 < 30) {  // 30% chance
                    switch(rand() % 3) {
                        case 0:
                            apply_status_effect(STATUS_POISON, 3, enemy_damage/3);
                            break;
                        case 1:
                            apply_status_effect(STATUS_BURNING, 3, enemy_damage/3);
                            break;
                        case 2:
                            apply_status_effect(STATUS_FROZEN, 3, 2);
                            break;
                    }
                }
            }
            
            player.health -= enemy_damage;
            enemy->health -= player_damage;
            
            add_message("Combat with %s! You deal %d damage and take %d damage.", 
                       enemy->name, player_damage, enemy_damage);
            
            if (enemy->health <= 0) {
                enemy->active = 0;
                player.exp += enemy->exp_value;
                add_message("%s defeated! Gained %d experience.", 
                           enemy->name, enemy->exp_value);
                
                // Drop loot
                if (rand() % 100 < 30) {  // 30% chance to drop item
                    for (int j = 0; j < MAX_ITEMS; j++) {
                        if (!floor->items[j].active) {
                            floor->items[j] = generate_item(current_floor);
                            floor->items[j].x = enemy->x;
                            floor->items[j].y = enemy->y;
                            floor->items[j].active = 1;
                            add_message("%s dropped %s!", 
                                      enemy->name, floor->items[j].name);
                            break;
                        }
                    }
                }
                
                if (player.exp >= player.exp_next) {
                    level_up();
                }
            }
        }
    }
    
    game_turn++;
} 