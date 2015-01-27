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

translation_unit ::= error.
{
    fprintf(stderr, "%s\n", "Translation Unit");
}
