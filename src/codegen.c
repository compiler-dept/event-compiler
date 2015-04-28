#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <llvm-c/Analysis.h>
#include "codegen.h"

int generate_event_fields(struct stack **, struct node *);

void generate_event_declaration(LLVMModuleRef module, struct node *node)
{
  struct payload *payload = node->payload;
  LLVMTypeRef event_struct = LLVMStructCreateNamed(LLVMGetModuleContext(module),
    payload->event_declaration.type[0]);

  struct stack *members = NULL;
  int member_count = generate_event_fields(&members, node);

  LLVMTypeRef member_types[2 * member_count];
  for (int i = 0; i < 2 * member_count; i+=2){
    member_types[i] = stack_pop(&members);
    member_types[i+1] = stack_pop(&members);
  }

  LLVMStructSetBody(event_struct, member_types, 2 * member_count, 0);

  // This is only necessary to have at least one useage of the type. Otherwise
  // LLVM will not include it in the module. It can be removed as soon as
  // the generated events are used for the first time
  int dummyFuncNameLen = 3+strlen(payload->event_declaration.type[0]);
  char dummyFuncName[dummyFuncNameLen + 1];
  sprintf(dummyFuncName, "use%s", payload->event_declaration.type[0]);
  LLVMAddFunction(module, dummyFuncName, LLVMFunctionType(event_struct, NULL, 0, 0));
}

int generate_event_fields(struct stack **members, struct node *node)
{
  struct payload *member_sequence_payload = node->childv[0]->payload;
  int member_count = member_sequence_payload->member_sequence.count;

  for (int i = 0; i < member_count; i++){
    stack_push(members, LLVMPointerType(LLVMDoubleType(), 0));
    stack_push(members, LLVMInt16Type());
  }

  struct payload *payload = node->payload;
  if (payload->event_declaration.type[1]
    && payload->event_declaration.parent_ref){
      member_count += generate_event_fields(members,
        payload->event_declaration.parent_ref);
  }

  return member_count;
}


void generate_rule_declaration(LLVMModuleRef module, struct node *node)
{
	struct payload *payload = node->payload;
	char *name = malloc((1 + 7 + strlen(payload->rule_declaration.name))
    * sizeof(char));
	sprintf(name, "%s_active", payload->rule_declaration.name);

  if (((struct payload *)node->childv[0]->payload)->alternative == ALT_EVENT_SEQUENCE){
    struct payload *es_payload = node->childv[0]->childv[0]->payload;
  }

	LLVMValueRef func =
	    LLVMAddFunction(module, name,
			    LLVMFunctionType(LLVMVoidType(), NULL, 0, 0));

	LLVMBasicBlockRef block = LLVMAppendBasicBlock(func, "entry");

	LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, block);
	LLVMBuildRetVoid(builder);

	LLVMVerifyFunction(func, LLVMPrintMessageAction);
	LLVMDisposeBuilder(builder);
	free(name);
}

LLVMModuleRef generate_module(struct node *ast, const char *name)
{
	LLVMModuleRef module = LLVMModuleCreateWithName(name);

	struct tree_iterator *it = tree_iterator_init(&ast, PREORDER);
	struct node *temp = NULL;
	struct payload *payload = NULL;
  int success = 1;

	while ((temp = tree_iterator_next(it)) != NULL) {
		payload = temp->payload;
		switch (payload->type) {
    case N_EVENT_DECLARATION:
  	  generate_event_declaration(module, temp);
  	  break;
    case N_RULE_DECLARATION:
			generate_rule_declaration(module, temp);
			break;
		default:
			break;
		}
    if (!success){
      break;
    }
	}

	tree_iterator_free(it);

  char *foo = NULL;
  if (!LLVMVerifyModule(module, LLVMPrintMessageAction, &foo)){
      puts(foo);
  }

  LLVMDumpModule(module);

	return module;
}
