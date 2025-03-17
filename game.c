#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdarg.h>

// Utility macros
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

// Game constants
#define MAP_WIDTH 80
#define MAP_HEIGHT 40
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 20
#define MAX_ROOMS 12
#define MIN_ROOM_SIZE 5
#define MAX_ROOM_SIZE 10
#define MAX_ENEMIES 5
#define MAX_ITEMS 10
#define MAX_FLOORS 5
#define INVENTORY_SIZE 20
#define MAX_ENEMY_TYPES 4
#define VIEW_RADIUS 8
#define MAX_NAME_LEN 32
#define MAX_DESC_LEN 128
#define MAX_MESSAGES 10
#define MESSAGE_LENGTH 80

// Item types
typedef enum {
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_POTION,
    ITEM_SCROLL,
    ITEM_FOOD,
    ITEM_KEY,
    ITEM_GOLD
} ItemType;

// Equipment slots
typedef enum {
    SLOT_WEAPON,
    SLOT_ARMOR,
    SLOT_RING,
    SLOT_AMULET,
    MAX_EQUIPMENT_SLOTS
} EquipmentSlot;

// Enemy types
typedef enum {
    ENEMY_BASIC,    // Normal enemy
    ENEMY_FAST,     // Moves twice per turn
    ENEMY_RANGED,   // Can attack from distance
    ENEMY_BOSS      // Stronger with special abilities
} EnemyType;

// Terrain types
typedef enum {
    TERRAIN_FLOOR = '.',
    TERRAIN_WALL = '#',
    TERRAIN_WATER = '~',
    TERRAIN_LAVA = '^',
    TERRAIN_DOOR = '+',
    TERRAIN_GRASS = '"',
    TERRAIN_TRAP = '_'
} TerrainType;

// Status effect types
typedef enum {
    STATUS_NONE,
    STATUS_POISON,     // Damage over time
    STATUS_BURNING,    // More damage over time
    STATUS_FROZEN,     // Reduced speed
    STATUS_BLESSED,    // Increased power
    STATUS_CURSED,     // Decreased defense
    MAX_STATUS_EFFECTS
} StatusType;

// Special ability types
typedef enum {
    ABILITY_NONE,
    ABILITY_HEAL,          // Heal self
    ABILITY_FIREBALL,      // Ranged attack
    ABILITY_BLINK,         // Short teleport
    ABILITY_SHIELD,        // Temporary defense boost
    ABILITY_RAGE,          // Temporary power boost
    MAX_ABILITIES
} AbilityType;

// Item structure
typedef struct {
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    int x;
    int y;
    char symbol;
    int active;
    ItemType type;
    int value;      // Gold value or effect value
    int power;      // Damage for weapons, defense for armor
    int durability; // Number of uses remaining
} Item;

// Enemy structure
typedef struct {
    char name[MAX_NAME_LEN];
    int x;
    int y;
    char symbol;
    int health;
    int max_health;
    int active;
    EnemyType type;
    int power;      // Base damage
    int defense;    // Damage reduction
    int speed;      // Movement per turn
    int range;      // Attack range
    int exp_value;  // Experience points when defeated
} Enemy;

// Status effect structure
typedef struct {
    StatusType type;
    int duration;
    int power;
} StatusEffect;

// Special ability structure
typedef struct {
    AbilityType type;
    int cooldown;
    int current_cooldown;
    int power;
    char key;           // Hotkey to use ability
    char name[32];
    char description[128];
} Ability;

// Extended player structure with new fields
typedef struct {
    char name[MAX_NAME_LEN];
    int x;
    int y;
    int health;
    int max_health;
    int level;
    int exp;
    int exp_next;
    int power;
    int defense;
    int gold;
    Item inventory[INVENTORY_SIZE];
    Item* equipment[MAX_EQUIPMENT_SLOTS];
    int num_items;
    
    // New fields
    int mana;
    int max_mana;
    int mana_regen;
    StatusEffect status[MAX_STATUS_EFFECTS];
    Ability abilities[MAX_ABILITIES];
    int num_abilities;
    int critical_chance;    // Percentage chance for critical hits
    int dodge_chance;       // Percentage chance to dodge attacks
    int fire_resist;        // Percentage resistance to fire damage
    int ice_resist;         // Percentage resistance to ice damage
    int poison_resist;      // Percentage resistance to poison damage
} Player;

// Room structure
typedef struct {
    int x;
    int y;
    int width;
    int height;
} Room;

// Floor structure
typedef struct {
    char map[MAP_HEIGHT][MAP_WIDTH];
    char visible[MAP_HEIGHT][MAP_WIDTH];  // Fog of war
    char discovered[MAP_HEIGHT][MAP_WIDTH]; // Previously seen
    Room rooms[MAX_ROOMS];
    int num_rooms;
    int up_stairs_x;
    int up_stairs_y;
    int down_stairs_x;
    int down_stairs_y;
    Enemy enemies[MAX_ENEMIES];
    Item items[MAX_ITEMS];
    TerrainType terrain[MAP_HEIGHT][MAP_WIDTH];
} Floor;

// Message log structure
typedef struct {
    char messages[MAX_MESSAGES][MESSAGE_LENGTH];
    int num_messages;
} MessageLog;

// Game state
Floor floors[MAX_FLOORS];
int current_floor = 0;
Player player;
int camera_x = 0;
int camera_y = 0;
int game_turn = 0;
MessageLog message_log = {0};

// Color codes for terminal
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"

// Function prototypes
void init_enemies(void);
void init_items(void);
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2);
Floor* current_floor_ptr(void);
void init_floor(Floor* floor);
void init_game(void);
void update_camera(void);
void draw(void);
void move_enemies(void);
void check_combat(void);
void check_items(void);
void move_player(char input);
void init_player(void);
void level_up(void);
void update_fov(void);
void handle_combat(Enemy* enemy);
void apply_item_effect(Item* item);
void save_game(const char* filename);
void load_game(const char* filename);
int is_visible(int x, int y);
void update_discovered_map(void);
void render_ui(void);
void handle_inventory(void);
void generate_terrain(Floor* floor);
void spawn_enemies(Floor* floor);
void place_items(Floor* floor);
int add_to_inventory(Item item);
void remove_from_inventory(int index);
void use_item(int index);
void drop_item(int index);
void equip_item(int index);
Item generate_item(int floor_level);
char getch(void);
int random_range(int min, int max);
int rooms_overlap(Room* r1, Room* r2, int padding);
Room generate_room(void);
void place_stairs_in_room(Room* room, int* stair_x, int* stair_y);
void add_message(const char* fmt, ...);
void add_ability(AbilityType type);
void apply_status_effect(StatusType type, int duration, int power);
void update_status_effects();
void use_ability(int index);
void update_abilities();

// Function to get character input without waiting for Enter
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

// Random number between min and max (inclusive)
int random_range(int min, int max) {
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
}

// Check if two rooms overlap
int rooms_overlap(Room* r1, Room* r2, int padding) {
    return !(r1->x + r1->width + padding < r2->x ||
             r2->x + r2->width + padding < r1->x ||
             r1->y + r1->height + padding < r2->y ||
             r2->y + r2->height + padding < r1->y);
}

// Generate a random room
Room generate_room() {
    Room room;
    room.width = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    room.height = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    room.x = random_range(1, MAP_WIDTH - room.width - 1);
    room.y = random_range(1, MAP_HEIGHT - room.height - 1);
    return room;
}

// Create a tunnel between two points
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    int current_x = x1;
    int current_y = y1;
    
    // Randomly choose whether to go horizontal or vertical first
    if (rand() % 2 == 0) {
        // Horizontal then vertical
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
            current_x += (x2 > x1) ? 1 : -1;
        }
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            current_y += (y2 > y1) ? 1 : -1;
        }
    } else {
        // Vertical then horizontal
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            current_y += (y2 > y1) ? 1 : -1;
        }
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
            current_x += (x2 > x1) ? 1 : -1;
        }
    }
}

// Get current floor
Floor* current_floor_ptr() {
    return &floors[current_floor];
}

// Place stairs in a random room
void place_stairs_in_room(Room* room, int* stair_x, int* stair_y) {
    *stair_x = random_range(room->x + 1, room->x + room->width - 2);
    *stair_y = random_range(room->y + 1, room->y + room->height - 2);
}

// Initialize a single floor
void init_floor(Floor* floor) {
    // Fill map with walls
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            floor->map[y][x] = '#';
        }
    }
    
    // Generate rooms
    floor->num_rooms = 0;
    int max_attempts = 100;
    int attempts = 0;
    
    while (floor->num_rooms < MAX_ROOMS && attempts < max_attempts) {
        Room new_room = generate_room();
        int valid = 1;
        
        // Check if room overlaps with existing rooms
        for (int i = 0; i < floor->num_rooms; i++) {
            if (rooms_overlap(&new_room, &floor->rooms[i], 2)) {
                valid = 0;
                break;
            }
        }
        
        if (valid) {
            // Add room to map
            for (int y = new_room.y; y < new_room.y + new_room.height; y++) {
                for (int x = new_room.x; x < new_room.x + new_room.width; x++) {
                    floor->map[y][x] = '.';
                }
            }
            
            // Connect to previous room
            if (floor->num_rooms > 0) {
                int prev_center_x = floor->rooms[floor->num_rooms-1].x + floor->rooms[floor->num_rooms-1].width/2;
                int prev_center_y = floor->rooms[floor->num_rooms-1].y + floor->rooms[floor->num_rooms-1].height/2;
                int new_center_x = new_room.x + new_room.width/2;
                int new_center_y = new_room.y + new_room.height/2;
                
                create_tunnel(floor, prev_center_x, prev_center_y, new_center_x, new_center_y);
            }
            
            floor->rooms[floor->num_rooms++] = new_room;
        }
        attempts++;
    }
    
    // Ensure we have at least 2 rooms for stairs
    if (floor->num_rooms < 2) {
        printf("Error: Not enough rooms generated for floor %d\n", current_floor);
        return;
    }
    
    // Place stairs (except for first and last floor)
    if (current_floor > 0) {
        // Place up stairs in a random room (not the last room)
        int up_room_idx = random_range(0, floor->num_rooms - 1);
        Room* up_room = &floor->rooms[up_room_idx];
        place_stairs_in_room(up_room, &floor->up_stairs_x, &floor->up_stairs_y);
        floor->map[floor->up_stairs_y][floor->up_stairs_x] = '<';
    }
    
    if (current_floor < MAX_FLOORS - 1) {
        // Place down stairs in a different room (preferably the last room)
        int down_room_idx = floor->num_rooms - 1;  // Use the last room
        if (current_floor > 0 && down_room_idx == 0) {
            down_room_idx = 1;  // Use second room if only 2 rooms and we need both stairs
        }
        
        Room* down_room = &floor->rooms[down_room_idx];
        place_stairs_in_room(down_room, &floor->down_stairs_x, &floor->down_stairs_y);
        floor->map[floor->down_stairs_y][floor->down_stairs_x] = '>';
    }
    
    // Debug: Print stair positions
    printf("Floor %d: ", current_floor);
    if (current_floor > 0) {
        printf("Up stairs at (%d,%d) ", floor->up_stairs_x, floor->up_stairs_y);
    }
    if (current_floor < MAX_FLOORS - 1) {
        printf("Down stairs at (%d,%d)", floor->down_stairs_x, floor->down_stairs_y);
    }
    printf("\n");
}

// Initialize all game floors
void init_game() {
    // Initialize player first
    init_player();
    
    // Initialize all floors
    for (int i = 0; i < MAX_FLOORS; i++) {
        current_floor = i;
        init_floor(&floors[i]);
    }
    
    // Set player starting position on first floor
    current_floor = 0;
    Floor* first_floor = current_floor_ptr();
    Room* first_room = &first_floor->rooms[0];
    player.x = first_room->x + 1;
    player.y = first_room->y + 1;
    
    // Initialize enemies and items for each floor
    for (int i = 0; i < MAX_FLOORS; i++) {
        current_floor = i;
        init_enemies();
        init_items();
    }
    
    current_floor = 0;  // Reset to first floor
}

// Update camera position to follow player
void update_camera() {
    camera_x = player.x - SCREEN_WIDTH/2;
    camera_y = player.y - SCREEN_HEIGHT/2;
    
    // Keep camera within map bounds
    if (camera_x < 0) camera_x = 0;
    if (camera_y < 0) camera_y = 0;
    if (camera_x > MAP_WIDTH - SCREEN_WIDTH) camera_x = MAP_WIDTH - SCREEN_WIDTH;
    if (camera_y > MAP_HEIGHT - SCREEN_HEIGHT) camera_y = MAP_HEIGHT - SCREEN_HEIGHT;
}

// Draw the game state with camera view
void draw() {
    system("clear");
    Floor* floor = current_floor_ptr();
    
    update_camera();
    update_fov();
    
    // Draw map and message log side by side
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        // Draw map portion
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int map_x = x + camera_x;
            int map_y = y + camera_y;
            
            if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
                printf(" ");
                continue;
            }
            
            char cell = floor->map[map_y][map_x];
            
            // If not visible, show discovered areas in dark
            if (!floor->visible[map_y][map_x]) {
                if (floor->discovered[map_y][map_x]) {
                    printf("%s%c%s", COLOR_BLUE, cell, COLOR_RESET);
                } else {
                    printf(" ");
                }
                continue;
            }
            
            // Check for items
            int found_item = 0;
            for (int i = 0; i < MAX_ITEMS; i++) {
                if (floor->items[i].active && 
                    floor->items[i].x == map_x && 
                    floor->items[i].y == map_y) {
                    printf("%s%c%s", COLOR_YELLOW, floor->items[i].symbol, COLOR_RESET);
                    found_item = 1;
                    break;
                }
            }
            if (found_item) continue;
            
            // Check for enemies
            int found_enemy = 0;
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (floor->enemies[i].active && 
                    floor->enemies[i].x == map_x && 
                    floor->enemies[i].y == map_y) {
                    printf("%s%c%s", COLOR_RED, floor->enemies[i].symbol, COLOR_RESET);
                    found_enemy = 1;
                    break;
                }
            }
            if (found_enemy) continue;
            
            // Player position
            if (map_x == player.x && map_y == player.y) {
                printf("%s@%s", COLOR_GREEN, COLOR_RESET);
                continue;
            }
            
            // Terrain coloring
            switch (cell) {
                case '#': printf("%s%c%s", COLOR_WHITE, cell, COLOR_RESET); break;
                case '~': printf("%s%c%s", COLOR_BLUE, cell, COLOR_RESET); break;
                case '^': printf("%s%c%s", COLOR_RED, cell, COLOR_RESET); break;
                case '+': printf("%s%c%s", COLOR_YELLOW, cell, COLOR_RESET); break;
                case '"': printf("%s%c%s", COLOR_GREEN, cell, COLOR_RESET); break;
                default: printf("%c", cell);
            }
        }
        
        // Draw message log and stats to the right of the map
        printf("  "); // Add some spacing
        if (y == 0) {
            printf("=== Messages ===");
        } else if (y < MAX_MESSAGES + 1 && y - 1 < message_log.num_messages) {
            printf("%s", message_log.messages[y - 1]);
        } else if (y == MAX_MESSAGES + 2) {
            printf("=== Stats ===");
        } else if (y == MAX_MESSAGES + 3) {
            printf("Health: %d/%d", player.health, player.max_health);
        } else if (y == MAX_MESSAGES + 4) {
            printf("Mana: %d/%d (+%d)", player.mana, player.max_mana, player.mana_regen);
        } else if (y == MAX_MESSAGES + 5) {
            printf("Level: %d  Exp: %d/%d", player.level, player.exp, player.exp_next);
        } else if (y == MAX_MESSAGES + 6) {
            printf("Power: %d  Defense: %d", player.power, player.defense);
        } else if (y == MAX_MESSAGES + 7) {
            printf("Crit: %d%%  Dodge: %d%%", player.critical_chance, player.dodge_chance);
        } else if (y == MAX_MESSAGES + 8) {
            printf("Floor: %d/%d", current_floor + 1, MAX_FLOORS);
        } else if (y == MAX_MESSAGES + 9) {
            printf("Gold: %d", player.gold);
        } else if (y == MAX_MESSAGES + 10) {
            printf("=== Status Effects ===");
        } else if (y == MAX_MESSAGES + 11) {
            // Show active status effects
            int found = 0;
            for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
                if (player.status[i].type != STATUS_NONE) {
                    if (found) printf(", ");
                    printf("%s(%d)",
                           player.status[i].type == STATUS_POISON ? "Poison" :
                           player.status[i].type == STATUS_BURNING ? "Burning" :
                           player.status[i].type == STATUS_FROZEN ? "Frozen" :
                           player.status[i].type == STATUS_BLESSED ? "Blessed" :
                           player.status[i].type == STATUS_CURSED ? "Cursed" : "Unknown",
                           player.status[i].duration);
                    found = 1;
                }
            }
            if (!found) printf("None");
        } else if (y == MAX_MESSAGES + 12) {
            printf("=== Abilities ===");
        } else if (y >= MAX_MESSAGES + 13 && y < MAX_MESSAGES + 13 + player.num_abilities) {
            int ability_idx = y - (MAX_MESSAGES + 13);
            Ability* ability = &player.abilities[ability_idx];
            printf("%c) %s", ability->key, ability->name);
            if (ability->current_cooldown > 0) {
                printf(" (CD: %d)", ability->current_cooldown);
            }
        } else if (y == SCREEN_HEIGHT - 1) {
            printf("Controls: WASD=move  I=inventory  1-5=abilities  Q=quit");
        }
        
        printf("\n");
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

// Check for items
void check_items() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) continue;
        
        if (player.x == floor->items[i].x && player.y == floor->items[i].y) {
            Item* item = &floor->items[i];
            
            if (item->type == ITEM_GOLD) {
                player.gold += item->value;
                add_message("Picked up %d gold!", item->value);
                floor->items[i].active = 0;
            } else {
                if (add_to_inventory(floor->items[i])) {
                    add_message("Picked up %s", item->name);
                    floor->items[i].active = 0;
                } else {
                    add_message("Inventory full!");
                }
            }
        }
    }
}

// Handle player movement
void move_player(char input) {
    // Check for ability hotkeys
    for (int i = 0; i < player.num_abilities; i++) {
        if (input == player.abilities[i].key) {
            use_ability(i);
            return;
        }
    }
    
    Floor* floor = current_floor_ptr();
    int new_x = player.x;
    int new_y = player.y;
    
    switch(input) {
        case 'w': new_y--; break;
        case 's': new_y++; break;
        case 'a': new_x--; break;
        case 'd': new_x++; break;
        case 'i': handle_inventory(); return;
        default: return;
    }
    
    // Check if the new position is within bounds and not a wall
    if (new_x > 0 && new_x < MAP_WIDTH-1 && 
        new_y > 0 && new_y < MAP_HEIGHT-1 && 
        floor->map[new_y][new_x] != '#') {
        player.x = new_x;
        player.y = new_y;
        
        // Check if player is on stairs
        char current_tile = floor->map[player.y][player.x];
        if (current_tile == '<' && current_floor > 0) {
            // Moving up
            current_floor--;
            floor = current_floor_ptr();
            player.x = floor->down_stairs_x;
            player.y = floor->down_stairs_y;
        } else if (current_tile == '>' && current_floor < MAX_FLOORS - 1) {
            // Moving down
            current_floor++;
            floor = current_floor_ptr();
            player.x = floor->up_stairs_x;
            player.y = floor->up_stairs_y;
        }
    }
}

// Initialize enemies
void init_enemies() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        // Pick a random room (not the first one where player starts)
        int room_idx = random_range(1, floor->num_rooms - 1);  // Fix range
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

// Initialize items
void init_items() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ITEMS; i++) {
        // Pick a random room
        int room_idx = random_range(0, floor->num_rooms);
        Room* room = &floor->rooms[room_idx];
        
        // Generate and place item randomly within the room
        floor->items[i] = generate_item(current_floor);
        floor->items[i].x = random_range(room->x + 1, room->x + room->width - 2);
        floor->items[i].y = random_range(room->y + 1, room->y + room->height - 2);
        floor->items[i].active = 1;
        
        // Add description based on item type and floor level
        char desc[MAX_DESC_LEN];
        switch(floor->items[i].type) {
            case ITEM_WEAPON:
                sprintf(desc, "A level %d weapon that deals %d damage", 
                        current_floor + 1, floor->items[i].power);
                break;
            case ITEM_ARMOR:
                sprintf(desc, "A level %d armor that provides %d defense", 
                        current_floor + 1, floor->items[i].power);
                break;
            case ITEM_POTION:
                sprintf(desc, "Restores %d health points", 
                        floor->items[i].value);
                break;
            case ITEM_SCROLL:
                sprintf(desc, "A mysterious scroll with unknown effects");
                break;
            case ITEM_FOOD:
                sprintf(desc, "Restores %d health points when eaten", 
                        floor->items[i].value);
                break;
            case ITEM_KEY:
                sprintf(desc, "Might open something important");
                break;
            case ITEM_GOLD:
                sprintf(desc, "Worth %d gold pieces", 
                        floor->items[i].value);
                break;
        }
        strcpy(floor->items[i].description, desc);
    }
}

// Initialize player
void init_player() {
    strcpy(player.name, "Hero");
    player.health = 100;
    player.max_health = 100;
    player.level = 1;
    player.exp = 0;
    player.exp_next = 100;
    player.power = 10;
    player.defense = 5;
    player.gold = 0;
    player.num_items = 0;
    
    // Clear inventory and equipment
    memset(player.inventory, 0, sizeof(player.inventory));
    for (int i = 0; i < MAX_EQUIPMENT_SLOTS; i++) {
        player.equipment[i] = NULL;
    }
    
    // Initialize new fields
    player.mana = 100;
    player.max_mana = 100;
    player.mana_regen = 5;
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        player.status[i].type = STATUS_NONE;
        player.status[i].duration = 0;
        player.status[i].power = 0;
    }
    for (int i = 0; i < MAX_ABILITIES; i++) {
        player.abilities[i].type = ABILITY_NONE;
        player.abilities[i].cooldown = 0;
        player.abilities[i].current_cooldown = 0;
        player.abilities[i].power = 0;
        player.abilities[i].key = '\0';
        strcpy(player.abilities[i].name, "None");
        strcpy(player.abilities[i].description, "No ability");
    }
    player.num_abilities = 0;
    player.critical_chance = 5;
    player.dodge_chance = 5;
    player.fire_resist = 0;
    player.ice_resist = 0;
    player.poison_resist = 0;
}

// Calculate if a point is visible from the player's position using ray casting
int is_visible(int x, int y) {
    Floor* floor = current_floor_ptr();
    
    // If point is too far, it's not visible
    int dx = x - player.x;
    int dy = y - player.y;
    if (dx * dx + dy * dy > VIEW_RADIUS * VIEW_RADIUS) {
        return 0;
    }
    
    // Use Bresenham's line algorithm to check for walls
    int abs_dx = abs(dx);
    int abs_dy = abs(dy);
    int sx = dx > 0 ? 1 : -1;
    int sy = dy > 0 ? 1 : -1;
    
    int err = abs_dx - abs_dy;
    int current_x = player.x;
    int current_y = player.y;
    
    while (current_x != x || current_y != y) {
        if (floor->map[current_y][current_x] == '#') {
            return 0;  // Hit a wall
        }
        
        int e2 = 2 * err;
        if (e2 > -abs_dy) {
            err -= abs_dy;
            current_x += sx;
        }
        if (e2 < abs_dx) {
            err += abs_dx;
            current_y += sy;
        }
    }
    
    return 1;  // No walls in the way
}

// Update field of view
void update_fov() {
    Floor* floor = current_floor_ptr();
    
    // Reset visibility
    memset(floor->visible, 0, sizeof(floor->visible));
    
    // Check visibility for each point in view radius
    for (int y = max(0, player.y - VIEW_RADIUS); 
         y < min(MAP_HEIGHT, player.y + VIEW_RADIUS + 1); y++) {
        for (int x = max(0, player.x - VIEW_RADIUS);
             x < min(MAP_WIDTH, player.x + VIEW_RADIUS + 1); x++) {
            if (is_visible(x, y)) {
                floor->visible[y][x] = 1;
                floor->discovered[y][x] = 1;
            }
        }
    }
}

// Level up the player
void level_up() {
    player.level++;
    player.max_health += 10;
    player.health = player.max_health;
    player.power += 2;
    player.defense += 1;
    player.exp_next = player.level * 100;
    
    // Increase secondary stats
    player.max_mana += 10;
    player.mana = player.max_mana;
    player.mana_regen += 1;
    player.critical_chance += 1;
    player.dodge_chance += 1;
    
    add_message("Level Up! You are now level %d", player.level);
    add_message("Health +10, Power +2, Defense +1");
    add_message("Mana +10, Mana Regen +1");
    add_message("Critical Chance +1%%, Dodge Chance +1%%");
    
    // Learn new ability at certain levels
    if (player.level == 2) {
        add_ability(ABILITY_HEAL);
    } else if (player.level == 3) {
        add_ability(ABILITY_FIREBALL);
    } else if (player.level == 4) {
        add_ability(ABILITY_BLINK);
    } else if (player.level == 5) {
        add_ability(ABILITY_SHIELD);
    } else if (player.level == 6) {
        add_ability(ABILITY_RAGE);
    }
}

// Add item to inventory
int add_to_inventory(Item item) {
    if (player.num_items >= INVENTORY_SIZE) {
        return 0;  // Inventory full
    }
    
    player.inventory[player.num_items++] = item;
    return 1;
}

// Remove item from inventory
void remove_from_inventory(int index) {
    if (index < 0 || index >= player.num_items) return;
    
    for (int i = index; i < player.num_items - 1; i++) {
        player.inventory[i] = player.inventory[i + 1];
    }
    player.num_items--;
}

// Handle inventory menu
void handle_inventory() {
    char num_str[16];  // Increased buffer size
    while (1) {
        system("clear");
        printf("\n=== Inventory (%d/%d) ===\n", player.num_items, INVENTORY_SIZE);
        
        // Show equipped items
        printf("\nEquipped:\n");
        printf("Weapon: %s\n", player.equipment[SLOT_WEAPON] ? player.equipment[SLOT_WEAPON]->name : "None");
        printf("Armor: %s\n", player.equipment[SLOT_ARMOR] ? player.equipment[SLOT_ARMOR]->name : "None");
        printf("Ring: %s\n", player.equipment[SLOT_RING] ? player.equipment[SLOT_RING]->name : "None");
        printf("Amulet: %s\n", player.equipment[SLOT_AMULET] ? player.equipment[SLOT_AMULET]->name : "None");
        
        // Show inventory items
        printf("\nItems:\n");
        for (int i = 0; i < player.num_items; i++) {
            printf("%d) %s", i + 1, player.inventory[i].name);
            if (player.inventory[i].type == ITEM_WEAPON || player.inventory[i].type == ITEM_ARMOR) {
                printf(" (Power: %d, Durability: %d)", 
                       player.inventory[i].power, 
                       player.inventory[i].durability);
            }
            printf("\n");
        }
        
        printf("\nCommands: (u)se item, (d)rop item, (e)quip item, (q)uit inventory\n");
        printf("Enter command: ");
        fflush(stdout);
        
        char cmd = getch();
        printf("%c\n", cmd);  // Echo the command
        
        if (cmd == 'q') break;
        
        if (cmd == 'u' || cmd == 'd' || cmd == 'e') {
            printf("Enter item number (1-%d): ", player.num_items);
            fflush(stdout);
            
            // Clear input buffer
            while (getchar() != '\n');
            
            // Read the item number
            if (!fgets(num_str, sizeof(num_str), stdin)) {
                continue;
            }
            
            // Convert input to number
            int index = atoi(num_str) - 1;  // Convert to 0-based index
            
            // Validate input
            if (index < 0 || index >= player.num_items) {
                printf("Invalid item number! Press any key to continue...\n");
                getch();
                continue;
            }
            
            // Process command
            switch (cmd) {
                case 'u':
                    use_item(index);
                    break;
                case 'd':
                    drop_item(index);
                    break;
                case 'e':
                    equip_item(index);
                    break;
            }
            
            // Show result briefly
            printf("Press any key to continue...\n");
            getch();
        }
    }
}

// Use an item from inventory
void use_item(int index) {
    Item* item = &player.inventory[index];
    int used = 0;
    
    switch (item->type) {
        case ITEM_POTION:
            player.health += item->value;
            if (player.health > player.max_health) {
                player.health = player.max_health;
            }
            add_message("Drank potion. Health restored to %d", player.health);
            used = 1;
            break;
            
        case ITEM_FOOD:
            player.health += item->value / 2;
            if (player.health > player.max_health) {
                player.health = player.max_health;
            }
            add_message("Ate food. Health restored to %d", player.health);
            used = 1;
            break;
            
        case ITEM_SCROLL:
            // Add scroll effects here
            add_message("Read scroll. Nothing happens.");
            used = 1;
            break;
    }
    
    if (used) {
        remove_from_inventory(index);
    }
}

// Drop an item from inventory
void drop_item(int index) {
    Floor* floor = current_floor_ptr();
    Item* item = &player.inventory[index];
    
    // Find empty adjacent spot
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    
    for (int i = 0; i < 4; i++) {
        int new_x = player.x + dx[i];
        int new_y = player.y + dy[i];
        
        if (floor->map[new_y][new_x] == '.') {
            // Place item on map
            for (int j = 0; j < MAX_ITEMS; j++) {
                if (!floor->items[j].active) {
                    floor->items[j] = *item;
                    floor->items[j].x = new_x;
                    floor->items[j].y = new_y;
                    floor->items[j].active = 1;
                    remove_from_inventory(index);
                    add_message("Dropped %s", item->name);
                    return;
                }
            }
        }
    }
    
    add_message("No space to drop item!");
}

// Equip an item
void equip_item(int index) {
    if (index < 0 || index >= player.num_items) {
        add_message("Invalid item index!");
        return;
    }

    Item* item = &player.inventory[index];
    EquipmentSlot slot;
    
    // Determine equipment slot
    switch (item->type) {
        case ITEM_WEAPON:
            slot = SLOT_WEAPON;
            break;
        case ITEM_ARMOR:
            slot = SLOT_ARMOR;
            break;
        default:
            add_message("Cannot equip this type of item!");
            return;
    }
    
    // Create new equipment item
    Item* new_equipment = (Item*)malloc(sizeof(Item));
    if (new_equipment == NULL) {
        add_message("Failed to allocate memory for equipment!");
        return;
    }
    memcpy(new_equipment, item, sizeof(Item));
    
    // Handle current equipped item if any
    if (player.equipment[slot] != NULL) {
        // Try to add current equipment to inventory
        Item old_equipment = *player.equipment[slot];
        if (!add_to_inventory(old_equipment)) {
            add_message("Inventory full! Cannot unequip current item.");
            free(new_equipment);
            return;
        }
        free(player.equipment[slot]);
    }
    
    // Equip new item
    player.equipment[slot] = new_equipment;
    remove_from_inventory(index);
    
    add_message("Equipped %s", new_equipment->name);
}

// Generate a random item
Item generate_item(int floor_level) {
    Item item;
    int type = rand() % 7;  // Number of item types
    
    // Basic properties
    item.active = 1;
    item.durability = 100;
    
    switch (type) {
        case ITEM_WEAPON:
            strcpy(item.name, "Sword");
            item.symbol = '/';
            item.type = ITEM_WEAPON;
            item.power = 5 + floor_level * 2 + rand() % 5;
            item.value = 50 + floor_level * 20;
            break;
            
        case ITEM_ARMOR:
            strcpy(item.name, "Armor");
            item.symbol = ']';
            item.type = ITEM_ARMOR;
            item.power = 3 + floor_level + rand() % 3;
            item.value = 40 + floor_level * 15;
            break;
            
        case ITEM_POTION:
            strcpy(item.name, "Health Potion");
            item.symbol = '!';
            item.type = ITEM_POTION;
            item.value = 20 + floor_level * 5;
            break;
            
        case ITEM_SCROLL:
            strcpy(item.name, "Scroll");
            item.symbol = '?';
            item.type = ITEM_SCROLL;
            item.value = 30 + rand() % 20;
            break;
            
        case ITEM_FOOD:
            strcpy(item.name, "Food");
            item.symbol = '%';
            item.type = ITEM_FOOD;
            item.value = 10 + rand() % 10;
            break;
            
        case ITEM_KEY:
            strcpy(item.name, "Key");
            item.symbol = 'k';
            item.type = ITEM_KEY;
            item.value = 100;
            break;
            
        case ITEM_GOLD:
            strcpy(item.name, "Gold");
            item.symbol = '$';
            item.type = ITEM_GOLD;
            item.value = 10 + floor_level * 5 + rand() % 20;
            break;
    }
    
    return item;
}

// Add message to the message log
void add_message(const char* fmt, ...) {
    // Shift older messages up
    for (int i = MAX_MESSAGES - 1; i > 0; i--) {
        strncpy(message_log.messages[i], message_log.messages[i-1], MESSAGE_LENGTH - 1);
        message_log.messages[i][MESSAGE_LENGTH - 1] = '\0';
    }
    
    // Format and add new message
    va_list args;
    va_start(args, fmt);
    vsnprintf(message_log.messages[0], MESSAGE_LENGTH - 1, fmt, args);
    va_end(args);
    
    // Ensure null termination
    message_log.messages[0][MESSAGE_LENGTH - 1] = '\0';
    
    // Update message count
    if (message_log.num_messages < MAX_MESSAGES) {
        message_log.num_messages++;
    }
}

// Add ability to player
void add_ability(AbilityType type) {
    if (player.num_abilities >= MAX_ABILITIES) {
        add_message("Cannot learn more abilities!");
        return;
    }
    
    Ability ability;
    ability.type = type;
    ability.current_cooldown = 0;
    
    switch(type) {
        case ABILITY_HEAL:
            strcpy(ability.name, "Heal");
            strcpy(ability.description, "Restore health using mana");
            ability.cooldown = 5;
            ability.power = 20 + player.level * 5;
            ability.key = '1';
            break;
            
        case ABILITY_FIREBALL:
            strcpy(ability.name, "Fireball");
            strcpy(ability.description, "Launch a ball of fire at enemies");
            ability.cooldown = 3;
            ability.power = 15 + player.level * 3;
            ability.key = '2';
            break;
            
        case ABILITY_BLINK:
            strcpy(ability.name, "Blink");
            strcpy(ability.description, "Teleport a short distance");
            ability.cooldown = 7;
            ability.power = 5;
            ability.key = '3';
            break;
            
        case ABILITY_SHIELD:
            strcpy(ability.name, "Shield");
            strcpy(ability.description, "Temporarily increase defense");
            ability.cooldown = 10;
            ability.power = 10 + player.level * 2;
            ability.key = '4';
            break;
            
        case ABILITY_RAGE:
            strcpy(ability.name, "Rage");
            strcpy(ability.description, "Temporarily increase attack power");
            ability.cooldown = 15;
            ability.power = 15 + player.level * 2;
            ability.key = '5';
            break;
            
        default:
            return;
    }
    
    player.abilities[player.num_abilities++] = ability;
    add_message("Learned new ability: %s!", ability.name);
}

// Apply status effect to entity
void apply_status_effect(StatusType type, int duration, int power) {
    // Find existing status or empty slot
    int slot = -1;
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        if (player.status[i].type == type) {
            slot = i;
            break;
        } else if (player.status[i].type == STATUS_NONE && slot == -1) {
            slot = i;
        }
    }
    
    if (slot == -1) {
        add_message("Cannot apply more status effects!");
        return;
    }
    
    // Apply resistance for damage effects
    if (type == STATUS_POISON) {
        power = power * (100 - player.poison_resist) / 100;
    } else if (type == STATUS_BURNING) {
        power = power * (100 - player.fire_resist) / 100;
    } else if (type == STATUS_FROZEN) {
        power = power * (100 - player.ice_resist) / 100;
    }
    
    player.status[slot].type = type;
    player.status[slot].duration = duration;
    player.status[slot].power = power;
    
    switch(type) {
        case STATUS_POISON:
            add_message("You are poisoned!");
            break;
        case STATUS_BURNING:
            add_message("You are burning!");
            break;
        case STATUS_FROZEN:
            add_message("You are frozen!");
            break;
        case STATUS_BLESSED:
            add_message("You feel blessed!");
            break;
        case STATUS_CURSED:
            add_message("You feel cursed!");
            break;
        default:
            break;
    }
}

// Update status effects
void update_status_effects() {
    for (int i = 0; i < MAX_STATUS_EFFECTS; i++) {
        if (player.status[i].type == STATUS_NONE) continue;
        
        switch(player.status[i].type) {
            case STATUS_POISON:
                player.health -= player.status[i].power;
                add_message("Poison deals %d damage!", player.status[i].power);
                break;
                
            case STATUS_BURNING:
                player.health -= player.status[i].power;
                add_message("Burning deals %d damage!", player.status[i].power);
                break;
                
            case STATUS_BLESSED:
                player.power += player.status[i].power;
                break;
                
            case STATUS_CURSED:
                player.defense -= player.status[i].power;
                break;
                
            default:
                break;
        }
        
        player.status[i].duration--;
        if (player.status[i].duration <= 0) {
            // Remove effect
            if (player.status[i].type == STATUS_BLESSED) {
                player.power -= player.status[i].power;
            } else if (player.status[i].type == STATUS_CURSED) {
                player.defense += player.status[i].power;
            }
            
            add_message("Status effect %s wore off!", 
                       player.status[i].type == STATUS_POISON ? "Poison" :
                       player.status[i].type == STATUS_BURNING ? "Burning" :
                       player.status[i].type == STATUS_FROZEN ? "Frozen" :
                       player.status[i].type == STATUS_BLESSED ? "Blessed" :
                       player.status[i].type == STATUS_CURSED ? "Cursed" : "Unknown");
            
            player.status[i].type = STATUS_NONE;
            player.status[i].duration = 0;
            player.status[i].power = 0;
        }
    }
}

// Use ability
void use_ability(int index) {
    if (index < 0 || index >= player.num_abilities) return;
    
    Ability* ability = &player.abilities[index];
    if (ability->current_cooldown > 0) {
        add_message("%s is on cooldown: %d turns remaining",
                   ability->name, ability->current_cooldown);
        return;
    }
    
    int mana_cost = 20;  // Base mana cost
    if (player.mana < mana_cost) {
        add_message("Not enough mana!");
        return;
    }
    
    switch(ability->type) {
        case ABILITY_HEAL:
            player.health = min(player.max_health, 
                              player.health + ability->power);
            add_message("Healed for %d health!", ability->power);
            break;
            
        case ABILITY_FIREBALL: {
            Floor* floor = current_floor_ptr();
            // Find nearest enemy within range
            int range = 5;
            Enemy* target = NULL;
            int min_dist = range + 1;
            
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!floor->enemies[i].active) continue;
                
                int dx = floor->enemies[i].x - player.x;
                int dy = floor->enemies[i].y - player.y;
                int dist = (int)sqrt(dx*dx + dy*dy);
                
                if (dist <= range && dist < min_dist) {
                    target = &floor->enemies[i];
                    min_dist = dist;
                }
            }
            
            if (target) {
                target->health -= ability->power;
                apply_status_effect(STATUS_BURNING, 3, ability->power / 3);
                add_message("Fireball hits %s for %d damage!", 
                           target->name, ability->power);
                
                if (target->health <= 0) {
                    target->active = 0;
                    player.exp += target->exp_value;
                    add_message("%s was incinerated!", target->name);
                    
                    if (player.exp >= player.exp_next) {
                        level_up();
                    }
                }
            } else {
                add_message("No target in range!");
                return;
            }
            break;
        }
            
        case ABILITY_BLINK: {
            Floor* floor = current_floor_ptr();
            // Find random empty spot within range
            int range = ability->power;
            int attempts = 20;
            while (attempts-- > 0) {
                int new_x = player.x + random_range(-range, range);
                int new_y = player.y + random_range(-range, range);
                
                if (new_x > 0 && new_x < MAP_WIDTH-1 && 
                    new_y > 0 && new_y < MAP_HEIGHT-1 && 
                    floor->map[new_y][new_x] == '.') {
                    player.x = new_x;
                    player.y = new_y;
                    add_message("Teleported!");
                    break;
                }
            }
            if (attempts <= 0) {
                add_message("No valid location to teleport!");
                return;
            }
            break;
        }
            
        case ABILITY_SHIELD:
            apply_status_effect(STATUS_BLESSED, 5, ability->power);
            break;
            
        case ABILITY_RAGE:
            player.power += ability->power;
            add_message("Entered rage mode! Power increased by %d", 
                       ability->power);
            break;
            
        default:
            return;
    }
    
    player.mana -= mana_cost;
    ability->current_cooldown = ability->cooldown;
}

// Update ability cooldowns
void update_abilities() {
    // Regenerate mana
    player.mana = min(player.max_mana, player.mana + player.mana_regen);
    
    // Update cooldowns
    for (int i = 0; i < player.num_abilities; i++) {
        if (player.abilities[i].current_cooldown > 0) {
            player.abilities[i].current_cooldown--;
        }
    }
}

// Modified main function
int main() {
    char input;
    srand(time(NULL));
    
    init_game();
    
    while(1) {
        draw();
        
        // Update game state
        update_status_effects();
        update_abilities();
        
        if (player.health <= 0) {
            printf("\nGame Over!\n");
            break;
        }
        
        input = getch();
        
        if (input == 'q')
            break;
            
        move_player(input);
        move_enemies();
        check_combat();
        check_items();
    }
    
    return 0;
} 