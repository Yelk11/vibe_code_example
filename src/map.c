#include "map.h"

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

// Initialize a single floor
void init_floor(Floor* floor) {
    // Fill map with walls
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            floor->map[y][x] = '#';
        }
    }
    
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
        floor->map[floor->down_stairs_y][floor->down_stairs_x] = '>';
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