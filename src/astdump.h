#ifndef ASTDUMP_H
#define ASTDUMP_H
#include "ast.h"
#include <stdio.h>
#include <string.h>
#include <stack.h>
#include <tree.h>

void dump_ast(struct node *root, const char *path);

#endif
