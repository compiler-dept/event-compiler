#include <stdlib.h>
#include <stack.h>
#include <string.h>
#include <stdio.h>
#include <hashmap.h>
#include "ast.h"
#include "validator.h"

enum types {
    T_NUMBER = 1,
    T_BOOL,
    T_VECTOR,
    T_EVENT,
    T_GENERIC_EVENT
};

const char *type_names[] = {
    "NUMBER",
    "BOOL",
    "VECTOR",
    "EVENT",
    "GENERIC_EVENT"
};

static enum types *new_type(enum types type) {
    enum types *t = malloc(sizeof(enum types));
    *t = type;
    return t;
}

int stack_size = 0;

static enum types *type_stack_pop(struct stack **stack) {
    stack_size--;
    enum types *type = stack_pop(stack);
    printf("Pop: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    return type;
}

static enum types *type_stack_peek(struct stack *stack) {
    enum types *type = stack_peek(stack);
    printf("Peek: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    return type;
}

static void type_stack_push(struct stack **stack, enum types *type) {
    stack_size++;
    printf("Push: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    stack_push(stack, type);
}

int check_duplicate_members(struct node *event_declaration, char *member)
{
    struct payload *payload = event_declaration->payload;
    if (hashmap_get(payload->event_declaration.scope, member)) {
        return 1;
    } else if (payload->event_declaration.parent_ref) {
        return check_duplicate_members(payload->event_declaration.parent_ref, member);
    } else {
        return 0;
    }
}

int validate(struct node *root)
{
    struct tree_iterator *it = tree_iterator_init(&root, POSTORDER);
    struct node *temp = NULL;
    struct payload *payload = NULL;
    struct stack *type_stack = NULL;
    enum types *op2 = NULL;
    enum types *op1 = NULL;
    struct node *tempnode1 = NULL;
    struct node *tempnode2 = NULL;
    char *typename1 = NULL;
    char *typename2 = NULL;
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
            case N_EVENT_DECLARATION:
                puts("EVENT_DECLARATION");
                if (payload->event_declaration.type[1] != NULL) {
                    if (payload->event_declaration.parent_ref == NULL) {
                        puts("fail0.2");
                        success = 0;
                    } else {
                        int count = ((struct payload *)temp->childv[0]->payload)->member_sequence.count;
                        for (int i = 0; i < count; i++) {
                            if (check_duplicate_members(payload->event_declaration.parent_ref, ((struct payload *)temp->childv[0]->payload)->member_sequence.identifier[i])) {
                                puts("fail0.3");
                                success = 0;
                            }
                        }
                    }
                }

                break;
            case N_MEMBER_SEQUENCE:
                break;
            case N_RULE_DECLARATION:
                break;
            case N_RULE_SIGNATURE:
                puts("RULE_SIGNATURE");
                /* loop over predicates */
                if (payload->alternative == ALT_PREDICATE_SEQUENCE) {
                    for (int i = 0; i < temp->childv[1]->childc; i++) {
                        /* get predicate definition */
                        tempnode1 = ((struct payload *)(temp->childv[1]->childv[i]->payload))->predicate.ref;
                        if (tempnode1) {
                            if (tempnode1->childc == 2) {
                                /* loop over parameters */
                                int num_predicate_params = tempnode1->childv[0]->childc;
                                int num_events = temp->childv[0]->childc;
                                if (num_predicate_params == num_events) {
                                    for (int j = 0; j < tempnode1->childv[0]->childc; j++) {
                                        tempnode2 = tempnode1->childv[0]->childv[j];
                                        typename1 = ((struct payload *)(tempnode2->payload))->parameter.type;
                                        typename2 = ((struct payload *)(temp->childv[0]->childv[j]->payload))->event.type;
                                        if (strcmp(typename1, typename2) != 0) {
                                            puts("fail1");
                                            success = 0;
                                            break;
                                        }
                                    }
                                } else {
                                    puts("fail2");
                                    success = 0;
                                }

                                if (!success) {
                                    break;
                                }
                            } else if (temp->childc != 1) {
                                puts("fail3");
                                success = 0;
                                break;
                            }
                        } else {
                            puts("fail3.1");
                            success = 0;
                            break;
                        }
                    }
                }
                break;
            case N_EVENT_SEQUENCE:
                break;
            case N_EVENT:
                if (!payload->event.ref){
                  puts("fail3.1.1");
                  success = 0;
                }
                break;
            case N_PREDICATE_SEQUENCE:
                break;
            case N_PREDICATE:
                if (!payload->predicate.ref){
                  puts("fail3.1.2");
                  success = 0;
                }
                break;
            case N_PREDICATE_DEFINITION:
                puts("N_PREDICATE_DEFINITION");
                op1 = type_stack_pop(&type_stack);
                if (*op1 != T_BOOL) {
                    puts("fail4");
                    success = 0;
                }
                free(op1);
                op1 = NULL;
                break;
            case N_FUNCTION_DEFINITION:
                puts("N_FUNCTION_DEFINITION");
                if (payload->function_definition.event_ref == NULL) {
                    success = 0;
                    break;
                }

                op1 = type_stack_pop(&type_stack);
                if (*op1 == T_EVENT) {
                    typename1 = stack_pop(&type_stack);
                    if (strcmp(typename1, payload->function_definition.type) != 0) {
                        puts("fail5");
                        success = 0;
                    }
                } else if (*op1 != T_GENERIC_EVENT) {
                    puts("fail6");
                    success = 0;
                }
                free(op1);
                op1 = NULL;
                break;
            case N_PARAMETER_LIST:
                break;
            case N_PARAMETER:
                break;
            case N_FUNCTION_CALL:
                puts("N_FUNCTION_CALL");
                // get corresponding function definition from scope
                tempnode1 = ((struct payload *)payload)->function_call.ref;

                // get expression sequence via argument sequence
                tempnode2 = temp->childv[0]->childv[0];

                // check number of parameters
                if (tempnode1->childv[0]->childc == tempnode2->childc) {
                    // check parameter types against function definition
                    for (int i = 0; i < tempnode2->childc; i++) {
                        enum types *type = type_stack_pop(&type_stack);
                        if (*type == T_EVENT) {
                            typename1 = stack_pop(&type_stack);
                            typename2 = ((struct payload *)tempnode1->childv[0]->childv[i]->payload)->parameter.type;
                            if (strcmp(typename1, typename2) != 0) {
                                puts("fail7");
                                success = 0;
                                free(type);
                                break;
                            }
                        } else {
                            puts("fail8");
                            success = 0;
                            free(type);
                            break;
                        }

                        free(type);
                    }
                } else {
                    puts("fail9");
                    success = 0;
                    break;
                }

                if (success) {
                    // push return type
                    typename1 = ((struct payload *)tempnode1->payload)->function_definition.type;
                    stack_push(&type_stack, typename1);
                    type_stack_push(&type_stack, new_type(T_EVENT));
                }
                break;
            case N_ARGUMENT_SEQUENCE:
                break;
            case N_EVENT_DEFINITION:
                puts("N_EVENT_DEFINITION");
                free(type_stack_pop(&type_stack));
                type_stack_push(&type_stack, new_type(T_GENERIC_EVENT));
                break;
            case N_INITIALIZER_SEQUENCE:
                puts("N_INITIALIZER_SEQUENCE");
                for (int i = 0; i < temp->childc - 1; i++) {
                    free(type_stack_pop(&type_stack));
                }
                break;
            case N_INITIALIZER:
                puts("N_INITIALIZER");
                if (*((enum types *)type_stack_peek(type_stack)) != T_VECTOR) {
                    puts("fail10");
                    success = 0;
                }

                if (payload->initializer.ref_index == -1) {
                    puts("fail10.1");
                    success = 0;
                }
                break;
            case N_VECTOR:
                puts("N_VECTOR");
                op1 = type_stack_pop(&type_stack);
                if (*op1 != T_NUMBER) {
                    puts("fail11");
                    success = 0;
                } else {
                    type_stack_push(&type_stack, new_type(T_VECTOR));
                }
                free(op1);
                op1 = NULL;
                break;
            case N_COMPONENT_SEQUENCE:
                puts("N_COMPONENT_SEQUENCE");
                op1 = type_stack_pop(&type_stack);
                for (int i = 1; i < temp->childv[0]->childc; i++) {
                    op2 = (enum types *)type_stack_pop(&type_stack);
                    if (*op1 != *op2) {
                        puts("fail12");
                        success = 0;
                        break;
                    }
                    free(op2);
                }
                if (success) {
                    type_stack_push(&type_stack, op1);
                }
                break;
            case N_EXPRESSION_SEQUENCE:
                break;
            case N_COMPARISON_EXPRESSION:
                puts("N_COMPARISON_EXPRESSION");
                if (payload->alternative != ALT_ADDITIVE_EXPRESSION) {
                    op2 = type_stack_pop(&type_stack);
                    op1 = type_stack_pop(&type_stack);
                    if (*op1 != T_VECTOR || *op2 != T_VECTOR) {
                        puts("fail13");
                        success = 0;
                    } else {
                        type_stack_push(&type_stack, new_type(T_BOOL));
                    }
                    free(op1);
                    op1 = NULL;
                    free(op2);
                    op2 = NULL;
                }
                break;
            case N_EXPRESSION:
                break;
            case N_ADDITIVE_EXPRESSION:
                break;
            case N_ADDITION:
                puts("N_ADDITION");
                op2 = type_stack_pop(&type_stack);
                op1 = type_stack_peek(type_stack);
                if (*op1 == *op2) {
                    if (*op1 != T_NUMBER && *op1 != T_VECTOR) {
                        puts("fail14");
                        success = 0;
                    }
                } else {
                    puts("fail15");
                    success = 0;
                }
                free(op2);
                op2 = NULL;
                break;
            case N_MULTIPLICATIVE_EXPRESSION:
                break;
            case N_MULTIPLICATION:
                puts("N_MULTIPLICATION");
                op2 = type_stack_pop(&type_stack);
                op1 = type_stack_pop(&type_stack);
                if (*op2 == T_NUMBER) {
                    if (*op1 != T_NUMBER) {
                        puts("fail16");
                        success = 0;
                    }
                } else if (*op2 == T_VECTOR) {
                    if (*op1 != T_NUMBER && *op1 != T_VECTOR) {
                        puts("fail17");
                        success = 0;
                    }
                }
                type_stack_push(&type_stack, op2);
                free(op1);
                op2 = NULL;
                break;
            case N_NEGATION:
                puts("N_NEGATION");
                if (payload->alternative == ALT_NEGATION) {
                    op1 = (enum types *)(type_stack_peek(type_stack));
                    if (*op1 != T_NUMBER && *op1 != T_VECTOR) {
                        puts("fail1");
                        success = 0;
                    }
                }
                break;
            case N_PRIMARY_EXPRESSION:
                break;
            case N_ATOMIC:
                puts("N_ATOMIC");
                switch (payload->alternative) {
                    case ALT_IDENTIFIER:
                        ;
                        struct payload *ref = (struct payload *)(payload->atomic.ref->payload);
                        if (ref->type == N_PARAMETER) {
                            if (payload->atomic.identifier[1]) {
                                type_stack_push(&type_stack, new_type(T_VECTOR));
                            } else {
                                typename1 = strdup(ref->parameter.type);
                                stack_push(&type_stack, typename1);
                                type_stack_push(&type_stack, new_type(T_EVENT));
                            }
                        } else {
                            puts("fail2");
                            success = 0;
                        }
                        break;
                    case ALT_NUMBER:
                        type_stack_push(&type_stack, new_type(T_NUMBER));
                        break;
                    case ALT_VECTOR:
                        // Should be ok
                        break;
                    case ALT_EVENT_DEFINITION:
                        // Should be ok
                        break;
                    case ALT_FUNCTION_CALL:
                        // Should be ok
                        break;
                    default:
                        puts("fail3");
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
