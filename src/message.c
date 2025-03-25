#include "../include/message.h"
#include "../include/globals.h"
#include <stdarg.h>
#include <string.h>

void add_message(const char* format, ...) {
    if (message_log.num_messages >= MAX_MESSAGES) {
        // Shift messages down
        for (int i = MAX_MESSAGES - 1; i > 0; i--) {
            strcpy(message_log.messages[i], message_log.messages[i - 1]);
        }
        message_log.num_messages = MAX_MESSAGES;
    }
    
    va_list args;
    va_start(args, format);
    vsnprintf(message_log.messages[0], MESSAGE_LENGTH, format, args);
    va_end(args);
    
    if (message_log.num_messages < MAX_MESSAGES) {
        message_log.num_messages++;
    }
}

void clear_messages(void) {
    message_log.num_messages = 0;
} 

MessageLog init_message_log(void) {
    MessageLog log;
    log.num_messages = 0;
    return log;
}
