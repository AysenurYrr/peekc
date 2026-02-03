#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"


// helpers
static int is_ident_char(int ch) {
    return (ch == '_' || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'));
}
static int is_space(int ch) {
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\f' || ch == '\v';
}

void parse_fields_into_typedb(const char *src, size_t len,
                               const struct StructBlock *blk,
                               struct TypeDB *db)
{
    if (!src || !blk || !db) return;

    // Extract the struct name from "struct <name> {"
    size_t pos = blk->start;
    // advance past "struct"
    if (pos + 6 <= len && strncmp(src + pos, "struct", 6) == 0) pos += 6; else return;
    while (pos < len && is_space((unsigned char)src[pos])) pos++;
    size_t name_begin = pos;
    while (pos < len && is_ident_char((unsigned char)src[pos])) pos++;
    size_t name_end = pos;
    if (name_end <= name_begin) return;

    // Initialize TypeDB with one Type entry for this struct
    db->types = (struct Type*)realloc(db->types, (db->type_count + 1) * sizeof(struct Type));
    if (!db->types) { db->type_count = 0; return; }
    struct Type *t = &db->types[db->type_count++];
    memset(t, 0, sizeof(*t));
    t->kind = TYPE_STRUCT;
    t->name = (char*)malloc(name_end - name_begin + 1);
    if (t->name) {
        memcpy(t->name, src + name_begin, name_end - name_begin);
        t->name[name_end - name_begin] = '\0';
    }

    // Parse body: naive fields "type name;" where type is single identifier
    const char *body = src + blk->lbrace + 1;
    size_t body_len = blk->rbrace - blk->lbrace - 1;
    size_t i = 0;
    while (i < body_len) {
        // skip spaces and newlines
        while (i < body_len && is_space((unsigned char)body[i])) i++;
        if (i >= body_len) break;

        // parse type identifier
        size_t type_begin = i;
        while (i < body_len && is_ident_char((unsigned char)body[i])) i++;
        size_t type_end = i;
        if (type_end <= type_begin) { i++; continue; }

        // skip spaces
        while (i < body_len && is_space((unsigned char)body[i])) i++;

        // parse field name identifier
        size_t name2_begin = i;
        while (i < body_len && is_ident_char((unsigned char)body[i])) i++;
        size_t name2_end = i;
        if (name2_end <= name2_begin) { i++; continue; }

        // advance to ';'
        while (i < body_len && body[i] != ';') i++;
        if (i < body_len && body[i] == ';') i++;

        // append field
        t->fields = (struct Field*)realloc(t->fields, (t->field_count + 1) * sizeof(struct Field));
        if (!t->fields) { t->field_count = 0; break; }
        struct Field *f = &t->fields[t->field_count++];
        memset(f, 0, sizeof(*f));
        f->name = (char*)malloc(name2_end - name2_begin + 1);
        if (f->name) {
            memcpy(f->name, body + name2_begin, name2_end - name2_begin);
            f->name[name2_end - name2_begin] = '\0';
        }
        // minimal TypeRef for primitive/unknown type names
        f->type = (TypeRef*)calloc(1, sizeof(TypeRef));
        if (f->type) {
            f->type->kind = TYPE_PRIMITIVE; // naive
            char *tn = (char*)malloc(type_end - type_begin + 1);
            if (tn) {
                memcpy(tn, body + type_begin, type_end - type_begin);
                tn[type_end - type_begin] = '\0';
            }
            f->type->name = tn;
        }
    }
}
