#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "operators.h"

uint16_t maximum(uint16_t operand1, uint16_t operand2)
{
    return operand1 == operand2 ? operand1 : (operand1 < operand2 ? operand2 : operand1);
}

uint16_t minimum(uint16_t operand1, uint16_t operand2)
{
    return operand1 == operand2 ? operand1 : (operand1 < operand2 ? operand1 : operand2);
}

struct vector *new_vector(int size, ...)
{
    struct vector *vector = malloc(sizeof(struct vector) + size * sizeof(double));
    vector->size = size;

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

uint8_t op_v_eq_v(struct vector *operand1, struct vector *operand2)
{
    if (operand1 == NULL || operand2 == NULL) {
        return 0;
    }

    if (operand1->size != operand2->size) {
        /* vectors have not the same length */
        return 0;
    } else {
        /* vectors have the same length */

        /* loop over components */
        for (int i = 0; i < operand1->size; i++) {
            if (operand1->components[i] != operand2->components[i]) {
                /* components are not equal */
                return 0;
            }
        }

        return 1;
    }
}

struct vector *op_v_add_v(struct vector *operand1, struct vector *operand2)
{
    if (operand1 == NULL || operand2 == NULL) {
        return NULL;
    }

    uint16_t min = minimum(operand1->size, operand2->size);
    uint16_t max = maximum(operand1->size, operand2->size);

    struct vector *vector = malloc(sizeof(struct vector) + max * sizeof(double));
    vector->size = max;

    for (int i = 0; i < min; i++) {
        vector->components[i] = operand1->components[i] + operand2->components[i];
    }

    for (int i = min; i < max; i++) {
        vector->components[i] =
            operand1->size < operand2->size ?
            operand2->components[i] : operand1->components[i];
    }

    return vector;
}

struct vector *op_v_sub_v(struct vector *operand1, struct vector *operand2)
{
    if (operand1 == NULL || operand2 == NULL) {
        return NULL;
    }

    uint16_t min = minimum(operand1->size, operand2->size);
    uint16_t max = maximum(operand1->size, operand2->size);

    struct vector *vector = malloc(sizeof(struct vector) + max * sizeof(double));
    vector->size = max;

    for (int i = 0; i < min; i++) {
        vector->components[i] = operand1->components[i] - operand2->components[i];
    }

    for (int i = min; i < max; i++) {
        vector->components[i] =
            operand1->size < operand2->size ?
            operand2->components[i] : operand1->components[i];
    }

    return vector;
}
