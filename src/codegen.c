#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <llvm-c/Analysis.h>
#include "codegen.h"

int generate_event_fields(struct stack **, struct node *);

LLVMTypeRef generate_event_declaration(LLVMModuleRef module, struct node *node)
{
  struct payload *payload = node->payload;
  LLVMTypeRef event_type = LLVMStructCreateNamed(LLVMGetModuleContext(module),
  payload->event_declaration.type[0]);

  struct stack *members = NULL;
  int member_count = generate_event_fields(&members, node);

  LLVMTypeRef member_types[2 * member_count];
  for (int i = 0; i < 2 * member_count; i+=2){
    member_types[i] = stack_pop(&members);
    member_types[i+1] = stack_pop(&members);
  }

  LLVMStructSetBody(event_type, member_types, 2 * member_count, 0);
  return event_type;
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

LLVMTypeRef generateEventTypeIfNecessary(LLVMModuleRef module, struct node * event_decl){
  struct payload *event_decl_payload = (struct payload *) event_decl->payload;
  char *event_name = event_decl_payload->event_declaration.type[0];
  LLVMTypeRef type = LLVMGetTypeByName(module, event_name);
  if (type){
    return type;
  } else {
    return generate_event_declaration(module, event_decl);
  }
}


void generate_rule_declaration(LLVMModuleRef module, struct node *node)
{
	struct payload *payload = node->payload;
	char active_name[1 + 7 + strlen(payload->rule_declaration.name)];
  char function_name[1 + 9 + strlen(payload->rule_declaration.name)];

	sprintf(active_name, "%s_active", payload->rule_declaration.name);
  sprintf(function_name, "%s_function", payload->rule_declaration.name);

  if (((struct payload *)node->childv[0]->payload)->alternative == ALT_PREDICATE_SEQUENCE){
    struct node *event_sequence = node->childv[0]->childv[0];
    LLVMTypeRef parameters[event_sequence->childc];
    for (int i = 0; i < event_sequence->childc; i++){
      struct node * event = event_sequence->childv[i];
      struct payload *event_payload = (struct payload *) event->payload;
      parameters[i] = generateEventTypeIfNecessary(module, event_payload->event.ref);
    }
    LLVMAddFunction(module, active_name,
      LLVMFunctionType(LLVMInt8Type(), parameters, event_sequence->childc, 0));

    struct payload *func_payload = ((struct payload *)payload->rule_declaration.ref->payload);
    struct node *event = func_payload->function_definition.event_ref;
    struct payload *event_payload = (struct payload *) event->payload;


    LLVMTypeRef return_type = generateEventTypeIfNecessary(module, event);

    LLVMAddFunction(module, function_name,
        LLVMFunctionType(return_type, parameters, event_sequence->childc, 0));
  }

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
