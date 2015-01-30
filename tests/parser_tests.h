#include <parser.h>
#include <parser_signatures.h>
#include <parser_state.h>
#include <stdlib.h>

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
}
