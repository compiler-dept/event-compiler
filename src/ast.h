#ifndef AST_H
#define AST_H

enum type {
    N_TRANSLATION_UNIT = 1,
    N_DECLARATION_SEQUENCE,
    N_DECLARATION,
    N_EVENT_INHERITANCE,
    N_RULE_DECLARATION,
    N_RULE_SIGNATURE,
    N_EVENT_SEQUENCE,
    N_PREDICATE_SEQUENCE,
    N_PREDICATE_DEFINITION,
    N_FUNCTION_DEFINITION,
    N_PARAMETER_LIST,
    N_PARAMETER,
    N_FUNCTION_CALL,
    N_ARGUMENT_SEQUENCE,
    N_EVENT_DEFINITION,
    N_INITIALIZER_SEQUENCE,
    N_INITIALIZER,
    N_VECTOR,
    N_COMPONENT_SEQUENCE,
    N_EXPRESSION_SEQUENCE,
    N_COMPARISON_EXPRESSION,
    N_EXPRESSION,
    N_ADDITIVE_EXPRESSION,
    N_ADDITION,
    N_MULTIPLICATIVE_EXPRESSION,
    N_MULTIPLICATION,
    N_NEGATION,
    N_PRIMARY_EXPRESSION,
    N_ATOMIC
};

enum alternative {
    ALT_DECLARATION_SEQUENCE = 1,
    ALT_DECLARATION,
    ALT_EVENT_INHERITANCE,
    ALT_RULE_DECLARATION,
    ALT_PREDICATE_DEFINITION,
    ALT_FUNCTION_DEFINITION,
    ALT_RULE_SIGNATURE,
    ALT_VECTOR,
    ALT_IDENTIFIER,
    ALT_EVENT_DEFINITION,
    ALT_FUNCTION_CALL,
    ALT_EXPRESSION,
    ALT_COMPARISON_EXPRESSION,
    ALT_ATOMIC,
    ALT_NEGATION,
    ALT_MULT,
    ALT_DIV,
    ALT_MULTIPLICATION,
    ALT_ADD,
    ALT_SUB,
    ALT_EQ,
    ALT_NEQ,
    ALT_GT,
    ALT_LT,
    ALT_ADDITION,
    ALT_MULTIPLICATIVE_EXPRESSION,
    ALT_ADDITIVE_EXPRESSION,
    ALT_EXPRESSION_SEQUENCE,
    ALT_COMPONENT_SEQUENCE,
    ALT_INITIALIZER,
    ALT_INITIALIZER_SEQUENCE,
    ALT_ARGUMENT_SEQUENCE,
    ALT_PARAMETER,
    ALT_PARAMETER_LIST,
    ALT_TYPE,
    ALT_EVENT_SEQUENCE,
    ALT_NONE,
    ALT_PREDICATE_SEQUENCE,
    ALT_NUMBER,
    ALT_PRIMARY_EXPRESSION
};

struct payload {
    enum type type;
    enum alternative alternative;

    union {
        struct {
            double number;
            char *identifier[2];
        } atomic;
        struct {
            char *identifier;
        } initializer;
        struct {
            char *identifier;
        } function_call;
        struct {
            char *type;
            char *identifier;
        } function_definition;
        struct {
            char *identifier;
        } predicate_definition;
        struct {
            char *type;
            char *identifier;
        } parameter;
        struct {
            char *name;
            char *identifier;
        } rule_declaration;
        struct {
            char *type[2];
        } event_inheritance;
        struct {
            int count;
            char **identifier;
        } predicate_sequence;
        struct {
            int count;
            char **type;
        } event_sequence;
    };
};

void payload_free(void *payload);

#endif
