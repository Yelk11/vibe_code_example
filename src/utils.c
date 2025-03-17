#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include "common.h"

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

// Add message to the message log
void add_message(const char* fmt, ...) {
    // Shift older messages up
    for (int i = MAX_MESSAGES - 1; i > 0; i--) {
        strncpy(message_log.messages[i], message_log.messages[i-1], MESSAGE_LENGTH - 1);
        message_log.messages[i][MESSAGE_LENGTH - 1] = '\0';
    }
    
    // Format and add new message
    va_list args;
    va_start(args, fmt);
    vsnprintf(message_log.messages[0], MESSAGE_LENGTH - 1, fmt, args);
    va_end(args);
    
    // Ensure null termination
    message_log.messages[0][MESSAGE_LENGTH - 1] = '\0';
    
    // Update message count
    if (message_log.num_messages < MAX_MESSAGES) {
        message_log.num_messages++;
    }
} 