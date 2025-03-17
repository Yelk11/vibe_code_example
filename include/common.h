#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

// Color codes for terminal
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"

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

// Forward declarations of structures
typedef struct Item Item;
typedef struct Enemy Enemy;
typedef struct StatusEffect StatusEffect;
typedef struct Ability Ability;
typedef struct Player Player;
typedef struct Room Room;
typedef struct Floor Floor;
typedef struct MessageLog MessageLog;

// Structure definitions
struct Item {
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
};

struct Enemy {
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
};

struct StatusEffect {
    StatusType type;
    int duration;
    int power;
};

struct Ability {
    AbilityType type;
    int cooldown;
    int current_cooldown;
    int power;
    char key;           // Hotkey to use ability
    char name[32];
    char description[128];
};

struct Player {
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
};

struct Room {
    int x;
    int y;
    int width;
    int height;
};

struct Floor {
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
};

struct MessageLog {
    char messages[MAX_MESSAGES][MESSAGE_LENGTH];
    int num_messages;
};

// Global game state
extern Floor floors[MAX_FLOORS];
extern int current_floor;
extern Player player;
extern int camera_x;
extern int camera_y;
extern int game_turn;
extern MessageLog message_log;

// Utility functions
int random_range(int min, int max);
char getch(void);
void add_message(const char* fmt, ...);

#endif // COMMON_H 