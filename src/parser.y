%include
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include "parser_state.h"
}

%token_type { const char * }
%token_destructor { free((char *)$$); }

%type translation_unit { char * }
%extra_argument { struct parser_state *parser_state }

%syntax_error
{
    fprintf(stderr, "%s\n", "Error parsing input.");
}

translation_unit ::= declaration_sequence.
{
    parser_state->state = OK;
}
translation_unit ::= error.
{
    parser_state->state = ERROR;
}

declaration_sequence ::= declaration_sequence declaration.
declaration_sequence ::= declaration.

declaration ::= event_inheritance SEMIC.
declaration ::= rule_declaration SEMIC.
declaration ::= function_definition SEMIC.


// event_inheritance

event_inheritance ::= TYPE EXTENDS TYPE.


// rule_declaration

rule_declaration ::= IDENTIFIER COLON rule_signature  RARROW IDENTIFIER.

rule_signature ::= LBRACKET event_sequence COLON predicate_sequence RBRACKET.
rule_signature ::= LBRACKET predicate_sequence RBRACKET.
rule_signature ::= LBRACKET RBRACKET.

event_sequence ::= event_sequence COMMA TYPE.
event_sequence ::= TYPE.

predicate_sequence ::= predicate_sequence COMMA IDENTIFIER.
predicate_sequence ::= IDENTIFIER.


// function_definition

function_definition ::= TYPE IDENTIFIER LPAREN RPAREN DEF expression.
function_definition ::= TYPE IDENTIFIER LPAREN parameter_list RPAREN DEF expression.

parameter_list ::= parameter_list COMMA parameter.
parameter_list ::= parameter.

parameter ::= TYPE IDENTIFIER.


// function_call

function_call ::= IDENTIFIER LPAREN argument_sequence RPAREN.

argument_sequence ::= expression_sequence.


// event_definition

event_definition ::= LBRACE initializer_sequence RBRACE.

initializer_sequence ::= initializer_sequence COMMA initializer.
initializer_sequence ::= initializer.

initializer ::= IDENTIFIER ASSIGN expression.


// vector definition

vector ::= LBRACKET component_sequence RBRACKET.

component_sequence ::= expression_sequence.


// expression_sequence
expression_sequence ::= expression_sequence COMMA expression.

expression_sequence ::= expression.


// expressions
expression ::= additive_expression.

additive_expression ::= addition.

additive_expression ::= multiplicative_expression.

addition ::= additive_expression ADD multiplicative_expression.

addition ::= additive_expression SUB multiplicative_expression.

multiplicative_expression ::= multiplication.

multiplicative_expression ::= negation.

multiplication ::= multiplicative_expression MULT negation.

multiplication ::= multiplicative_expression DIV negation.

negation ::= SUB negation.

negation ::= primary_expression.

primary_expression ::= LPAREN expression RPAREN.

primary_expression ::= atomic.

atomic ::= IDENTIFIER DOT IDENTIFIER.

atomic ::= IDENTIFIER.

atomic ::= NUMBER.

atomic ::= vector.

atomic ::= event_definition.

atomic ::= function_call.
