#include "item.h"

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

// Check for items at player's position
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