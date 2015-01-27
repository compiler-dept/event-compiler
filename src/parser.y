%include
{
    #include <stdio.h>
    #include <assert.h>
}

%type translation_unit { char * }

%syntax_error
{
    fprintf(stderr, "%s\n", "Error parsing input.");
}

translation_unit ::= declaration_sequence.

declaration_sequence ::= declaration_sequence declaration.
declaration_sequence ::= declaration.

declaration ::= rule_declaration SEMIC.
declaration ::= function_definition SEMIC.


// rule_declaration

rule_declaration ::= IDENTIFIER COLON predicate_sequence RARROW function_sequence.

predicate_sequence ::= identifier_sequence.

function_sequence ::= identifier_sequence.

identifier_sequence ::= identifier_sequence COMMA IDENTIFIER.
identifier_sequence ::= IDENTIFIER.


// function_definition

function_definition ::= TYPE IDENTIFIER LPAREN parameter_list RPAREN DEF expression SEMIC.

parameter_list ::= parameter_list COMMA parameter.
parameter_list ::= TYPE IDENTIFIER.


// function_call

function_call ::= IDENTIFIER LPAREN argument_sequence RPAREN.

argument_sequence ::= expression_sequence.


// event_definition

event_definition ::= LBRACE initializer_sequence RBRACE.

initializer_sequence ::= initializer_sequence initializer.

initializer ::= IDENTIFIER ASSIGN expression COMMA


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

atomic ::= TYPE DOT IDENTIFIER.

atomic ::= NUMBER.

atomic ::= vector.

atomic ::= event_definition.

atomic ::= function_call.
