#include "scope.h"
#include <tree.h>
#include <hashmap.h>
#include "ast.h"

struct node *find_scope_for(struct node *node)
{
    struct node *temp = node;
    while ((temp = temp->parent) != NULL) {
        if (((struct payload *)temp->payload)->type == N_FUNCTION_DEFINITION
                || ((struct payload *)temp->payload)->type == N_PREDICATE_DEFINITION
                || ((struct payload *)temp->payload)->type == N_TRANSLATION_UNIT) {
            return temp;
        }
    }
    return NULL;
}

struct node *resolve_reference(struct node *scope, const char *id)
{
    struct payload *temp = (struct payload *)scope->payload;
    switch (temp->type) {
        case N_FUNCTION_DEFINITION:
            return hashmap_get(temp->function_definition.scope, id);
        case N_PREDICATE_DEFINITION:
            return hashmap_get(temp->predicate_definition.scope, id);
        case N_TRANSLATION_UNIT:
            return hashmap_get(temp->translation_unit.scope, id);
        default:
            return NULL;
    }
}

void link_references(struct node *node)
{
    struct tree_iterator *it = tree_iterator_init(&node, POSTORDER);

    struct node *temp = NULL;
    while ((temp = tree_iterator_next(it)) != NULL) {
        struct node *scope = find_scope_for(temp);
        char *id = NULL;

        if (((struct payload *)temp->payload)->type == N_ATOMIC) {
            if (((struct payload *)temp->payload)->alternative == ALT_IDENTIFIER) {
                id = ((struct payload *)temp->payload)->atomic.identifier[0];
                ((struct payload *)temp->payload)->atomic.ref = resolve_reference(scope, id);
            }
        } else if (((struct payload *)temp->payload)->type == N_FUNCTION_CALL) {
            id = ((struct payload *)temp->payload)->function_call.identifier;
            ((struct payload *)temp->payload)->function_call.ref = resolve_reference(scope, id);
        } else if (((struct payload *)temp->payload)->type == N_PREDICATE) {
            id = ((struct payload *)temp->payload)->predicate.identifier;
            ((struct payload *)temp->payload)->predicate.ref = resolve_reference(scope, id);
        } else if (((struct payload *)temp->payload)->type == N_RULE_DECLARATION) {
            id = ((struct payload *)temp->payload)->rule_declaration.identifier;
            ((struct payload *)temp->payload)->rule_declaration.ref = resolve_reference(scope, id);
        }
    }

    tree_iterator_free(it);
}
