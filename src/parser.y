%include
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <string.h>
    #include <tree.h>
    #include <stack.h>
    #include <hashmap.h>
    #include <array_list.h>
    #include "ast.h"
    #include "parser_state.h"

    struct stack *allocated_nodes = NULL;
    struct stack *current_scope = NULL;
    struct stack *global_scope = NULL;
    struct stack *events = NULL;

    void remove_topmost_node_of_type(struct stack **target_stack, enum type target_type) {
        struct stack *temp_stack = NULL;
        struct node *temp_node = NULL;
        struct payload *temp_payload = NULL;
        do {
            temp_node = stack_pop(target_stack);
            temp_payload = (struct payload *) temp_node->payload;
            if (temp_payload->type != target_type){
                stack_push(&temp_stack, temp_node);
            }
        } while (temp_payload->type != target_type);

        while ((temp_node = stack_pop(&temp_stack)) != NULL){
            stack_push(target_stack, temp_node);
        }
    }
}

%token_type { const char * }
%token_destructor {
    free((char *)$$);
    parser_state->state = parser_state->state;
}

%default_type { struct node * }

%extra_argument { struct parser_state *parser_state }

%syntax_error
{
    fprintf(stderr, "%s\n", "Error parsing input.");

    struct node *temp = NULL;
    while ((temp = stack_pop(&allocated_nodes)) != NULL) {
        payload_free(temp->payload);
        free(temp);
    }

    while ((temp = stack_pop(&events)) != NULL) {
        payload_free(temp->payload);
        free(temp);
    }

    struct hashmap_entry *temp2 = NULL;
    while ((temp2 = stack_pop(&current_scope)) != NULL) {
        free(temp2->key);
        free(temp2);
    }

    while ((temp2 = stack_pop(&global_scope)) != NULL) {
        free(temp2->key);
        free(temp2);
    }

    parser_state->state = ERROR;
    parser_state->root = NULL;
}

translation_unit(NODE) ::= declaration_sequence(DS).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_TRANSLATION_UNIT;
    payload->alternative = ALT_DECLARATION_SEQUENCE;
    payload->translation_unit.scope = NULL;

    NODE = tree_create_node(payload, 1, DS);

    struct hashmap_entry *temp;
    while ((temp = stack_pop(&global_scope)) != NULL) {
        hashmap_put(&(payload->translation_unit.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }

    stack_free(&allocated_nodes, NULL);

    parser_state->root = NODE;
    parser_state->state = OK;
}
translation_unit ::= error.
{

}

declaration_sequence(NODE) ::= declaration_sequence(DS) declaration(D).
{
    remove_topmost_node_of_type(&allocated_nodes, N_DECLARATION_SEQUENCE);
    NODE = tree_append_node(DS, D);
    stack_push(&allocated_nodes, NODE);
}
declaration_sequence(NODE) ::= declaration(D).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_DECLARATION_SEQUENCE;
    payload->alternative = ALT_DECLARATION;

    NODE = tree_create_node(payload, 1, D);
    stack_push(&allocated_nodes, NODE);
}

declaration(NODE) ::= event_declaration(ED) SEMIC.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_DECLARATION;
    payload->alternative = ALT_EVENT_DECLARATION;

    NODE = tree_create_node(payload, 1, ED);
    stack_push(&allocated_nodes, NODE);
}
declaration(NODE) ::= rule_declaration(RD) SEMIC.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_DECLARATION;
    payload->alternative = ALT_RULE_DECLARATION;

    NODE = tree_create_node(payload, 1, RD);
    stack_push(&allocated_nodes, NODE);
}
declaration(NODE) ::= predicate_definition(PD) SEMIC.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_DECLARATION;
    payload->alternative = ALT_PREDICATE_DEFINITION;

    NODE = tree_create_node(payload, 1, PD);
    stack_push(&allocated_nodes, NODE);
}
declaration(NODE) ::= function_definition(FD) SEMIC.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_DECLARATION;
    payload->alternative = ALT_FUNCTION_DEFINITION;

    NODE = tree_create_node(payload, 1, FD);
    stack_push(&allocated_nodes, NODE);
}

// event_declaration
event_declaration(NODE) ::= EVENT TYPE(T) LBRACE member_sequence(MS) RBRACE.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_DECLARATION;
    payload->alternative = ALT_MEMBER_SEQUENCE;
    payload->event_declaration.type[0] = strdup(T);
    payload->event_declaration.type[1] = NULL;
    payload->event_declaration.scope = NULL;
    payload->event_declaration.parent_ref = NULL;

    NODE = tree_create_node(payload, 1, MS);
    stack_push(&allocated_nodes, NODE);

    for (int i = 0; i < MS->childc; i++){
        struct payload *member_payload = MS->childv[i]->payload;
        char *key = member_payload->member.identifier;
        int *value = malloc(sizeof(int));
        *value = i;
        hashmap_put(&(payload->event_declaration.scope), key, value);
    }

    struct hashmap_entry *temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(T);
    temp->value = NODE;
    stack_push(&global_scope, temp);

    free((char *)T);
}
event_declaration(NODE) ::= EVENT TYPE(TL) EXTENDS TYPE(TR) LBRACE member_sequence(MS) RBRACE.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_DECLARATION;
    payload->alternative = ALT_MEMBER_SEQUENCE;
    payload->event_declaration.type[0] = strdup(TL);
    payload->event_declaration.type[1] = strdup(TR);
    payload->event_declaration.scope = NULL;
    payload->event_declaration.parent_ref = NULL;

    NODE = tree_create_node(payload, 1, MS);
    stack_push(&allocated_nodes, NODE);


    for (int i = 0; i < MS->childc; i++){
        struct payload *member_payload = MS->childv[i]->payload;
        char *key = member_payload->member.identifier;
        int *value = malloc(sizeof(int));
        *value = i;
        hashmap_put(&(payload->event_declaration.scope), key, value);
    }

    struct hashmap_entry *temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(TL);
    temp->value = NODE;
    stack_push(&global_scope, temp);

    free((char *)TL);
    free((char *)TR);
}

member_sequence(NODE) ::= member_sequence(MS) COMMA member(M).
{
    remove_topmost_node_of_type(&allocated_nodes, N_MEMBER_SEQUENCE);
    NODE = tree_append_node(MS, M);
    stack_push(&allocated_nodes, NODE);
}
member_sequence(NODE) ::= member(M).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MEMBER_SEQUENCE;
    payload->alternative = ALT_MEMBER;

    struct node *node = tree_create_node(payload, 1, M);
    stack_push(&allocated_nodes, node);
    NODE = node;
}

member(NODE) ::= IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MEMBER;
    payload->alternative = ALT_IDENTIFIER;
    payload->member.identifier = strdup(I);

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}

// rule_declaration
rule_declaration(NODE) ::= TYPE(T) COLON rule_signature(RS) RARROW IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_DECLARATION;
    payload->alternative = ALT_RULE_SIGNATURE;
    payload->rule_declaration.name = strdup(T);
    payload->rule_declaration.identifier = strdup(I);
    payload->rule_declaration.ref = NULL;
    payload->rule_declaration.eventc = 0;
    payload->rule_declaration.eventv = NULL;

    NODE = tree_create_node(payload, 1, RS);
    stack_push(&allocated_nodes, NODE);

    struct node *temp;
    while ((temp = stack_pop(&events)) != NULL) {
        payload->rule_declaration.eventc++;
        array_list_set(&(payload->rule_declaration.eventv), payload->rule_declaration.eventc - 1, temp);
    }

    free((char *)T);
    free((char *)I);
}

rule_signature(NODE) ::= LBRACKET event_sequence(ES) COLON predicate_sequence(PS) RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_SIGNATURE;
    payload->alternative = ALT_PREDICATE_SEQUENCE;

    NODE = tree_create_node(payload, 2, ES, PS);
    stack_push(&allocated_nodes, NODE);
}
rule_signature(NODE) ::= LBRACKET event_sequence(ES) RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_SIGNATURE;
    payload->alternative = ALT_EVENT_SEQUENCE;

    NODE = tree_create_node(payload, 1, ES);
    stack_push(&allocated_nodes, NODE);
}
rule_signature(NODE) ::= LBRACKET RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_SIGNATURE;
    payload->alternative = ALT_NONE;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);
}

event_sequence(NODE) ::= event_sequence(ES) COMMA event(E).
{
    remove_topmost_node_of_type(&allocated_nodes, N_EVENT_SEQUENCE);
    NODE = tree_append_node(ES, E);
    stack_push(&allocated_nodes, NODE);
}
event_sequence(NODE) ::= event(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_SEQUENCE;
    payload->alternative = ALT_EVENT;

    NODE = tree_create_node(payload, 1, E);
    stack_push(&allocated_nodes, NODE);
}
event(NODE) ::= TYPE(T).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT;
    payload->alternative = ALT_TYPE;
    payload->event.ref = NULL;
    payload->event.type = strdup(T);

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    stack_push(&events, NODE);

    free((char *)T);
}

predicate_sequence(NODE) ::= predicate_sequence(PS) COMMA predicate(P).
{
    remove_topmost_node_of_type(&allocated_nodes, N_PREDICATE_SEQUENCE);
    NODE = tree_append_node(PS, P);
    stack_push(&allocated_nodes, NODE);
}
predicate_sequence(NODE) ::= predicate(P).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PREDICATE_SEQUENCE;
    payload->alternative = ALT_PREDICATE;

    NODE = tree_create_node(payload, 1, P);
    stack_push(&allocated_nodes, NODE);
}

predicate(NODE) ::= IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PREDICATE;
    payload->alternative = ALT_IDENTIFIER;
    payload->predicate.ref = NULL;
    payload->predicate.identifier = strdup(I);

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}

// predicate_definiton
predicate_definition(NODE) ::= PREDICATE IDENTIFIER(I) LPAREN parameter_list(PL) RPAREN DEF expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PREDICATE_DEFINITION;
    payload->alternative = ALT_PARAMETER_LIST;
    payload->predicate_definition.identifier = strdup(I);
    payload->predicate_definition.scope = NULL;

    NODE = tree_create_node(payload, 2, PL, E);
    stack_push(&allocated_nodes, NODE);

    struct hashmap_entry *temp;
    while ((temp = stack_pop(&current_scope)) != NULL) {
        hashmap_put(&(payload->predicate_definition.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }

    temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);

    free((char *)I);
}

// function_definition
function_definition(NODE) ::= TYPE(T) IDENTIFIER(I) LPAREN RPAREN DEF expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_FUNCTION_DEFINITION;
    payload->alternative = ALT_EXPRESSION;
    payload->function_definition.type = strdup(T);
    payload->function_definition.identifier = strdup(I);
    payload->function_definition.scope = NULL;
    payload->function_definition.event_ref = NULL;

    NODE = tree_create_node(payload, 1, E);
    stack_push(&allocated_nodes, NODE);

    struct hashmap_entry *temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);

    free((char *)T);
    free((char *)I);
}
function_definition(NODE) ::= TYPE(T) IDENTIFIER(I) LPAREN parameter_list(PL) RPAREN DEF expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_FUNCTION_DEFINITION;
    payload->alternative = ALT_PARAMETER_LIST;
    payload->function_definition.type = strdup(T);
    payload->function_definition.identifier = strdup(I);
    payload->function_definition.scope = NULL;
    payload->function_definition.event_ref = NULL;

    NODE = tree_create_node(payload, 2, PL, E);
    stack_push(&allocated_nodes, NODE);

    struct hashmap_entry *temp;
    while ((temp = stack_pop(&current_scope)) != NULL) {
        hashmap_put(&(payload->function_definition.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }

    temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);

    free((char *)T);
    free((char *)I);
}

parameter_list(NODE) ::= parameter_list(PL) COMMA parameter(P).
{
    remove_topmost_node_of_type(&allocated_nodes, N_PARAMETER_LIST);
    NODE = tree_append_node(PL, P);
    stack_push(&allocated_nodes, NODE);
}
parameter_list(NODE) ::= parameter(P).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PARAMETER_LIST;
    payload->alternative = ALT_PARAMETER;

    NODE = tree_create_node(payload, 1, P);
    stack_push(&allocated_nodes, NODE);
}

parameter(NODE) ::= TYPE(T) IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PARAMETER;
    payload->alternative = ALT_IDENTIFIER;
    payload->parameter.type = strdup(T);
    payload->parameter.identifier = strdup(I);
    payload->parameter.event_ref = NULL;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    struct hashmap_entry *entry = malloc(sizeof(struct hashmap_entry));
    entry->key = strdup(I);
    entry->value = NODE;
    stack_push(&current_scope, entry);

    free((char *)T);
    free((char *)I);
}

// function_call
function_call(NODE) ::= IDENTIFIER(I) LPAREN argument_sequence(AS) RPAREN.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_FUNCTION_CALL;
    payload->alternative = ALT_ARGUMENT_SEQUENCE;
    payload->function_call.identifier = strdup(I);
    payload->function_call.ref = NULL;

    NODE = tree_create_node(payload, 1, AS);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}
function_call(NODE) ::= IDENTIFIER(I) LPAREN RPAREN.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_FUNCTION_CALL;
    payload->alternative = ALT_ARGUMENT_SEQUENCE;
    payload->function_call.identifier = strdup(I);
    payload->function_call.ref = NULL;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}

argument_sequence(NODE) ::= expression_sequence(ES).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ARGUMENT_SEQUENCE;
    payload->alternative = ALT_EXPRESSION_SEQUENCE;

    NODE = tree_create_node(payload, 1, ES);
    stack_push(&allocated_nodes, NODE);
}

// event_definition
event_definition(NODE) ::= LBRACE initializer_sequence(IS) RBRACE.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_DEFINITION;
    payload->alternative = ALT_INITIALIZER_SEQUENCE;

    NODE = tree_create_node(payload, 1, IS);
    stack_push(&allocated_nodes, NODE);
}

initializer_sequence(NODE) ::= initializer_sequence(IS) COMMA initializer(I).
{
    remove_topmost_node_of_type(&allocated_nodes, N_INITIALIZER_SEQUENCE);
    NODE = tree_append_node(IS, I);
    stack_push(&allocated_nodes, NODE);
}
initializer_sequence(NODE) ::= initializer(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_INITIALIZER_SEQUENCE;
    payload->alternative = ALT_INITIALIZER;

    NODE = tree_create_node(payload, 1, I);
    stack_push(&allocated_nodes, NODE);
}

initializer(NODE) ::= IDENTIFIER(I) ASSIGN expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_INITIALIZER;
    payload->alternative = ALT_EXPRESSION;
    payload->initializer.identifier = strdup(I);
    payload->initializer.ref_index = -1;

    NODE = tree_create_node(payload, 1, E);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}

// vector definition
vector(NODE) ::= LBRACKET component_sequence(CS) RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_VECTOR;
    payload->alternative = ALT_COMPONENT_SEQUENCE;

    NODE = tree_create_node(payload, 1, CS);
    stack_push(&allocated_nodes, NODE);
}

component_sequence(NODE) ::= expression_sequence(ES).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPONENT_SEQUENCE;
    payload->alternative = ALT_EXPRESSION_SEQUENCE;

    NODE = tree_create_node(payload, 1, ES);
    stack_push(&allocated_nodes, NODE);
}

// expression_sequence
expression_sequence(NODE) ::= expression_sequence(ES) COMMA expression(E).
{
    remove_topmost_node_of_type(&allocated_nodes, N_EXPRESSION_SEQUENCE);
    NODE = tree_append_node(ES, E);
    stack_push(&allocated_nodes, NODE);
}
expression_sequence(NODE) ::= expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EXPRESSION_SEQUENCE;
    payload->alternative = ALT_EXPRESSION;

    NODE = tree_create_node(payload, 1, E);
    stack_push(&allocated_nodes, NODE);
}

// expressions
expression(NODE) ::= comparison_expression(CE).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EXPRESSION;
    payload->alternative = ALT_COMPARISON_EXPRESSION;

    NODE = tree_create_node(payload, 1, CE);
    stack_push(&allocated_nodes, NODE);
}

comparison_expression(NODE) ::= additive_expression(AEL) EQ additive_expression(AER).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPARISON_EXPRESSION;
    payload->alternative = ALT_EQ;

    NODE = tree_create_node(payload, 2, AEL, AER);
    stack_push(&allocated_nodes, NODE);
}
comparison_expression(NODE) ::= additive_expression(AEL) NEQ additive_expression(AER).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPARISON_EXPRESSION;
    payload->alternative = ALT_NEQ;

    NODE = tree_create_node(payload, 2, AEL, AER);
    stack_push(&allocated_nodes, NODE);
}
comparison_expression(NODE) ::= additive_expression(AEL) GT additive_expression(AER).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPARISON_EXPRESSION;
    payload->alternative = ALT_GT;

    NODE = tree_create_node(payload, 2, AEL, AER);
    stack_push(&allocated_nodes, NODE);
}
comparison_expression(NODE) ::= additive_expression(AEL) LT additive_expression(AER).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPARISON_EXPRESSION;
    payload->alternative = ALT_LT;

    NODE = tree_create_node(payload, 2, AEL, AER);
    stack_push(&allocated_nodes, NODE);
}
comparison_expression(NODE) ::= additive_expression(AE).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_COMPARISON_EXPRESSION;
    payload->alternative = ALT_ADDITIVE_EXPRESSION;

    NODE = tree_create_node(payload, 1, AE);
    stack_push(&allocated_nodes, NODE);
}

additive_expression(NODE) ::= addition(A).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ADDITIVE_EXPRESSION;
    payload->alternative = ALT_ADDITION;

    NODE = tree_create_node(payload, 1, A);
    stack_push(&allocated_nodes, NODE);
}
additive_expression(NODE) ::= multiplicative_expression(ME).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ADDITIVE_EXPRESSION;
    payload->alternative = ALT_MULTIPLICATIVE_EXPRESSION;

    NODE = tree_create_node(payload, 1, ME);
    stack_push(&allocated_nodes, NODE);
}

addition(NODE) ::= additive_expression(AE) ADD multiplicative_expression(ME).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ADDITION;
    payload->alternative = ALT_ADD;

    NODE = tree_create_node(payload, 2, AE, ME);
    stack_push(&allocated_nodes, NODE);
}
addition(NODE) ::= additive_expression(AE) SUB multiplicative_expression(ME).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ADDITION;
    payload->alternative = ALT_SUB;

    NODE = tree_create_node(payload, 2, AE, ME);
    stack_push(&allocated_nodes, NODE);
}

multiplicative_expression(NODE) ::= multiplication(M).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MULTIPLICATIVE_EXPRESSION;
    payload->alternative = ALT_MULTIPLICATION;

    NODE = tree_create_node(payload, 1, M);
    stack_push(&allocated_nodes, NODE);
}
multiplicative_expression(NODE) ::= negation(N).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MULTIPLICATIVE_EXPRESSION;
    payload->alternative = ALT_NEGATION;

    NODE = tree_create_node(payload, 1, N);
    stack_push(&allocated_nodes, NODE);
}

multiplication(NODE) ::= multiplicative_expression(ME) MULT negation(N).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MULTIPLICATION;
    payload->alternative = ALT_MULT;

    NODE = tree_create_node(payload, 2, ME, N);
    stack_push(&allocated_nodes, NODE);
}
multiplication(NODE) ::= multiplicative_expression(ME) DIV negation(N).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MULTIPLICATION;
    payload->alternative = ALT_DIV;

    NODE = tree_create_node(payload, 2, ME, N);
    stack_push(&allocated_nodes, NODE);
}

negation(NODE) ::= SUB negation(N).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_NEGATION;
    payload->alternative = ALT_NEGATION;

    NODE = tree_create_node(payload, 1, N);
    stack_push(&allocated_nodes, NODE);
}
negation(NODE) ::= primary_expression(PE).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_NEGATION;
    payload->alternative = ALT_PRIMARY_EXPRESSION;

    NODE = tree_create_node(payload, 1, PE);
    stack_push(&allocated_nodes, NODE);
}

primary_expression(NODE) ::= LPAREN expression(E) RPAREN.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PRIMARY_EXPRESSION;
    payload->alternative = ALT_EXPRESSION;

    NODE = tree_create_node(payload, 1, E);
    stack_push(&allocated_nodes, NODE);
}
primary_expression(NODE) ::= atomic(A).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PRIMARY_EXPRESSION;
    payload->alternative = ALT_ATOMIC;

    NODE = tree_create_node(payload, 1, A);
    stack_push(&allocated_nodes, NODE);
}

atomic(NODE) ::= IDENTIFIER(IL) DOT IDENTIFIER(IR).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_IDENTIFIER;
    payload->atomic.identifier[0] = strdup(IL);
    payload->atomic.identifier[1] = strdup(IR);
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)IL);
    free((char *)IR);
}
atomic(NODE) ::= IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_IDENTIFIER;
    payload->atomic.identifier[0] = strdup(I);
    payload->atomic.identifier[1] = NULL;
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)I);
}
atomic(NODE) ::= NUMBER(N).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_NUMBER;
    payload->atomic.number = atof(N);
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);

    free((char *)N);
}
atomic(NODE) ::= vector(V).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_VECTOR;
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 1, V);
    stack_push(&allocated_nodes, NODE);
}
atomic(NODE) ::= event_definition(ED).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_EVENT_DEFINITION;
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 1, ED);
    stack_push(&allocated_nodes, NODE);
}
atomic(NODE) ::= function_call(FC).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_FUNCTION_CALL;
    payload->atomic.ref = NULL;

    NODE = tree_create_node(payload, 1, FC);
    stack_push(&allocated_nodes, NODE);
}
