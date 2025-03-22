#include <ncurses.h>
#include "store.h"
#include "player.h"
#include "ui.h"
#include "item.h"
#include "globals.h"

// Initialize a store with random items based on its type
void init_store(Store* store, StoreType type) {
    store->type = type;
    store->num_items = 0;
    store->restock_timer = 0;
    
    // Set store name and description based on type
    switch (type) {
        case STORE_GENERAL:
            strcpy(store->name, "General Store");
            strcpy(store->description, "A shop selling various useful items");
            break;
        case STORE_WEAPONS:
            strcpy(store->name, "Weapon Shop");
            strcpy(store->description, "A shop specializing in deadly weapons");
            break;
        case STORE_ARMOR:
            strcpy(store->name, "Armor Shop");
            strcpy(store->description, "A shop selling protective gear");
            break;
        case STORE_POTIONS:
            strcpy(store->name, "Potion Shop");
            strcpy(store->description, "A shop selling magical items and potions");
            break;
    }
    
    // Generate initial inventory
    restock_store(store);
}

// Restock store inventory with new items
void restock_store(Store* store) {
    store->num_items = 0;
    
    // Number of items to stock based on store type
    int num_items;
    switch (store->type) {
        case STORE_GENERAL:
            num_items = 5 + rand() % 3;  // 5-7 items
            break;
        case STORE_WEAPONS:
        case STORE_ARMOR:
            num_items = 3 + rand() % 2;  // 3-4 items
            break;
        case STORE_POTIONS:
            num_items = 4 + rand() % 2;  // 4-5 items
            break;
    }
    
    // Generate items based on store type
    for (int i = 0; i < num_items; i++) {
        Item item;
        
        switch (store->type) {
            case STORE_GENERAL:
                // Random mix of items
                item = create_random_item(current_floor);
                break;
                
            case STORE_WEAPONS:
                // Only weapons
                item = create_random_item(current_floor);
                while (item.type != ITEM_WEAPON) {
                    item = create_random_item(current_floor);
                }
                break;
                
            case STORE_ARMOR:
                // Only armor
                item = create_random_item(current_floor);
                while (item.type != ITEM_ARMOR) {
                    item = create_random_item(current_floor);
                }
                break;
                
            case STORE_POTIONS:
                // Potions and scrolls
                item = create_random_item(current_floor);
                while (item.type != ITEM_POTION && item.type != ITEM_SCROLL) {
                    item = create_random_item(current_floor);
                }
                break;
        }
        
        // Add item to store inventory
        store->inventory[store->num_items++] = item;
    }
    
    // Set restock timer (20-30 turns)
    store->restock_timer = 20 + rand() % 11;
}

// Update store state (called each turn)
void update_store(Store* store) {
    if (store->restock_timer > 0) {
        store->restock_timer--;
        if (store->restock_timer == 0) {
            restock_store(store);
        }
    }
}

// Buy an item from the store
int buy_item(Store* store, int index) {
    if (index < 0 || index >= store->num_items) {
        add_message("Invalid item selection!");
        return 0;
    }
    
    Item* item = &store->inventory[index];
    
    // Check if player has enough gold
    if (player.gold < item->value) {
        add_message("Not enough gold!");
        return 0;
    }
    
    // Try to add item to player inventory
    if (add_to_inventory(*item)) {
        player.gold -= item->value;
        add_message("Bought %s for %d gold", item->name, item->value);
        
        // Remove item from store inventory
        for (int i = index; i < store->num_items - 1; i++) {
            store->inventory[i] = store->inventory[i + 1];
        }
        store->num_items--;
        
        return 1;
    } else {
        add_message("Inventory full!");
        return 0;
    }
}

// Sell an item to the store
int sell_item(int inventory_index) {
    if (inventory_index < 0 || inventory_index >= player.num_items) {
        add_message("Invalid item selection!");
        return 0;
    }
    
    Item* item = &player.inventory[inventory_index];
    
    // Calculate sell value (50% of buy value)
    int sell_value = item->value / 2;
    
    // Add gold to player
    player.gold += sell_value;
    add_message("Sold %s for %d gold", item->name, sell_value);
    
    // Remove item from inventory
    remove_from_inventory(inventory_index);
    
    return 1;
}

// Display store interface
void display_store(Store* store) {
    int term_width, term_height;
    getmaxyx(stdscr, term_height, term_width);
    
    while (1) {
        clear();
        
        // Calculate center position
        int center_x = term_width / 2;
        int center_y = term_height / 2;
        
        // Draw store header
        attron(COLOR_PAIR(7));
        mvprintw(center_y - 10, center_x - 20, "=== %s ===", store->name);
        mvprintw(center_y - 9, center_x - 30, "%s", store->description);
        mvhline(center_y - 8, center_x - 30, '-', 60);  // Draw separator line
        attroff(COLOR_PAIR(7));
        
        // Draw store inventory
        int y = center_y - 7;
        attron(COLOR_PAIR(7));
        mvprintw(y++, center_x - 28, "Available Items:");
        attroff(COLOR_PAIR(7));
        
        for (int i = 0; i < store->num_items; i++) {
            Item* item = &store->inventory[i];
            attron(COLOR_PAIR(3));  // Yellow for items
            if (item->type == ITEM_WEAPON || item->type == ITEM_ARMOR) {
                mvprintw(y++, center_x - 28, "%d. %s (Power: %d, Value: %d)", 
                        i + 1, item->name, item->power, item->value);
            } else if (item->type == ITEM_POTION) {
                mvprintw(y++, center_x - 28, "%d. %s (Heals: %d, Value: %d)", 
                        i + 1, item->name, item->power, item->value);
            } else {
                mvprintw(y++, center_x - 28, "%d. %s (Value: %d)", 
                        i + 1, item->name, item->value);
            }
            attroff(COLOR_PAIR(3));
        }
        
        // Draw player gold
        attron(COLOR_PAIR(6));  // Cyan for gold
        mvprintw(y + 2, center_x - 28, "Your Gold: %d", player.gold);
        attroff(COLOR_PAIR(6));
        
        // Draw controls
        y += 4;
        attron(COLOR_PAIR(7));
        mvprintw(y, center_x - 28, "Commands: (b)uy item, (s)ell item, (q)uit store");
        mvprintw(y + 1, center_x - 28, "Enter command: ");
        attroff(COLOR_PAIR(7));
        
        refresh();
        
        // Get input
        char cmd = getch();
        
        if (cmd == 'q') break;
        
        if (cmd == 'b' || cmd == 's') {
            mvprintw(y + 2, center_x - 28, "Enter item number: ");
            refresh();
            
            char num_str[16];
            int i = 0;
            while (1) {
                char c = getch();
                if (c == '\n' || c == '\r') break;
                if (i < 15) num_str[i++] = c;
            }
            num_str[i] = '\0';
            
            int index = atoi(num_str) - 1;  // Convert to 0-based index
            
            if (cmd == 'b') {
                buy_item(store, index);
            } else {
                sell_item(index);
            }
            
            // Brief pause to show the result message
            refresh();
            napms(500);  // 500ms delay to show the message
        }
    }
} 

// Spawn store for the current floor
void spawn_store() 
{
    // Floor* floor = current_floor_ptr();
    // int random_room = rand() % (floor->num_rooms + 1);
    // Room* room = &floor->rooms[random_room];
    // int x = room->x + 1 + rand() % (room->width - 2);
    // int y = room->y + 1 + rand() % (room->height - 2);
    // // TODO: spawn store at location
    // floor->npcs->x = x;
    // floor->npcs->y = y;
} 