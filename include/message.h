#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"

// Message functions
void add_message(const char* format, ...);
void clear_messages(void);
MessageLog init_message_log(void);

#endif // MESSAGE_H 