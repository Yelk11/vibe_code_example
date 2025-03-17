#include "map.h"
#include "player.h"

// Get current floor
Floor* current_floor_ptr() {
    return &floors[current_floor];
}

// Generate a random room
Room generate_room() {
    Room room;
    room.width = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    room.height = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    room.x = random_range(1, MAP_WIDTH - room.width - 1);
    room.y = random_range(1, MAP_HEIGHT - room.height - 1);
    return room;
}

// Check if two rooms overlap
int rooms_overlap(Room* r1, Room* r2, int padding) {
    return !(r1->x + r1->width + padding < r2->x ||
             r2->x + r2->width + padding < r1->x ||
             r1->y + r1->height + padding < r2->y ||
             r2->y + r2->height + padding < r1->y);
}

// Create a tunnel between two points
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2) {
    int current_x = x1;
    int current_y = y1;
    
    // Randomly choose whether to go horizontal or vertical first
    if (rand() % 2 == 0) {
        // Horizontal then vertical
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
            current_x += (x2 > x1) ? 1 : -1;
        }
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            current_y += (y2 > y1) ? 1 : -1;
        }
    } else {
        // Vertical then horizontal
        while (current_y != y2) {
            floor->map[current_y][current_x] = '.';
            current_y += (y2 > y1) ? 1 : -1;
        }
        while (current_x != x2) {
            floor->map[current_y][current_x] = '.';
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

// Initialize a single floor
void init_floor(Floor* floor) {
    // Fill map with walls and initialize visibility arrays
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            floor->map[y][x] = '#';
            floor->visible[y][x] = 0;
            floor->discovered[y][x] = 0;
        }
    }
    
    floor->has_floor_key = 0;  // Initialize floor key status
    
    // Generate rooms
    floor->num_rooms = 0;
    int max_attempts = 100;
    int attempts = 0;
    
    while (floor->num_rooms < MAX_ROOMS && attempts < max_attempts) {
        Room new_room = generate_room();
        int valid = 1;
        
        // Check if room overlaps with existing rooms
        for (int i = 0; i < floor->num_rooms; i++) {
            if (rooms_overlap(&new_room, &floor->rooms[i], 2)) {
                valid = 0;
                break;
            }
        }
        
        if (valid) {
            // Add room to map
            for (int y = new_room.y; y < new_room.y + new_room.height; y++) {
                for (int x = new_room.x; x < new_room.x + new_room.width; x++) {
                    floor->map[y][x] = '.';
                }
            }
            
            // Connect to previous room
            if (floor->num_rooms > 0) {
                int prev_center_x = floor->rooms[floor->num_rooms-1].x + floor->rooms[floor->num_rooms-1].width/2;
                int prev_center_y = floor->rooms[floor->num_rooms-1].y + floor->rooms[floor->num_rooms-1].height/2;
                int new_center_x = new_room.x + new_room.width/2;
                int new_center_y = new_room.y + new_room.height/2;
                
                create_tunnel(floor, prev_center_x, prev_center_y, new_center_x, new_center_y);
            }
            
            floor->rooms[floor->num_rooms++] = new_room;
        }
        attempts++;
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