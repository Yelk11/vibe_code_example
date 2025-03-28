#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/map.h"
#include "../include/globals.h"
#include "../include/enemy.h"
#include "../include/item.h"
#include "../include/player.h"
#include "../include/store.h"

// Get current floor
Floor* current_floor_ptr() {
    return &floors[current_floor];
}

// Generate a random room with different types
Room generate_room() {
    Room room;
    
    // Initialize common room properties
    room.type = ROOM_NORMAL;  // Only use normal square rooms
    
    // Generate square room dimensions with larger minimum size
    room.width = random_range(MIN_ROOM_SIZE + 4, MAX_ROOM_SIZE);  // Increased minimum size
    room.height = room.width;  // Make it square
    room.x = random_range(1, MAP_WIDTH - room.width - 1);
    room.y = random_range(1, MAP_HEIGHT - room.height - 1);
    
    return room;
}

// Check if two rooms overlap
int rooms_overlap(Room* r1, Room* r2, int padding) {
    // Add extra padding to ensure rooms don't touch
    return !(r1->x + r1->width + padding + 2 < r2->x ||
             r2->x + r2->width + padding + 2 < r1->x ||
             r1->y + r1->height + padding + 2 < r2->y ||
             r2->y + r2->height + padding + 2 < r1->y);
}

// Create a tunnel between two points with improved pathing
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    // Create shorter tunnel by connecting to room edges instead of centers
    int prev_x = x1;
    int prev_y = y1;
    int curr_x = x2;
    int curr_y = y2;
    
    // Find the closest points between rooms
    if (abs(x1 - x2) > abs(y1 - y2)) {
        // Rooms are further apart horizontally
        if (x1 < x2) {
            prev_x = x1 + 1;  // Start from right edge of first room
            curr_x = x2 - 1;  // End at left edge of second room
        } else {
            prev_x = x1 - 1;  // Start from left edge of first room
            curr_x = x2 + 1;  // End at right edge of second room
        }
    } else {
        // Rooms are further apart vertically
        if (y1 < y2) {
            prev_y = y1 + 1;  // Start from bottom edge of first room
            curr_y = y2 - 1;  // End at top edge of second room
        } else {
            prev_y = y1 - 1;  // Start from top edge of first room
            curr_y = y2 + 1;  // End at bottom edge of second room
        }
    }
    
    // Create straight tunnel between the closest points
    create_straight_tunnel(floor, prev_x, prev_y, curr_x, curr_y);
}

// Create a straight tunnel between two points
void create_straight_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    int current_x = x1;
    int current_y = y1;
    
    // Always go horizontal then vertical for consistent hallways
    while (current_x != x2) {
        // Only create one tile wide hallway
        floor->map[current_y][current_x] = '.';
        floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
        current_x += (x2 > x1) ? 1 : -1;
    }
    while (current_y != y2) {
        // Only create one tile wide hallway
        floor->map[current_y][current_x] = '.';
        floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
        current_y += (y2 > y1) ? 1 : -1;
    }
}

// Check if a position is at least one space away from walls
int is_away_from_walls(Floor* floor, int x, int y) {
    // Check all 8 surrounding tiles
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int check_x = x + dx;
            int check_y = y + dy;
            if (check_x < 0 || check_x >= MAP_WIDTH || check_y < 0 || check_y >= MAP_HEIGHT) {
                return 0;  // Consider out of bounds as wall
            }
            if (floor->map[check_y][check_x] == '#') {
                return 0;  // Found a wall nearby
            }
        }
    }
    return 1;  // No walls found nearby
}

// Place stairs in a room
void place_stairs_in_room(Floor* floor, Room* room) {
    int attempts = 50;  // Maximum attempts to find a valid position
    
    // Place up stairs in the center of the room
    int stairs_x = room->x + room->width / 2;
    int stairs_y = room->y + room->height / 2;
    
    // Ensure up stairs are away from walls
    while (!is_away_from_walls(floor, stairs_x, stairs_y) && attempts > 0) {
        stairs_x = room->x + random_range(2, room->width - 3);
        stairs_y = room->y + random_range(2, room->height - 3);
        attempts--;
    }
    
    if (attempts > 0 &&
        floor->floor_num != 0) 
    {
        floor->map[stairs_y][stairs_x] = '<';
        floor->terrain[stairs_y][stairs_x] = TERRAIN_STAIRS;
        floor->up_stairs_x = stairs_x;
        floor->up_stairs_y = stairs_y;
    }
    
    // Place locked down stairs in the last room
    Room* down_room = &floor->rooms[floor->num_rooms - 1];
    if (floor->num_rooms == 1) {
        down_room = &floor->rooms[0];
    }
    
    attempts = 50;
    int down_x = down_room->x + down_room->width / 2;
    int down_y = down_room->y + down_room->height / 2;
    
    // Ensure down stairs are away from walls
    while (!is_away_from_walls(floor, down_x, down_y) && attempts > 0) {
        down_x = down_room->x + random_range(2, down_room->width - 3);
        down_y = down_room->y + random_range(2, down_room->height - 3);
        attempts--;
    }
    
    if (attempts > 0) {
        floor->map[down_y][down_x] = '%';
        floor->terrain[down_y][down_x] = TERRAIN_LOCKED_DOOR;
        floor->down_stairs_x = down_x;
        floor->down_stairs_y = down_y;
    }
}

// Place a locked door between two rooms
void place_locked_door(Floor* floor, Room* room1, Room* room2, int key_id) {
    int x1 = room1->x + room1->width / 2;
    int y1 = room1->y + room1->height / 2;
    int x2 = room2->x + room2->width / 2;
    int y2 = room2->y + room2->height / 2;
    
    // Find a point along the path between rooms for the door
    int door_x = (x1 + x2) / 2;
    int door_y = (y1 + y2) / 2;
    
    // Adjust door position to be on a wall
    if (floor->map[door_y-1][door_x] == '#' && floor->map[door_y+1][door_x] == '#') {
        // Vertical door
        floor->map[door_y][door_x] = TERRAIN_LOCKED_DOOR;
    } else if (floor->map[door_y][door_x-1] == '#' && floor->map[door_y][door_x+1] == '#') {
        // Horizontal door
        floor->map[door_y][door_x] = TERRAIN_LOCKED_DOOR;
    }
    
    // Add door to floor's door array
    if (floor->num_doors < MAX_DOORS) {
        Door door = {
            .x = door_x,
            .y = door_y,
            .floor_num = current_floor,
            .key_id = key_id,
            .is_locked = 1
        };
        floor->doors[floor->num_doors++] = door;
    }
}

// Place a key in a room
void place_key(Floor* floor, Room* room, int key_id, int target_floor) {
    // Find an empty spot in the room
    int x = random_range(room->x + 2, room->x + room->width - 4);
    int y = random_range(room->y + 2, room->y + room->height - 4);

    // Create the key
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) {
            floor->items[i] = (Item){
                .name = "Ancient Key",
                .description = "A mysterious key that might open something important",
                .x = x,
                .y = y,
                .symbol = 'k',
                .active = 1,
                .type = ITEM_KEY,
                .value = 100,
                .key_id = key_id,
                .target_floor = target_floor
            };
            strcpy(floor->items[i].name, "Ancient Key");
            strcpy(floor->items[i].description, "A mysterious key that might open something important");
            break;
        }
    }
}

// Place a floor key in a room
void place_floor_key(Floor* floor, Room* room) {
    // Find an empty spot in the room
    int x = random_range(room->x + 2, room->x + room->width - 4);
    int y = random_range(room->y + 2, room->y + room->height - 4);

    // Create the floor key
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) {
            floor->items[i] = (Item){
                .name = "Floor Key",
                .description = "A key that unlocks the way forward",
                .x = x,
                .y = y,
                .symbol = 'K',
                .active = 1,
                .type = ITEM_KEY,
                .value = 200,
                .key_id = current_floor + 1,  // Key ID matches next floor
                .target_floor = current_floor  // Used on current floor
            };
            strcpy(floor->items[i].name, "Floor Key");
            strcpy(floor->items[i].description, "A key that unlocks the way forward");
            break;
        }
    }
}

// Create a room in the map based on its type
void create_room_in_map(Floor* floor, Room* room) {
    // Create square room
    for (int y = room->y; y < room->y + room->height; y++) {
        for (int x = room->x; x < room->x + room->width; x++) {
            if (y == room->y || y == room->y + room->height - 1 ||
                x == room->x || x == room->x + room->width - 1) {
                floor->map[y][x] = '#';
                floor->terrain[y][x] = TERRAIN_WALL;
            } else {
                floor->map[y][x] = '.';
                floor->terrain[y][x] = TERRAIN_FLOOR;
            }
        }
    }
}

// Initialize a floor
void init_floor(int floor_num) {
    Floor* floor = &floors[floor_num];
    current_floor = floor_num;
    
    // Generate new floor if not visited before
    if (!floor->has_visited) {
        generate_floor(floor);
        
        // Place up stairs in a random room
        Room* up_room = &floor->rooms[rand() % floor->num_rooms];
        
        // Place down stairs in the last room (or second room if first is only room)
        Room* down_room = &floor->rooms[floor->num_rooms - 1];
        if (floor->num_rooms == 1) {
            down_room = &floor->rooms[0];
        }
        
        // Place stairs
        place_stairs_in_room(floor, up_room);
        
        // Place floor key in a different room than stairs
        Room* key_room;
        do {
            key_room = &floor->rooms[rand() % floor->num_rooms];
        } while (key_room == up_room || key_room == down_room);
        
        // Add floor key to items array
        place_floor_key(floor, key_room);
        
        floor->has_visited = 1;
        floor->has_stairs = 1;
    }
    
    // Spawn enemies for this floor
    spawn_floor_enemies();
}

// Calculate if a point is visible from the player's position
int is_visible(int x, int y) {
    Floor* floor = current_floor_ptr();
    
    // If point is too far, it's not visible
    int dx = x - player.x;
    int dy = y - player.y;
    if (dx * dx + dy * dy > VIEW_RADIUS * VIEW_RADIUS) {
        return 0;
    }
    
    // Use Bresenham's line algorithm to check for walls
    int abs_dx = abs(dx);
    int abs_dy = abs(dy);
    int sx = dx > 0 ? 1 : -1;
    int sy = dy > 0 ? 1 : -1;
    
    int err = abs_dx - abs_dy;
    int current_x = player.x;
    int current_y = player.y;
    
    while (current_x != x || current_y != y) {
        if (floor->map[current_y][current_x] == '#') {
            return 0;  // Hit a wall
        }
        
        int e2 = 2 * err;
        if (e2 > -abs_dy) {
            err -= abs_dy;
            current_x += sx;
        }
        if (e2 < abs_dx) {
            err += abs_dx;
            current_y += sy;
        }
    }
    
    return 1;  // No walls in the way
}

// Update field of view
void update_fov() {
    Floor* floor = current_floor_ptr();
    
    // Reset visibility
    memset(floor->visible, 0, sizeof(floor->visible));
    
    // Check visibility for each point in view radius
    for (int y = max(0, player.y - VIEW_RADIUS); 
         y < min(MAP_HEIGHT, player.y + VIEW_RADIUS + 1); y++) {
        for (int x = max(0, player.x - VIEW_RADIUS);
             x < min(MAP_WIDTH, player.x + VIEW_RADIUS + 1); x++) {
            if (is_visible(x, y)) {
                floor->visible[y][x] = 1;
                floor->discovered[y][x] = 1;
            }
        }
    }
}

// Place a random item in a room
void place_random_item(Floor* floor, Room* room) {
    // Find an empty spot in the room
    int x = random_range(room->x + 2, room->x + room->width - 4);
    int y = random_range(room->y + 2, room->y + room->height - 4);

    // Create a random item
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) {
            // First clear the item struct completely
            memset(&floor->items[i], 0, sizeof(Item));
            
            ItemType type = get_item_type_from_int(random_range(ITEM_WEAPON, ITEM_GOLD));
            
            // Set common properties
            floor->items[i].x = x;
            floor->items[i].y = y;
            floor->items[i].active = 1;
            floor->items[i].type = type;
            
            // Set item properties based on type
            switch(type) {
                case ITEM_WEAPON:
                    strcpy(floor->items[i].name, "Iron Sword");
                    strcpy(floor->items[i].description, "A basic but reliable weapon");
                    floor->items[i].symbol = '/';
                    floor->items[i].power = random_range(3, 7);
                    floor->items[i].value = random_range(50, 150);
                    break;
                    
                case ITEM_ARMOR:
                    strcpy(floor->items[i].name, "Leather Armor");
                    strcpy(floor->items[i].description, "Basic protective gear");
                    floor->items[i].symbol = '[';
                    floor->items[i].power = random_range(2, 5);
                    floor->items[i].value = random_range(40, 120);
                    break;
                    
                case ITEM_POTION:
                    strcpy(floor->items[i].name, "Health Potion");
                    strcpy(floor->items[i].description, "Restores some health");
                    floor->items[i].symbol = '!';
                    floor->items[i].power = random_range(10, 25);
                    floor->items[i].value = random_range(30, 80);
                    break;
                    
                case ITEM_GOLD:
                    strcpy(floor->items[i].name, "Gold");
                    strcpy(floor->items[i].description, "Shiny coins");
                    floor->items[i].symbol = '$';
                    floor->items[i].value = random_range(10, 100);
                    floor->items[i].power = floor->items[i].value;  // For gold, power = value
                    break;
                    
                default:
                    // If somehow we get an invalid type, make it inactive
                    floor->items[i].active = 0;
                    break;
            }
            
            // Only break if we successfully created an item
            if (floor->items[i].active) {
                break;
            }
        }
    }
}



// Add store to a room
void place_store(Floor* floor, Room* room) {
    // Find an empty spot in the room
    int x = random_range(room->x + 2, room->x + room->width - 4);
    int y = random_range(room->y + 2, room->y + room->height - 4);
    
    // Create a store
    for (int i = 0; i < MAX_NPCS; i++) {
        if (!floor->npcs[i].active) {
            floor->npcs[i] = (NPC){
                .x = x,
                .y = y,
                .symbol = 'S',  // Store symbol
                .active = TRUE,
                .type = NPC_STOREKEEPER,
                .store = (Store*)malloc(sizeof(Store))
            };
            
            if (floor->npcs[i].store) {
                // Initialize store with random type
                StoreType store_type = get_store_type_from_int(rand() % 4);  // 0-3 for different store types
                init_store(floor->npcs[i].store, store_type);
            }
            break;
        }
    }
}

// Modify generate_floor to include stores
void generate_floor(Floor* floor) {
    // Clear the floor
    memset(floor, 0, sizeof(Floor));
    
    // Fill with walls
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            floor->map[y][x] = '#';
            floor->visible[y][x] = 0;
            floor->discovered[y][x] = 0;
            floor->terrain[y][x] = TERRAIN_WALL;
        }
    }
    
    // Generate rooms
    int attempts = 0;
    int max_attempts = 100;
    
    while (floor->num_rooms < MAX_ROOMS && attempts < max_attempts) {
        Room new_room = generate_room();
        
        // Check if room overlaps with existing rooms
        int overlaps = 0;
        for (int i = 0; i < floor->num_rooms; i++) {
            if (rooms_overlap(&new_room, &floor->rooms[i], 2)) {  // Increased padding to 2
                overlaps = 1;
                break;
            }
        }
        
        if (!overlaps) {
            // Add room to floor
            floor->rooms[floor->num_rooms++] = new_room;
            
            // Create room in map
            for (int y = new_room.y; y < new_room.y + new_room.height; y++) {
                for (int x = new_room.x; x < new_room.x + new_room.width; x++) {
                    if (y == new_room.y || y == new_room.y + new_room.height - 1 ||
                        x == new_room.x || x == new_room.x + new_room.width - 1) {
                        floor->map[y][x] = '#';
                        floor->terrain[y][x] = TERRAIN_WALL;
                    } else {
                        floor->map[y][x] = '.';
                        floor->terrain[y][x] = TERRAIN_FLOOR;
                    }
                }
            }
        }
        attempts++;
    }
    
    // Create tunnels between rooms
    for (int i = 1; i < floor->num_rooms; i++) {
        Room* current = &floor->rooms[i];
        Room* previous = &floor->rooms[i - 1];
        
        // Create tunnel from edge of previous room to edge of current room
        create_tunnel(floor, 
                     previous->x + previous->width / 2,
                     previous->y + previous->height / 2,
                     current->x + current->width / 2,
                     current->y + current->height / 2);
    }
    
    // Place random items in rooms
    for (int i = 0; i < floor->num_rooms; i++) {
        // 70% chance to place an item in each room
        if (rand() % 100 < 70) {
            place_random_item(floor, &floor->rooms[i]);
        }
    }
    
    // After placing rooms and items, add stores
    for (int i = 0; i < floor->num_rooms; i++) {
        Room* room = &floor->rooms[i];
        
        // 20% chance for each room to have a store
        if (rand() % 5 == 0) {
            place_store(floor, room);
        }
    }
    
    // Set player position in first room if this is floor 0
    if (current_floor == 0) {
        Room* first_room = &floor->rooms[0];
        player.x = first_room->x + first_room->width / 2;
        player.y = first_room->y + first_room->height / 2;
    }
}

// Check for items at player's position
void check_items() {
    Floor* floor = current_floor_ptr();
   
    // Check for regular items
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