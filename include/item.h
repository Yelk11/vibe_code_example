#ifndef ITEM_H
#define ITEM_H

#include "common.h"

// Item management functions
void init_items(void);
Item generate_item(int floor_level);
void check_items(void);
void apply_item_effect(Item* item);
void place_items(Floor* floor);

#endif // ITEM_H 