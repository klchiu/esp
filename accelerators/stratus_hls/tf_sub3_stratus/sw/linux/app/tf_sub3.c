// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "libesp.h"
#include "cfg_tf_sub3.h"

#include <stdio.h>
#include <stdlib.h>

static void init_parameters(int len)
{
    printf("-- init_parameters\n");

    base_addr_0 = 0;
    base_addr_1 = round_up(len, DMA_WORD_PER_BEAT(sizeof(token_t)));
    base_addr_2 = base_addr_1 + round_up(len, DMA_WORD_PER_BEAT(sizeof(token_t)));
    //chunk_size = 64;

    int round_len = round_up(len, DMA_WORD_PER_BEAT(sizeof(token_t)));

    struct tf_sub3_stratus_access *tmp;
    tmp = (struct tf_sub3_stratus_access *)cfg_tf_sub3[0].esp_desc;
    tmp->tf_length = round_len;
    tmp->tf_src_dst_offset_0 = base_addr_0;
    tmp->tf_src_dst_offset_1 = base_addr_1;
    tmp->tf_src_dst_offset_2 = base_addr_2;
    tmp->chunk_size = chunk_size;

    // tf_sub3_cfg_000[0].tf_length = len;
    // tf_sub3_cfg_000[0].tf_src_dst_offset_0 = base_addr_0;
	// tf_sub3_cfg_000[0].tf_src_dst_offset_1 = base_addr_1;
	// tf_sub3_cfg_000[0].tf_src_dst_offset_2 = base_addr_2;

    printf("  %s parameters\n", cfg_tf_sub3[0].devname);
    printf("    .length = %d\n", tf_sub3_cfg_000[0].tf_length);
    printf("    .src_dst_offset_0 = %d\n", tf_sub3_cfg_000[0].tf_src_dst_offset_0);
    printf("    .src_dst_offset_1 = %d\n", tf_sub3_cfg_000[0].tf_src_dst_offset_1);
    printf("    .src_dst_offset_2 = %d\n", tf_sub3_cfg_000[0].tf_src_dst_offset_2);
    printf("    .chunk_size = %d\n", tf_sub3_cfg_000[0].chunk_size);
}

static void malloc_arrays(int len)
{
    printf("-- malloc_arrays\n");

    output_0 = (native_t*)malloc(sizeof(native_t) * len);
    input_1 = (native_t*)malloc(sizeof(native_t) * len);
    input_2 = (native_t*)malloc(sizeof(native_t) * len);
    gold_0 = (native_t*)malloc(sizeof(native_t) * len);
}

static void init_arrays(int len)
{
    printf("-- init_arrays\n");

    int i;

    for (i = 0 ; i < len; i++){
        // float val_1 = i *100 / 3.0 - 17;
        // float val_2 = i *100 / 9.0 - 17;
        native_t val_1 = rand() % 100 / 3.0 - 17;
        native_t val_2 = rand() % 100 / 9.0 - 17;


        input_1[i] = val_1;
        input_2[i] = val_2;

        if(i < 10){
            printf("%d: val_1: %f\tval_2: %f\n", i, val_1, val_2);
        }
    }
}

static void load_buffer(token_t *acc_buf, unsigned in_len)
{
    printf("-- load_buffer\n");

    int i;

    // load input_1
    for (i = 0; i < in_len; i++) {
        acc_buf[base_addr_1 + i] = float2fx(input_1[i], FX_IL);
    }
    // load input_2
    for (i = 0; i < in_len; i++) {
        acc_buf[base_addr_2 + i] = float2fx(input_2[i], FX_IL);
    }
}





static void store_buffer(token_t *acc_buf, unsigned len)
{
    printf("-- store_buffer\n");

    int i;

    for (i = 0; i < len; i++) {
    	output_0[i] = fx2float(acc_buf[base_addr_0 + i], FX_IL);
    }
}

static int validate_array(unsigned len)
{
    printf("-- validate_array\n");

    int i;
    unsigned errors = 0;

    for (i = 0; i < len; i++) {
        if (abs(gold_0[i] - output_0[i]) > 0.001){
            errors++;
            if (errors < 20)
    		    printf("index: %d, output: %f, gold: %f <-- ERROR\n", 
                    i, output_0[i], gold_0[i]);
        }
	}

    if (!errors)
	    printf("\n  ** Test PASSED! **\n");
    else
	    printf("\n  ** Test FAILED! ** Error counts: %d\n", errors);
    
    return errors;
}


static void free_arrays()
{
    printf("-- free_arrays\n");

    free(output_0);
    free(input_1);
    free(input_2);
    free(gold_0);
}

static void run_sw(int len)
{
    printf("-- run_sw\n");

    int i;
    for (i = 0 ; i < len; i++){
        gold_0[i] = input_1[i] - input_2[i];
    }
}



int run_test(int test_len, unsigned long long *hw_ns, unsigned long long *sw_ns)
{
    struct timespec t_sw_start, t_sw_end;
    struct timespec t_hw_start, t_hw_end;

    token_t *acc_buf;
    
    int ret_validate;

    printf("\n====== START: %s ======\n\n", cfg_tf_sub3[0].devname);

    // int test_len = 1024;

    init_parameters(test_len);
    
    malloc_arrays(test_len);
    init_arrays(test_len);

    int round_len = round_up(test_len, DMA_WORD_PER_BEAT(sizeof(token_t)));
    //acc_buf = (token_t *) esp_alloc(MAX_LENGTH*3);
    acc_buf = (token_t *) esp_alloc(round_len*3);
    cfg_tf_sub3[0].hw_buf = acc_buf;
    

	printf("\n-------------------\n");

	// load input data to acc
	load_buffer(acc_buf, test_len);

	// hardware execution
	printf("  Start accelerator execution\n");
    gettime(&t_hw_start);
	esp_run_no_print(cfg_tf_sub3, 1);
	// esp_run(cfg_tf_sub3, 1);
    gettime(&t_hw_end);
	printf("  Completed accelerator execution\n");

	// software execution
	printf("  Start software execution\n");
    gettime(&t_sw_start);
	run_sw(test_len);
    gettime(&t_sw_end);
    printf("  Completed software execution\n");

  
    store_buffer(acc_buf, test_len);

	ret_validate = validate_array(test_len);

    *hw_ns = ts_subtract(&t_hw_start, &t_hw_end);
    printf("    Hardware execution time: %llu ns\n", *hw_ns);
    *sw_ns = ts_subtract(&t_sw_start, &t_sw_end);
    printf("    Software execution time: %llu ns\n", *sw_ns);

    printf(" HW/SW Speedup: %f\n", (float)(*sw_ns)/(float)(*hw_ns));

    // free
    esp_free(acc_buf);
    free_arrays();

    printf("\n====== DONE! ======\n\n");

    return ret_validate;
}


int main(int argc, char **argv)
{
    FILE* log_0320 = fopen("log_0320_tf_sub3.txt", "w");

    int i, j, k;
    int len;
    // int tests[6] = {128, 256, 512, 1024, 2048, 4096};
    unsigned long long hw_ns;
    unsigned long long sw_ns;

    unsigned long long hw_total;
    unsigned long long sw_total;

    unsigned long long hw_avg;
    unsigned long long sw_avg;
    float speedup;

    MAX_LENGTH = 2 << 17; // acc limitation, PLM size

    int TEST_NO_AVG = 10;
    int no_error = 0;
    // int no_tests = 0;

    for(k = 0; k < 18; k++){
	    for(i = 0 ; i < 11; i++){
		    // for(i = 12 ; i < 15; i++){
		    hw_total = 0;
		    sw_total = 0;
		    no_error = 0;
		    // no_tests = 0;
		    // len = tests[i];
		    len = 2 << k;//MAX_LENGTH;
		    chunk_size = (2 << 3) << i;

		    for(j = 0 ; j < TEST_NO_AVG ; j++){

			    if(len > MAX_LENGTH){   // when len is too big split into acc runs
				    int remainder = len;
				    for(k = 0 ; k < len ; k+=MAX_LENGTH){
					    int run_len = remainder > MAX_LENGTH ? MAX_LENGTH : remainder;
					    remainder = remainder > MAX_LENGTH ? (remainder - MAX_LENGTH) : 0;
					    no_error += run_test(run_len, &hw_ns, &sw_ns);
					    // fprintf(log_0320, "len: %d, hw_ns: %llu, sw_ns: %llu\n", len, hw_ns, sw_ns);
					    hw_total += hw_ns;
					    sw_total += sw_ns;
					    // no_tests++;
				    }
			    }
			    else{                   // when len is within the limit
			    no_error += run_test(len, &hw_ns, &sw_ns);
			    hw_total += hw_ns;
			    sw_total += sw_ns;
			    }


		    }

		    hw_avg = hw_total / TEST_NO_AVG;
		    sw_avg = sw_total / TEST_NO_AVG;
		    speedup = (float)sw_avg/(float)hw_avg;

		    fprintf(log_0320, "len: %d hw_ns: %llu sw_ns: %llu chunk_size = %d speedup: %f no_error: %d\n", len, hw_avg, sw_avg, chunk_size, speedup, no_error);
	    }
    }

    fclose(log_0320);
    
    return 0;
}