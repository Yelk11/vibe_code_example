#ifndef STORE_H
#define STORE_H

#include "common.h"

// Store functions
void init_store(Store* store, StoreType type);
void restock_store(Store* store);
void update_store(Store* store);
int buy_item(Store* store, int index);
int sell_item(int inventory_index);
void display_store(Store* store);

#endif // STORE_H 