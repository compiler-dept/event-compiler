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


struct node *resolve_reference(struct node *node, const char *id)
{
    struct node *scope = node;
    struct node *ref = NULL;
    while ((scope = find_scope_for(scope)) != NULL && ref == NULL) {
        struct payload *temp = (struct payload *)scope->payload;
        switch (temp->type) {
            case N_FUNCTION_DEFINITION:
                ref = hashmap_get(temp->function_definition.scope, id);
                break;
            case N_PREDICATE_DEFINITION:
                ref = hashmap_get(temp->predicate_definition.scope, id);
                break;
            case N_TRANSLATION_UNIT:
                ref = hashmap_get(temp->translation_unit.scope, id);
                break;
            default:
                ref = NULL;
        }
    }
    return ref;
}

int index_of_id(struct node *parent, char *type, char *id) {
    struct node *ref_node = resolve_reference(parent, type);
    if (ref_node) {
        struct payload *ref_payload = ref_node->payload;
        int *idx = hashmap_get(ref_payload->event_declaration.scope, id);
        if (idx) {
            return *idx;
        } else if (ref_payload->event_declaration.type[1] != NULL) {
            return index_of_id(parent, ref_payload->event_declaration.type[1], id);
        }
    }

    return -1;
}

void link_references(struct node *node)
{
    struct tree_iterator *it = tree_iterator_init(&node, POSTORDER);

    struct node *temp = NULL;
    while ((temp = tree_iterator_next(it)) != NULL) {
        char *id = NULL;

        if (((struct payload *)temp->payload)->type == N_ATOMIC) {
            if (((struct payload *)temp->payload)->alternative == ALT_IDENTIFIER) {
                id = ((struct payload *)temp->payload)->atomic.identifier[0];
                ((struct payload *)temp->payload)->atomic.ref = resolve_reference(temp, id);
            }
        } else if (((struct payload *)temp->payload)->type == N_FUNCTION_CALL) {
            id = ((struct payload *)temp->payload)->function_call.identifier;
            ((struct payload *)temp->payload)->function_call.ref = resolve_reference(temp, id);
        } else if (((struct payload *)temp->payload)->type == N_PREDICATE) {
            id = ((struct payload *)temp->payload)->predicate.identifier;
            ((struct payload *)temp->payload)->predicate.ref = resolve_reference(temp, id);
        } else if (((struct payload *)temp->payload)->type == N_RULE_DECLARATION) {
            id = ((struct payload *)temp->payload)->rule_declaration.identifier;
            ((struct payload *)temp->payload)->rule_declaration.ref = resolve_reference(temp, id);
        } else if (((struct payload *)temp->payload)->type == N_INITIALIZER) {
            struct node *parent = temp->parent;
            while (parent && ((struct payload *)parent->payload)->type != N_FUNCTION_DEFINITION) {
                parent = parent->parent;
            }

            if (parent != NULL) {
                char *type = ((struct payload *)parent->payload)->function_definition.type;
                id = ((struct payload *)temp->payload)->initializer.identifier;
                ((struct payload *)temp->payload)->initializer.ref_index = index_of_id(parent, type, id);
            }
        }
    }

    tree_iterator_free(it);
}
