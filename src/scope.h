#ifndef SCOPE_H
#define SCOPE_H

#include "ast.h"

struct node *find_scope_for(struct node *node);

void link_references(struct node *node);

#endif
