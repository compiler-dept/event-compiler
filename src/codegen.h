#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>
#include <tree.h>
#include "ast.h"

LLVMModuleRef generate_module(struct node *ast, const char *name);

#endif
