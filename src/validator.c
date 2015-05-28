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
    T_EVENT_DEFINITION
};

const char *type_names[] = {
    "NUMBER",
    "BOOL",
    "VECTOR",
    "EVENT",
    "EVENT_DEFINITION"
};

static enum types *new_type(enum types type) {
    enum types *t = malloc(sizeof(enum types));
    *t = type;
    return t;
}

int stack_size = 0;

int validate_event_declaration(struct node *temp, struct payload *payload);

int validate_rule_declaration(struct node *temp, struct payload *payload);

int validate_rule_signature(struct node *temp, struct payload *payload);

int validate_event(struct node *temp, struct payload *payload);

int validate_predicate_definition(struct node *temp, struct payload *payload,
                                  struct stack **type_stack);

int validate_function_definition(struct node *temp, struct payload *payload,
                                 struct stack **type_stack);

int validate_parameter_list(struct node *temp);

int validate_function_call(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_event_definition(struct node *temp, struct stack **type_stack);

int validate_initializer(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_vector(struct node *temp, struct stack **type_stack);

int validate_component_sequence(struct node *temp, struct stack **type_stack);

int validate_comparison_expression(struct node *temp, struct payload *payload,
                                   struct stack **type_stack);

int validate_addition(struct node *temp, struct stack **type_stack);

int validate_multiplication(struct node *temp, struct payload *payload, struct stack **type_stack);

int validate_atomic(struct node *temp, struct payload *payload, struct stack **type_stack);

static enum types *type_stack_pop(struct stack **stack) {
    stack_size--;
    enum types *type = stack_pop(stack);
    //printf("Pop: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
    return type;
}

static void type_stack_push(struct stack **stack, enum types *type) {
    stack_size++;
    //printf("Push: %s (Stack Size: %i)\n", type_names[*type - 1], stack_size);
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
            case N_EVENT_DECLARATION:
                success = validate_event_declaration(temp, payload);
                break;
            case N_RULE_DECLARATION:
                success = validate_rule_declaration(temp, payload);
                break;
            case N_RULE_SIGNATURE:
                success = validate_rule_signature(temp, payload);
                break;
            case N_EVENT:
                success = validate_event(temp, payload);
                break;
            case N_PREDICATE_DEFINITION:
                success = validate_predicate_definition(temp, payload, &type_stack);
                break;
            case N_FUNCTION_DEFINITION:
                success = validate_function_definition(temp, payload, &type_stack);
                break;
            case N_PARAMETER_LIST:
                success = validate_parameter_list(temp);
                break;
            case N_FUNCTION_CALL:
                success = validate_function_call(temp, payload, &type_stack);
                break;
            case N_EVENT_DEFINITION:
                success = validate_event_definition(temp, &type_stack);
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
            case N_COMPARISON_EXPRESSION:
                success = validate_comparison_expression(temp, payload, &type_stack);
                break;
            case N_ADDITION:
                success = validate_addition(temp, &type_stack);
                break;
            case N_MULTIPLICATION:
                success = validate_multiplication(temp, payload, &type_stack);
                break;
            case N_ATOMIC:
                success = validate_atomic(temp, payload, &type_stack);
                break;
            default:
                /* passed by */
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
    int success = 1;

    if (payload->event.ref == NULL) {
        /* event does not exist */
        success = 0;
    }

    return success;
}

int validate_rule_signature(struct node *temp, struct payload *payload)
{
    int success = 1;

    if (payload->alternative == ALT_PREDICATE_SEQUENCE) {
        /* a predicate sequence is present */

        /* loop over predicates */
        for (int i = 0; i < temp->childv[1]->childc; i++) {
            /* get predicate definition */
            struct node *predicate_definition =
                ((struct payload *)(temp->childv[1]->childv[i]->payload))->predicate.ref;
            if (predicate_definition == NULL) {
                /* referenced predicate does not exist */
                success = 0;
                break;
            } else {
                /* predicate found */

                if (temp->childv[0]->childc != predicate_definition->childv[0]->childc) {
                    /* unequal number of parameters and events */
                    success = 0;
                } else {
                    /* number of parameters und events are equal */

                    /* loop over parameters */
                    char *parameter_type = NULL;
                    char *event_type = NULL;
                    for (int j = 0; j < predicate_definition->childv[0]->childc; j++) {
                        event_type = ((struct payload *)(temp->childv[0]->childv[j]->payload))->event.type;
                        parameter_type = ((struct payload *)(predicate_definition->childv[0]->childv[j]->payload))->parameter.type;
                        if (strcmp(event_type, parameter_type) != 0) {
                            /* one parameter does not match the the events in order or type */
                            success = 0;
                            break;
                        }
                    }
                }
            }
        }
    }

    return success;
}

int validate_rule_declaration(struct node *temp, struct payload *payload)
{
    int success = 1;

    if (NULL) {
        /* rule does already exist */
        success = 0;
    } else {
        /* rule does not exist */
        if (payload->rule_declaration.ref == NULL) {
            /* referenced function does not exist */
            success = 0;
        } else {
            /* referenced function exists */
            struct node *function_definition = payload->rule_declaration.ref;
            if (payload->rule_declaration.eventc != 0) {
                /* event sequence is not empty */
                if (((struct payload *)(function_definition->payload))->alternative == ALT_PARAMETER_LIST) {
                    /* parameter_list exists */
                    if (payload->rule_declaration.eventc != function_definition->childv[0]->childc) {
                        /* count of function parameters does not match the event sequence size */
                        success = 0;
                    } else {
                        /* same number of function parameters as events */

                        /* loop over parameters */
                        for (int i = 0; i < payload->rule_declaration.eventc; i++) {
                            struct node *event = array_list_get(&(payload->rule_declaration.eventv),
                                                                payload->rule_declaration.eventc - 1 - i);
                            struct node *parameter = function_definition->childv[0]->childv[i];
                            char *event_type = ((struct payload *)event->payload)->event.type;
                            char *parameter_type = ((struct payload *)parameter->payload)->parameter.type;
                            if (strcmp(event_type, parameter_type) != 0) {
                                /* at least one parameter does not match in type or order */
                                success = 0;
                                break;
                            }
                        }
                    }
                } else {
                    /* needed parameter_list is missing */
                    success = 0;
                }
            } else {
                /* event sequence is empty */
                if (((struct payload *)(function_definition->payload))->alternative == ALT_PARAMETER_LIST) {
                    success = 0;
                }
            }
        }
    }

    return success;
}

int validate_event_declaration(struct node *temp, struct payload *payload)
{
    int success = 1;

    if (NULL) {
        /* event already exists */
        success = 0;
    } else {
        /* event does not already exist */
        if (payload->event_declaration.type[1] != NULL) {
            /* event is inherited */
            if (payload->event_declaration.parent_ref == NULL) {
                /* parent event does not exist */
                success = 0;
            } else {
                /* parent event exists */
                for (int i = 0; i < temp->childv[0]->childc; i++) {
                    if (check_duplicate_members(payload->event_declaration.parent_ref,
                                                ((struct payload *)temp->childv[0]->childv[i]->payload)->member.identifier)) {
                        /* at least one member exists already in the parent */
                        success = 0;
                    }
                }
            }
        }
    }

    return success;
}

int validate_predicate_definition(struct node *temp, struct payload *payload,
                                  struct stack **type_stack)
{
    int success = 1;

    if (NULL) {
        /* predicate definition already exists */
        success = 0;
    } else {
        /* predicate definition does not exist */

        enum types *type = type_stack_pop(type_stack);
        if (*type != T_BOOL) {
            /* expression is not boolean */
            success = 0;
        }
        free(type);
    }

    return success;
}

int validate_function_definition(struct node *temp, struct payload *payload,
                                 struct stack **type_stack)
{
    int success = 1;

    if (NULL) {
        /* function already exists */
        success = 0;
    } else {
        /* function does not exists */
        if (payload->function_definition.event_ref == NULL) {
            /* return event type does not exist */
            success = 0;
        } else {
            /* return event type exists */

            /* comes from comparison_expression */
            enum types *type = type_stack_pop(type_stack);
            if (*type == T_EVENT) {
                /* expression is an event */
                char *ret_event_name = payload->function_definition.type;
                char *exp_event_name = stack_pop(type_stack);
                if (strcmp(ret_event_name, exp_event_name) != 0) {
                    /* return event and expression event have not the same type */
                    success = 0;
                }
                free(exp_event_name);
            } else if (*type == T_EVENT_DEFINITION) {
                /* the expression is an event definition */
                // TODO: CHECK THAT EVERY MEMBER IS PRESENT IN EVENT_DEFINITION
            } else {
                /* the expression is not an event or event_definition */
                success = 0;
            }
            free(type);
        }
    }

    return success;
}

int validate_parameter_list(struct node *temp)
{
    int success = 1;

    /* loop over parameters */
    for (int i = 0; i < temp->childc; i++) {
        struct payload *parameter_payload_one = temp->childv[i]->payload;
        /* loop over parameters */
        for (int j = i + 1; j < temp->childc; j++) {
            struct payload *parameter_payload_two = temp->childv[j]->payload;
            if (parameter_payload_one->parameter.identifier == parameter_payload_two->parameter.identifier) {
                /* there are double parameter names */
                success = 0;
                break;
            }
        }

        if (!success) {
            break;
        }
    }

    return success;
}

int validate_function_call(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    int success = 1;

    struct node *function_definition = payload->function_call.ref;
    if (function_definition == NULL) {
        /* called function does not exist */
        success = 0;
    } else {
        /* function exists */
        stack_push(type_stack, strdup(((struct payload *)(function_definition->payload))->function_definition.type));
        type_stack_push(type_stack, new_type(T_EVENT));
    }

    return success;
}

int validate_event_definition(struct node *temp, struct stack **type_stack)
{
    int success = 1;

    /* TODO: check that no initializer is doubled */

    type_stack_push(type_stack, new_type(T_EVENT_DEFINITION));

    return success;
}

int validate_initializer(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    int success = 1;

    if (payload->initializer.ref_index == -1) {
        /* no corresponding event member found */
        success = 0;
    } else {
        /* event member found */

        enum types *type = type_stack_pop(type_stack);
        if (*type != T_VECTOR) {
            /* expression is not a vector */
            success = 0;
        }
        free(type);
    }

    return success;
}

int validate_vector(struct node *temp, struct stack **type_stack)
{
    int success = 1;

    type_stack_push(type_stack, new_type(T_VECTOR));

    return success;
}

int validate_component_sequence(struct node *temp, struct stack **type_stack)
{
    int success = 1;

    /* loop over all expressions in the expression sequence and check that
     * they're all numbers
     */
    enum types *type = NULL;
    for (int i = 0; i < temp->childv[0]->childc; i++) {
        type = type_stack_pop(type_stack);
        if (*type != T_NUMBER) {
            /* one expression is not a number */
            success = 0;
            free(type);
            break;
        }
        free(type);
    }

    return success;
}

int validate_comparison_expression(struct node *temp, struct payload *payload,
                                   struct stack **type_stack)
{
    int success = 1;

    if (payload->alternative != ALT_ADDITIVE_EXPRESSION) {
        enum types *add_exp_one = type_stack_pop(type_stack);
        enum types *add_exp_two = type_stack_pop(type_stack);
        if (*add_exp_one != T_VECTOR || *add_exp_two != T_VECTOR) {
            /* comparison needs two vectors */
            success = 0;
        } else {
            /* two vectors are given */
            type_stack_push(type_stack, new_type(T_BOOL));
        }
        free(add_exp_one);
        free(add_exp_two);
    }

    return success;
}

int validate_addition(struct node *temp, struct stack **type_stack)
{
    int success = 1;

    enum types *mult_exp = type_stack_pop(type_stack);
    enum types *add_exp = type_stack_pop(type_stack);

    if (*mult_exp != *add_exp) {
        /* the expressions have not the same type */
        success = 0;
    } else {
        /* the expressions have the same type */
        type_stack_push(type_stack, mult_exp);
    }

    free(add_exp);

    return success;
}

int validate_multiplication(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    int success = 1;

    enum types *neg = type_stack_pop(type_stack);
    enum types *mult_exp = type_stack_pop(type_stack);

    if (*mult_exp != T_NUMBER) {
        /* first operand is not a number */
        success = 0;
    } else {
        /* first operand is a number */
        if (*neg != T_NUMBER && *neg != T_VECTOR) {
            /* second operand is not a number or not a vector */
            success = 0;
        } else if (*neg == T_NUMBER) {
            /* second operand is a number */
            type_stack_push(type_stack, new_type(T_NUMBER));
        } else {
            /* second operator is a vector */
            if (payload->alternative == ALT_DIV) {
                /* times vector is not defined */
                success = 0;
            } else {
                type_stack_push(type_stack, new_type(T_VECTOR));
            }
        }
    }

    free(neg);
    free(mult_exp);

    return success;
}

int validate_atomic(struct node *temp, struct payload *payload, struct stack **type_stack)
{
    int success = 1;
    if (payload->alternative == ALT_IDENTIFIER) {
        /* atomic is a vector or an event */

        if (payload->atomic.ref == NULL) {
            /* vector or event does not exist */
            success = 0;
        } else {
            /* vector or event exists */
            if (payload->atomic.identifier[1] != NULL) {
                /* it's a vector */
                type_stack_push(type_stack, new_type(T_VECTOR));
            } else {
                /* it's an event */
                stack_push(type_stack,
                           strdup(((struct payload *)payload->atomic.ref->payload)->parameter.type));
                type_stack_push(type_stack, new_type(T_EVENT));
            }
        }

        return success;
    }

    if (payload->alternative == ALT_NUMBER) {
        /* atomic is a number */

        type_stack_push(type_stack, new_type(T_NUMBER));
    }

    /* other alternatives are passed by */

    return success;
}
