#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get character input without waiting for Enter
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return buf;
}

// Random number between min and max (inclusive)
int random_range(int min, int max) {
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
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

// Add message to the message log
void add_message(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // Shift messages up
    for (int i = 0; i < MAX_MESSAGES - 1; i++) {
        strcpy(messages[i], messages[i + 1]);
    }
    
    // Add new message at bottom
    vsnprintf(messages[MAX_MESSAGES - 1], MESSAGE_LENGTH, fmt, args);
    
    va_end(args);
} 