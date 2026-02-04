
#ifndef PEEKC_LEXER_H
#define PEEKC_LEXER_H

#include <stdio.h>
#include <stdlib.h>

//forward declare
struct Buffer;

typedef enum {
    TK_EOF=0,
    TK_IDENT,
    TK_NUMBER,

    TK_KW_STRUCT,
    TK_KW_UNION,
    TK_KW_ENUM,
    TK_KW_TYPEDEF,
    TK_KW_CONST,
    TK_KW_VOLATILE,
    TK_KW_UNSIGNED,
    TK_KW_SIGNED,
    TK_KW_LONG,
    TK_KW_SHORT,

    TK_LBRACE, TK_RBRACE,
    TK_LBRACK, TK_RBRACK,
    TK_LPAREN, TK_RPAREN,
    TK_SEMI, TK_STAR, TK_COMMA, TK_COLON
} TokenKind;


typedef struct {
    TokenKind kind;
    size_t a, b; // [a,b) span in src
} Token;

typedef struct {
    const char *src;
    size_t len;
    size_t pos;
} Lexer;

enum TypeKind {
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_ARRAY,
    TYPE_POINTER
};

typedef struct TypeRef {
    char *type_str;          // "struct Species", "const char", "enum AnimalType", "uint32_t"
    unsigned pointer_level;  // 0,1,2...
    int is_array;
    char *array_expr;        // "50" veya NULL
    int is_bitfield;
    unsigned bit_width;
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