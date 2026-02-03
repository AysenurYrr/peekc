#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "generate.h"


const char *format_for_primitive(const char *tname)
{
    if (!tname) return "%d";
    if (strcmp(tname, "int") == 0) return "%d";
    if (strcmp(tname, "unsigned") == 0) return "%u";
    if (strcmp(tname, "unsignedint") == 0) return "%u";
    if (strcmp(tname, "long") == 0) return "%ld";
    if (strcmp(tname, "unsignedlong") == 0) return "%lu";
    if (strcmp(tname, "short") == 0) return "%hd";
    if (strcmp(tname, "unsignedshort") == 0) return "%hu";
    if (strcmp(tname, "char") == 0) return "%c";
    if (strcmp(tname, "float") == 0) return "%f";
    if (strcmp(tname, "double") == 0) return "%f";
    if (strcmp(tname, "size_t") == 0) return "%zu";
    return "%d";
}

void emit_printer(const struct Type *t)
{
    if (!t || !t->name) return;
    printf("void peekc_print_%s(const struct %s *p) {\n", t->name, t->name);
    printf("    if (!p) return;\n");
    for (size_t i = 0; i < t->field_count; ++i) {
        const struct Field *f = &t->fields[i];
        const char *tn = (f->type && f->type->name) ? f->type->name : NULL;
        const char *fmt = format_for_primitive(tn);
        if (!f->name) continue;

        if (tn && strcmp(tn, "char") == 0) {
            printf("    printf(\"%s=%s\\n\", p->%s);\n", f->name, "%c", f->name);
        } else {
            // Generic primitive case
            printf("    printf(\"%s=%s\\n\", p->%s);\n", f->name, fmt, f->name);
        }
    }
    printf("}\n");
}