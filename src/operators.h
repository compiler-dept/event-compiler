#ifndef OPERATORS_H
#define OPERATORS_H

#include <stdint.h>

struct vector {
    uint16_t size;
    double components[];
};

struct vector *new_vector(int size, ...);

struct vector *new_vector_from_array(int size, double *array);

void print_vector(struct vector *vector);

uint8_t op_v_eq_v(struct vector *operand1, struct vector *operand2);

struct vector *op_v_add_v(struct vector *operand1, struct vector *operand2);

struct vector *op_v_sub_v(struct vector *operand1, struct vector *operand2);

#endif
