#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "parser_signatures.h"
#include "ast.h"
#include <tree.h>

struct node *parse_ast(const char *str)
{
    struct parser_state parser_state;
    yyscan_t scanner;
    yylex_init(&scanner);
    YY_BUFFER_STATE bufferState = yy_scan_string(str, scanner);

    int token = 0;

    void *parser = ParseAlloc(malloc);
    while ((token = yylex(scanner))) {
        if (token == -1) {
            break;
        }
        char *text = strdup(yyget_text(scanner));
        Parse(parser, token, text, &parser_state);
    }
    Parse(parser, 0, NULL, &parser_state);
    ParseFree(parser, free);
    yy_delete_buffer(bufferState, scanner);
    yylex_destroy(scanner);
    if (parser_state.state != OK) {
        return NULL;
    } else {
        return parser_state.root;
    }
}

void *get_field_address(void *base, struct node *field)
{
    // TODO

    return NULL;
}
