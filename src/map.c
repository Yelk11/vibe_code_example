#include "map.h"
#include "player.h"

// Get current floor
Floor* current_floor_ptr() {
    return &floors[current_floor];
}

// Generate a random room with different types
Room generate_room() {
    Room room;
    RoomType type = random_range(0, 4);  // 5 different room types
    
    // Initialize common room properties
    room.type = type;
    
    switch (type) {
        case ROOM_CIRCULAR:
            // Circular room
            room.width = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            room.height = room.width;  // Make it square for circular shape
            room.x = random_range(1, MAP_WIDTH - room.width - 1);
            room.y = random_range(1, MAP_HEIGHT - room.height - 1);
            break;
            
        case ROOM_CROSS:
            // Cross-shaped room
            room.width = random_range(MIN_ROOM_SIZE + 4, MAX_ROOM_SIZE + 4);
            room.height = room.width;
            room.x = random_range(1, MAP_WIDTH - room.width - 1);
            room.y = random_range(1, MAP_HEIGHT - room.height - 1);
            break;
            
        case ROOM_LARGE:
            // Large rectangular room
            room.width = random_range(MAX_ROOM_SIZE, MAX_ROOM_SIZE + 4);
            room.height = random_range(MAX_ROOM_SIZE, MAX_ROOM_SIZE + 4);
            room.x = random_range(1, MAP_WIDTH - room.width - 1);
            room.y = random_range(1, MAP_HEIGHT - room.height - 1);
            break;
            
        case ROOM_SMALL:
            // Small room
            room.width = random_range(MIN_ROOM_SIZE, MIN_ROOM_SIZE + 2);
            room.height = random_range(MIN_ROOM_SIZE, MIN_ROOM_SIZE + 2);
            room.x = random_range(1, MAP_WIDTH - room.width - 1);
            room.y = random_range(1, MAP_HEIGHT - room.height - 1);
            break;
            
        default:
            // Normal rectangular room
            room.width = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            room.height = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
            room.x = random_range(1, MAP_WIDTH - room.width - 1);
            room.y = random_range(1, MAP_HEIGHT - room.height - 1);
    }
    
    return room;
}

// Check if two rooms overlap
int rooms_overlap(Room* r1, Room* r2, int padding) {
    return !(r1->x + r1->width + padding < r2->x ||
             r2->x + r2->width + padding < r1->x ||
             r1->y + r1->height + padding < r2->y ||
             r2->y + r2->height + padding < r1->y);
}

// Create a tunnel between two points with improved pathing
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    int current_x = x1;
    int current_y = y1;
    
    // Add some randomness to the path
    int num_turns = random_range(1, 3);  // 1-3 turns in the path
    int* turn_points_x = malloc(num_turns * sizeof(int));
    int* turn_points_y = malloc(num_turns * sizeof(int));
    
    // Generate random turn points
    for (int i = 0; i < num_turns; i++) {
        turn_points_x[i] = random_range(min(x1, x2), max(x1, x2));
        turn_points_y[i] = random_range(min(y1, y2), max(y1, y2));
    }
    
    // Create path through turn points
    int prev_x = x1;
    int prev_y = y1;
    
    for (int i = 0; i < num_turns; i++) {
        // Create path to turn point
        create_straight_tunnel(floor, prev_x, prev_y, turn_points_x[i], turn_points_y[i]);
        prev_x = turn_points_x[i];
        prev_y = turn_points_y[i];
    }
    
    // Create final path to destination
    create_straight_tunnel(floor, prev_x, prev_y, x2, y2);
    
    free(turn_points_x);
    free(turn_points_y);
}

// Create a straight tunnel between two points
void create_straight_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    int current_x = x1;
    int current_y = y1;
    
    // Randomly choose whether to go horizontal or vertical first
    if (rand() % 2 == 0) {
        // Horizontal then vertical
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
            floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
            current_x += (x2 > x1) ? 1 : -1;
        }
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
            current_y += (y2 > y1) ? 1 : -1;
        }
    } else {
        // Vertical then horizontal
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
            current_y += (y2 > y1) ? 1 : -1;
        }
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
            floor->terrain[current_y][current_x] = TERRAIN_FLOOR;
            current_x += (x2 > x1) ? 1 : -1;
        }
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
    switch (room->type) {
        case ROOM_CIRCULAR:
            // Create circular room
            for (int y = room->y; y < room->y + room->height; y++) {
                for (int x = room->x; x < room->x + room->width; x++) {
                    // Calculate distance from center
                    int center_x = room->x + room->width / 2;
                    int center_y = room->y + room->height / 2;
                    int dx = x - center_x;
                    int dy = y - center_y;
                    float dist = sqrt(dx * dx + dy * dy);
                    
                    // Only place floor if within radius
                    if (dist <= room->width / 2) {
                        floor->map[y][x] = '.';
                        floor->terrain[y][x] = TERRAIN_FLOOR;
                    }
                }
            }
            break;
            
        case ROOM_CROSS:
            // Create cross-shaped room
            for (int y = room->y; y < room->y + room->height; y++) {
                for (int x = room->x; x < room->x + room->width; x++) {
                    // Check if point is in the cross shape
                    int center_x = room->x + room->width / 2;
                    int center_y = room->y + room->height / 2;
                    int dx = abs(x - center_x);
                    int dy = abs(y - center_y);
                    
                    // Place floor in cross pattern
                    if (dx < room->width / 4 || dy < room->height / 4) {
                        floor->map[y][x] = '.';
                        floor->terrain[y][x] = TERRAIN_FLOOR;
                    }
                }
            }
            break;
            
        case ROOM_LARGE:
        case ROOM_SMALL:
        case ROOM_NORMAL:
        default:
            // Create normal rectangular room
            for (int y = room->y; y < room->y + room->height; y++) {
                for (int x = room->x; x < room->x + room->width; x++) {
                    floor->map[y][x] = '.';
                    floor->terrain[y][x] = TERRAIN_FLOOR;
                }
            }
    }
}

// Initialize a floor
void init_floor(Floor* floor) {
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
    
    // If this floor has been visited before, restore its state
    if (floor->has_visited) {
        // Restore the map, items, and doors
        // Note: We don't restore enemies as they should respawn
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (floor->discovered[y][x] == '.' || floor->discovered[y][x] == '+' || 
                    floor->discovered[y][x] == '<' || floor->discovered[y][x] == '>' ||
                    floor->discovered[y][x] == TERRAIN_LOCKED_DOOR || 
                    floor->discovered[y][x] == TERRAIN_LOCKED_STAIRS) {
                    floor->map[y][x] = floor->discovered[y][x];
                    if (floor->map[y][x] == '.') {
                        floor->terrain[y][x] = TERRAIN_FLOOR;
                    }
                }
            }
        }
        
        // Mark stairs as unlocked for previously visited floors
        if (floor->down_stairs_x > 0 && floor->down_stairs_y > 0) {
            floor->map[floor->down_stairs_y][floor->down_stairs_x] = '>';
        }
        if (floor->up_stairs_x > 0 && floor->up_stairs_y > 0) {
            floor->map[floor->up_stairs_y][floor->up_stairs_x] = '<';
        }
        
        // Restore doors
        for (int i = 0; i < floor->num_doors; i++) {
            Door* door = &floor->doors[i];
            if (!door->is_locked) {
                floor->map[door->y][door->x] = '+';
            }
        }
        
        return;  // Skip generating new rooms for visited floors
    }
    
    // Generate rooms for new floors
    int attempts = 0;
    int max_attempts = 100;
    
    while (floor->num_rooms < MAX_ROOMS && attempts < max_attempts) {
        Room new_room = generate_room();
        
        // Check if room overlaps with existing rooms
        int overlaps = 0;
        for (int i = 0; i < floor->num_rooms; i++) {
            if (rooms_overlap(&new_room, &floor->rooms[i], 1)) {
                overlaps = 1;
                break;
            }
        }
        
        if (!overlaps) {
            // Add room to floor
            floor->rooms[floor->num_rooms++] = new_room;
            
            // Create room in map based on its type
            create_room_in_map(floor, &new_room);
        }
        attempts++;
    }
    
    // Create tunnels between rooms
    for (int i = 1; i < floor->num_rooms; i++) {
        Room* current = &floor->rooms[i];
        Room* previous = &floor->rooms[i - 1];
        
        // Create tunnel from center of previous room to center of current room
        int prev_x = previous->x + previous->width / 2;
        int prev_y = previous->y + previous->height / 2;
        int curr_x = current->x + current->width / 2;
        int curr_y = current->y + current->height / 2;
        
        create_tunnel(floor, prev_x, prev_y, curr_x, curr_y);
    }
    
    floor->num_doors = 0;  // Initialize door count
    
    // Set player position in first room if this is floor 0
    if (current_floor == 0) {
        Room* first_room = &floor->rooms[0];
        player.x = first_room->x + first_room->width / 2;
        player.y = first_room->y + first_room->height / 2;
    }
    
    // After generating basic rooms and connections
    if (current_floor > 0) {  // Don't place locked doors on first floor
        // Place 1-2 locked doors per floor
        int num_locked_doors = random_range(1, 2);
        for (int i = 0; i < num_locked_doors && floor->num_rooms > 2; i++) {
            // Pick two random rooms that aren't the first room
            int room1_idx = random_range(1, floor->num_rooms - 1);
            int room2_idx = random_range(1, floor->num_rooms - 1);
            if (room1_idx == room2_idx) continue;
            
            int key_id = current_floor * 100 + i;  // Generate unique key ID
            place_locked_door(floor, &floor->rooms[room1_idx], &floor->rooms[room2_idx], key_id);
            
            // Place key in a room before the door
            int key_room_idx = random_range(0, room1_idx);
            place_key(floor, &floor->rooms[key_room_idx], key_id, current_floor);
        }
    }
    
    // Place stairs
    if (current_floor > 0) {
        int up_room_idx = random_range(0, floor->num_rooms - 1);
        Room* up_room = &floor->rooms[up_room_idx];
        place_stairs_in_room(up_room, &floor->up_stairs_x, &floor->up_stairs_y);
        floor->map[floor->up_stairs_y][floor->up_stairs_x] = '<';
    }
    
    if (current_floor < MAX_FLOORS - 1) {
        int down_room_idx = floor->num_rooms - 1;
        if (current_floor > 0 && down_room_idx == 0) {
            down_room_idx = 1;
        }
        Room* down_room = &floor->rooms[down_room_idx];
        place_stairs_in_room(down_room, &floor->down_stairs_x, &floor->down_stairs_y);
        // Place locked stairs instead of normal stairs
        floor->map[floor->down_stairs_y][floor->down_stairs_x] = TERRAIN_LOCKED_STAIRS;
        
        // Place floor key in a room that's not the stairs room
        int key_room_idx;
        do {
            key_room_idx = random_range(0, floor->num_rooms - 1);
        } while (key_room_idx == down_room_idx);
        
        place_floor_key(floor, &floor->rooms[key_room_idx]);
    }
    
    // Mark this floor as visited
    floor->has_visited = 1;
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