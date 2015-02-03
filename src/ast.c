#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

void payload_free(void *payload)
{
    struct payload *temp_payload = payload;
    if (temp_payload) {
        switch (temp_payload->type) {
            case N_EVENT_INHERITANCE:
                if (temp_payload->alternative == ALT_TYPE) {
                    free(temp_payload->event_inheritance.type[0]);
                    free(temp_payload->event_inheritance.type[1]);
                }
                break;
            case N_RULE_DECLARATION:
                if (temp_payload->alternative == ALT_RULE_SIGNATURE) {
                    free(temp_payload->rule_declaration.type);
                    free(temp_payload->rule_declaration.identifier);
                }
                break;
            case N_EVENT_SEQUENCE:
                if (temp_payload->alternative == ALT_TYPE) {
                    for (int i = 0; i < temp_payload->event_sequence.count; i++) {
                        free(temp_payload->event_sequence.type[i]);
                    }
                    free(temp_payload->event_sequence.type);
                }
                break;
            case N_FUNCTION_DEFINITION:
                if (temp_payload->alternative == ALT_EXPRESSION ||
                    temp_payload->alternative == ALT_PARAMETER_LIST) {
                    free(temp_payload->function_definition.type);
                    free(temp_payload->function_definition.identifier);
                }
                break;
            case N_PARAMETER:
                if (temp_payload->alternative == ALT_IDENTIFIER) {
                    free(temp_payload->parameter.type);
                    free(temp_payload->parameter.identifier);
                }
                break;
            case N_PREDICATE_SEQUENCE:
                if (temp_payload->alternative == ALT_IDENTIFIER) {
                    for (int i = 0; i < temp_payload->predicate_sequence.count; i++) {
                        free(temp_payload->predicate_sequence.identifier[i]);
                    }
                    free(temp_payload->predicate_sequence.identifier);
                }
                break;
            case N_FUNCTION_CALL:
                if (temp_payload->alternative == ALT_ARGUMENT_SEQUENCE) {
                    free(temp_payload->function_call.identifier);
                }
                break;
            case N_INITIALIZER:
                if (temp_payload->alternative == ALT_EXPRESSION) {
                    free(temp_payload->initializer.identifier);
                }
                break;
            case N_ATOMIC:
                if (temp_payload->alternative == ALT_IDENTIFIER) {
                    free(temp_payload->atomic.identifier[0]);
                    free(temp_payload->atomic.identifier[1]);
                }
                break;
        }

        free(temp_payload);
    }
}
