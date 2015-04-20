%include
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <string.h>
    #include <tree.h>
    #include <stack.h>
    #include <hashmap.h>
    #include "ast.h"
    #include "parser_state.h"

    struct stack *allocated_nodes = NULL;
    struct stack *current_scope = NULL;
    struct stack *global_scope = NULL;
}

%token_type { const char * }
%token_destructor {
    free((char *)$$);
    parser_state->state = parser_state->state;
}

%type translation_unit { struct node * }
%type declaration_sequence { struct node * }
%type declaration { struct node * }
%type event_declaration { struct node * }
%type member_sequence { struct node * }
%type rule_declaration { struct node * }
%type rule_signature { struct node * }
%type event_sequence { struct node * }
%type predicate_sequence { struct node * }
%type predicate { struct node * }
%type predicate_definition { struct node * }
%type function_definition { struct node * }
%type parameter_list { struct node * }
%type parameter { struct node * }
%type function_call { struct node * }
%type argument_sequence { struct node * }
%type event_definition { struct node * }
%type initializer_sequence { struct node * }
%type initializer { struct node * }
%type vector { struct node * }
%type component_sequence { struct node * }
%type expression_sequence { struct node * }
%type expression { struct node * }
%type comparison_expression { struct node * }
%type additive_expression { struct node * }
%type addition { struct node * }
%type multiplicative_expression { struct node * }
%type multiplication { struct node * }
%type negation { struct node * }
%type primary_expression { struct node * }
%type atomic { struct node * }

%extra_argument { struct parser_state *parser_state }

%syntax_error
{
    fprintf(stderr, "%s\n", "Error parsing input.");
    parser_state->state = ERROR;
}

translation_unit(NODE) ::= declaration_sequence(DS).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_TRANSLATION_UNIT;
    payload->alternative = ALT_DECLARATION_SEQUENCE;
    payload->translation_unit.scope = NULL;
    struct hashmap_entry *temp;
    while ((temp = stack_pop(&global_scope)) != NULL){
        hashmap_put(&(payload->translation_unit.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }
    NODE = tree_create_node(payload, 1, DS);
    parser_state->root = NODE;
    parser_state->state = OK;
    while (stack_pop(&allocated_nodes));
}
translation_unit ::= error.
{
    struct node *temp = NULL;
    while ((temp = stack_pop(&allocated_nodes)) != NULL) {
        payload_free(temp->payload);
        free(temp);
    }
    parser_state->state = ERROR;
    parser_state->root = NULL;
}

declaration_sequence(NODE) ::= declaration_sequence(DS) declaration(D).
{
    NODE = malloc(sizeof(struct node) + sizeof(struct node *) * (DS->childc + 1));
    NODE->payload = DS->payload;
    NODE->childc = DS->childc + 1;
    memcpy(NODE->childv, DS->childv, sizeof(struct node *) * DS->childc);
    NODE->childv[NODE->childc - 1] = D;
    for (int i = 0; i < NODE->childc; i++) {
        NODE->childv[i]->parent = NODE;
    }
    stack_pop(&allocated_nodes);
    stack_push(&allocated_nodes, NODE);
    free(DS);
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
    NODE = tree_create_node(payload, 1, MS);
    stack_push(&allocated_nodes, NODE);
    free((char *)T);
}
event_declaration(NODE) ::= EVENT TYPE(TL) EXTENDS TYPE(TR) LBRACE member_sequence(MS) RBRACE.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_DECLARATION;
    payload->alternative = ALT_MEMBER_SEQUENCE;
    payload->event_declaration.type[0] = strdup(TL);
    payload->event_declaration.type[1] = strdup(TR);
    NODE = tree_create_node(payload, 1, MS);
    stack_push(&allocated_nodes, NODE);
    free((char *)TL);
    free((char *)TR);
}

member_sequence(NODE) ::= member_sequence(MS) COMMA IDENTIFIER(I).
{
    struct payload *payload = MS->payload;
    payload->member_sequence.count += 1;
    payload->member_sequence.identifier =
    realloc(payload->member_sequence.identifier,
        payload->member_sequence.count * sizeof(char *));
    payload->member_sequence.identifier[payload->member_sequence.count - 1] =
        strdup(I);
    NODE = MS;
    free((char *)I);
}
member_sequence(NODE) ::= IDENTIFIER(I).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_MEMBER_SEQUENCE;
    payload->alternative = ALT_IDENTIFIER;
    payload->member_sequence.count = 1;
    payload->member_sequence.identifier = malloc(sizeof(char *));
    payload->member_sequence.identifier[0] = strdup(I);
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
    payload->rule_declaration.ref = NULL;
    payload->rule_declaration.name = strdup(T);
    payload->rule_declaration.identifier = strdup(I);
    NODE = tree_create_node(payload, 1, RS);
    stack_push(&allocated_nodes, NODE);
    free((char *)T);
    free((char *)I);
}

rule_signature(NODE) ::= LBRACKET event_sequence(ES) COLON predicate_sequence(PS) RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_SIGNATURE;
    payload->alternative = ALT_EVENT_SEQUENCE;
    NODE = tree_create_node(payload, 2, ES, PS);
    stack_push(&allocated_nodes, NODE);
}
rule_signature(NODE) ::= LBRACKET predicate_sequence(PS) RBRACKET.
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_RULE_SIGNATURE;
    payload->alternative = ALT_PREDICATE_SEQUENCE;
    NODE = tree_create_node(payload, 1, PS);
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

event_sequence(NODE) ::= event_sequence(ES) COMMA TYPE(T).
{
    struct payload *payload = ES->payload;
    payload->event_sequence.count += 1;
    payload->event_sequence.type =
        realloc(payload->event_sequence.type,
            payload->event_sequence.count * sizeof(char *));
    payload->event_sequence.type[payload->event_sequence.count - 1] = strdup(T);
    NODE = ES;
    free((char *)T);
}
event_sequence(NODE) ::= TYPE(T).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_EVENT_SEQUENCE;
    payload->alternative = ALT_TYPE;
    payload->event_sequence.count = 1;
    payload->event_sequence.type = malloc(sizeof(char *));
    payload->event_sequence.type[0] = strdup(T);
    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);
    free((char *)T);
}

predicate_sequence(NODE) ::= predicate_sequence(PS) COMMA predicate(P).
{
    NODE = malloc(sizeof(struct node) + ((PS->childc + 1) * sizeof(struct node *)));
    NODE->payload = PS->payload;
    NODE->childc = PS->childc + 1;
    memcpy(NODE->childv, PS->childv, PS->childc * sizeof(struct node *));
    NODE->childv[NODE->childc - 1] = P;
    for (int i = 0; i < NODE->childc; i++) {
        NODE->childv[i]->parent = NODE;
    }
    stack_pop(&allocated_nodes);
    free(PS);
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
predicate_definition(NODE) ::= PREDICATE IDENTIFIER(I) LPAREN RPAREN DEF expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PREDICATE_DEFINITION;
    payload->alternative = ALT_EXPRESSION;
    payload->predicate_definition.identifier = strdup(I);
    payload->predicate_definition.scope = NULL;
    NODE = tree_create_node(payload, 1, E);
    struct hashmap_entry *temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);
    stack_push(&allocated_nodes, NODE);
    free((char *)I);
}
predicate_definition(NODE) ::= PREDICATE IDENTIFIER(I) LPAREN parameter_list(PL) RPAREN DEF expression(E).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_PREDICATE_DEFINITION;
    payload->alternative = ALT_PARAMETER_LIST;
    payload->predicate_definition.identifier = strdup(I);
    payload->predicate_definition.scope = NULL;
    struct hashmap_entry *temp;
    while ((temp = stack_pop(&current_scope)) != NULL){
        hashmap_put(&(payload->predicate_definition.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }
    NODE = tree_create_node(payload, 2, PL, E);
    temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);
    stack_push(&allocated_nodes, NODE);
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
    NODE = tree_create_node(payload, 1, E);
    struct hashmap_entry *temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);
    stack_push(&allocated_nodes, NODE);
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
    struct hashmap_entry *temp;
    while ((temp = stack_pop(&current_scope)) != NULL){
        hashmap_put(&(payload->function_definition.scope), temp->key, temp->value);
        free(temp->key);
        free(temp);
    }
    NODE = tree_create_node(payload, 2, PL, E);
    temp = malloc(sizeof(struct hashmap_entry));
    temp->key = strdup(I);
    temp->value = NODE;
    stack_push(&global_scope, temp);
    stack_push(&allocated_nodes, NODE);
    free((char *)T);
    free((char *)I);
}

parameter_list(NODE) ::= parameter_list(PL) COMMA parameter(P).
{
    NODE = malloc(sizeof(struct node) + sizeof(struct node *) * (PL->childc + 1));
    NODE->payload = PL->payload;
    NODE->childc = PL->childc + 1;
    memcpy(NODE->childv, PL->childv, sizeof(struct node *) * PL->childc);
    NODE->childv[NODE->childc - 1] = P;
    for (int i = 0; i < NODE->childc; i++) {
        NODE->childv[i]->parent = NODE;
    }
    stack_pop(&allocated_nodes);
    stack_push(&allocated_nodes, NODE);
    free(PL);
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
    NODE = tree_create_node(payload, 0);

    struct hashmap_entry *entry = malloc(sizeof(struct hashmap_entry));
    entry->key = strdup(I);
    entry->value = NODE;
    stack_push(&current_scope, entry);

    stack_push(&allocated_nodes, NODE);
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
    NODE = tree_create_node(payload, 1, AS);
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
    NODE = malloc(sizeof(struct node) + sizeof(struct node *) * (IS->childc + 1));
    NODE->payload = IS->payload;
    NODE->childc = IS->childc + 1;
    memcpy(NODE->childv, IS->childv, sizeof(struct node *) * IS->childc);
    NODE->childv[NODE->childc - 1] = I;
    for (int i = 0; i < NODE->childc; i++) {
        NODE->childv[i]->parent = NODE;
    }
    stack_pop(&allocated_nodes);
    stack_push(&allocated_nodes, NODE);
    free(IS);
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
    NODE = malloc(sizeof(struct node) + sizeof(struct node *) * (ES->childc + 1));
    NODE->payload = ES->payload;
    NODE->childc = ES->childc + 1;
    memcpy(NODE->childv, ES->childv, sizeof(struct node *) * ES->childc);
    NODE->childv[NODE->childc - 1] = E;
    for (int i = 0; i < NODE->childc; i++) {
        NODE->childv[i]->parent = NODE;
    }
    stack_pop(&allocated_nodes);
    stack_push(&allocated_nodes, NODE);
    free(ES);
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
    NODE = tree_create_node(payload, 0);
    stack_push(&allocated_nodes, NODE);
    free((char *)N);
}
atomic(NODE) ::= vector(V).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_VECTOR;
    NODE = tree_create_node(payload, 1, V);
    stack_push(&allocated_nodes, NODE);
}
atomic(NODE) ::= event_definition(ED).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_EVENT_DEFINITION;
    NODE = tree_create_node(payload, 1, ED);
    stack_push(&allocated_nodes, NODE);
}
atomic(NODE) ::= function_call(FC).
{
    struct payload *payload = malloc(sizeof(struct payload));
    payload->type = N_ATOMIC;
    payload->alternative = ALT_FUNCTION_CALL;
    NODE = tree_create_node(payload, 1, FC);
    stack_push(&allocated_nodes, NODE);
}
