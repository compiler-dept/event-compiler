#include <CUnit/Basic.h>
#include <stdio.h>

/* Test includes */
#include "parser_tests.h"

#define TEXTIFY(x) #x
#define ADD_TEST(x) !CU_add_test(suite, TEXTIFY(x), x)

int init_suite(void)
{
    return 0;
}

int clean_suite(void)
{
    return 0;
}

int main(void)
{
    CU_pSuite suite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    suite = CU_add_suite("event-compiler testsuite", init_suite, clean_suite);
    if (NULL == suite) {
        CU_cleanup_registry();

        return CU_get_error();
    }

    if (
        ADD_TEST(test_event_inheritance) ||
        ADD_TEST(test_constant_function_definition) ||
        ADD_TEST(test_function_definition) ||
        ADD_TEST(test_function_definition_function)
    ) {
        CU_cleanup_registry();

        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    int failures = CU_get_number_of_failures();

    CU_cleanup_registry();

    int ret = CU_get_error();
    if (failures > 0) {
        ret = -1;
    }

    return ret;
}
