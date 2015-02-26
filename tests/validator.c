#include "clar.h"

#include <string.h>

#include <parser.h>
#include <parser_state.h>
#include <parser_signatures.h>
#include <ast.h>
#include <scope.h>
#include <validator.h>

void test_validator__function(void)
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

    cl_assert_equal_i(validate(parser_state.root), 1);

    tree_free(&(parser_state.root), payload_free);
}
