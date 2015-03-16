#ifndef COMPILER_H
#define COMPILER_H

#include <tree.h>

struct node *parse_ast(const char *);

void *get_field_address(void *base, struct node *field);

#endif
