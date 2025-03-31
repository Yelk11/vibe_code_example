#include "../include/player.h"
#include "../include/map.h"
#include "../include/enemy.h"
#include "../include/store.h"


// Initialize player
void init_player() {
    strcpy(player.name, "Hero");
    player.symbol = '@';
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

// Handle player movement and actions
void move_player(int dx, int dy) {
    int new_x = player.x + dx;
    int new_y = player.y + dy;
    Floor* floor = current_floor_ptr();
    
    // Check if new position is within bounds
    if (new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT) {
        return;
    }
    
    // Check for enemies at the new position
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &floor->enemies[i];
        if (enemy->active && enemy->x == new_x && enemy->y == new_y) {
            // Attack the enemy
            int damage = max(0, player.power - enemy->defense);
            enemy->health -= damage;
            
            if (damage > 0) {
                add_message("You hit %s for %d damage!", enemy->name, damage);
            } else {
                add_message("You attack %s but do no damage!", enemy->name);
            }
            
            // Check if enemy died
            if (enemy->health <= 0) {
                kill_enemy(enemy);
            }
            return;
        }
    }
    // check for store interaction
    for (int i = 0; i < MAX_NPCS; i++) {
        if (floor->npcs[i].active && 
            floor->npcs[i].type == NPC_STOREKEEPER &&
            player.x + dx == floor->npcs[i].x && 
            player.y + dy == floor->npcs[i].y) {
            
            // Display store interface
            if (floor->npcs[i].store) {
                display_store(floor->npcs[i].store);
            }
            return;
        }
    }


    // Check for locked stairs
    if (floor->map[new_y][new_x] == TERRAIN_LOCKED_STAIRS) {
        // Check inventory for floor key
        for (int i = 0; i < player.num_items; i++) {
            if (player.inventory[i].type == ITEM_KEY &&
                player.inventory[i].key_id == current_floor + 1) {
                // Unlock the stairs
                floor->map[new_y][new_x] = '>';
                add_message("You unlock the stairs with %s!", player.inventory[i].name);
                remove_from_inventory(i);
                floor->has_floor_key = 1;
                return;
            }
        }
        add_message("The stairs are locked. You need to find the floor key.");
        return;
    }
    
    // Check if new position is walkable
    if (floor->map[new_y][new_x] == TERRAIN_WALL) {
        return;
    }
    
    // Move player
    player.x = new_x;
    player.y = new_y;
    
    // Update field of view after movement
    update_fov();
    
    // Check for items at new position
    check_player_items();
    
    // Handle stairs movement
    if (new_x == player.x && new_y == player.y) {
        char current_tile = current_floor_ptr()->map[new_y][new_x];
        
        if (current_tile == '<') {  // Up stairs
            if (current_floor > 0) {
                current_floor--;
                init_floor(current_floor);
                // Find up stairs on new floor
                for (int y = 0; y < MAP_HEIGHT; y++) {
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        if (current_floor_ptr()->map[y][x] == '>') {
                            player.x = x;
                            player.y = y;
                            break;
                        }
                    }
                }
                add_message("You climb up the stairs.");
            }
        } else if (current_tile == '>') {  // Down stairs (unlocked)
            if (current_floor < MAX_FLOORS - 1) {
                current_floor++;
                init_floor(current_floor);
                // Find down stairs on new floor
                for (int y = 0; y < MAP_HEIGHT; y++) {
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        if (current_floor_ptr()->map[y][x] == '<') {
                            player.x = x;
                            player.y = y;
                            break;
                        }
                    }
                }
                add_message("You climb down the stairs.");
            }
        } else if (current_tile == '%') {  // Locked stairs
            // Check inventory for floor key
            for (int i = 0; i < player.num_items; i++) {
                if (player.inventory[i].type == ITEM_KEY &&
                    player.inventory[i].key_id == current_floor + 1) {
                    // Unlock the stairs
                    current_floor_ptr()->map[new_y][new_x] = '>';
                    add_message("You unlock the stairs with %s!", player.inventory[i].name);
                    remove_from_inventory(i);
                    current_floor_ptr()->has_floor_key = 1;
                    return;
                }
            }
            add_message("The stairs are locked. You need to find the floor key.");
            return;
        }
    }
    
    game_turn++;
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
    
    // Set the item to inactive and ITEM_NONE
    player.inventory[index].active = 0;
    player.inventory[index].type = ITEM_NONE;
    
    // Shift all items after this one forward
    for (int i = index; i < player.num_items - 1; i++) {
        player.inventory[i] = player.inventory[i + 1];
    }
    
    // Clear the last slot
    player.inventory[player.num_items - 1].type = ITEM_NONE;
    player.inventory[player.num_items - 1].active = 0;
    
    // Decrease inventory size
    player.num_items--;
}

// Use an item from inventory
void use_item(Item* item) {
    if (!item->active) return;
    
    switch (item->type) {
        case ITEM_WEAPON:
            equip_weapon(*item);
            add_message("Equipped %s", item->name);
            break;
            
        case ITEM_ARMOR:
            equip_armor(*item);
            add_message("Equipped %s", item->name);
            break;
            
        case ITEM_POTION:
            player.health = min(player.health + item->power, player.max_health);
            add_message("Used potion, restored %d health", item->power);
            item->active = 0;
            break;
            
        case ITEM_SCROLL:
            add_message("Used scroll, gained temporary power!");
            item->active = 0;
            break;
            
        case ITEM_FOOD:
            player.health = min(player.health + item->power, player.max_health);
            add_message("Ate food, restored %d health", item->power);
            item->active = 0;
            break;
            
        case ITEM_GOLD:
            player.gold += item->value;
            add_message("Added %d gold to wallet", item->value);
            item->active = 0;
            break;
            
        case ITEM_KEY:
            add_message("This key might open something nearby...");
            break;
        case ITEM_NONE:
            /* do nothing */
            break;
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
    
    // Handle current equipped item if any
    if (player.equipment[slot] != NULL) {
        // Try to add current equipment to inventory
        Item old_equipment = *player.equipment[slot];
        free(player.equipment[slot]);
        player.equipment[slot] = NULL;
        
        if (!add_to_inventory(old_equipment)) {
            add_message("Inventory full! Cannot unequip current item.");
            return;
        }
    }
    
    // Create new equipment item
    Item* new_equipment = (Item*)malloc(sizeof(Item));
    if (new_equipment == NULL) {
        add_message("Failed to allocate memory for equipment!");
        return;
    }
    
    // Copy item to equipment slot and remove from inventory
    memcpy(new_equipment, item, sizeof(Item));
    player.equipment[slot] = new_equipment;
    
    // Remove the item from inventory
    remove_from_inventory(index);
    
    add_message("Equipped %s", new_equipment->name);
}

// Equip a weapon
void equip_weapon(Item weapon) {
    // Unequip current weapon if any
    if (player.equipment[SLOT_WEAPON]) {
        // Add current weapon to inventory if possible
        if (player.num_items < INVENTORY_SIZE) {
            player.inventory[player.num_items++] = *player.equipment[SLOT_WEAPON];
            free(player.equipment[SLOT_WEAPON]);
        } else {
            add_message("Inventory full! Cannot unequip current weapon.");
            return;
        }
    }
    
    // Create new weapon item
    Item* new_weapon = (Item*)malloc(sizeof(Item));
    if (!new_weapon) {
        add_message("Failed to allocate memory for weapon!");
        return;
    }
    *new_weapon = weapon;
    
    // Equip new weapon
    player.equipment[SLOT_WEAPON] = new_weapon;
    add_message("Equipped %s", weapon.name);
}

// Equip armor
void equip_armor(Item armor) {
    // Unequip current armor if any
    if (player.equipment[SLOT_ARMOR]) {
        // Add current armor to inventory if possible
        if (player.num_items < INVENTORY_SIZE) {
            player.inventory[player.num_items++] = *player.equipment[SLOT_ARMOR];
            free(player.equipment[SLOT_ARMOR]);
        } else {
            add_message("Inventory full! Cannot unequip current armor.");
            return;
        }
    }
    
    // Create new armor item
    Item* new_armor = (Item*)malloc(sizeof(Item));
    if (!new_armor) {
        add_message("Failed to allocate memory for armor!");
        return;
    }
    *new_armor = armor;
    
    // Equip new armor
    player.equipment[SLOT_ARMOR] = new_armor;
    add_message("Equipped %s", armor.name);
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



void check_player_items() {
    Floor* floor = current_floor_ptr();
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) continue;
        
        if (player.x == floor->items[i].x && player.y == floor->items[i].y) {
            Item* item = &floor->items[i];
            
            if (item->type == ITEM_KEY && item->key_id == current_floor + 1) {
                // Found the floor key
                if (add_to_inventory(*item)) {
                    add_message("Found %s! This will unlock the way forward.", item->name);
                    floor->items[i].active = 0;
                } else {
                    add_message("Inventory full! Cannot pick up the floor key.");
                }
            } else if (item->type == ITEM_GOLD) {
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