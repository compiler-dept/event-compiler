#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "operators.h"

uint16_t maximum(uint16_t vector_left, uint16_t vector_right)
{
    return vector_left == vector_right ? vector_left : (vector_left < vector_right ? vector_right : vector_left);
}

uint16_t minimum(uint16_t vector_left, uint16_t vector_right)
{
    return vector_left == vector_right ? vector_left : (vector_left < vector_right ? vector_left : vector_right);
}

struct vector *new_vector(int size, ...)
{
    struct vector *vector = malloc(sizeof(struct vector));
    vector->size = size;
    vector->components = malloc(size * sizeof(double));

    va_list ap;

    va_start(ap, size);

    for (int i = 0; i < size; i++) {
        vector->components[i] = va_arg(ap, double);
    }

    va_end(ap);

    return vector;
}

struct vector *new_vector_from_array(int size, double *array)
{
    struct vector *vector = malloc(sizeof(struct vector) + size * sizeof(double));
    vector->size = size;

    for (int i = 0; i < size; i++) {
        vector->components[i] = array[i];
    }

    return vector;
}

void print_vector(struct vector *vector)
{
    printf("[");

    for (int i = 0; i < vector->size; i++) {
        if (i == vector->size - 1) {
            printf("%f", vector->components[i]);
        } else {
            printf("%f, ", vector->components[i]);
        }
    }

    printf("]\n");
}

uint8_t op_v_eq_v(struct vector *vector_left, struct vector *vector_right)
{
    if (vector_left == NULL || vector_right == NULL) {
        return 0;
    }

    if (vector_left->size != vector_right->size) {
        /* vectors have not the same length */
        return 0;
    } else {
        /* vectors have the same length */

        /* loop over components */
        for (int i = 0; i < vector_left->size; i++) {
            if (vector_left->components[i] != vector_right->components[i]) {
                /* components are not equal */
                return 0;
            }
        }

        return 1;
    }
}

uint8_t op_v_neq_v(struct vector *vector_left, struct vector *vector_right)
{
    return op_v_eq_v(vector_left, vector_right) == 1 ? 0 : 1;
}

struct vector *op_v_add_v(struct vector *vector_left, struct vector *vector_right)
{
    if (vector_left == NULL || vector_right == NULL) {
        return NULL;
    }

    uint16_t min = minimum(vector_left->size, vector_right->size);
    uint16_t max = maximum(vector_left->size, vector_right->size);

    struct vector *vector = malloc(sizeof(struct vector));
    vector->size = max;
    vector->components = malloc(max * sizeof(double));

    for (int i = 0; i < min; i++) {
        vector->components[i] = vector_left->components[i] + vector_right->components[i];
    }

    for (int i = min; i < max; i++) {
        vector->components[i] =
            vector_left->size < vector_right->size ?
            vector_right->components[i] : vector_left->components[i];
    }

    return vector;
}

struct vector *op_v_sub_v(struct vector *vector_left, struct vector *vector_right)
{
    if (vector_left == NULL || vector_right == NULL) {
        return NULL;
    }

    uint16_t min = minimum(vector_left->size, vector_right->size);
    uint16_t max = maximum(vector_left->size, vector_right->size);

    struct vector *vector = malloc(sizeof(struct vector));
    vector->size = max;
    vector->components = malloc(max * sizeof(double));

    for (int i = 0; i < min; i++) {
        vector->components[i] = vector_left->components[i] - vector_right->components[i];
    }

    for (int i = min; i < max; i++) {
        vector->components[i] =
            vector_left->size < vector_right->size ?
            -1 * vector_right->components[i] : vector_left->components[i];
    }

    return vector;
}

struct vector *op_s_mult_v(double scalar, struct vector *vector)
{
    if (vector == NULL) {
        return NULL;
    }

    uint16_t max = vector->size;

    struct vector *ret_vector = malloc(sizeof(struct vector));
    ret_vector->size = max;
    ret_vector->components = malloc(max * sizeof(double));

    for (int i = 0; i < max; i++) {
        ret_vector->components[i] = vector->components[i] * scalar;
    }

    return ret_vector;
}

uint8_t op_v_lt_v(struct vector *vector_left, struct vector *vector_right)
{
    if (vector_left == NULL || vector_right == NULL) {
        return 0;
    }

    uint16_t min = minimum(vector_left->size, vector_right->size);

    for (int i = 0; i < min; i++) {
        if (vector_left->components[i] >= vector_right->components[i]) {
            return 0;
        }
    }

    return 1;
}

uint8_t op_v_gt_v(struct vector *vector_left, struct vector *vector_right)
{
    if (vector_left == NULL || vector_right == NULL) {
        return 0;
    }

    uint16_t min = minimum(vector_left->size, vector_right->size);

    for (int i = 0; i < min; i++) {
        if (vector_left->components[i] <= vector_right->components[i]) {
            return 0;
        }
    }

    return 1;
}
