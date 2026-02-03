#ifndef PEEKC_GENERATE_H
#define PEEKC_GENERATE_H


const char *format_for_primitive(const char *tname);

void emit_printer(const struct Type *t);


#endif