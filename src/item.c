#include "item.h"
#include "player.h"
#include "map.h"

// Get random item type
ItemType get_random_item_type() {
    int r = rand() % 100;
    
    if (r < 30) return ITEM_GOLD;      // 30%
    if (r < 50) return ITEM_POTION;    // 20%
    if (r < 65) return ITEM_FOOD;      // 15%
    if (r < 75) return ITEM_SCROLL;    // 10%
    if (r < 90) return ITEM_WEAPON;    // 15%
    return ITEM_ARMOR;                 // 10%
}

// Create a random item
Item create_random_item(int floor_level) {
    Item item;
    ItemType type = get_random_item_type();
    
    // Set common properties
    item.active = 1;
    item.durability = 100;
    
    switch(type) {
        case ITEM_WEAPON: {
            static const char* prefixes[] = {
                "Rusty", "Sharp", "Mighty", "Ancient",
                "Glowing", "Cursed", "Blessed", "Dragon"
            };
            static const char* types[] = {
                "Dagger", "Sword", "Axe", "Mace",
                "Spear", "Hammer", "Blade", "Scythe"
            };
            
            int prefix_idx = rand() % (sizeof(prefixes) / sizeof(prefixes[0]));
            int type_idx = rand() % (sizeof(types) / sizeof(types[0]));
            
            snprintf(item.name, MAX_NAME_LEN, "%s %s", 
                    prefixes[prefix_idx], types[type_idx]);
            snprintf(item.description, MAX_DESC_LEN,
                    "A %s weapon that deals %d damage",
                    prefixes[prefix_idx], 5 + floor_level * 2);
            
            item.type = ITEM_WEAPON;
            item.symbol = '|';
            item.power = 5 + floor_level * 2;
            item.value = 50 + floor_level * 25;
            break;
        }
            
        case ITEM_ARMOR: {
            static const char* materials[] = {
                "Leather", "Chain", "Scale", "Plate",
                "Crystal", "Dragon", "Shadow", "Holy"
            };
            static const char* types[] = {
                "Armor", "Mail", "Guard", "Shield",
                "Aegis", "Plate", "Cover", "Ward"
            };
            
            int material_idx = rand() % (sizeof(materials) / sizeof(materials[0]));
            int type_idx = rand() % (sizeof(types) / sizeof(types[0]));
            
            snprintf(item.name, MAX_NAME_LEN, "%s %s",
                    materials[material_idx], types[type_idx]);
            snprintf(item.description, MAX_DESC_LEN,
                    "Protective %s armor that blocks %d damage",
                    materials[material_idx], 3 + floor_level);
            
            item.type = ITEM_ARMOR;
            item.symbol = ']';
            item.power = 3 + floor_level;
            item.value = 40 + floor_level * 20;
            break;
        }
            
        case ITEM_POTION: {
            strcpy(item.name, "Health Potion");
            snprintf(item.description, MAX_DESC_LEN,
                    "Restores %d health when consumed",
                    20 + floor_level * 5);
            item.type = ITEM_POTION;
            item.symbol = '!';
            item.value = 20 + floor_level * 5;
            item.power = 20 + floor_level * 5;
            break;
        }
            
        case ITEM_SCROLL: {
            strcpy(item.name, "Magic Scroll");
            strcpy(item.description, "A mysterious scroll");
            item.type = ITEM_SCROLL;
            item.symbol = '?';
            item.value = 30 + floor_level * 10;
            item.power = floor_level;
            break;
        }
            
        case ITEM_FOOD: {
            strcpy(item.name, "Ration");
            snprintf(item.description, MAX_DESC_LEN,
                    "Restores %d health when eaten",
                    10 + floor_level * 2);
            item.type = ITEM_FOOD;
            item.symbol = '%';
            item.value = 10 + floor_level * 2;
            item.power = 10 + floor_level * 2;
            break;
        }
            
        case ITEM_GOLD: {
            int amount = 10 + rand() % (20 + floor_level * 10);
            snprintf(item.name, MAX_NAME_LEN, "%d Gold", amount);
            snprintf(item.description, MAX_DESC_LEN,
                    "A pile of %d gold coins", amount);
            item.type = ITEM_GOLD;
            item.symbol = '$';
            item.value = amount;
            item.power = 0;
            break;
        }
            
        default:
            break;
    }
    
    return item;
}

// Initialize items on a floor
void init_items(Floor* floor) {
    // Clear existing items
    for (int i = 0; i < MAX_ITEMS; i++) {
        floor->items[i].active = 0;
    }
    
    // Place random items in rooms
    for (int i = 0; i < floor->num_rooms; i++) {
        Room* room = &floor->rooms[i];
        
        // 50% chance for each room to have an item
        if (rand() % 2 == 0) {
            for (int j = 0; j < MAX_ITEMS; j++) {
                if (!floor->items[j].active) {
                    floor->items[j] = create_random_item(current_floor);
                    floor->items[j].x = room->x + 1 + rand() % (room->width - 2);
                    floor->items[j].y = room->y + 1 + rand() % (room->height - 2);
                    floor->items[j].active = 1;
                    break;
                }
            }
        }
    }
} 