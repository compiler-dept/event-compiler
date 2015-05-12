#ifndef OPERATORS_H
#define OPERATORS_H

#include <stdint.h>

struct __attribute__((__packed__)) vector {
    uint16_t size;
    double *components;
};

struct vector *new_vector(int size, ...);

struct vector *new_vector_from_array(int size, double *array);

void print_vector(struct vector *vector);

uint8_t op_v_eq_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_v_add_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_v_sub_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_s_mult_v(double scalar, struct vector *vector);

#endif
