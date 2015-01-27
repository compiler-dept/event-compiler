%{
#include "parser.h"
%}

%option reentrant
%option noyywrap
%option noinput
%option nounput

%x C_COMMENT

%%

[A_Z][a-zA-Z0-9]*    return TYPE;
[a-z][a-zA-Z0-9]*    return IDENTIFIER;
[0-9]+(\.[0-9]+)?    return NUMBER;
[ \t\n]+             ;
"->"                 return RARROW;
":="                 return DEF;
"."                  return DOT;
":"                  return COLON;
","                  return COMMA;
";"                  return SEMIC;
"["                  return LBRACKET;
"]"                  return RBRACKET;
"("                  return LPAREN;
")"                  return RPAREN;
"="                  return EQ;
"+"                  return ADD;
"-"                  return SUB;
"*"                  return MULT;
"/"                  return DIV;

"/*"                 BEGIN(C_COMMENT);
<C_COMMENT>"*/"      BEGIN(INITIAL);
<C_COMMENT>.         ;

%%
