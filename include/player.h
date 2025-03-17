#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

// Player management functions
void init_player(void);
void move_player(char input);
void level_up(void);
void handle_inventory(void);
int add_to_inventory(Item item);
void remove_from_inventory(int index);
void use_item(int index);
void drop_item(int index);
void equip_item(int index);

// Ability and status effect functions
void add_ability(AbilityType type);
void use_ability(int index);
void update_abilities(void);
void apply_status_effect(StatusType type, int duration, int power);
void update_status_effects(void);

#endif // PLAYER_H 