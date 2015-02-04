#ifndef PARSER_SIGNATURES_H
#define PARSER_SIGNATURES_H

#include "parser_state.h"

void *ParseAlloc(void *(*allocProc) (size_t));
void Parse(void *, int, const char *, struct parser_state *);
void ParseFree(void *, void (*freeProc) (void *));

#endif
