#ifndef OPERATORS_H
#define OPERATORS_H

#include <stdint.h>

struct __attribute__((__packed__)) vector {
    uint16_t size;
    double *components;
};

struct vector *new_vector(int size, ...);

struct vector *new_vector_from_array(int size, double *array);

void vector_free(struct vector *vector);

void print_vector(struct vector *vector);

uint8_t op_v_eq_v(struct vector *vector_left, struct vector *vector_right);

uint8_t op_v_neq_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_v_add_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_v_sub_v(struct vector *vector_left, struct vector *vector_right);

struct vector *op_s_mult_v(double scalar, struct vector *vector);

double op_v_pow_s(struct vector *vector, double scalar);

uint8_t op_v_lt_v(struct vector *vector_left, struct vector *vector_right);

uint8_t op_v_gt_v(struct vector *vector_left, struct vector *vector_right);

uint8_t op_i_eq_i(uint8_t left, uint8_t right);

#endif
