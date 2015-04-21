#include "clar.h"

#include <string.h>

#include <parser.h>
#include <parser_state.h>
#include <parser_signatures.h>
#include <ast.h>
#include <scope.h>
#include <validator.h>
#include <compiler.h>

void test_validator__function(void)
{
    struct node *root =
        parse_ast("event SampleEvent { pos }; SampleEvent sampleEvent() := { pos = 5 * ([1, 2, 3] + [1, 2, 3]) };");

    link_references(root);

    cl_assert_equal_i(validate(root), 1);

    tree_free(&root, payload_free);
}
