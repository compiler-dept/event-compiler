#include <compiler.h>

void test_integration_event_inheritance(void)
{
	struct node *root =
	    parse_ast("Alfred extends Bazinga; Bazinga extends Carlos;");

    CU_ASSERT_PTR_NOT_NULL(root);

	tree_free(&root, payload_free);

}
