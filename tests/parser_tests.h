#include <parser.h>
#include <parser_signatures.h>
#include <parser_state.h>
#include <stdlib.h>
#include <tree.h>
#include <ast.h>

void test_event_inheritance(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("InheritedEvent"), &parser_state);
    Parse(parser, EXTENDS, strdup("extends"), &parser_state);
    Parse(parser, TYPE, strdup("BasicEvent"), &parser_state);
    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, TYPE, strdup("InheritedEvent2"), &parser_state);
    Parse(parser, EXTENDS, strdup("extends"), &parser_state);
    Parse(parser, TYPE, strdup("BasicEvent"), &parser_state);
    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    CU_ASSERT_EQUAL(parser_state.state, OK);

    tree_free(&(parser_state.root), payload_free);
}

void test_constant_function_definition(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("fType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("fName"), &parser_state);
    Parse(parser, LPAREN, strdup("("), &parser_state);
    Parse(parser, RPAREN, strdup(")"), &parser_state);

    Parse(parser, DEF, strdup(":="), &parser_state);

    Parse(parser, LBRACE, strdup("{"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("a"), &parser_state);
    Parse(parser, ASSIGN, strdup("="), &parser_state);
    Parse(parser, NUMBER, strdup("1"), &parser_state);

    Parse(parser, COMMA, strdup(","), &parser_state);

    Parse(parser, IDENTIFIER, strdup("b"), &parser_state);
    Parse(parser, ASSIGN, strdup("="), &parser_state);
    Parse(parser, NUMBER, strdup("1+2"), &parser_state);
    Parse(parser, RBRACE, strdup("}"), &parser_state);

    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    CU_ASSERT_EQUAL(parser_state.state, OK);

    tree_free(&(parser_state.root), payload_free);
}

void test_function_definition(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("fType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("fName"), &parser_state);
    Parse(parser, LPAREN, strdup("("), &parser_state);
    Parse(parser, TYPE, strdup("PType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("pName"), &parser_state);
    Parse(parser, RPAREN, strdup(")"), &parser_state);

    Parse(parser, DEF, strdup(":="), &parser_state);

    Parse(parser, LBRACE, strdup("{"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("a"), &parser_state);
    Parse(parser, ASSIGN, strdup("="), &parser_state);
    Parse(parser, IDENTIFIER, strdup("pName"), &parser_state);
    Parse(parser, DOT, strdup("."), &parser_state);
    Parse(parser, IDENTIFIER, strdup("a"), &parser_state);

    Parse(parser, COMMA, strdup(","), &parser_state);

    Parse(parser, IDENTIFIER, strdup("b"), &parser_state);
    Parse(parser, ASSIGN, strdup("="), &parser_state);
    Parse(parser, NUMBER, strdup("1+2"), &parser_state);
    Parse(parser, RBRACE, strdup("}"), &parser_state);

    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    CU_ASSERT_EQUAL(parser_state.state, OK);

    tree_free(&(parser_state.root), payload_free);
}

void test_function_definition_function(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("fType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("fName"), &parser_state);
    Parse(parser, LPAREN, strdup("("), &parser_state);
    Parse(parser, TYPE, strdup("PType"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("pName"), &parser_state);
    Parse(parser, RPAREN, strdup(")"), &parser_state);

    Parse(parser, DEF, strdup(":="), &parser_state);

    Parse(parser, IDENTIFIER, strdup("f2Name"), &parser_state);
    Parse(parser, LPAREN, strdup("("), &parser_state);
    Parse(parser, IDENTIFIER, strdup("pName"), &parser_state);
    Parse(parser, RPAREN, strdup(")"), &parser_state);

    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    CU_ASSERT_EQUAL(parser_state.state, OK);

    tree_free(&(parser_state.root), payload_free);
}

void test_rule_declaration(void)
{
    struct parser_state parser_state;
    void *parser = ParseAlloc(malloc);

    Parse(parser, TYPE, strdup("A"), &parser_state);
    Parse(parser, COLON, strdup(":"), &parser_state);
    Parse(parser, LBRACKET, strdup("["), &parser_state);
    Parse(parser, TYPE, strdup("SampleEvent"), &parser_state);
    Parse(parser, COLON, strdup(":"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("p1"), &parser_state);
    Parse(parser, COMMA, strdup(","), &parser_state);
    Parse(parser, IDENTIFIER, strdup("p2"), &parser_state);
    Parse(parser, RBRACKET, strdup("]"), &parser_state);
    Parse(parser, RARROW, strdup("->"), &parser_state);
    Parse(parser, IDENTIFIER, strdup("test"), &parser_state);
    Parse(parser, SEMIC, strdup(";"), &parser_state);

    Parse(parser, 0, 0, &parser_state);
    ParseFree(parser, free);

    CU_ASSERT_EQUAL(parser_state.state, OK);

    tree_free(&(parser_state.root), payload_free);
}
