#include <stdlib.h>
#include <stack.h>
#include <string.h>
#include "ast.h"
#include "validator.h"

enum types {
    T_NUMBER = 1,
    T_VECTOR,
    T_EVENT
};

static enum types *new_type(enum types type)
{
    enum types *t = malloc(sizeof(enum types));
    *t = type;
    return t;
}

int validate(struct node *root)
{
    struct tree_iterator *it = tree_iterator_init(&root, POSTORDER);
    struct node *temp = NULL;
    struct payload *payload = NULL;
    struct stack *type_stack = NULL;
    int success = 1;

    while ((temp = tree_iterator_next(it)) != NULL) {
        payload = (struct payload *)temp->payload;
        switch (payload->type) {
            case N_TRANSLATION_UNIT:
                break;
        	case N_DECLARATION_SEQUENCE:
                break;
        	case N_DECLARATION:
                break;
        	case N_EVENT_INHERITANCE:
                break;
        	case N_RULE_DECLARATION:
                break;
        	case N_RULE_SIGNATURE:
                break;
        	case N_EVENT_SEQUENCE:
                break;
        	case N_PREDICATE_SEQUENCE:
                break;
        	case N_PREDICATE_DEFINITION:
                break;
        	case N_FUNCTION_DEFINITION:
                break;
        	case N_PARAMETER_LIST:
                break;
        	case N_PARAMETER:
                break;
        	case N_FUNCTION_CALL:
                break;
        	case N_ARGUMENT_SEQUENCE:
                break;
        	case N_EVENT_DEFINITION:
                break;
        	case N_INITIALIZER_SEQUENCE:
                break;
        	case N_INITIALIZER:
                break;
        	case N_VECTOR:
                break;
        	case N_COMPONENT_SEQUENCE:
                break;
        	case N_EXPRESSION_SEQUENCE:
                break;
        	case N_COMPARISON_EXPRESSION:
                break;
        	case N_EXPRESSION:
                break;
        	case N_ADDITIVE_EXPRESSION:
                break;
        	case N_ADDITION:
                break;
        	case N_MULTIPLICATIVE_EXPRESSION:
                break;
        	case N_MULTIPLICATION:
                break;
        	case N_NEGATION:
                break;
        	case N_PRIMARY_EXPRESSION:
                break;
        	case N_ATOMIC:
                switch (payload->alternative) {
                    case ALT_IDENTIFIER: ;
                        struct payload *ref = (struct payload *)(payload->atomic.ref->payload);
                        if (ref->type == N_PARAMETER) {
                            if (payload->atomic.identifier[1]) {
                                stack_push(&type_stack, new_type(T_VECTOR));
                            } else {
                                stack_push(&type_stack, strdup(ref->parameter.type));
                                stack_push(&type_stack, new_type(T_EVENT));
                            }
                        } else {
                            success = 0;
                        }
                        break;
                    case ALT_NUMBER:
                        stack_push(&type_stack, new_type(T_NUMBER));
                        break;
                    // TODO missing cases
                    default:
                        success = 0;
                        break;
                }
                break;
        }

        if (!success) {
            break;
        }
    }

    stack_free(&type_stack, free);
    tree_iterator_free(it);

    return success;
}
