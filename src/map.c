#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "map.h"
#include "globals.h"
#include "enemy.h"
#include "item.h"
#include "player.h"

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

// Place stairs in a random room
void place_stairs_in_room(Room* room, int* stair_x, int* stair_y) {
    *stair_x = random_range(room->x + 1, room->x + room->width - 2);
    *stair_y = random_range(room->y + 1, room->y + room->height - 2);
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
    int x = random_range(room->x + 1, room->x + room->width - 2);
    int y = random_range(room->y + 1, room->y + room->height - 2);
    
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
    int x = random_range(room->x + 1, room->x + room->width - 2);
    int y = random_range(room->y + 1, room->y + room->height - 2);
    
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
        floor->has_visited = 1;
    }
    
    // Place stairs if not already placed
    if (!floor->has_stairs) {
        // Place up stairs in a random room
        Room* up_room = &floor->rooms[rand() % floor->num_rooms];
        int up_x, up_y;
        int valid_spot = 0;
        
        // Try to find a valid spot for up stairs
        for (int attempts = 0; attempts < 50 && !valid_spot; attempts++) {
            up_x = up_room->x + 1 + rand() % (up_room->width - 2);
            up_y = up_room->y + 1 + rand() % (up_room->height - 2);
            
            // Check if the spot is not blocking a hallway or entrance
            if (floor->map[up_y-1][up_x] != '.' && floor->map[up_y+1][up_x] != '.' &&
                floor->map[up_y][up_x-1] != '.' && floor->map[up_y][up_x+1] != '.') {
                valid_spot = 1;
            }
        }
        
        if (valid_spot) {
            floor->up_stairs_x = up_x;
            floor->up_stairs_y = up_y;
            floor->map[floor->up_stairs_y][floor->up_stairs_x] = '<';
        }
        
        // Place down stairs in the last room (or second room if first is only room)
        Room* down_room = &floor->rooms[floor->num_rooms - 1];
        if (floor->num_rooms == 1) {
            down_room = &floor->rooms[0];
        }
        
        int down_x, down_y;
        valid_spot = 0;
        
        // Try to find a valid spot for down stairs
        for (int attempts = 0; attempts < 50 && !valid_spot; attempts++) {
            down_x = down_room->x + 1 + rand() % (down_room->width - 2);
            down_y = down_room->y + 1 + rand() % (down_room->height - 2);
            
            // Check if the spot is not blocking a hallway or entrance
            if (floor->map[down_y-1][down_x] != '.' && floor->map[down_y+1][down_x] != '.' &&
                floor->map[down_y][down_x-1] != '.' && floor->map[down_y][down_x+1] != '.') {
                valid_spot = 1;
            }
        }
        
        if (valid_spot) {
            floor->down_stairs_x = down_x;
            floor->down_stairs_y = down_y;
            // Place locked stairs initially
            floor->map[floor->down_stairs_y][floor->down_stairs_x] = '%';
            floor->terrain[floor->down_stairs_y][floor->down_stairs_x] = TERRAIN_LOCKED_DOOR;
        }
        
        // Place floor key in a different room than stairs
        Room* key_room;
        do {
            key_room = &floor->rooms[rand() % floor->num_rooms];
        } while (key_room == up_room || key_room == down_room);
        
        // Add floor key to items array
        for (int i = 0; i < MAX_ITEMS; i++) {
            if (!floor->items[i].active) {
                floor->items[i] = (Item){
                    .name = "Floor Key",
                    .description = "A key that unlocks the way forward",
                    .x = key_room->x + 1 + rand() % (key_room->width - 2),
                    .y = key_room->y + 1 + rand() % (key_room->height - 2),
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
        
        floor->has_stairs = 1;
    }
    
    // Place stairs in the last room (or first room if it's the only room)
    Room* stairs_room = &floor->rooms[floor->num_rooms - 1];
    if (floor->num_rooms == 1) stairs_room = &floor->rooms[0];
    
    // Place up stairs in the center of the room
    int stairs_x = stairs_room->x + stairs_room->width / 2;
    int stairs_y = stairs_room->y + stairs_room->height / 2;
    floor->map[stairs_y][stairs_x] = '<';
    floor->terrain[stairs_y][stairs_x] = TERRAIN_STAIRS;
    
    // Place locked down stairs in a random position in the room
    int attempts = 0;
    while (attempts < 50) {
        int x = stairs_room->x + rand() % stairs_room->width;
        int y = stairs_room->y + rand() % stairs_room->height;
        
        // Don't place stairs on walls or on top of up stairs
        if (floor->map[y][x] != '#' && floor->map[y][x] != '<') {
            floor->map[y][x] = '%';
            floor->terrain[y][x] = TERRAIN_LOCKED_DOOR;
            break;
        }
        attempts++;
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
    int x = random_range(room->x + 1, room->x + room->width - 2);
    int y = random_range(room->y + 1, room->y + room->height - 2);
    
    // Create a random item
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!floor->items[i].active) {
            // First clear the item struct completely
            memset(&floor->items[i], 0, sizeof(Item));
            
            ItemType type = random_range(ITEM_WEAPON, ITEM_GOLD);
            
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

// Generate a new floor
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
    
    // Set player position in first room if this is floor 0
    if (current_floor == 0) {
        Room* first_room = &floor->rooms[0];
        player.x = first_room->x + first_room->width / 2;
        player.y = first_room->y + first_room->height / 2;
    }
} 