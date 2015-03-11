#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <llvm-c/Analysis.h>
#include "codegen.h"

void generate_rule_declaration(LLVMModuleRef module, struct node *node)
{
    struct payload *payload = node->payload;
    char *name = malloc((7 + strlen(payload->rule_declaration.name)) * sizeof(char));
    sprintf(name, "%s_active", payload->rule_declaration.name);

    LLVMValueRef func = LLVMAddFunction(module, name, LLVMFunctionType(LLVMVoidType(), NULL, 0, 0));

    LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "entry");

    LLVMBuilderRef builder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(builder, block);
    LLVMBuildRetVoid(builder);

    LLVMVerifyFunction(func, LLVMPrintMessageAction);
    LLVMDisposeBuilder(builder);
}

LLVMModuleRef generate_module(struct node *ast, const char *name)
{
    LLVMModuleRef module = LLVMModuleCreateWithName(name);

    struct tree_iterator *it = tree_iterator_init(&ast, PREORDER);
    struct node *temp = NULL;
    struct payload *payload = NULL;

    while ((temp = tree_iterator_next(it)) != NULL) {
        payload = temp->payload;
        switch (payload->type) {
            case N_RULE_DECLARATION:
                generate_rule_declaration(module, temp);
                break;
            default:
                break;
        }
    }

    tree_iterator_free(it);

    return module;
}
