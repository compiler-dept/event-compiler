#include "astdump.h"

const char *astdump_type_names[] = {
    "N_TRANSLATION_UNIT",
    "N_DECLARATION_SEQUENCE",
    "N_DECLARATION",
    "N_EVENT_DECLARATION",
    "N_MEMBER_SEQUENCE",
    "N_MEMBER",
    "N_RULE_DECLARATION",
    "N_RULE_SIGNATURE",
    "N_EVENT_SEQUENCE",
    "N_EVENT",
    "N_PREDICATE_SEQUENCE",
    "N_PREDICATE_DEFINITION",
    "N_PREDICATE",
    "N_FUNCTION_DEFINITION",
    "N_PARAMETER_LIST",
    "N_PARAMETER",
    "N_FUNCTION_CALL",
    "N_ARGUMENT_SEQUENCE",
    "N_EVENT_DEFINITION",
    "N_INITIALIZER_SEQUENCE",
    "N_INITIALIZER",
    "N_VECTOR",
    "N_COMPONENT_SEQUENCE",
    "N_EXPRESSION_SEQUENCE",
    "N_COMPARISON_EXPRESSION",
    "N_EXPRESSION",
    "N_ADDITIVE_EXPRESSION",
    "N_ADDITION",
    "N_MULTIPLICATIVE_EXPRESSION",
    "N_MULTIPLICATION",
    "N_NEGATION",
    "N_PRIMARY_EXPRESSION",
    "N_ATOMIC"
};

void dump_ast(struct node *root, const char *path)
{
	FILE *fp = fopen(path, "w");

	struct tree_iterator *it = tree_iterator_init(&root, PREORDER);
	struct node *current = NULL;
	struct stack *parents = NULL;

	while ((current = tree_iterator_next(it)) != NULL){
		while (current->parent != NULL && current->parent != stack_peek(parents)){
			fwrite(")", 1, 1, fp);
			stack_pop(&parents);
		}

		fwrite("(", 1, 1, fp);
		struct payload *payload = current->payload;
		const char *name = astdump_type_names[payload->type - 1];
		fwrite(name, strlen(name), 1, fp);

    switch (payload->type){
      case N_ATOMIC:
        if (payload->alternative == ALT_IDENTIFIER){
          if (payload->atomic.identifier[1] == NULL){
            fprintf(fp, ": %s", payload->atomic.identifier[0]);
          } else {
            fprintf(fp, ": %s.%s", payload->atomic.identifier[0],
              payload->atomic.identifier[1]);
          }
        } else if (payload->alternative == ALT_NUMBER){
          fprintf(fp, ": %f", payload->atomic.number);
        }
        break;
      default:
        break;
    }

		stack_push(&parents, current);
	}

	while (stack_pop(&parents) != NULL){
		fwrite(")", 1, 1, fp);
	}

	tree_iterator_free(it);
	fclose(fp);
}
