#include <stdlib.h>
#include "buffer_helper.h"


// Minimal buffer helpers to append cleaned text into buffer->data
// Assumes Buffer has fields: char *data; size_t len; size_t capacity

static int growth_size = 128;

int buffer_init(struct Buffer *b) 
{
    if (NULL == b)
    {
        return -1;
    }

    // idempotent init 
    if (NULL != b->data)
    {
        // already initialized, free it
        free(b->data);
        b->data = NULL;
    }

    // Capacity will grow 128 bytes at a time
    b->len = 0;
    b->capacity = growth_size;
    b->data = (char*)malloc(b->capacity);
    b->data[0] = '\0';
    return 0;

}

// simple growth strategy: increase capacity by growth_size
static int buffer_grow(struct Buffer *b) 
{
    if (NULL == b || NULL == b->data) 
    {
        return -1;
    }

    size_t new_size = b->capacity + growth_size;
    char *new_data = (char*)realloc(b->data, new_size);
    if (NULL != new_data) 
    {
        b->data = new_data;
        b->capacity = new_size;
        return 0;
    }
    return -1;
}

int buffer_append_char(struct Buffer *b, char ch) 
{
    // appending one char will make it grow one byte 
    if(b->len + 1 >= b->capacity)
    {
        if (buffer_grow(b) == -1)
        {
            return -1;
        }
    }
    b->data[b->len++] = ch;
    b->data[b->len] = '\0';
    return 0;
}

