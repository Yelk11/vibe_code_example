#ifndef QUEST_H
#define QUEST_H

#include "common.h"
#include "item.h"

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
void view_quests(void);
void view_achievements(void);

// NPC interaction functions
void init_dialogue(void);
void handle_npc_interaction(NPC* npc);
void display_dialogue(DialogueNode* node);
void process_dialogue_choice(NPC* npc, int choice);
void update_npc_positions(void);

// Achievement functions
void check_achievement_progress(void);
void unlock_achievement(int achievement_id);

#endif // QUEST_H 