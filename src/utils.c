#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Random number generator between min and max (inclusive)
int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Get status effect name
const char* get_status_name(StatusType type) {
    switch (type) {
        case STATUS_NONE:
            return "None";
        case STATUS_POISON:
            return "Poisoned";
        case STATUS_BURN:
            return "Burning";
        case STATUS_FREEZE:
            return "Frozen";
        case STATUS_STUN:
            return "Stunned";
        case STATUS_BLIND:
            return "Blinded";
        case STATUS_BERSERK:
            return "Berserk";
        default:
            return "Unknown";
    }
} 