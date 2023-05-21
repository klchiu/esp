// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "libesp.h"
#include "cfg_tf_fc.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NACC 1

static float ActivationFunctionWithMinMax(float total, int output_activation_min, int output_activation_max)
{
    // [humu]: why just return total?
    // This is a TODO
    return total;
}

static void init_array(float *array, int size)
{
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        array[i] = 0.1 * (rand() % 10);
    }
}

static void init_array_0(float *array, int size)
{
    //srand(time(NULL));
    /* for (int i = 0; i < size; i++) { */
    /*     array[i] = 0.0; */
    /* } */
    memset(array, 0, sizeof(float)*size);
}

static void load_buffer(token_t *acc_buf, uint32_t base_addr_1, float *input_1, uint32_t base_addr_2, float *input_2,
                        unsigned len)
{
    // fprintf(stderr, "-- load_buffer, len = %d\n", len);

    int i;

    // load input_1
    for (i = 0; i < len; i++) {
        acc_buf[len + i] = float2fx(input_1[base_addr_1 + i], FX_IL);
        // fprintf(stderr, "-- load_buffer, i = %d\n", i);
    }

    // fprintf(stderr, "-- load_buffer 2\n");

    // load input_2
    for (i = 0; i < len; i++) {
        acc_buf[len + len + i] = float2fx(input_2[base_addr_2 + i], FX_IL);
        // fprintf(stderr, "-- load_buffer, i = %d\n", i);
    }
}

static void store_buffer(token_t *acc_buf, uint32_t base_addr_0, float *output_0, unsigned len)
{
    // fprintf(stderr, "-- store_buffer, len = %d\n", len);

    int i;

    for (i = 0; i < len; i++) {
        output_0[base_addr_0 + i] = fx2float(acc_buf[i], FX_IL);
    }
}

int main(int argc, char **argv)
{
    int      mult_len;
    int      add_len;
    int      total_len;
    token_t *acc_buf_4;
    token_t *acc_buf;
    token_t *mult_acc_buf;
    token_t *add_acc_buf;

    fprintf(stderr, "[humu]: ================== doFineGrainedFC: \n");

    int batches      = 50;
    int output_depth = 100;
    int accum_depth  = 100;
    fprintf(stderr, "[humu]:      doFCFineGrained: batches = %d\n", batches);
    fprintf(stderr, "[humu]:      doFCFineGrained: output_depth = %d\n", output_depth);
    fprintf(stderr, "[humu]:      doFCFineGrained: accum_depth = %d\n", accum_depth);

    float input_data[batches * accum_depth];
    float weights_data[output_depth * accum_depth];
    float bias_data[output_depth];
    float output_data[batches * output_depth], total_1[batches * output_depth];
    float output_data_2[batches * output_depth];
    float output_data_3[batches * output_depth];
    float output_data_4[batches * output_depth];
    int   output_activation_min = 0, output_activation_max = 0;
    init_array(input_data, batches * accum_depth);
    init_array(weights_data, output_depth * accum_depth);
    init_array(bias_data, output_depth);
    init_array_0(output_data, batches * output_depth);
    init_array_0(output_data_2, batches * output_depth);
    init_array_0(output_data_3, batches * output_depth);
    init_array_0(output_data_4, batches * output_depth);
    // int bias_data = 1;
    int print_all = 0;
    int errors    = 0;

    // 1 - original

    fprintf(stderr, "[humu]: fine-grained #1 original\n");

    struct timespec t_orig_start, t_orig_end;
    unsigned long long orig_ns;

    gettime(&t_orig_start);

    for (int b = 0; b < batches; ++b) {
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            float total = 0.f;
            for (int d = 0; d < accum_depth; ++d) {
                total += input_data[b * accum_depth + d] * weights_data[out_c * accum_depth + d];
            }
            float bias_value = 0.0f;
            if (bias_data) {
                bias_value = bias_data[out_c];
            }
            output_data[out_c + output_depth * b] =
                ActivationFunctionWithMinMax(total + bias_value, output_activation_min, output_activation_max);

            if (print_all)
                fprintf(stderr, "total_1 is: %f batch: %d out_c: %d\n", output_data[out_c + output_depth * b], b,
                        out_c);

            // debugging
            total_1[out_c + output_depth * b] = output_data[out_c + output_depth * b];
        }
    }

    gettime(&t_orig_end);

    orig_ns = ts_subtract(&t_orig_start, &t_orig_end);
    printf("Original execution time: %llu ns\n", orig_ns);

    // 2 - basic accumulation level

    fprintf(stderr, "[humu]: fine-grained #2 basic accumulation level\n");

    struct timespec t_basic_start, t_basic_end;
    struct timespec t_init_start, t_init_end;
    unsigned long long basic_ns, init_ns;
    int basic_hw = 1;

    gettime(&t_init_start);

    mult_acc_buf = (token_t*)esp_alloc(accum_depth * 3);
    cfg_tf_mult3[0].hw_buf = mult_acc_buf;

    tf_mult3_cfg_000[0].tf_length = accum_depth;
    tf_mult3_cfg_000[0].tf_src_dst_offset_0 = 0;
    tf_mult3_cfg_000[0].tf_src_dst_offset_1 = accum_depth;
    tf_mult3_cfg_000[0].tf_src_dst_offset_2 = accum_depth + accum_depth;
    tf_mult3_cfg_000[0].chunk_size = 4096;

    //printf("Configured mult\n");

    add_acc_buf = (token_t*)esp_alloc(output_depth * 3);
    cfg_tf_add3[0].hw_buf = add_acc_buf;

    tf_add3_cfg_000[0].tf_length = output_depth;
    tf_add3_cfg_000[0].tf_src_dst_offset_0 = 0;
    tf_add3_cfg_000[0].tf_src_dst_offset_1 = output_depth;
    tf_add3_cfg_000[0].tf_src_dst_offset_2 = output_depth + output_depth;
    tf_add3_cfg_000[0].chunk_size = 4096;

    //printf("Configured add\n");

    gettime(&t_init_end);

    /* for(int t = 0; t < 2; t++){ */

	    gettime(&t_basic_start);

	    float mult[accum_depth];
	    float total_2[batches * output_depth];
	    init_array_0(total_2, batches * output_depth);

	    //printf("Starting loop buffer\n");

	    for (int b = 0; b < batches; ++b) {
		    for (int out_c = 0; out_c < output_depth; ++out_c) {
			    //float total = 0.f;

			    if(basic_hw == 0){
				    // Run with mult accelerator - sending input_data[0:accum_depth] and weights_data[0:accum_depth]
				    for (int d = 0; d < accum_depth; ++d) {
					    mult[d] = input_data[b * accum_depth + d] * weights_data[out_c * accum_depth + d];
				    }
			    }
			    else{
				    // print_tf_mult3_cfg(&cfg_tf_mult3[0], &tf_mult3_cfg_000[0]);

				    //printf("Loading buffer\n");
				    load_buffer(mult_acc_buf, b * accum_depth, input_data, out_c * accum_depth, weights_data, accum_depth);

				    //printf("Running mult\n");
				    esp_run_no_print(cfg_tf_mult3, NACC);

				    //printf("Storing buffer\n");
				    store_buffer(mult_acc_buf, 0, mult, accum_depth);

			    }

			    // Accumulate
			    for (int d = 0; d < accum_depth; ++d) {
				    total_2[out_c + output_depth * b] += mult[d];
			    }
		    }
		    if (bias_data) {

			    if(basic_hw == 0){
				    // Run with Add accelerator
				    for (int out_c = 0; out_c < output_depth; ++out_c) {
					    total_2[out_c + output_depth * b] += bias_data[out_c];
				    }
			    }
			    else{
				    // print_tf_add3_cfg(&cfg_tf_add3[0], &tf_add3_cfg_000[0]);

				    load_buffer(add_acc_buf, output_depth * b, total_2, 0, bias_data, output_depth);

				    esp_run_no_print(cfg_tf_add3, NACC);

				    store_buffer(add_acc_buf, output_depth * b, total_2, output_depth);

			    }
		    }

		    // Activation
		    for (int out_c = 0; out_c < output_depth; ++out_c) {
			    int temp = out_c + output_depth * b;

			    output_data_2[out_c + output_depth * b] = ActivationFunctionWithMinMax(
				    total_2[out_c + output_depth * b], output_activation_min, output_activation_max);

			    if (print_all)
				    fprintf(stderr, "total_2 is: %f batch: %d out_c: %d\n", output_data_2[out_c + output_depth * b], b,
					    out_c);

			    // debugging
			    total_2[temp] = output_data_2[temp];
			    if (abs(total_2[temp] - total_1[temp]) / total_1[temp] > 0.01) {
				    // if (errors < 20) {
				    fprintf(stderr, "mismatch: total_1[%d] = %f, total_2[%d] = %f\n", temp, total_1[temp], temp,
					    total_2[temp]);
				    // }
				    errors++;
			    }
		    }
	    }

	    if(basic_hw == 1){
		    esp_free(mult_acc_buf);
		    esp_free(add_acc_buf);
	    }

	    gettime(&t_basic_end);

	    basic_ns = ts_subtract(&t_basic_start, &t_basic_end);
	    init_ns = ts_subtract(&t_init_start, &t_init_end);
	    if(basic_hw == 0)
		    printf("SW: Basic accumulation level execution time: %llu ns\n", basic_ns);
	    else{
		    printf("HW: Initialization execution time: %llu ns\n", init_ns);
		    printf("HW: Basic accumulation level execution time: %llu ns\n", basic_ns);
	    }
    /* } */

    // 3 - output level

    fprintf(stderr, "[humu]: fine-grained #3 output level\n");

    struct timespec t_output_start, t_output_end;
    unsigned long long output_ns;

    gettime(&t_output_start);

    mult_len = output_depth * accum_depth;
    add_len  = output_depth;
    total_len = batches * output_depth;

    float mult_batch[mult_len];
    float total_3[total_len];
    init_array_0(total_3, total_len);

    for (int b = 0; b < batches; ++b) {
        // Initialize vector for inputs
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            for (int d = 0; d < accum_depth; ++d) {
                mult_batch[d + out_c * accum_depth] = input_data[b * accum_depth + d];
            }
        }

        // Run mult accelerator
        for (int i = 0; i < output_depth * accum_depth; i++) {
            mult_batch[i] *= weights_data[i];
        }
        // acc_buf = (token_t*)esp_alloc(mult_len * 3);
        // cfg_tf_mult3[0].hw_buf = acc_buf;

        // tf_mult3_cfg_000[0].tf_length = mult_len;
        // tf_mult3_cfg_000[0].tf_src_dst_offset_0 = 0;
        // tf_mult3_cfg_000[0].tf_src_dst_offset_1 = mult_len;
        // tf_mult3_cfg_000[0].tf_src_dst_offset_2 = mult_len + mult_len;

        // // print_tf_mult3_cfg(&cfg_tf_mult3[0], &tf_mult3_cfg_000[0]);

        // load_buffer(acc_buf, 0, mult_batch, 0, weights_data, mult_len);

        // esp_run_no_print(cfg_tf_mult3, NACC);

        // store_buffer(acc_buf, 0, mult_batch, mult_len);

        // esp_free(acc_buf);

        // Accumulate output
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            for (int d = 0; d < accum_depth; ++d) {
                total_3[out_c + output_depth * b] += mult_batch[out_c * accum_depth + d];
            }
        }

        if (bias_data) {
            // replace with Add accelerator
            for (int out_c = 0; out_c < output_depth; ++out_c) {
                total_3[out_c + output_depth * b] += bias_data[out_c];
            }
            // acc_buf = (token_t*)esp_alloc(add_len * 3);
            // cfg_tf_add3[0].hw_buf = acc_buf;

            // tf_add3_cfg_000[0].tf_length = add_len;
            // tf_add3_cfg_000[0].tf_src_dst_offset_0 = 0;
            // tf_add3_cfg_000[0].tf_src_dst_offset_1 = add_len;
            // tf_add3_cfg_000[0].tf_src_dst_offset_2 = add_len + add_len;

            // // print_tf_add3_cfg(&cfg_tf_add3[0], &tf_add3_cfg_000[0]);

            // load_buffer(acc_buf, output_depth * b, total_3, 0, bias_data, add_len);

            // esp_run_no_print(cfg_tf_add3, NACC);

            // store_buffer(acc_buf, output_depth * b, total_3, add_len);

            // esp_free(acc_buf);
        }

        // Activation
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            int temp = out_c + output_depth * b;

            output_data_3[out_c + output_depth * b] = ActivationFunctionWithMinMax(
                total_3[out_c + output_depth * b], output_activation_min, output_activation_max);

            if (print_all)
                fprintf(stderr, "total_3 is: %f batch: %d out_c: %d\n", output_data_3[out_c + output_depth * b], b,
                        out_c);

            // debugging
            total_3[temp] = output_data_3[temp];
            if (abs(total_3[temp] - total_1[temp]) / total_1[temp] > 0.01) {
                // if (errors < 20) {
                //   fprintf(stderr, "mismatch: total_1[%d] = %f, total_3[%d] = %f\n", temp, total_1[temp], temp,
                //   total_3[temp]);
                // }
                errors++;
            }
        }
    }

    gettime(&t_output_end);

    output_ns = ts_subtract(&t_output_start, &t_output_end);
    printf("Output level execution time: %llu ns\n", output_ns);

    // 4 - batch level
    fprintf(stderr, "[humu]: fine-grained #4 batch level\n");

    struct timespec t_batch_start, t_batch_end;
    unsigned long long batch_ns;

    gettime(&t_batch_start);

    mult_len = batches * accum_depth * output_depth;
    add_len  = batches * output_depth;

    acc_buf_4                               = (token_t *)esp_alloc(mult_len * 3);
    cfg_tf_mult3[0].hw_buf                  = acc_buf_4;
    tf_mult3_cfg_000[0].tf_length           = mult_len;
    tf_mult3_cfg_000[0].tf_src_dst_offset_0 = 0;
    tf_mult3_cfg_000[0].tf_src_dst_offset_1 = mult_len;
    tf_mult3_cfg_000[0].tf_src_dst_offset_2 = mult_len + mult_len;
    tf_mult3_cfg_000[0].chunk_size          = 4096;

    // acc_buf_4 = (token_t*)esp_alloc(add_len * 3);
    cfg_tf_add3[0].hw_buf                  = acc_buf_4;
    tf_add3_cfg_000[0].tf_length           = mult_len;
    tf_add3_cfg_000[0].tf_src_dst_offset_0 = 0;
    tf_add3_cfg_000[0].tf_src_dst_offset_1 = mult_len;
    tf_add3_cfg_000[0].tf_src_dst_offset_2 = mult_len + mult_len;
    tf_add3_cfg_000[0].chunk_size          = 4096;

    float mult_above_batch[mult_len];
    float above_batch_weights_data[mult_len];
    float above_batch_bias_data[add_len];
    float total_4[add_len];
    init_array_0(total_4, add_len);

    // Initialize vector for inputs
    for (int b = 0; b < batches; ++b) {
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            for (int d = 0; d < accum_depth; ++d) {
                mult_above_batch[d + out_c * accum_depth + b * accum_depth * output_depth] =
                    input_data[b * accum_depth + d];
            }
        }
    }

    // Initialize vector for weights
    for (int b = 0; b < batches; ++b) {
        for (int i = 0; i < output_depth * accum_depth; i++) {
            above_batch_weights_data[i + b * output_depth * accum_depth] = weights_data[i];
        }
    }

    // Run mult accelerator
    for (int j = 0; j < mult_len; ++j) {
        mult_above_batch[j] *= above_batch_weights_data[j];
    }

    // print_tf_mult3_cfg(&cfg_tf_mult3[0], &tf_mult3_cfg_000[0]);

    // load_buffer(acc_mult_buf, 0, above_batch_weights_data, 0, mult_above_batch, mult_len);

    // esp_run(cfg_tf_add3, NACC); // esp_run(cfg_tf_mult3, NACC);

    // store_buffer(acc_mult_buf, 0, mult_above_batch, mult_len);

    // tf_add3_cfg_000[0].tf_length           = add_len;
    // tf_add3_cfg_000[0].tf_src_dst_offset_0 = 0;
    // tf_add3_cfg_000[0].tf_src_dst_offset_1 = add_len;
    // tf_add3_cfg_000[0].tf_src_dst_offset_2 = add_len + add_len;

    // Accumulate output vector
    for (int b = 0; b < batches; ++b) {
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            for (int d = 0; d < accum_depth; ++d) {
                total_4[out_c + output_depth * b] +=
                    mult_above_batch[out_c * accum_depth + d + b * accum_depth * output_depth];
            }
        }
    }

    if (bias_data) {
        // Initialize vector for bias
        for (int b = 0; b < batches; ++b) {
            for (int i = 0; i < output_depth; i++) {
                above_batch_bias_data[i + b * output_depth] = bias_data[i];
            }
        }
        // Run with Add accelerator
        for (int i = 0; i < add_len; ++i) {
          total_4[i] += above_batch_bias_data[i];
        }

        // print_tf_add3_cfg(&cfg_tf_add3[0], &tf_add3_cfg_000[0]);

        // load_buffer(acc_add_buf, 0, total_4, 0, above_batch_bias_data, add_len);

        // esp_run(cfg_tf_add3, NACC);

        // store_buffer(acc_add_buf, 0, total_4, add_len);
    }

    // Activation
    for (int b = 0; b < batches; ++b) {
        for (int out_c = 0; out_c < output_depth; ++out_c) {
            int temp = out_c + output_depth * b;

            output_data_4[temp] =
                ActivationFunctionWithMinMax(total_4[temp], output_activation_min, output_activation_max);

            if (print_all)
                fprintf(stderr, "total_4 is: %f batch: %d out_c: %d\n", output_data_4[temp], b, out_c);

            // debugging
            total_4[temp] = output_data_4[temp];
            if (abs(total_4[temp] - total_1[temp]) / total_1[temp] > 0.01) {
                if (errors < 20) {
                    fprintf(stderr, "mismatch: total_1[%d] = %f, total_4[%d] = %f\n", temp, total_1[temp], temp,
                            total_4[temp]);
                }
                errors++;
            }
        }
    }

    esp_free(acc_buf_4);
    // esp_free(acc_mult_buf);

    gettime(&t_batch_end);

    batch_ns = ts_subtract(&t_batch_start, &t_batch_end);
    printf("Batch level execution time: %llu ns\n", batch_ns);

    fprintf(stderr, "Number of errors: %d \n", errors);

    return 0;
}
