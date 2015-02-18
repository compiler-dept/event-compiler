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

struct node *resolve_reference(struct node *scope, const char *id){
    struct payload *temp = (struct payload *)scope->payload;
    switch (temp->type){
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

void link_references(struct node *node){
    struct tree_iterator *it = tree_iterator_init(&node, POSTORDER);

    struct node *temp = NULL;
    while ((temp = tree_iterator_next(it)) != NULL) {
        if (((struct payload *)temp->payload)->type == N_ATOMIC){
            if (((struct payload *)temp->payload)->alternative == ALT_IDENTIFIER){
                struct node *scope = find_scope_for(temp);
                const char *id = ((struct payload *)temp->payload)->atomic.identifier[0];
                ((struct payload *)temp->payload)->atomic.ref = resolve_reference(scope, id);
            }
        }
    }

    tree_iterator_free(it);
}
