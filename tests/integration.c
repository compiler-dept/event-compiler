#include "clar.h"

#include <compiler.h>
#include <ast.h>
#include <tree.h>

void test_integration__event_inheritance(void)
{
    struct node *root =
        parse_ast("Alfred extends Bazinga; Bazinga extends Carlos;");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}
