
#ifndef PEEKC_PARSER_H
#define PEEKC_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

enum TypeKind {
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_ARRAY,
    TYPE_POINTER
};

typedef struct TypeRef {
    enum TypeKind kind;
    const char *name;      // "Animal", 
    struct TypeRef *elem;  // for ptr/array
    size_t array_len;      // for array
} TypeRef;

typedef struct Field {
    char *name;      // field name
    TypeRef *type;   // type tree
} Field;

struct Type {
    enum TypeKind kind;
    char *name;
    struct Field *fields;
    size_t field_count;
};

struct TypeDB {
    struct Type *types;
    size_t type_count;
};

void parse_fields_into_typedb(const char *src, size_t len,
                               const struct StructBlock *blk,
                               struct TypeDB *db);
                              
#endif // PEEKC_LEXER_H