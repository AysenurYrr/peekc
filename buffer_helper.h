
#ifndef PEEKC_BUFFER_HELPER_H
#define PEEKC_BUFFER_HELPER_H

#include <stdlib.h>

// Dynamic Buffer with minimal growth strategy
struct Buffer {
    char *data;
    size_t len;
    size_t capacity;
};

int buffer_init(struct Buffer *b);

int buffer_append_char(struct Buffer *b, char ch);

#endif