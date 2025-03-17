void init_game() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game state
    current_floor = 0;
    game_turn = 0;
    
    // Initialize all floors
    for (int i = 0; i < MAX_FLOORS; i++) {
        init_floor(&floors[i]);
    }
    
    // Initialize player
    init_player();
    
    // Place player in first room of first floor
    Floor* floor = current_floor_ptr();
    Room* first_room = &floor->rooms[0];
    player.x = first_room->x + first_room->width / 2;
    player.y = first_room->y + first_room->height / 2;
    
    // Initialize enemies on each floor
    for (int i = 0; i < MAX_FLOORS; i++) {
        current_floor = i;
        init_enemies();
        init_items();
    }
    current_floor = 0;
    
    // Initialize quest system
    init_quests();
    init_npcs();
    init_dialogue();
    init_achievements();
    
    // Initial message
    add_message("Welcome to the dungeon!");
    add_message("Use WASD to move, I for inventory");
    add_message("Press T to talk when near an NPC");
    add_message("Press Q to view quests");
    add_message("Press V to view achievements");
}

int game_loop() {
    char input;
    Floor* floor;
    
    while (1) {
        floor = current_floor_ptr();
        
        // Update field of view
        update_fov();
        
        // Draw game state
        draw();
        
        // Get input
        input = getch();
        
        // Process input
        switch (input) {
            case 'w':
            case 'a':
            case 's':
            case 'd':
                move_player(input);
                move_enemies();
                check_combat();
                check_items();
                update_abilities();
                update_status_effects();
                update_quests();
                check_achievement_progress();
                break;
                
            case 'i':
                handle_inventory();
                break;
                
            case 't': {
                // Check for nearby NPCs
                for (int i = 0; i < num_npcs; i++) {
                    int dx = abs(player.x - npcs[i].x);
                    int dy = abs(player.y - npcs[i].y);
                    if (dx <= 1 && dy <= 1) {
                        handle_npc_interaction(&npcs[i]);
                        break;
                    }
                }
                break;
            }
                
            case 'q': {
                // Display active quests
                system("clear");
                printf("\n=== Quests ===\n\n");
                
                // Show main quest first
                for (int i = 0; i < num_quests; i++) {
                    if (!quests[i].is_main_quest) continue;
                    
                    Quest* quest = &quests[i];
                    printf("%s%s\n", 
                           quest->status == QUEST_COMPLETED ? "[X] " :
                           quest->status == QUEST_ACTIVE ? "[*] " : "[ ] ",
                           quest->name);
                    printf("    %s\n", quest->description);
                    
                    if (quest->status == QUEST_ACTIVE) {
                        for (int j = 0; j < quest->num_objectives; j++) {
                            printf("    - %s (%d/%d)\n",
                                   quest->objectives[j].description,
                                   quest->objectives[j].current_amount,
                                   quest->objectives[j].required_amount);
                        }
                    }
                    printf("\n");
                }
                
                // Show side quests
                for (int i = 0; i < num_quests; i++) {
                    if (quests[i].is_main_quest) continue;
                    
                    Quest* quest = &quests[i];
                    if (quest->status == QUEST_INACTIVE) continue;
                    
                    printf("%s%s\n", 
                           quest->status == QUEST_COMPLETED ? "[X] " :
                           quest->status == QUEST_ACTIVE ? "[*] " : "[ ] ",
                           quest->name);
                    printf("    %s\n", quest->description);
                    
                    if (quest->status == QUEST_ACTIVE) {
                        for (int j = 0; j < quest->num_objectives; j++) {
                            printf("    - %s (%d/%d)\n",
                                   quest->objectives[j].description,
                                   quest->objectives[j].current_amount,
                                   quest->objectives[j].required_amount);
                        }
                    }
                    printf("\n");
                }
                
                printf("\nPress any key to continue...");
                getch();
                break;
            }
                
            case 'v':
                display_achievements();
                break;
                
            case 'Q':
                return 0;  // Quit game
                
            default:
                break;
        }
        
        // Check if player is dead
        if (player.health <= 0) {
            add_message("You have died!");
            return 1;
        }
    }
    
    return 0;
} 