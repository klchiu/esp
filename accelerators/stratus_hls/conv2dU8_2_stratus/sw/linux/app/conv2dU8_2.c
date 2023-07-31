// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"

#include <stdio.h>
#include <stdlib.h>

/* User-defined code */
static int validate_buffer(token_t *acc_buf, native_t *sw_buf, unsigned len)
{
    int      i;
    native_t val;
    unsigned errors = 0;

    fprintf(stderr, "\nPrint output\n");

    for (i = 0; i < len; i++) {

#ifdef __FIXED
        val = fx2float(acc_buf[i], FX_IL);
#else
        val        = acc_buf[i];
#endif
        if (sw_buf[i] != val) {
            errors++;
            // if (errors <= MAX_PRINTED_ERRORS)
            //     fprintf(stderr, "index %d : output %d : expected %d <-- ERROR\n", i, (int)val, (int)sw_buf[i]);
        }
    }

    if (!errors)
        fprintf(stderr, "\n  ** Test PASSED! **\n");
    else
        fprintf(stderr, "\n  ** Test FAILED! **\n");

    return errors;
}

/* User-defined code */
static void init_buffer(token_t *acc_buf, native_t *sw_buf, unsigned in_len)
{
    int i;

    fprintf(stderr, "  Initialize inputs\n");

    for (i = 0; i < in_len; i++) {
        native_t val = i % 17 - 8;
#ifdef __FIXED
        acc_buf[i] = float2fx(val, FX_IL);
#else
        acc_buf[i] = val;
#endif
        sw_buf[i] = val;
    }
    fprintf(stderr, "  Initialize inputs 2\n");
}

static void init_parameters(int test, int32_t n_channels, int32_t feature_map_height, int32_t feature_map_width,
                            int32_t n_filters, int32_t filter_dim, int32_t is_padded, int32_t stride, int32_t do_relu,
                            int32_t pool_type, int32_t batch_size, unsigned *in_len, unsigned *weights_len,
                            unsigned *bias_len, unsigned *out_len, unsigned *in_size, unsigned *weights_size,
                            unsigned *bias_size, unsigned *out_size, unsigned *weights_offset, unsigned *bias_offset,
                            unsigned *out_offset, unsigned *size)
{
    int32_t output_h;
    // int32_t output_w;
    int32_t output_pool_h;
    // int32_t output_pool_w;
    int32_t pad_dim = 0;

    if (is_padded) {
        pad_dim = filter_dim / 2;
    }

    output_h      = (feature_map_height + 2 * pad_dim - ((filter_dim - 1) + 1)) / stride + 1;
    output_pool_h = pool_type ? output_h / 2 : output_h;

    // Input data and golden output (aligned to DMA_WIDTH makes your life easier)
    *in_len = round_up(
        batch_size * round_up(n_channels * round_up(feature_map_height * feature_map_width, DMA_RATIO), DMA_RATIO),
        DMA_RATIO);
    *weights_len = round_up(n_filters * n_channels * filter_dim * filter_dim, DMA_RATIO);
    *bias_len    = round_up(n_filters, DMA_RATIO);
    *out_len     = round_up(
        batch_size * round_up(n_filters * round_up(output_pool_h * output_pool_h, DMA_RATIO), DMA_RATIO), DMA_RATIO);

    *in_size        = *in_len * sizeof(token_t);
    *weights_size   = *weights_len * sizeof(token_t);
    *bias_size      = *bias_len * sizeof(token_t);
    *out_size       = *out_len * sizeof(token_t);
    *weights_offset = *in_len;
    *bias_offset    = *in_len + *weights_len;
    *out_offset     = *in_len + *weights_len + *bias_len;
    *size           = *in_size + *weights_size + *bias_size + *out_size;

    conv2dU8_2_cfg_000[0].n_channels         = n_channels;
    conv2dU8_2_cfg_000[0].feature_map_height = feature_map_height;
    conv2dU8_2_cfg_000[0].feature_map_width  = feature_map_width;
    conv2dU8_2_cfg_000[0].n_filters          = n_filters;
    conv2dU8_2_cfg_000[0].filter_dim         = filter_dim;
    conv2dU8_2_cfg_000[0].is_padded          = is_padded;
    conv2dU8_2_cfg_000[0].stride             = stride;
    conv2dU8_2_cfg_000[0].do_relu            = do_relu;
    conv2dU8_2_cfg_000[0].pool_type          = pool_type;
    conv2dU8_2_cfg_000[0].batch_size         = batch_size;

    // print test info
    fprintf(stderr, "  Prepare test %d parameters\n", test);
    fprintf(stderr, "    .n_channels = %d\n", n_channels);
    fprintf(stderr, "    .feature_map_height = %d\n", feature_map_height);
    fprintf(stderr, "    .feature_map_width = %d\n", feature_map_width);
    fprintf(stderr, "    .n_filters = %d\n", n_filters);
    fprintf(stderr, "    .filter_dim = %d\n", filter_dim);
    fprintf(stderr, "    .is_padded = %d\n", is_padded);
    fprintf(stderr, "    .stride = %d\n", stride);
    fprintf(stderr, "    .do_relu = %d\n", do_relu);
    fprintf(stderr, "    .pool_type = %d\n", pool_type);
    fprintf(stderr, "    .batch_size = %d\n", batch_size);
}

inline bool sw_is_a_ge_zero_and_a_lt_b(int a, int b)
{
    return (a >= 0 && a < b);
}

inline int max_of_4(int a, int b, int c, int d)
{
    if (a >= b && a >= c && a >= d) {
        return a;
    }
    if (b >= c && b >= d) {
        return b;
    }
    if (c >= d) {
        return c;
    }
    return d;
}

inline int avg_of_4(int a, int b, int c, int d)
{
    return (a + b + c + d) / 4;
}

inline void pooling_2x2(int *in, int *out, unsigned size, unsigned type)
{

    assert(type >= 1 && type <= 2);

    unsigned i, j, out_i;
    int    a, b, c, d;
    for (i = 0; i < size - 1; i += 2) {
        for (j = 0; j < size - 1; j += 2) {
            a     = in[i * size + j];
            b     = in[(i + 1) * size + j];
            c     = in[i * size + (j + 1)];
            d     = in[(i + 1) * size + (j + 1)];
            out_i = (i / 2) * (size / 2) + (j / 2);
            if (type == 1)
                out[out_i] = max_of_4(a, b, c, d);
            else
                out[out_i] = avg_of_4(a, b, c, d);
        }
    }
}

void sw_conv_layer(const int *input, const int channels, const int height, const int width, const int kernel_h,
                   const int kernel_w, const int pad_h, const int pad_w, const int stride_h, const int stride_w,
                   const int dilation_h, const int dilation_w, const int num_filters, const int *weights,
                   const int *biases, int *output, const bool do_relu, const int pool_type, const int batch_size)
{

    const int channel_size      = round_up(height * width, DMA_RATIO);
    const int filter_size       = channels * kernel_w * kernel_h;
    const int output_h          = (height + 2 * pad_h - (dilation_h * (kernel_h - 1) + 1)) / stride_h + 1;
    const int output_w          = (width + 2 * pad_w - (dilation_w * (kernel_w - 1) + 1)) / stride_w + 1;
    const int out_channel_size  = round_up(output_w * output_h, DMA_RATIO);
    const int pool_channel_size = round_up((output_w / 2) * (output_h / 2), DMA_RATIO);

    for (int batch_i = 0; batch_i < batch_size; batch_i++) {
        for (int num_filter = 0; num_filter < num_filters; num_filter++) {
            for (int output_row = 0; output_row < output_h; output_row++) {
                for (int output_col = 0; output_col < output_w; output_col++) {
                    int   k         = 0;
                    int out_value = 0;
                    for (int channel = 0; channel < channels; channel++) {
                        for (int kernel_row = 0; kernel_row < kernel_h; kernel_row++) {
                            for (int kernel_col = 0; kernel_col < kernel_w; kernel_col++) {
                                int input_row = (output_row * stride_h) - pad_h + kernel_row * dilation_h;
                                int input_col = (output_col * stride_w) - pad_w + kernel_col * dilation_w;
                                if (!(!sw_is_a_ge_zero_and_a_lt_b(input_row, height) ||
                                      (sw_is_a_ge_zero_and_a_lt_b(input_row, height) &&
                                       !sw_is_a_ge_zero_and_a_lt_b(input_col, width)))) {
                                    out_value += input[batch_i * channels * channel_size + channel * channel_size +
                                                       input_row * width + input_col] *
                                                 weights[num_filter * filter_size + k];
                                }
                                k++;
                            }
                        }
                    }
                    out_value += biases[num_filter];

                    if (do_relu && out_value < 0)
                        out_value = 0;

                    output[batch_i * num_filters * out_channel_size + num_filter * out_channel_size +
                           output_row * output_w + output_col] = out_value;
                }
            }

            if (pool_type)
                pooling_2x2(&output[batch_i * num_filters * out_channel_size + num_filter * out_channel_size],
                            &output[batch_i * num_filters * pool_channel_size + num_filter * pool_channel_size],
                            output_w, pool_type);
        }
    }
}

static long long sw_run(int32_t n_channels, int32_t feature_map_height, int32_t feature_map_width, int32_t n_filters,
                        int32_t filter_dim, int32_t is_padded, int32_t stride, int32_t do_relu, int32_t pool_type,
                        int32_t batch_size, native_t *in, native_t *weights, native_t *bias, native_t *out)
{
    struct timespec th_start, th_end;
    int32_t         pad_dim = 0;

    if (is_padded) {
        pad_dim = filter_dim / 2;
    }

    gettime(&th_start);

    sw_conv_layer(in, n_channels, feature_map_height, feature_map_width, filter_dim, filter_dim, pad_dim, pad_dim,
                  stride, stride, 1, 1, n_filters, weights, bias, out, do_relu, pool_type, batch_size);

    gettime(&th_end);

    unsigned long long sw_ns = ts_subtract(&th_start, &th_end);
    fprintf(stderr, "    Software execution time: %llu ns\n", sw_ns);

    return sw_ns;
}

int main(int argc, char **argv)
{
    FILE              *log_0323 = fopen("log_0323_conv2dU8_2.txt", "w");
    int                i, j, k;
    unsigned long long hw_ns;
    unsigned long long sw_ns;

    unsigned long long hw_total;
    unsigned long long sw_total;

    unsigned long long hw_avg;
    unsigned long long sw_avg;

    int test, n_tests, start_test = 1;

    unsigned in_len, weights_len, bias_len, out_len;
    unsigned in_size, weights_size, bias_size, out_size, size;
    unsigned weights_offset, bias_offset, out_offset;

    token_t  *acc_buf;
    native_t *sw_buf;

    // int32_t n_channels[MAX_TESTS] = {16, 2, 2, 2, 64, 3, 4, 2, 4, 2};
    // int32_t feature_map_height[MAX_TESTS] = {8, 6, 14, 14, 32, 220, 32, 64, 8, 6};
    // int32_t feature_map_width[MAX_TESTS] = {8, 6, 14, 14, 32, 220, 32, 64, 8, 6};
    // int32_t n_filters[MAX_TESTS] = {16, 2, 128, 16, 128, 2, 2, 2, 2, 4};
    // int32_t filter_dim[MAX_TESTS] = {3, 5, 1, 1, 1, 3, 3, 3, 3, 3};
    // int32_t is_padded[MAX_TESTS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    // int32_t stride[MAX_TESTS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    // int32_t do_relu[MAX_TESTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // int32_t pool_type[MAX_TESTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // int32_t batch_size[MAX_TESTS] = {1, 1, 1, 1, 1, 1, 4, 8, 12, 16};

    // int32_t n_channels_3[8] = {8, 16, 32, 64, 128, 256, 512, 1024};
    int32_t n_channels_3[5] = {128, 128, 128, 128, 128};
    int32_t n_channels      = 100;
    // int32_t feature_map_height_3[8] = {8, 16, 32, 64, 128, 256, 512, 1024};
    int32_t feature_map_height_3[5] = {32, 32, 32, 32, 32};
    int32_t feature_map_height      = 100;
    int32_t feature_map_width       = 100; // same as feature_map_height_3
    int32_t n_filters               = 2;   // same as n_channels_3
    // int32_t filter_dim_3[3] = {1, 3, 5};
    int32_t filter_dim_3[3] = {5, 5, 5};
    int32_t filter_dim      = 100;
    int32_t is_padded       = 1;
    int32_t stride          = 1;
    int32_t do_relu         = 0;
    int32_t pool_type       = 0;
    int32_t batch_size      = 1;

    fprintf(stderr, "\n====== %s ====== 2 \n\n", cfg_000[0].devname);

    // command line arguments
    if (argc < 3) {
        n_tests = 1;
    } else if (argc == 3) {
        n_tests = strtol(argv[1], NULL, 10);
        if (n_tests > MAX_TESTS) {
            fprintf(stderr, "Wrong input arguments!");
            return 1;
        }
        start_test = strtol(argv[2], NULL, 10);
        if (start_test > MAX_TESTS) {
            fprintf(stderr, "Wrong input arguments!");
            return 1;
        }
    } else {
        fprintf(stderr, "Wrong input arguments!");
        return 1;
    }
    fprintf(stderr, "  Executing %d tests\n", n_tests);

    // allocations
    fprintf(stderr, "  Allocations\n");
    acc_buf           = (token_t *)esp_alloc(MAX_SIZE);
    cfg_000[0].hw_buf = acc_buf;

    sw_buf = malloc(MAX_SIZE);

    for (k = 0; k < 3; k++) {
        filter_dim = filter_dim_3[k];

        for (i = 0; i < 5; i++) {
            n_channels = n_channels_3[i];
            n_filters  = n_channels;

            for (j = 0; j < 5; j++) {
                feature_map_height = feature_map_height_3[j];
                feature_map_width  = feature_map_height;

                // calculate test parameters
                init_parameters(test, n_channels, feature_map_height, feature_map_width, n_filters, filter_dim,
                                is_padded, stride, do_relu, pool_type, batch_size, &in_len, &weights_len, &bias_len,
                                &out_len, &in_size, &weights_size, &bias_size, &out_size, &weights_offset, &bias_offset,
                                &out_offset, &size);

                // initialize input data
                init_buffer(acc_buf, sw_buf, out_offset);

                // hardware execution
                struct timespec th_hw_1, th_hw_2;
                fprintf(stderr, "\n  Start accelerator execution\n");
                gettime(&th_hw_1);
                esp_run(cfg_000, NACC);
                gettime(&th_hw_2);
                fprintf(stderr, "  Completed accelerator execution\n");
                hw_ns = ts_subtract(&th_hw_1, &th_hw_2);

                // software execution
                fprintf(stderr, "\n  Start software execution\n");
                // sw_ns = sw_run(n_channels, feature_map_height, feature_map_width,
                //                n_filters, filter_dim, is_padded, stride,
                //                do_relu, pool_type, batch_size,
                //                sw_buf, &sw_buf[weights_offset], &sw_buf[bias_offset], &sw_buf[out_offset]);
                fprintf(stderr, "  Completed software execution\n");

                // validation
                int no_error = validate_buffer(&acc_buf[out_offset], &sw_buf[out_offset], out_len);
                /* validate_buffer(acc_buf, sw_buf, out_len + out_offset); */

                fprintf(log_0323,
                        "filter_dim: %d n_channels: %d feature_map_height: %d hw_ns: %llu sw_ns: %llu no_error: %d\n",
                        filter_dim, n_channels, feature_map_height, hw_ns, sw_ns, no_error);
            }
        }
    }

    // free
    esp_free(acc_buf);
    free(sw_buf);

    fclose(log_0323);

    fprintf(stderr, "\n====== %s ======\n\n", cfg_000[0].devname);

    return 0;
}
