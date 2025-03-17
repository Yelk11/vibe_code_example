#ifndef ITEM_H
#define ITEM_H

#include "common.h"

// Item creation and initialization
ItemType get_random_item_type(void);
Item create_random_item(int floor_level);
void init_items(Floor* floor);

// Item management functions
void apply_item_effect(Item* item);
void place_items(Floor* floor);

#endif // ITEM_H 