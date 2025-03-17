#include "quest.h"
#include "map.h"

// Global state
Quest quests[MAX_QUESTS];
int num_quests = 0;
NPC npcs[MAX_NPCS];
int num_npcs = 0;
Achievement achievements[MAX_ACHIEVEMENTS];
int num_achievements = 0;

// Dialogue tree
#define MAX_DIALOGUE_NODES 100
static DialogueNode dialogue_tree[MAX_DIALOGUE_NODES];
static int num_dialogue_nodes = 0;

// Initialize the quest system
void init_quests() {
    // Main quest
    Quest main_quest = {
        .id = num_quests++,
        .name = "The Ancient Evil",
        .description = "Defeat the dragon terrorizing the realm",
        .status = QUEST_INACTIVE,
        .is_main_quest = 1,
        .num_objectives = 3,
        .prereq_quest_id = -1,
        .reward_gold = 1000,
        .reward_exp = 500,
        .has_choice = 0
    };
    
    // Set objectives for main quest
    main_quest.objectives[0] = (Objective){
        .type = OBJECTIVE_REACH_LOCATION,
        .required_amount = 1,
        .current_amount = 0,
        .description = "Reach the sage's tower on floor 3",
        .target_id = 3  // Floor 3
    };
    
    main_quest.objectives[1] = (Objective){
        .type = OBJECTIVE_COLLECT_ITEMS,
        .required_amount = 3,
        .current_amount = 0,
        .description = "Collect 3 ancient artifacts",
        .target_id = ITEM_KEY  // Special key items
    };
    
    main_quest.objectives[2] = (Objective){
        .type = OBJECTIVE_KILL_ENEMIES,
        .required_amount = 1,
        .current_amount = 0,
        .description = "Defeat the dragon",
        .target_id = ENEMY_BOSS
    };
    
    quests[0] = main_quest;
    
    // Side quest 1: The Lost Sword
    Quest side_quest1 = {
        .id = num_quests++,
        .name = "The Lost Sword",
        .description = "Find the blacksmith's prized sword",
        .status = QUEST_INACTIVE,
        .is_main_quest = 0,
        .num_objectives = 1,
        .prereq_quest_id = -1,
        .reward_gold = 100,
        .reward_exp = 50,
        .has_choice = 1  // Can return or keep the sword
    };
    
    side_quest1.objectives[0] = (Objective){
        .type = OBJECTIVE_COLLECT_ITEMS,
        .required_amount = 1,
        .current_amount = 0,
        .description = "Find the ancient sword",
        .target_id = ITEM_WEAPON
    };
    
    quests[1] = side_quest1;
    
    // Side quest 2: Pest Control
    Quest side_quest2 = {
        .id = num_quests++,
        .name = "Pest Control",
        .description = "Clear the basement of monsters",
        .status = QUEST_INACTIVE,
        .is_main_quest = 0,
        .num_objectives = 1,
        .prereq_quest_id = -1,
        .reward_gold = 75,
        .reward_exp = 40,
        .has_choice = 0
    };
    
    side_quest2.objectives[0] = (Objective){
        .type = OBJECTIVE_KILL_ENEMIES,
        .required_amount = 5,
        .current_amount = 0,
        .description = "Defeat 5 goblins",
        .target_id = ENEMY_BASIC
    };
    
    quests[2] = side_quest2;
}

// Initialize NPCs
void init_npcs() {
    // Quest giver
    NPC elder = {
        .id = num_npcs++,
        .name = "Village Elder",
        .description = "A wise old man with many tales",
        .type = NPC_QUEST_GIVER,
        .symbol = 'E',
        .current_dialogue_id = 0
    };
    npcs[0] = elder;
    
    // Blacksmith
    NPC blacksmith = {
        .id = num_npcs++,
        .name = "Master Smith",
        .description = "A skilled weaponsmith",
        .type = NPC_BLACKSMITH,
        .symbol = 'B',
        .current_dialogue_id = 10
    };
    npcs[1] = blacksmith;
    
    // Sage
    NPC sage = {
        .id = num_npcs++,
        .name = "Mysterious Sage",
        .description = "Knows ancient secrets",
        .type = NPC_SAGE,
        .symbol = 'S',
        .current_dialogue_id = 20
    };
    npcs[2] = sage;
    
    // Place NPCs in appropriate rooms
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < num_npcs; i++) {
        Room* room = &floor->rooms[i % floor->num_rooms];
        npcs[i].x = room->x + room->width / 2;
        npcs[i].y = room->y + room->height / 2;
    }
}

// Initialize dialogue tree
void init_dialogue() {
    // Elder's dialogue
    DialogueNode elder_greeting = {
        .id = num_dialogue_nodes++,
        .text = "Welcome, brave adventurer. Our village needs your help.",
        .num_options = 2
    };
    
    elder_greeting.options[0] = (DialogueOption){
        .text = "What's the problem?",
        .next_dialogue_id = 1,
        .quest_id = -1,
        .required_quest_id = -1
    };
    
    elder_greeting.options[1] = (DialogueOption){
        .text = "Sorry, not interested.",
        .next_dialogue_id = -1,
        .quest_id = -1,
        .required_quest_id = -1
    };
    
    dialogue_tree[0] = elder_greeting;
    
    DialogueNode elder_quest = {
        .id = num_dialogue_nodes++,
        .text = "A terrible dragon threatens our land. Will you help us?",
        .num_options = 2
    };
    
    elder_quest.options[0] = (DialogueOption){
        .text = "I'll help you.",
        .next_dialogue_id = 2,
        .quest_id = 0,  // Start main quest
        .required_quest_id = -1
    };
    
    elder_quest.options[1] = (DialogueOption){
        .text = "I need to prepare first.",
        .next_dialogue_id = -1,
        .quest_id = -1,
        .required_quest_id = -1
    };
    
    dialogue_tree[1] = elder_quest;
    
    // Add more dialogue nodes for other NPCs...
}

// Initialize achievements
void init_achievements() {
    // Dragon Slayer
    Achievement ach1 = {
        .name = "Dragon Slayer",
        .description = "Defeat the mighty dragon",
        .type = ACHIEVEMENT_KILL_BOSS,
        .required_amount = 1,
        .current_amount = 0,
        .is_unlocked = 0
    };
    achievements[num_achievements++] = ach1;
    
    // Dungeon Delver
    Achievement ach2 = {
        .name = "Dungeon Delver",
        .description = "Reach the final floor",
        .type = ACHIEVEMENT_REACH_FLOOR,
        .required_amount = MAX_FLOORS,
        .current_amount = 0,
        .is_unlocked = 0
    };
    achievements[num_achievements++] = ach2;
    
    // Treasure Hunter
    Achievement ach3 = {
        .name = "Treasure Hunter",
        .description = "Collect 1000 gold",
        .type = ACHIEVEMENT_COLLECT_GOLD,
        .required_amount = 1000,
        .current_amount = 0,
        .is_unlocked = 0
    };
    achievements[num_achievements++] = ach3;
}

// Update quest progress
void update_quests() {
    for (int i = 0; i < num_quests; i++) {
        if (quests[i].status != QUEST_ACTIVE) continue;
        
        // Check if all objectives are completed
        int completed = 1;
        for (int j = 0; j < quests[i].num_objectives; j++) {
            if (quests[i].objectives[j].current_amount < 
                quests[i].objectives[j].required_amount) {
                completed = 0;
                break;
            }
        }
        
        if (completed) {
            complete_quest(i);
        }
    }
}

// Start a quest
void start_quest(int quest_id) {
    if (quest_id < 0 || quest_id >= num_quests) return;
    
    Quest* quest = &quests[quest_id];
    if (quest->prereq_quest_id != -1 && 
        quests[quest->prereq_quest_id].status != QUEST_COMPLETED) {
        add_message("Cannot start quest: prerequisites not met");
        return;
    }
    
    quest->status = QUEST_ACTIVE;
    add_message("Started quest: %s", quest->name);
    add_message("%s", quest->description);
}

// Complete a quest
void complete_quest(int quest_id) {
    if (quest_id < 0 || quest_id >= num_quests) return;
    
    Quest* quest = &quests[quest_id];
    quest->status = QUEST_COMPLETED;
    
    // Give rewards
    player.gold += quest->reward_gold;
    player.exp += quest->reward_exp;
    if (quest->reward_item.type != ITEM_GOLD) {
        add_to_inventory(quest->reward_item);
    }
    
    add_message("Completed quest: %s", quest->name);
    add_message("Received %d gold and %d experience!", 
                quest->reward_gold, quest->reward_exp);
    
    // Check for level up
    if (player.exp >= player.exp_next) {
        level_up();
    }
}

// Update quest objective
void update_objective(int quest_id, ObjectiveType type, int target_id, int amount) {
    if (quest_id < 0 || quest_id >= num_quests) return;
    
    Quest* quest = &quests[quest_id];
    if (quest->status != QUEST_ACTIVE) return;
    
    for (int i = 0; i < quest->num_objectives; i++) {
        Objective* obj = &quest->objectives[i];
        if (obj->type == type && obj->target_id == target_id) {
            obj->current_amount += amount;
            if (obj->current_amount >= obj->required_amount) {
                add_message("Objective completed: %s", obj->description);
            } else {
                add_message("Progress: %s (%d/%d)", 
                           obj->description, 
                           obj->current_amount, 
                           obj->required_amount);
            }
        }
    }
}

// Handle NPC interaction
void handle_npc_interaction(NPC* npc) {
    if (npc->current_dialogue_id >= 0 && 
        npc->current_dialogue_id < num_dialogue_nodes) {
        display_dialogue(&dialogue_tree[npc->current_dialogue_id]);
    }
}

// Display dialogue
void display_dialogue(DialogueNode* node) {
    system("clear");
    printf("\n=== Dialogue ===\n\n");
    printf("%s\n\n", node->text);
    
    for (int i = 0; i < node->num_options; i++) {
        // Check if option is available
        if (node->options[i].required_quest_id != -1 && 
            quests[node->options[i].required_quest_id].status != QUEST_COMPLETED) {
            continue;
        }
        printf("%d) %s\n", i + 1, node->options[i].text);
    }
    
    printf("\nEnter choice (1-%d): ", node->num_options);
    fflush(stdout);
    
    char choice = getch();
    int option = choice - '1';
    
    if (option >= 0 && option < node->num_options) {
        process_dialogue_choice(NULL, option);  // TODO: Pass current NPC
    }
}

// Process dialogue choice
void process_dialogue_choice(NPC* npc, int choice) {
    DialogueNode* current = &dialogue_tree[npc->current_dialogue_id];
    DialogueOption* option = &current->options[choice];
    
    // Start or update quest if needed
    if (option->quest_id >= 0) {
        start_quest(option->quest_id);
    }
    
    // Move to next dialogue or end conversation
    if (option->next_dialogue_id >= 0) {
        npc->current_dialogue_id = option->next_dialogue_id;
        display_dialogue(&dialogue_tree[option->next_dialogue_id]);
    }
}

// Update NPC positions
void update_npc_positions() {
    // NPCs could move around, patrol areas, etc.
}

// Check achievement progress
void check_achievement_progress() {
    for (int i = 0; i < num_achievements; i++) {
        if (achievements[i].is_unlocked) continue;
        
        Achievement* ach = &achievements[i];
        switch (ach->type) {
            case ACHIEVEMENT_KILL_BOSS:
                // Check in combat function
                break;
                
            case ACHIEVEMENT_REACH_FLOOR:
                ach->current_amount = current_floor + 1;
                break;
                
            case ACHIEVEMENT_COLLECT_GOLD:
                ach->current_amount = player.gold;
                break;
                
            case ACHIEVEMENT_COMPLETE_QUESTS:
                ach->current_amount = 0;
                for (int j = 0; j < num_quests; j++) {
                    if (quests[j].status == QUEST_COMPLETED) {
                        ach->current_amount++;
                    }
                }
                break;
                
            // Add other achievement types...
        }
        
        if (ach->current_amount >= ach->required_amount) {
            unlock_achievement(i);
        }
    }
}

// Unlock achievement
void unlock_achievement(int achievement_id) {
    if (achievement_id < 0 || achievement_id >= num_achievements) return;
    
    Achievement* ach = &achievements[achievement_id];
    if (ach->is_unlocked) return;
    
    ach->is_unlocked = 1;
    add_message("Achievement unlocked: %s", ach->name);
    add_message("%s", ach->description);
}

// Display achievements
void display_achievements() {
    system("clear");
    printf("\n=== Achievements ===\n\n");
    
    for (int i = 0; i < num_achievements; i++) {
        Achievement* ach = &achievements[i];
        printf("%s %s\n", 
               ach->is_unlocked ? "[X]" : "[ ]",
               ach->name);
        printf("    %s\n", ach->description);
        if (!ach->is_unlocked) {
            printf("    Progress: %d/%d\n", 
                   ach->current_amount, 
                   ach->required_amount);
        }
        printf("\n");
    }
    
    printf("\nPress any key to continue...");
    getch();
} 