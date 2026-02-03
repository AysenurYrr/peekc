#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "generate.h"

/*

peekc is struct printing tool.
It takes struct.h file and struct name as arguments.

it inspect struct.h and find the struct. 

Its purpose is generating C code for printing the contents of the struct.

*/ 
int main( int argc, char *argv[])
{

    // look arguments for input control. 

    //is first argument a file,and is it valid .h file?
    if (argc < 2 || !strstr(argv[1], ".h")) {
        fprintf(stderr, "Usage: %s <file.h> <struct_name>\n", argv[0]);
        return 1;
    }

    // is second argument a struct name?
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file.h> <struct_name>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // read file char by char and clean it from unwanted characters
    // for now lets just read it to buffer
    struct Buffer buffer = {0};
    if (0 != clean_file_to_buffer(file, &buffer)) { // 0 ok, nonzero error
        printf("Failed to clean file to buffer\n");
    }
    // for debug
    printf("Buffer contents:\n%s\n", buffer.data);

    struct StructBlock blk = {0};
    if (find_struct_block(buffer.data, buffer.len, argv[2], &blk) == 0) {
        printf("Found struct '%s' at %zu, body [%zu..%zu)\n", argv[2], blk.start, blk.lbrace, blk.rbrace);
    } else {
        printf("Struct '%s' not found in cleaned buffer\n", argv[2]);
    }

    // next MVP step:
    struct TypeDB db = {0};
    parse_fields_into_typedb(buffer.data, buffer.len, &blk, &db);

    // Debug: print parsed types
    printf("Parsed types:\n");
    for (size_t i = 0; i < db.type_count; ++i) {
        struct Type *t = &db.types[i];
        printf("Parsed type %s with %zu fields\n", t->name ? t->name : "(unnamed)", t->field_count);
        for (size_t j = 0; j < t->field_count; ++j) {
            struct Field *f = &t->fields[j];
            printf("  %s %s\n", f->type && f->type->name ? f->type->name : "<type>", f->name ? f->name : "<field>");
        }
    }

    // Generate C code for printing the struct
    if (db.type_count > 0) {
        emit_printer(&db.types[0]);
    }

    free(buffer.data);

    fclose(file);
    return 0;
}