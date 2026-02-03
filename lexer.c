#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"

// Minimal buffer helpers to append cleaned text into buffer->data
// Assumes Buffer has fields: char *data; size_t len;
static void buffer_init(struct Buffer *b) {
    if (!b->data) {
        b->len = 0;
        b->data = (char*)malloc(1);
        if (b->data) b->data[0] = '\0';
    }
}
static void buffer_grow(struct Buffer *b, size_t need) {
    size_t new_size = b->len + need + 1; // +1 for null terminator
    char *nd = (char*)realloc(b->data, new_size);
    if (nd) {
        b->data = nd;
    }
}
static void buffer_append_char(struct Buffer *b, char ch) {
    buffer_grow(b, 1);
    if (!b->data) return;
    b->data[b->len++] = ch;
    b->data[b->len] = '\0';
}


/*
for minimum viable product i just write it to a buffer but later 
i can implement more sophisticated handling 
todo: parse it when reading that way we can emit the buffer contents more easily
*/ 
int clean_file_to_buffer(FILE *file, struct Buffer *buffer)
{
    buffer_init(buffer);

    int c;
    int in_line_comment = 0;
    int in_block_comment = 0;
    int in_string = 0;
    int in_char = 0;
    int escape = 0;

    int pending_slash = 0; // we saw '/', not yet emitted

    while ((c = fgetc(file)) != EOF) {

        // If we are in a line comment, ignore until newline
        if (in_line_comment) {
            if (c == '\n') {
                in_line_comment = 0;
                buffer_append_char(buffer, '\n'); // keep newlines to preserve rough structure
            }
            continue;
        }

        // If we are in a block comment, ignore until */
        if (in_block_comment) {
            static int prev = 0;
            if (prev == '*' && c == '/') {
                in_block_comment = 0;
                prev = 0;
            } else {
                prev = c;
            }
            continue;
        }

        // If we are in string literal, ignore until unescaped "
        if (in_string) {
            if (!escape && c == '"') {
                in_string = 0;
            }
            escape = (!escape && c == '\\');
            continue;
        }

        // If we are in char literal, ignore until unescaped '
        if (in_char) {
            if (!escape && c == '\'') {
                in_char = 0;
            }
            escape = (!escape && c == '\\');
            continue;
        }

        // If we had a pending '/', decide now based on current char
        if (pending_slash) {
            if (c == '/') {
                // start line comment
                pending_slash = 0;
                in_line_comment = 1;
                continue;
            }
            if (c == '*') {
                // start block comment
                pending_slash = 0;
                in_block_comment = 1;
                continue;
            }

            // it was a normal '/', emit it now
            buffer_append_char(buffer, '/');
            pending_slash = 0;
            // then fall through to process current c normally
        }

        // Enter literals
        if (c == '"') {
            in_string = 1;
            escape = 0;
            continue;
        }
        if (c == '\'') {
            in_char = 1;
            escape = 0;
            continue;
        }

        // Maybe start of comment: defer emission of '/'
        if (c == '/') {
            pending_slash = 1;
            continue;
        }

        // Normal output
        buffer_append_char(buffer, (char)c);
    }

    // If file ends with a pending '/', emit it
    if (pending_slash) {
        buffer_append_char(buffer, '/');
    }

    // No fflush for memory buffer; ensure null-terminated
    if (buffer->data) buffer->data[buffer->len] = '\0';
    return 0;
}

static int is_ident_char(int ch) {
    return (ch == '_' || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}

int find_struct_block(const char *src, size_t len,
                      const char *target_name,
                      struct StructBlock *out)
{
    if (!src || !target_name || !out) return -1;
    const char *name = target_name;

    // scan for either "struct" or "typedef struct"
    for (size_t i = 0; i + 6 < len; ++i) {
        // skip whitespace
        if (src[i] == ' ' || src[i] == '\n' || src[i] == '\t' || src[i] == '\r') continue;

        int is_typedef = 0;
        size_t pos = i;
        // try match "typedef struct"
        if (pos + 7 < len && strncmp(src + pos, "typedef", 7) == 0) {
            pos += 7;
            while (pos < len && (src[pos] == ' ' || src[pos] == '\n' || src[pos] == '\t')) pos++;
            if (pos + 6 < len && strncmp(src + pos, "struct", 6) == 0) {
                is_typedef = 1;
            } else {
                continue;
            }
        }
        // try match "struct"
        if (pos + 6 < len && strncmp(src + pos, "struct", 6) == 0) {
            size_t start = pos;
            pos += 6;
            // skip whitespace
            while (pos < len && (src[pos] == ' ' || src[pos] == '\n' || src[pos] == '\t')) pos++;
            // optional tag name must match target_name
            size_t tag_begin = pos;
            while (pos < len && is_ident_char((unsigned char)src[pos])) pos++;
            size_t tag_end = pos;
            if (tag_end > tag_begin) {
                size_t tag_len = tag_end - tag_begin;
                if (strlen(name) == tag_len && strncmp(src + tag_begin, name, tag_len) == 0) {
                    // ok
                } else {
                    // not our target
                    continue;
                }
            } else {
                // anonymous struct; skip
                continue;
            }
            // skip whitespace to '{'
            while (pos < len && (src[pos] == ' ' || src[pos] == '\n' || src[pos] == '\t')) pos++;
            if (pos >= len || src[pos] != '{') {
                continue;
            }
            size_t lbrace = pos;

            // find matching '}' with simple brace counter
            size_t rbrace = lbrace;
            int depth = 0;
            for (; rbrace < len; ++rbrace) {
                if (src[rbrace] == '{') depth++;
                else if (src[rbrace] == '}') {
                    depth--;
                    if (depth == 0) break;
                }
            }
            if (rbrace >= len || depth != 0) {
                continue; // unmatched
            }

            // find the ending ';' after rbrace (typdef may have alias)
            size_t end = rbrace + 1;
            while (end < len && (src[end] == ' ' || src[end] == '\n' || src[end] == '\t')) end++;
            if (is_typedef) {
                while (end < len && src[end] != ';') end++;
                if (end < len) end++; // include ';'
            }

            out->start = start;
            out->lbrace = lbrace;
            out->rbrace = rbrace;
            out->end = end;
            out->is_typedef = is_typedef;
            return 0;
        }
    }
    return -1; // not found
}