#include "message.h"
#include "globals.h"
#include <stdarg.h>
#include <string.h>

void add_message(const char* format, ...) {
    if (message_log.num_messages >= MAX_MESSAGES) {
        // Shift messages up
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            strcpy(message_log.messages[i], message_log.messages[i + 1]);
        }
        message_log.num_messages--;
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(message_log.messages[message_log.num_messages], MESSAGE_LENGTH, format, args);
    va_end(args);
    
    message_log.num_messages++;
}

void clear_messages(void) {
    message_log.num_messages = 0;
} 