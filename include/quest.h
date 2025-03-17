#ifndef QUEST_H
#define QUEST_H

#include "common.h"

#define MAX_QUESTS 20
#define MAX_OBJECTIVES 5
#define MAX_DIALOGUE_OPTIONS 4
#define MAX_NPCS 10
#define MAX_ACHIEVEMENTS 30
#define MAX_DIALOGUE_LENGTH 256

typedef enum {
    QUEST_INACTIVE,
    QUEST_ACTIVE,
    QUEST_COMPLETED,
    QUEST_FAILED
} QuestStatus;

typedef enum {
    OBJECTIVE_KILL_ENEMIES,
    OBJECTIVE_COLLECT_ITEMS,
    OBJECTIVE_REACH_LOCATION,
    OBJECTIVE_TALK_TO_NPC,
    OBJECTIVE_CLEAR_FLOOR
} ObjectiveType;

typedef enum {
    NPC_QUEST_GIVER,
    NPC_MERCHANT,
    NPC_SAGE,
    NPC_BLACKSMITH,
    NPC_HEALER
} NPCType;

typedef struct {
    ObjectiveType type;
    int required_amount;
    int current_amount;
    char description[MAX_DESC_LEN];
    int target_id;  // Enemy type, item type, or NPC id
} Objective;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    QuestStatus status;
    int is_main_quest;
    Objective objectives[MAX_OBJECTIVES];
    int num_objectives;
    int prereq_quest_id;  // Quest that must be completed before this one
    int reward_gold;
    int reward_exp;
    Item reward_item;
    int has_choice;  // Whether quest has multiple outcomes
} Quest;

typedef struct {
    char text[MAX_DIALOGUE_LENGTH];
    int next_dialogue_id;  // -1 for end of conversation
    int quest_id;         // Quest to start/update/complete
    int required_quest_id; // Quest required to see this option
} DialogueOption;

typedef struct {
    int id;
    char text[MAX_DIALOGUE_LENGTH];
    DialogueOption options[MAX_DIALOGUE_OPTIONS];
    int num_options;
} DialogueNode;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    NPCType type;
    int x;
    int y;
    char symbol;
    int current_dialogue_id;
    int shop_inventory[INVENTORY_SIZE];
    int num_shop_items;
} NPC;

typedef enum {
    ACHIEVEMENT_KILL_BOSS,
    ACHIEVEMENT_REACH_FLOOR,
    ACHIEVEMENT_COLLECT_GOLD,
    ACHIEVEMENT_COMPLETE_QUESTS,
    ACHIEVEMENT_FIND_UNIQUE_ITEMS,
    ACHIEVEMENT_MAX_LEVEL
} AchievementType;

typedef struct {
    char name[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    AchievementType type;
    int required_amount;
    int current_amount;
    int is_unlocked;
} Achievement;

// Global quest state
extern Quest quests[MAX_QUESTS];
extern int num_quests;
extern NPC npcs[MAX_NPCS];
extern int num_npcs;
extern Achievement achievements[MAX_ACHIEVEMENTS];
extern int num_achievements;

// Quest management functions
void init_quests(void);
void init_npcs(void);
void init_achievements(void);
void update_quests(void);
void update_achievements(void);
void start_quest(int quest_id);
void complete_quest(int quest_id);
void fail_quest(int quest_id);
void update_objective(int quest_id, ObjectiveType type, int target_id, int amount);

// NPC interaction functions
void init_dialogue(void);
void handle_npc_interaction(NPC* npc);
void display_dialogue(DialogueNode* node);
void process_dialogue_choice(NPC* npc, int choice);
void update_npc_positions(void);

// Achievement functions
void check_achievement_progress(void);
void unlock_achievement(int achievement_id);
void display_achievements(void);

#endif // QUEST_H 