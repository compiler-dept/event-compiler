#include "clar.h"

#include <compiler.h>
#include <ast.h>
#include <tree.h>

void test_integration__event_declaration(void)
{
    struct node *root =
        parse_ast("Alfred extends Bazinga { foo, bar }; Bazinga { baz };");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}

void test_integration__rule_declaration_without_events(void)
{
    struct node *root =
        parse_ast("A: [p1, p2] -> a;");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}

void test_integration__rule_declaration_with_events(void)
{
    struct node *root =
        parse_ast("B: [SampleEvent : p3, p4] -> foo;");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}

void test_integration__function_definition_constant(void)
{
    struct node *root =
        parse_ast("SampleEvent a() := { pos = [1, 2, 3], angle = 30.0 };");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}

void test_integration__function_definition(void)
{
    struct node *root =
        parse_ast("SampleEvent foo(SampleEvent s1) := { pos = 2 * s1.pos,  angle = 0 };");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}

void test_integration__function_definition_function(void)
{
    struct node *root =
        parse_ast("SampleEvent foo(SampleEvent s1) := f1(f2(s1), f3(s1));");

    cl_assert(root != NULL);

    tree_free(&root, payload_free);
}
