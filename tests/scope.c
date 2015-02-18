#include "clar.h"

#include <string.h>

#include <parser.h>
#include <parser_state.h>
#include <parser_signatures.h>
#include <ast.h>
#include <scope.h>

void test_scope__function(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("fType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("fName"), &parser_state);
    Parse(parser, LPAREN, strdup("("), &parser_state);
    Parse(parser, TYPE, strdup("E1"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("e1"), &parser_state);
    Parse(parser, RPAREN, strdup(")"), &parser_state);

    Parse(parser, DEF, strdup(":="), &parser_state);

    Parse(parser, LBRACE, strdup("{"), &parser_state);

    Parse(parser, IDENTIFIER, strdup("a"), &parser_state);
    Parse(parser, ASSIGN, strdup("="), &parser_state);
    Parse(parser, IDENTIFIER, strdup("e1"), &parser_state);
    Parse(parser, DOT, strdup("."), &parser_state);
    Parse(parser, IDENTIFIER, strdup("x"), &parser_state);

    Parse(parser, RBRACE, strdup("}"), &parser_state);
    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    cl_assert_equal_i(parser_state.state, OK);

    link_references(parser_state.root);

    struct tree_iterator *it = tree_iterator_init(&parser_state.root, POSTORDER);
    struct node *temp = NULL;
    while ((temp = tree_iterator_next(it)) != NULL) {
        if (((struct payload *)temp->payload)->type == N_ATOMIC) {
            if (((struct payload *)temp->payload)->alternative == ALT_IDENTIFIER) {
                cl_assert(((struct payload *)temp->payload)->atomic.ref != NULL);
            }
        }
    }
    tree_iterator_free(it);

    tree_free(&(parser_state.root), payload_free);
}
