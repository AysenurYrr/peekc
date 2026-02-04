#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "buffer_helper.h"


static int is_space_c(int c){ return c==' '||c=='\n'||c=='\t'||c=='\r'||c=='\f'||c=='\v'; }
static int is_ident_start_c(int c){ return c=='_'||(c>='a'&&c<='z')||(c>='A'&&c<='Z'); }
static int is_ident_char_c(int c){ return is_ident_start_c(c)||(c>='0'&&c<='9'); }
static int is_digit_c(int c){ return c>='0'&&c<='9'; }

static TokenKind kw_kind(const char *s, size_t n) {
    // compare with fixed keywords
    #define KW(x, k) if (n==sizeof(x)-1 && memcmp(s, x, sizeof(x)-1)==0) return k
    KW("struct",   TK_KW_STRUCT);
    KW("union",    TK_KW_UNION);
    KW("enum",     TK_KW_ENUM);
    KW("typedef",  TK_KW_TYPEDEF);
    KW("const",    TK_KW_CONST);
    KW("volatile", TK_KW_VOLATILE);
    KW("unsigned", TK_KW_UNSIGNED);
    KW("signed",   TK_KW_SIGNED);
    KW("long",     TK_KW_LONG);
    KW("short",    TK_KW_SHORT);
    #undef KW
    return TK_IDENT;
}

/*
for minimum viable product i just write it to a buffer but later 
i can implement more sophisticated handling 
TODO: parse it when reading that way we can emit the buffer contents more easily
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


static void lex_init(Lexer *lx, const char *src, size_t len) {
    lx->src = src; lx->len = len; lx->pos = 0;
}

static Token lex_next(Lexer *lx) {
    const char *s = lx->src;
    size_t n = lx->len;
    size_t p = lx->pos;

    while (p < n && is_space_c((unsigned char)s[p])) p++;
    if (p >= n) { lx->pos = p; return (Token){TK_EOF, p, p}; }

    char c = s[p];

    // punctuation
    switch (c) {
        case '{': lx->pos=p+1; return (Token){TK_LBRACE,p,p+1};
        case '}': lx->pos=p+1; return (Token){TK_RBRACE,p,p+1};
        case '[': lx->pos=p+1; return (Token){TK_LBRACK,p,p+1};
        case ']': lx->pos=p+1; return (Token){TK_RBRACK,p,p+1};
        case '(': lx->pos=p+1; return (Token){TK_LPAREN,p,p+1};
        case ')': lx->pos=p+1; return (Token){TK_RPAREN,p,p+1};
        case ';': lx->pos=p+1; return (Token){TK_SEMI,p,p+1};
        case '*': lx->pos=p+1; return (Token){TK_STAR,p,p+1};
        case ',': lx->pos=p+1; return (Token){TK_COMMA,p,p+1};
        case ':': lx->pos=p+1; return (Token){TK_COLON,p,p+1};
        default: break;
    }

    // number
    if (is_digit_c((unsigned char)c)) {
        size_t a=p;
        while (p<n && is_digit_c((unsigned char)s[p])) p++;
        lx->pos=p;
        return (Token){TK_NUMBER,a,p};
    }

    // identifier / keyword
    if (is_ident_start_c((unsigned char)c)) {
        size_t a=p;
        while (p<n && is_ident_char_c((unsigned char)s[p])) p++;
        lx->pos=p;
        TokenKind k = kw_kind(s+a, p-a);
        return (Token){k,a,p};
    }

    // unknown char: skip it (MVP)
    lx->pos = p+1;
    return lex_next(lx);
}

static const char* tok_text(const Lexer *lx, Token t, size_t *out_len) {
    if (out_len) *out_len = t.b - t.a;
    return lx->src + t.a;
}