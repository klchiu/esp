// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <stdint.h>
#include "conv2dU8_data.hpp"
// #include "fpdata.hpp"
#include "conv2dU8_directives.hpp"
#include "conv2dU8.hpp"

// Initialization functions
void init_tensor(int8_t *tensor, const int size, bool random);

// Print functions
void print_image(const char *name, int8_t *image, const int batch_size, const int channels, const int height,
                 const int width, const bool fpdata);

void print_weights(const char *name, int8_t *weights, const int filters, const int channels, const int height,
                   const int width, const bool fpdata);

void print_bias(const char *name, int8_t *bias, const int n_filters, const bool fpdata);

void print_array(const char *name, int8_t *image, const int length);

// Manipulation functions
void transpose_matrix(int8_t *image, const int height, const int width);

// Comparison functions
int _validate(int8_t *hw_data_array, int8_t *sw_data_array, int batch_size, int filters, int output_h, int output_w);

typedef union {
    int8_t        fval;
    unsigned int rawbits;
} int8_t_union_t;

#endif // _UTILS_HPP_
