#include "enemy.h"
#include "player.h"
#include "map.h"

// Helper function declarations
static int is_enemy_at(int x, int y);

// Initialize enemies
void init_enemies() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        floor->enemies[i].active = 0;
    }
}

// Update a single enemy
void update_enemy(Enemy* enemy) {
    if (!enemy->active) return;
    
    // Calculate distance to player
    int dx = player.x - enemy->x;
    int dy = player.y - enemy->y;
    int dist = abs(dx) + abs(dy);  // Manhattan distance
    
    // If player is adjacent, attack
    if (dist == 1) {
        enemy_attack(enemy, player.x, player.y);
        return;
    }
    
    // If player is in range and enemy is ranged, attack
    if (enemy->type == ENEMY_RANGED && dist <= enemy->range) {
        enemy_attack(enemy, player.x, player.y);
        return;
    }
    
    // Move towards player
    int move_dx = 0;
    int move_dy = 0;
    
    if (dx > 0) move_dx = 1;
    else if (dx < 0) move_dx = -1;
    
    if (dy > 0) move_dy = 1;
    else if (dy < 0) move_dy = -1;
    
    // Try to move
    move_enemy(enemy, move_dx, move_dy);
    
    // Fast enemies get a second move
    if (enemy->type == ENEMY_FAST) {
        move_enemy(enemy, move_dx, move_dy);
    }
}

// Move enemy by dx, dy
void move_enemy(Enemy* enemy, int dx, int dy) {
    int new_x = enemy->x + dx;
    int new_y = enemy->y + dy;
    Floor* floor = current_floor_ptr();
    
    // Check if move is valid
    if (new_x >= 0 && new_x < MAP_WIDTH &&
        new_y >= 0 && new_y < MAP_HEIGHT &&
        floor->map[new_y][new_x] == '.' &&
        !is_enemy_at(new_x, new_y)) {
        enemy->x = new_x;
        enemy->y = new_y;
    }
}

// Enemy attacks target location
void enemy_attack(Enemy* enemy, int target_x, int target_y) {
    // Only attack if target is player
    if (target_x == player.x && target_y == player.y) {
        // Calculate damage with defense reduction
        int damage = max(0, enemy->power - player.defense);
        
        // Apply damage to player
        player.health -= damage;
        
        if (damage > 0) {
            add_message("%s hits you for %d damage!", enemy->name, damage);
            
            // Check if player died
            if (player.health <= 0) {
                add_message("You have died!");
            }
        } else {
            add_message("%s attacks but does no damage!", enemy->name);
        }
    }
}

// Spawn a new enemy
void spawn_enemy(int x, int y, EnemyType type) {
    Floor* floor = current_floor_ptr();
    
    // Find empty enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!floor->enemies[i].active) {
            Enemy* enemy = &floor->enemies[i];
            enemy->active = 1;
            enemy->x = x;
            enemy->y = y;
            enemy->type = type;
            
            // Set enemy stats based on type
            switch (type) {
                case ENEMY_BASIC:
                    strcpy(enemy->name, "Goblin");
                    enemy->symbol = 'g';
                    enemy->health = enemy->max_health = 10;
                    enemy->power = 10;
                    enemy->defense = 1;
                    enemy->speed = 1;
                    enemy->range = 1;
                    enemy->exp_value = 10;
                    break;
                    
                case ENEMY_FAST:
                    strcpy(enemy->name, "Wolf");
                    enemy->symbol = 'w';
                    enemy->health = enemy->max_health = 8;
                    enemy->power = 20;
                    enemy->defense = 0;
                    enemy->speed = 2;
                    enemy->range = 1;
                    enemy->exp_value = 15;
                    break;
                    
                case ENEMY_RANGED:
                    strcpy(enemy->name, "Archer");
                    enemy->symbol = 'a';
                    enemy->health = enemy->max_health = 6;
                    enemy->power = 25;
                    enemy->defense = 5;
                    enemy->speed = 1;
                    enemy->range = 5;
                    enemy->exp_value = 20;
                    break;
                    
                case ENEMY_BOSS:
                    strcpy(enemy->name, "Dragon");
                    enemy->symbol = 'D';
                    enemy->health = enemy->max_health = 75;
                    enemy->power = 25;
                    enemy->defense = 50;
                    enemy->speed = 1;
                    enemy->range = 3;
                    enemy->exp_value = 100;
                    break;
            }
            break;
        }
    }
}

// Kill an enemy
void kill_enemy(Enemy* enemy) {
    if (!enemy->active) return;
    
    add_message("The %s dies!", enemy->name);
    player.exp += enemy->exp_value;
    enemy->active = 0;
    
    // Check for level up
    if (player.exp >= player.exp_next) {
        level_up();
    }
}

// Helper function to check if an enemy is at a location
static int is_enemy_at(int x, int y) {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &floor->enemies[i];
        if (enemy->active && enemy->x == x && enemy->y == y) {
            return 1;
        }
    }
    return 0;
}

// Spawn enemies for the current floor
void spawn_floor_enemies() {
    Floor* floor = current_floor_ptr();
    
    // Spawn enemies in random rooms
    for (int i = 0; i < floor->num_rooms; i++) {
        Room* room = &floor->rooms[i];
        // Skip the first room on floor 0 (player's starting room)
        if (current_floor == 0 && i == 0) continue;
        
        // 70% chance to spawn enemies in each room
        if (rand() % 100 < 70) {
            // Spawn 1-3 enemies per room
            int num_enemies = random_range(1, 3);
            for (int j = 0; j < num_enemies; j++) {
                // Find a random position in the room
                int x = room->x + 1 + rand() % (room->width - 2);
                int y = room->y + 1 + rand() % (room->height - 2);
                
                // Randomly choose enemy type based on floor level
                EnemyType type;
                int r = rand() % 100;
                if (current_floor < 3) {
                    type = ENEMY_BASIC;
                } else if (current_floor < 6) {
                    if (r < 70) type = ENEMY_BASIC;
                    else type = ENEMY_FAST;
                } else if (current_floor < 9) {
                    if (r < 50) type = ENEMY_BASIC;
                    else if (r < 80) type = ENEMY_FAST;
                    else type = ENEMY_RANGED;
                } else {
                    if (r < 40) type = ENEMY_BASIC;
                    else if (r < 70) type = ENEMY_FAST;
                    else if (r < 90) type = ENEMY_RANGED;
                    else type = ENEMY_BOSS;
                }
                
                spawn_enemy(x, y, type);
            }
        }
    }
} 