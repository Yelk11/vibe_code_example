#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include "common.h"

// Map generation and management functions
void init_floor(Floor* floor);
void create_tunnel(Floor* floor, int x1, int y1, int x2, int y2);
void create_straight_tunnel(Floor* floor, int x1, int y1, int x2, int y2);
Room generate_room(void);
int rooms_overlap(Room* r1, Room* r2, int padding);
void place_stairs_in_room(Room* room, int* stair_x, int* stair_y);
void generate_terrain(Floor* floor);
Floor* current_floor_ptr(void);
void update_fov(void);
int is_visible(int x, int y);
void update_discovered_map(void);

#endif // MAP_H 