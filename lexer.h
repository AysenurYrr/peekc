
#ifndef PEEKC_LEXER_H
#define PEEKC_LEXER_H

#include <stdio.h>
#include <stdlib.h>

struct Buffer {
    char *data;
    size_t len;
};

struct StructBlock {
    size_t start;   // index of 's' in "struct" (or "typedef")
    size_t lbrace;  // index of '{'
    size_t rbrace;  // matching '}'
    size_t end;     // index after ';' (if you want)
    int is_typedef;
};


int clean_file_to_buffer(FILE *file, struct Buffer *buffer);

int is_struct_name_in_file(const char *filename, const char *struct_name);

int find_struct_block(const char *src, size_t len,
                      const char *target_name,
                      struct StructBlock *out);

#endif