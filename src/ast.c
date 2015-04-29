#include <stdlib.h>
#include <stdio.h>
#include <hashmap.h>
#include "ast.h"

void payload_free(void *payload)
{
    struct payload *temp_payload = payload;
    if (temp_payload) {
        switch (temp_payload->type) {
            case N_TRANSLATION_UNIT:
                hashmap_free(temp_payload->translation_unit.scope, NULL);
                break;
            case N_EVENT_DECLARATION:
                if (temp_payload->alternative == ALT_MEMBER_SEQUENCE) {
                    free(temp_payload->event_declaration.type[0]);
                    if (temp_payload->event_declaration.type[1]) {
                        free(temp_payload->event_declaration.type[1]);
                    }
                    hashmap_free(temp_payload->event_declaration.scope, free);
                }
                break;
            case N_MEMBER:
                /*if (temp_payload->alternative == ALT_IDENTIFIER) {
                    free(temp_payload->member.identifier);
                }*/
                break;
            case N_RULE_DECLARATION:
                if (temp_payload->alternative == ALT_RULE_SIGNATURE) {
                    free(temp_payload->rule_declaration.name);
                    free(temp_payload->rule_declaration.identifier);
                }
                break;
            case N_PREDICATE_DEFINITION:
                if (temp_payload->alternative == ALT_EXPRESSION ||
                        temp_payload->alternative == ALT_PARAMETER_LIST) {
                    free(temp_payload->predicate_definition.identifier);
                    hashmap_free(temp_payload->predicate_definition.scope, NULL);
                }
                break;
            case N_FUNCTION_DEFINITION:
                if (temp_payload->alternative == ALT_EXPRESSION ||
                        temp_payload->alternative == ALT_PARAMETER_LIST) {
                    free(temp_payload->function_definition.type);
                    free(temp_payload->function_definition.identifier);
                    hashmap_free(temp_payload->function_definition.scope, NULL);
                }
                break;
            case N_PARAMETER:
                if (temp_payload->alternative == ALT_IDENTIFIER) {
                    free(temp_payload->parameter.type);
                    free(temp_payload->parameter.identifier);
                }
                break;
            case N_PREDICATE:
                if (temp_payload->alternative == ALT_IDENTIFIER) {
                    free(temp_payload->predicate.identifier);
                }
                break;
            case N_EVENT:
                if (temp_payload->alternative == ALT_TYPE) {
                    free(temp_payload->event.type);
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
