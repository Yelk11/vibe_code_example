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
#define MAX_FLOORS 26
#define INVENTORY_SIZE 20
#define MAX_ENEMY_TYPES 4
#define VIEW_RADIUS 8
#define MAX_NAME_LEN 32
#define MAX_DESC_LEN 128
#define MAX_MESSAGES 10
#define MESSAGE_LENGTH 80
#define MAX_DOORS 10  // Maximum number of doors per floor
#define MAX_DIALOGUE_LEN 256
#define MAX_NPCS 20
#define MAX_QUESTS 20
#define MAX_OBJECTIVES 5
#define MAX_ACHIEVEMENTS 20
#define MAX_INVENTORY 20
#define MAX_STATUS_EFFECTS 10

// Color codes for terminal
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_WHITE   "\x1b[37m"

// Forward declarations of structures
typedef struct Item Item;
typedef struct Enemy Enemy;
typedef struct StatusEffect StatusEffect;
typedef struct Ability Ability;
typedef struct Player Player;
typedef struct Room Room;
typedef struct Floor Floor;
typedef struct MessageLog MessageLog;
typedef struct Door Door;
typedef struct Quest Quest;
typedef struct NPC NPC;
typedef struct Achievement Achievement;
typedef struct Objective Objective;
typedef struct DialogueNode DialogueNode;
typedef struct DialogueOption DialogueOption;

// Item types
typedef enum {
    ITEM_NONE,
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
    TERRAIN_LOCKED_DOOR = 'D',
    TERRAIN_LOCKED_STAIRS = '%',
    TERRAIN_GRASS = '"',
    TERRAIN_TRAP = '_'
} TerrainType;

// Status effect types
typedef enum {
    STATUS_NONE,
    STATUS_POISON,     // Damage over time
    STATUS_BURN,       // Damage over time + reduced defense
    STATUS_FREEZE,     // Reduced speed
    STATUS_STUN,       // Skip turn
    STATUS_BLIND,      // Reduced accuracy
    STATUS_BERSERK     // Increased damage + reduced defense
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

// Quest status
typedef enum {
    QUEST_INACTIVE,
    QUEST_ACTIVE,
    QUEST_COMPLETED,
    QUEST_FAILED
} QuestStatus;

// Quest objective types
typedef enum {
    OBJECTIVE_KILL_ENEMIES,
    OBJECTIVE_COLLECT_ITEMS,
    OBJECTIVE_REACH_LOCATION,
    OBJECTIVE_TALK_TO_NPC
} ObjectiveType;

// NPC types
typedef enum {
    NPC_QUEST_GIVER,
    NPC_BLACKSMITH,
    NPC_SAGE,
    NPC_MERCHANT
} NPCType;

// Achievement types
typedef enum {
    ACHIEVEMENT_KILL_BOSS,
    ACHIEVEMENT_REACH_FLOOR,
    ACHIEVEMENT_COLLECT_GOLD,
    ACHIEVEMENT_COMPLETE_QUESTS,
    ACHIEVEMENT_FIND_UNIQUE_ITEMS,
    ACHIEVEMENT_MAX_LEVEL
} AchievementType;

// Door structure definition
struct Door {
    int x;
    int y;
    int floor_num;  // Which floor the door is on
    int key_id;     // Unique ID to match with corresponding key
    int is_locked;  // Whether the door is currently locked
};

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
    int key_id;     // Unique ID for keys to match with doors
    int target_floor;  // Floor where this key should be used
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
    Item inventory[MAX_INVENTORY];
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
    Door doors[MAX_DOORS];  // Array of doors on this floor
    int num_doors;         // Number of doors currently on floor
    TerrainType terrain[MAP_HEIGHT][MAP_WIDTH];
    int has_floor_key;  // Whether the floor key has been collected
    int has_visited;    // Whether the player has visited this floor before
};

struct MessageLog {
    char messages[MAX_MESSAGES][MESSAGE_LENGTH];
    int num_messages;
};

struct Objective {
    ObjectiveType type;
    int required_amount;
    int current_amount;
    char description[MAX_DESC_LEN];
    int target_id;  // Enemy type, item type, or location ID
};

struct Quest {
    int id;
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    QuestStatus status;
    int is_main_quest;
    int num_objectives;
    Objective objectives[MAX_OBJECTIVES];
    int prereq_quest_id;
    int reward_gold;
    int reward_exp;
    Item reward_item;
    int has_choice;
};

struct NPC {
    int id;
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    NPCType type;
    char symbol;
    int x;
    int y;
    int floor;
    int active;
    int current_dialogue_id;
    int shop_inventory[INVENTORY_SIZE];
    int num_shop_items;
    int quest_id;  // -1 if no quest
    char dialogue[MAX_DIALOGUE_LEN];
};

struct Achievement {
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    AchievementType type;
    int required_amount;
    int current_amount;
    int is_unlocked;
};

struct DialogueOption {
    char text[MAX_DIALOGUE_LEN];
    int next_dialogue_id;
    int quest_id;
    int required_quest_id;
};

struct DialogueNode {
    int id;
    char text[MAX_DIALOGUE_LEN];
    int num_options;
    DialogueOption options[4];  // Max 4 options per dialogue node
};

// Global variables (moved after struct definitions)
extern int camera_x;
extern int camera_y;
extern char messages[MAX_MESSAGES][MESSAGE_LENGTH];
extern int current_floor;
extern Floor floors[MAX_FLOORS];
extern Player player;
extern int game_turn;
extern MessageLog message_log;

// Utility functions
int random_range(int min, int max);
int getch(void);
void add_message(const char* fmt, ...);
Floor* current_floor_ptr(void);
const char* get_status_name(StatusType type);

#endif // COMMON_H 