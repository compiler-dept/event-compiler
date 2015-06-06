%{
#include "parser.h"
%}

%option reentrant
%option noyywrap
%option noinput
%option nounput

%x C_COMMENT

%%

"event"              return EVENT;
"extends"            return EXTENDS;
"predicate"          return PREDICATE;
[A-Z][a-zA-Z0-9_]*   return TYPE;
[a-z][a-zA-Z0-9_]*   return IDENTIFIER;
[0-9]+(\.[0-9]+)?    return NUMBER;
[ \t\n]+             ;
"=="                 return EQ;
"!="                 return NEQ;
">"                  return GT;
"<"                  return LT;
"->"                 return RARROW;
":="                 return DEF;
"."                  return DOT;
":"                  return COLON;
","                  return COMMA;
";"                  return SEMIC;
"{"                  return LBRACE;
"}"                  return RBRACE;
"["                  return LBRACKET;
"]"                  return RBRACKET;
"("                  return LPAREN;
")"                  return RPAREN;
"="                  return ASSIGN;
"+"                  return ADD;
"-"                  return SUB;
"*"                  return MULT;
"/"                  return DIV;
"^"                  return POW;

"/*"                 BEGIN(C_COMMENT);
<C_COMMENT>"*/"      BEGIN(INITIAL);
<C_COMMENT>.         ;

%%
