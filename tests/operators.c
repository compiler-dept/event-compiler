#include "clar.h"

#include <stdio.h>
#include <operators.h>

void test_operators__op_v_eq_v(void)
{
    struct vector *vector1 = new_vector(3, 1.0, 2.0, 3.0);
    struct vector *vector2 = new_vector(3, 1.0, 2.0, 3.0);

    uint8_t result = op_v_eq_v(vector1, vector2);

    cl_assert_equal_i(result, 1);

    free(vector1);
    free(vector2);
}

void test_operators__op_v_add_v(void)
{
    struct vector *vector1 = new_vector(3, 1.0, 2.0, 3.0);
    struct vector *vector2 = new_vector(3, 1.0, 2.0, 3.0);
    struct vector *vector3 = new_vector(3, 2.0, 4.0, 6.0);

    struct vector *vresult = op_v_add_v(vector1, vector2);
    uint8_t result = op_v_eq_v(vresult, vector3);

    cl_assert_equal_i(result, 1);

    free(vector1);
    free(vector2);
    free(vector3);
    free(vresult);
}

void test_operators__op_v_sub_v(void)
{
    struct vector *vector1 = new_vector(3, 1.0, 2.0, 3.0);
    struct vector *vector2 = new_vector(3, 1.0, 2.0, 3.0);
    struct vector *vector3 = new_vector(3, 0.0, 0.0, 0.0);

    struct vector *vresult = op_v_sub_v(vector1, vector2);
    uint8_t result = op_v_eq_v(vresult, vector3);

    cl_assert_equal_i(result, 1);

    free(vector1);
    free(vector2);
    free(vector3);
    free(vresult);
}
