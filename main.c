#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "generate.h"
#include "buffer_helper.h"

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

    /*
    struct Species *: i need to look keyword with its name for examples like this. 
    Because there can be multiple struct definitions in a single file.
    So type is not the only key, name is also important.
    */
    
    // Look until you see ";"
    // before ; its the field name
    // and before field name its the type with type name.
    /*
    struct Species *species;
    field name: species
    type with type name: struct Species *
    */ 

    /*
    Elimde temiz bir buffer var peki bunlardan neler nedir? 
    Turleri nedir? 
    */


    // Find every struct block. And store them. Not just the target struct.

    // struct StructBlock blk = {0};
    // if (find_struct_block(buffer.data, buffer.len, &blk) == 0) {
    // 
    //} else {
    // }

    free(buffer.data);

    fclose(file);
    return 0;
}