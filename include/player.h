#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "common.h"
#include "item.h"

// Functions
void init_player(void);
void move_player(int dx, int dy);
int add_to_inventory(Item item);  // Returns 1 on success, 0 on failure
void remove_from_inventory(int index);
void use_item(Item* item);
void equip_weapon(Item weapon);
void equip_armor(Item armor);
void view_inventory(void);
void level_up(void);
void apply_status_effect(StatusType type, int duration, int power);
void handle_inventory(void);
void drop_item(int index);
void equip_item(int index);
void add_ability(AbilityType type);
void use_ability(int index);
void update_abilities(void);
void update_status_effects(void);
void check_player_items(void);

// Global player state
extern Player player;

#endif 