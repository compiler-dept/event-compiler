%{
#include "parser.h"
%}

%option reentrant
%option noyywrap
%option noinput
%option nounput

%x C_COMMENT

%%

[a-zA-Z][a-zA-Z0-9]* return IDENTIFIER;
[0-9]+(\.[0-9]+)?    return NUMBER;
[ \t\n]+             ;
"->"                 return RARROW;
"!"                  return NOT;
"&"                  return AND;
"|"                  return OR;
"["                  return LBRACKET;
"]"                  return RBRACKET;
"("                  return LPAREN;
")"                  return RPAREN;
","                  return COMMA;
";"                  return SEMIC;
"="                  return EQ;
"+"                  return ADD;
"-"                  return SUB;
"*"                  return MULT;
"/"                  return DIV;

"/*"                 BEGIN(C_COMMENT);
<C_COMMENT>"*/"      BEGIN(INITIAL);
<C_COMMENT>.         ;

%%
