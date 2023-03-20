// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "libesp.h"
#include "cfg_tf_add3.h"

static void validate_buffer(token_t *acc_buf, native_t *sw_buf, unsigned len)
{
    int i;
    native_t val;
    unsigned errors = 0;

    printf("\nPrint output\n");

    for (i = 0; i < len; i++) {

#ifdef __FIXED
	val = fx2float(acc_buf[i], FX_IL);
#else
	val = acc_buf[i];
#endif
	if (sw_buf[i] != val) {
	    errors++;
	    if (errors <= MAX_PRINTED_ERRORS)
		printf("index %d : output %d : expected %d <-- ERROR\n", i, (int) val, (int) sw_buf[i]);
	}
    }

    if (!errors)
	printf("\n  ** Test PASSED! **\n");
    else
	printf("\n  ** Test FAILED! **\n");
}

static void init_buffer(token_t *acc_buf, native_t *sw_buf, unsigned in_len)
{
    int i;

    printf("  Initialize inputs\n");

    for (i = 0; i < in_len; i++) {
	native_t val = i % 17 - 8;
#ifdef __FIXED
        acc_buf[i] = float2fx(val, FX_IL);
#else
        acc_buf[i] = val;
#endif
	sw_buf[i] = val;
    }
}

static void init_parameters(int len)
{
    base_addr_0 = 0;
    base_addr_1 = len;
    base_addr_2 = len*2;


    cfg_tf_add3[0].tf_length = len;
    cfg_tf_add3[0].tf_src_dst_offset_0 = base_addr_0;
	cfg_tf_add3[0].tf_src_dst_offset_1 = base_addr_1;
	cfg_tf_add3[0].tf_src_dst_offset_2 = base_addr_2;

    printf("  %s parameters\n", cfg_tf_add3[0].devname);
    printf("    .length = %d\n", cfg_tf_add3[0].tf_length);
    printf("    .src_dst_offset_0 = %d\n", cfg_tf_add3[0].tf_src_dst_offset_0);
    printf("    .src_dst_offset_1 = %d\n", cfg_tf_add3[0].tf_src_dst_offset_1);
    printf("    .src_dst_offset_2 = %d\n", cfg_tf_add3[0].tf_src_dst_offset_2);
}


static void malloc_arrays(int len)
{
    output_0 = (float*)malloc(sizeof(float) * len);
    input_1 = (float*)malloc(sizeof(float) * len);
    input_2 = (float*)malloc(sizeof(float) * len);
    gold_0 = (float*)malloc(sizeof(float) * len);
}

static void free_arrays()
{
    free(output_0);
    free(input_1);
    free(input_2);
    free(gold_0);
}

static void sw_run(int32_t do_relu, int32_t transpose, int32_t ninputs,
		   int32_t d3, int32_t d2, int32_t d1,
		   native_t *in1, native_t *in2, native_t *out)
{
    int i, j, k, l;
    struct timespec th_start, th_end;
    native_t *in1_l, *in2_l, *out_l;

    gettime(&th_start);

    for (l = 0; l < ninputs; ++l)
    {
	in1_l = &in1[l * d1 * d2];
	in2_l = &in2[l * d2 * d3];
	out_l = &out[l * d1 * d3];

	for (i = 0; i < d1; ++i)
	{
	    for (j = 0; j < d3; ++j)
	    {
		native_t accumulator = 0.0;

		for (k = 0; k < d2; ++k)
		{
		    int mtx_in1_i = i * d2 + k;
		    int mtx_in2_i = transpose ? (j * d2 + k) : (k * d3 + j);

		    accumulator += in1_l[mtx_in1_i] * in2_l[mtx_in2_i];
		}

		out_l[i * d3 + j] = accumulator;
	    }
	}
    }

    gettime(&th_end);

    unsigned long long hw_ns = ts_subtract(&th_start, &th_end);
    printf("    Software execution time: %llu ns\n", hw_ns);
}


static void run_pv(int len)
{
    int i;
    for (i = 0 ; i < len; i++){
        gold_0[i] = input_1[i] + input_2[i];
    }
}

int main(int argc, char **argv)
{
    struct timespec t_sw_start, t_sw_end;
    struct timespec t_hw_start, t_hw_end;

    token_t *acc_buf;
    native_t *sw_buf;

    printf("\n====== %s ======\n\n", cfg_tf_add3[0].devname);

    int test_len = 1024;

    init_parameters(test_len);
    
    malloc_arrays(test_len);
    acc_buf = (token_t *) esp_alloc(MAX_SIZE);
    cfg_tf_add3[0].hw_buf = acc_buf;
    sw_buf = malloc(MAX_SIZE);
    

	printf("\n\n-------------------\n");

	// initialize input data
	init_buffer(acc_buf, sw_buf, test_len);

	// hardware execution
	printf("  Start accelerator execution\n");
    gettime(&t_hw_start);
	esp_run_no_print(cfg_tf_add3, 1);
    gettime(&t_hw_end);
	printf("  Completed accelerator execution\n");

	// software execution
	printf("  Start software execution\n");
    gettime(&t_sw_start);
	run_pv(test_len);
    gettime(&t_sw_end);
    printf("  Completed software execution\n");


    unsigned long long hw_ns = ts_subtract(&t_hw_start, &t_hw_end);
    printf("    Hardware execution time: %llu ns\n", hw_ns);
    unsigned long long sw_ns = ts_subtract(&t_sw_start, &t_sw_end);
    printf("    Software execution time: %llu ns\n", sw_ns);

	// validation
	// errors = print_input(buf, gold);
	validate_buffer(&acc_buf[test_len], &sw_buf[test_len], test_len);



    // free
    esp_free(acc_buf);
    free(sw_buf);
    free_arrays();

    printf("\n====== %s ======\n\n", cfg_000[0].devname);

    return 0;
}
