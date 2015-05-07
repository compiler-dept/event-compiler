#include <stdlib.h>
#include <stack.h>
#include <string.h>
#include <stdio.h>
#include <hashmap.h>
#include <array_list.h>
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

static enum types *new_type(enum types type)
{
    enum types *t = malloc(sizeof(enum types));
    *t = type;
    return t;
}

int stack_size = 0;

int validate_event_declaration(struct node *temp, struct payload *payload);

int validate_rule_declaration(struct node *temp, struct payload *payload);

int validate_rule_signature(struct node *temp, struct payload *payload);

int validate_event(struct node *temp, struct payload *payload);

int validate_predicate_definition(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_function_definition(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_function_call(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_event_definition(struct node *temp, struct stack **type_stack);

int validate_initializer_sequence(struct node *temp, struct stack **type_stack);

int validate_initializer(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_vector(struct node *temp, struct stack **type_stack);

int validate_component_sequence(struct node *temp, struct stack **type_stack);

int validate_comparison_expression(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_addition(struct node *temp, struct stack **type_stack);

int validate_multiplication(struct node *temp, struct stack **type_stack);

int validate_negation(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_atomic(struct node *temp, struct payload *payload, struct stack **type_stack);

static enum types *type_stack_pop(struct stack **stack)
{
    stack_size--;
    enum types *type = stack_pop(stack);
    printf("Pop: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    return type;
}

static enum types *type_stack_peek(struct stack *stack)
{
    enum types *type = stack_peek(stack);
    printf("Peek: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    return type;
}

static void type_stack_push(struct stack **stack, enum types *type)
{
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
                success = validate_event_declaration(temp, payload);
                break;
            case N_MEMBER_SEQUENCE:
                break;
            case N_RULE_DECLARATION:
                success = validate_rule_declaration(temp, payload);
                break;
            case N_RULE_SIGNATURE:
                success = validate_rule_signature(temp, payload);
                break;
            case N_EVENT_SEQUENCE:
                break;
            case N_EVENT:
                validate_event(temp, payload);
                break;
            case N_PREDICATE_SEQUENCE:
                break;
            case N_PREDICATE:
                break;
            case N_PREDICATE_DEFINITION:
                success = validate_predicate_definition(temp, payload, &type_stack);
                break;
            case N_FUNCTION_DEFINITION:
                success = validate_function_definition(temp, payload, &type_stack);
                break;
            case N_PARAMETER_LIST:
                break;
            case N_PARAMETER:
                break;
            case N_FUNCTION_CALL:
                success = validate_function_call(temp, payload, &type_stack);
                break;
            case N_ARGUMENT_SEQUENCE:
                break;
            case N_EVENT_DEFINITION:
                success = validate_event_definition(temp, &type_stack);
                break;
            case N_INITIALIZER_SEQUENCE:
                success = validate_initializer_sequence(temp, &type_stack);
                break;
            case N_INITIALIZER:
                success = validate_initializer(temp, payload, &type_stack);
                break;
            case N_VECTOR:
                success = validate_vector(temp, &type_stack);
                break;
            case N_COMPONENT_SEQUENCE:
                success = validate_component_sequence(temp, &type_stack);
                break;
            case N_EXPRESSION_SEQUENCE:
                break;
            case N_COMPARISON_EXPRESSION:
                success = validate_comparison_expression(temp, payload, &type_stack);
                break;
            case N_EXPRESSION:
                break;
            case N_ADDITIVE_EXPRESSION:
                break;
            case N_ADDITION:
                success = validate_addition(temp, &type_stack);
                break;
            case N_MULTIPLICATIVE_EXPRESSION:
                break;
            case N_MULTIPLICATION:
                success = validate_multiplication(temp, &type_stack);
                break;
            case N_NEGATION:
                success = validate_negation(temp, payload, &type_stack);
                break;
            case N_PRIMARY_EXPRESSION:
                break;
            case N_ATOMIC:
                success = validate_atomic(temp, payload, &type_stack);
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

int validate_event(struct node *temp, struct payload *payload)
{
    int success;
    if (!payload->event.ref) {
        struct node *rule_declaration = temp->parent->parent->parent;
        printf("Event \"%s\" in rule \"%s\" does not exist!\n",
               payload->event.type, ((struct payload *)rule_declaration->payload)->rule_declaration.name);
        success = 0;
    }
    return success;
}

int validate_rule_signature(struct node *temp, struct payload *payload)
{
    struct node *tempnode1, *tempnode2;
    char *typename1, *typename2;
    int success = 1;
    if (payload->alternative == ALT_PREDICATE_SEQUENCE) {
        /* loop over predicates */
        for (int i = 0; i < temp->childv[1]->childc; i++) {
            /* get predicate definition */
            tempnode1 = ((struct payload *)(temp->childv[1]->childv[i]->payload))->predicate.ref;
            if (tempnode1) {
                /* loop over parameters */
                int num_predicate_params = tempnode1->childv[0]->childc;
                int num_events = temp->childv[0]->childc;
                if (num_predicate_params == num_events) {
                    for (int j = 0; j < tempnode1->childv[0]->childc; j++) {
                        tempnode2 = tempnode1->childv[0]->childv[j];
                        typename1 = ((struct payload *)(tempnode2->payload))->parameter.type;
                        typename2 =
                            ((struct payload *)(temp->childv
                                                [0]->childv[j]->payload))->event.type;
                        if (strcmp(typename1, typename2)
                                != 0) {
                            printf
                            ("Type of parameter %d \"%s\" of predicate \"%s\" does not match the type in rule \"%s\"!\n",
                             j + 1,
                             ((struct payload
                               *)(tempnode2->payload))->parameter.identifier,
                             ((struct payload *)
                              tempnode1->payload)->predicate_definition.identifier,
                             ((struct payload *)
                              temp->parent->payload)->rule_declaration.name);
                            success = 0;
                            break;
                        }
                    }
                } else if (num_predicate_params < num_events) {
                    printf
                    ("To few parameters in predicate \"%s\" in Rule \"%s\"!\n",
                     ((struct payload *)tempnode1->payload)->predicate_definition.identifier,
                     ((struct payload *)temp->parent->payload)->rule_declaration.name);
                    success = 0;
                } else if (num_predicate_params > num_events) {
                    printf
                    ("To many parameters in predicate \"%s\" in Rule \"%s\"!\n",
                     ((struct payload *)tempnode1->payload)->predicate_definition.identifier,
                     ((struct payload *)temp->parent->payload)->rule_declaration.name);
                    success = 0;
                }

                if (!success) {
                    break;
                }
            } else {
                printf
                ("Predicate \"%s\" for rule \"%s\" does not exist!\n",
                 ((struct payload *)(temp->childv[1]->childv[i]->payload))->predicate.identifier,
                 ((struct payload *)temp->parent->payload)->rule_declaration.name);
                success = 0;
                break;
            }
        }
    }
    return success;
}

int validate_rule_declaration(struct node *temp, struct payload *payload)
{
    int success = 1;
    char *typename1, *typename2;
    if (payload->alternative == ALT_RULE_SIGNATURE) {
        /* get function definition */
        struct node *function_definition = payload->rule_declaration.ref;
        if (function_definition) {
            if (((struct payload *)function_definition->payload)->type == N_FUNCTION_DEFINITION) {
                if (payload->rule_declaration.eventc <= 0) {
                    if (((struct payload *)
                            function_definition->payload)->alternative != ALT_EXPRESSION) {
                        printf
                        ("Rule \"%s\" requires function without arguments!\n",
                         payload->rule_declaration.name);
                        success = 0;
                    }
                } else {	/* eventc > 0 */
                    if (((struct payload *)
                            function_definition->payload)->alternative != ALT_PARAMETER_LIST
                            || payload->rule_declaration.eventc !=
                            function_definition->childv[0]->childc) {
                        printf
                        ("Rule \"%s\" requires function with %d argument(s)!\n",
                         payload->rule_declaration.name, payload->rule_declaration.eventc);
                        success = 0;
                    } else {
                        /* loop over parameters */
                        for (int i = 0; i < payload->rule_declaration.eventc; i++) {
                            struct node *event =
                                array_list_get(&(payload->rule_declaration.eventv),
                                               payload->rule_declaration.eventc - 1 - i);
                            struct node *parameter =
                                    function_definition->childv[0]->childv[i];
                            typename1 = ((struct payload *)
                                         event->payload)->event.type;
                            typename2 = ((struct payload *)
                                         parameter->payload)->parameter.type;
                            if (strcmp(typename1, typename2) != 0) {
                                printf
                                ("Parameter %d of function \"%s\" has wrong type!\n",
                                 i + 1, payload->rule_declaration.identifier);
                                success = 0;
                                break;
                            }
                        }
                    }
                }
            } else {
                printf
                ("Function assigned to rule \"%s\" is not a function!\n",
                 payload->rule_declaration.name);
                success = 0;
            }
        } else {
            printf("No function named \"%s\"!\n", payload->rule_declaration.identifier);
            success = 0;
        }
    }
    return success;
}

int validate_event_declaration(struct node *temp, struct payload *payload)
{
    int success = 1;
    if (payload->event_declaration.type[1] != NULL) {
        if (payload->event_declaration.parent_ref == NULL) {
            printf("Parent event \"%s\" does not exist!\n", payload->event_declaration.type[1]);
            success = 0;
        } else {
            int count = temp->childv[0]->childc;
            for (int i = 0; i < count; i++) {
                if (check_duplicate_members
                        (payload->event_declaration.parent_ref,
                         ((struct payload *)temp->childv[0]->childv[i]->payload)->member.identifier)) {
                    printf
                    ("Duplicate members exist in inherited event \"%s\"!\n",
                     payload->event_declaration.type[0]);
                    success = 0;
                }
            }
        }
    }
    return success;
}

int validate_predicate_definition(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    enum types *op1;
    int success = 1;
    op1 = type_stack_pop(type_stack);
    if (*op1 != T_BOOL) {
        printf("Expression of predicate \"%s\" is not boolean!\n", payload->predicate_definition.identifier);
        success = 0;
    }
    free(op1);
    op1 = NULL;
    return success;
}

int validate_function_definition(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    char *typename1;
    enum types *op1;
    int success = 1;

    if (payload->function_definition.event_ref == NULL) {
        printf
        ("Return type \"%s\" of function \"%s\" does not exist!\n",
         payload->function_definition.type, payload->function_definition.identifier);
        success = 0;
    } else {
        op1 = type_stack_pop(type_stack);
        if (*op1 == T_EVENT) {
            typename1 = stack_pop(type_stack);
            if (strcmp(typename1, payload->function_definition.type)
                    != 0) {
                puts("fail5");
                success = 0;
            }
        } else if (*op1 != T_GENERIC_EVENT) {
            puts("fail6");
            success = 0;
        }
        free(op1);
        op1 = NULL;
    }
    return success;
}

int validate_function_call(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    int success = 1;
    struct node *tempnode1, *tempnode2;
    char *typename1, *typename2;

    puts("N_FUNCTION_CALL");
    // get corresponding function definition from scope
    tempnode1 = ((struct payload *)payload)->function_call.ref;

    // get expression sequence via argument sequence
    tempnode2 = temp->childv[0]->childv[0];

    // check number of parameters
    if (tempnode1->childv[0]->childc == tempnode2->childc) {
        // check parameter types against function definition
        for (int i = 0; i < tempnode2->childc; i++) {
            enum types *type = type_stack_pop(type_stack);
            if (*type == T_EVENT) {
                typename1 = stack_pop(type_stack);
                typename2 =
                    ((struct payload *)tempnode1->childv[0]->childv[i]->payload)->parameter.type;
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
    }

    if (success) {
        // push return type
        typename1 = ((struct payload *)tempnode1->payload)->function_definition.type;
        stack_push(type_stack, typename1);
        type_stack_push(type_stack, new_type(T_EVENT));
    }

    return success;
}

int validate_event_definition(struct node *temp, struct stack **type_stack)
{
    puts("N_EVENT_DEFINITION");
    free(type_stack_pop(type_stack));
    type_stack_push(type_stack, new_type(T_GENERIC_EVENT));
    return 1;
}

int validate_initializer_sequence(struct node *temp, struct stack **type_stack)
{
    puts("N_INITIALIZER_SEQUENCE");
    for (int i = 0; i < temp->childc - 1; i++) {
        free(type_stack_pop(type_stack));
    }
    return 1;
}

int validate_initializer(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    puts("N_INITIALIZER");
    int success = 1;
    if (*((enum types *)type_stack_peek(*type_stack)) != T_VECTOR) {
        puts("fail10");
        success = 0;
    }

    if (payload->initializer.ref_index == -1) {
        puts("fail10.1");
        success = 0;
    }
    return success;
}

int validate_vector(struct node *temp, struct stack **type_stack)
{
    puts("N_VECTOR");
    enum types *op1;
    int success = 1;
    op1 = type_stack_pop(type_stack);
    if (*op1 != T_NUMBER) {
        puts("fail11");
        success = 0;
    } else {
        type_stack_push(type_stack, new_type(T_VECTOR));
    }
    free(op1);
    op1 = NULL;
    return success;
}

int validate_component_sequence(struct node *temp, struct stack **type_stack)
{
    puts("N_COMPONENT_SEQUENCE");
    enum types *op1, *op2;
    int success = 1;
    op1 = type_stack_pop(type_stack);
    for (int i = 1; i < temp->childv[0]->childc; i++) {
        op2 = (enum types *)type_stack_pop(type_stack);
        if (*op1 != *op2) {
            puts("fail12");
            success = 0;
            break;
        }
        free(op2);
    }
    if (success) {
        type_stack_push(type_stack, op1);
    }
    return success;
}

int validate_comparison_expression(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    puts("N_COMPARISON_EXPRESSION");
    enum types *op1, *op2;
    int success = 1;
    if (payload->alternative != ALT_ADDITIVE_EXPRESSION) {
        op2 = type_stack_pop(type_stack);
        op1 = type_stack_pop(type_stack);
        if (*op1 != T_VECTOR || *op2 != T_VECTOR) {
            puts("fail13");
            success = 0;
        } else {
            type_stack_push(type_stack, new_type(T_BOOL));
        }
        free(op1);
        op1 = NULL;
        free(op2);
        op2 = NULL;
    }
    return success;
}

int validate_addition(struct node *temp, struct stack **type_stack)
{
    puts("N_ADDITION");
    enum types *op1, *op2;
    int success = 1;
    op2 = type_stack_pop(type_stack);
    op1 = type_stack_peek(*type_stack);
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
    return success;
}

int validate_multiplication(struct node *temp, struct stack **type_stack)
{
    puts("N_MULTIPLICATION");
    enum types *op1, *op2;
    int success = 1;
    op2 = type_stack_pop(type_stack);
    op1 = type_stack_pop(type_stack);
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
    type_stack_push(type_stack, op2);
    free(op1);
    op2 = NULL;
    return success;
}

int validate_negation(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    puts("N_NEGATION");
    enum types *op1;
    int success = 1;
    if (payload->alternative == ALT_NEGATION) {
        op1 = (enum types *)(type_stack_peek(*type_stack));
        if (*op1 != T_NUMBER && *op1 != T_VECTOR) {
            puts("fail1");
            success = 0;
        }
    }
    return success;
}

int validate_atomic(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    puts("N_ATOMIC");
    char *typename1;
    int success = 1;
    switch (payload->alternative) {
        case ALT_IDENTIFIER:
            ;
            struct payload *ref = (struct payload *)(payload->atomic.ref->payload);
            if (ref->type == N_PARAMETER) {
                if (payload->atomic.identifier[1]) {
                    type_stack_push(type_stack, new_type(T_VECTOR));
                } else {
                    typename1 = strdup(ref->parameter.type);
                    stack_push(type_stack, typename1);
                    type_stack_push(type_stack, new_type(T_EVENT));
                }
            } else {
                puts("fail2");
                success = 0;
            }
            break;
        case ALT_NUMBER:
            type_stack_push(type_stack, new_type(T_NUMBER));
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
    return success;
}
