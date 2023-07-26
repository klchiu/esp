// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_GOLDEN_CONV_LAYER_H_
#define SRC_GOLDEN_CONV_LAYER_H_

#include <stdlib.h>
#ifndef __GEMM_DATA_HPP__
    #include "conv2dU8_data.hpp"
    // #include "fpdata.hpp"
    #include "conv2dU8.hpp"
#else
    #define DMA_WORD_PER_BEAT 2
    // [humu]: shoud I change it to 2*4 ?
#endif

inline bool sw_is_a_ge_zero_and_a_lt_b(int a, int b)
{
    return static_cast<unsigned>(a) < static_cast<unsigned>(b);
}
inline int8_t max_of_4(int8_t a, int8_t b, int8_t c, int8_t d);
inline int8_t avg_of_4(int8_t a, int8_t b, int8_t c, int8_t d);
inline void   pooling_2x2(int8_t *in, int8_t *out, unsigned size, bool type);
void sw_conv_layer(const int8_t *input, const int channels, const int height, const int width, const int kernel_h,
                   const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w,
                   const int dilation_h, const int dilation_w, const int num_filters, const int8_t *weights,
                   const int8_t *biases, int8_t *output, const bool do_relu, const int pool_type, const int batch_size);

#endif /* SRC_GOLDEN_CONV_LAYER_H_ */
