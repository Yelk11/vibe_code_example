#include "enemy.h"
#include "map.h"
#include "item.h"
#include "player.h"

// Update all enemies (movement and combat)
void update_enemies() {
    move_enemies();
    check_combat();
}

// Initialize enemies on current floor
void init_enemies() {
    Floor* floor = current_floor_ptr();
    
    // Clear existing enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        floor->enemies[i].active = 0;
    }
    
    // Place enemies in rooms (except first room)
    int num_enemies = min(3 + current_floor, MAX_ENEMIES);
    int enemies_placed = 0;
    
    for (int i = 1; i < floor->num_rooms && enemies_placed < num_enemies; i++) {
        Room* room = &floor->rooms[i];
        
        // 75% chance for each room to have an enemy
        if (rand() % 4 < 3) {
            Enemy enemy;
            
            // Determine enemy type
            int r = rand() % 100;
            if (r < 60) enemy.type = ENEMY_BASIC;     // 60%
            else if (r < 80) enemy.type = ENEMY_FAST; // 20%
            else if (r < 95) enemy.type = ENEMY_RANGED; // 15%
            else enemy.type = ENEMY_BOSS;             // 5%
            
            // Set enemy properties based on type and floor level
            switch(enemy.type) {
                case ENEMY_BASIC:
                    strcpy(enemy.name, "Goblin");
                    enemy.symbol = 'g';
                    enemy.health = 20 + current_floor * 5;
                    enemy.power = 5 + current_floor;
                    enemy.defense = 2 + current_floor / 2;
                    enemy.speed = 1;
                    enemy.range = 1;
                    enemy.exp_value = 10 + current_floor * 2;
                    break;
                    
                case ENEMY_FAST:
                    strcpy(enemy.name, "Wolf");
                    enemy.symbol = 'w';
                    enemy.health = 15 + current_floor * 4;
                    enemy.power = 4 + current_floor;
                    enemy.defense = 1 + current_floor / 3;
                    enemy.speed = 2;
                    enemy.range = 1;
                    enemy.exp_value = 15 + current_floor * 3;
                    break;
                    
                case ENEMY_RANGED:
                    strcpy(enemy.name, "Archer");
                    enemy.symbol = 'a';
                    enemy.health = 12 + current_floor * 3;
                    enemy.power = 6 + current_floor * 2;
                    enemy.defense = 1 + current_floor / 4;
                    enemy.speed = 1;
                    enemy.range = 4;
                    enemy.exp_value = 20 + current_floor * 4;
                    break;
                    
                case ENEMY_BOSS:
                    strcpy(enemy.name, "Ogre");
                    enemy.symbol = 'O';
                    enemy.health = 50 + current_floor * 10;
                    enemy.power = 10 + current_floor * 2;
                    enemy.defense = 5 + current_floor;
                    enemy.speed = 1;
                    enemy.range = 2;
                    enemy.exp_value = 50 + current_floor * 10;
                    break;
            }
            
            enemy.max_health = enemy.health;
            enemy.active = 1;
            
            // Place enemy in room
            enemy.x = room->x + 1 + rand() % (room->width - 2);
            enemy.y = room->y + 1 + rand() % (room->height - 2);
            
            // Add to floor's enemy list
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (!floor->enemies[j].active) {
                    floor->enemies[j] = enemy;
                    enemies_placed++;
                    break;
                }
            }
        }
    }
}

// Move enemies
void move_enemies() {
    Floor* floor = current_floor_ptr();
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &floor->enemies[i];
        if (!enemy->active) continue;
        
        // Calculate distance to player
        int dx = player.x - enemy->x;
        int dy = player.y - enemy->y;
        int dist = (int)sqrt(dx*dx + dy*dy);
        
        // If player is in range, try to attack
        if (dist <= enemy->range) {
            // Attack player
            int damage = max(0, enemy->power - player.defense);
            player.health -= damage;
            
            if (damage > 0) {
                add_message("%s hits you for %d damage!", 
                           enemy->name, damage);
            } else {
                add_message("%s's attack was blocked!", 
                           enemy->name);
            }
            continue;
        }
        
        // If player is visible, move toward them
        if (dist < VIEW_RADIUS && floor->visible[enemy->y][enemy->x]) {
            // Try to move closer to player
            int new_x = enemy->x;
            int new_y = enemy->y;
            
            if (abs(dx) > abs(dy)) {
                new_x += (dx > 0) ? 1 : -1;
            } else {
                new_y += (dy > 0) ? 1 : -1;
            }
            
            // Check if new position is valid
            if (new_x > 0 && new_x < MAP_WIDTH-1 &&
                new_y > 0 && new_y < MAP_HEIGHT-1 &&
                floor->map[new_y][new_x] == '.') {
                
                // Check for collisions with other enemies
                int collision = 0;
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (i != j && floor->enemies[j].active &&
                        floor->enemies[j].x == new_x &&
                        floor->enemies[j].y == new_y) {
                        collision = 1;
                        break;
                    }
                }
                
                if (!collision) {
                    enemy->x = new_x;
                    enemy->y = new_y;
                }
            }
        }
    }
}

// Handle enemy death
void kill_enemy(Enemy* enemy) {
    Floor* floor = current_floor_ptr();
    
    // Mark enemy as inactive
    enemy->active = 0;
    
    // Add experience to player
    player.exp += enemy->exp_value;
    add_message("Defeated %s! Gained %d experience.", 
               enemy->name, enemy->exp_value);
    
    // Check for level up
    if (player.exp >= player.exp_next) {
        level_up();
    }
    
    // Chance to drop item
    if (rand() % 100 < 30) {  // 30% chance
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (!floor->items[j].active) {
                floor->items[j] = create_random_item(current_floor);
                floor->items[j].x = enemy->x;
                floor->items[j].y = enemy->y;
                floor->items[j].active = 1;
                add_message("%s dropped %s!", 
                           enemy->name, floor->items[j].name);
                break;
            }
        }
    }
    
    // Chance to drop gold
    if (rand() % 100 < 50) {  // 50% chance
        int gold = 5 + rand() % (10 + current_floor * 5);
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (!floor->items[j].active) {
                floor->items[j].type = ITEM_GOLD;
                floor->items[j].value = gold;
                floor->items[j].x = enemy->x;
                floor->items[j].y = enemy->y;
                floor->items[j].active = 1;
                floor->items[j].symbol = '$';
                snprintf(floor->items[j].name, MAX_NAME_LEN,
                        "%d Gold", gold);
                add_message("%s dropped %d gold!", 
                           enemy->name, gold);
                break;
            }
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
                            apply_status_effect(STATUS_BURN, 3, enemy_damage/3);
                            break;
                        case 2:
                            apply_status_effect(STATUS_FREEZE, 3, 2);
                            break;
                    }
                }
            }
            
            player.health -= enemy_damage;
            enemy->health -= player_damage;
            
            add_message("Combat with %s! You deal %d damage and take %d damage.", 
                       enemy->name, player_damage, enemy_damage);
            
            if (enemy->health <= 0) {
                kill_enemy(enemy);
            }
        }
    }
    
    game_turn++;
} 