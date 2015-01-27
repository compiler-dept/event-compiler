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

declaration ::= input RARROW vector SEMIC.

input ::= LBRACE LPAREN event_sequence RPAREN COMMA predicate_sequence LBRACKET.

// vector ...
