// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"
#include <test/time.h>

#define FX_IL 12
// #define DBG_APP 1

// static unsigned in_words_adj;
// static unsigned out_words_adj;
static unsigned inA_len;
static unsigned inB_len;
static unsigned out_len;
static unsigned inA_size;
static unsigned inB_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;
static unsigned input_size;

/* User-defined code */

static void print_fixed32(token_t fixed_num, int integer_bit) {

	int float_bit = 32 - integer_bit;
	// printf("float bit: %d\n", float_bit);
	// preserve the floating points
	int huan_mask = (1 << float_bit) - 1;

	int integer_part = fixed_num >> float_bit;
	long long frac_part = ((double)(fixed_num & huan_mask) / (double)(1 << float_bit)) * 100000 ;


	printf("%d.%lld", integer_part, frac_part);
}

// static void dump_Matrix(token_t *A, int size, char* name){
// 	printf("=============== dump Matrix =============\n");
// 	for (int i = 0; i < size; i++){
// 		printf("%s[%d] = ", name, i);
// 		print_fixed32(A[i], FX_IL);
// 		// printf("%d", A[i * cols + j]);
// 		printf("\n");
// 	}
// }

static void dump_A_and_B(token_t *A, token_t *B){
	printf("=============== dump A and B =============\n");
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			printf("A[%d] = ", i * cols + j);
			print_fixed32(A[i * cols + j], FX_IL);
			// printf("%d", A[i * cols + j]);
			printf("\n");
		}
	}

	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			printf("B[%d] = ", i * cols + j);
			print_fixed32(B[i * cols + j], FX_IL);
			// printf("%d", B[i * cols + j]);
			printf("\n");
		}
	}
}

static void dump_mem(token_t *mem){
	printf("=============== dump memory =============\n");
	for (int n = 0; n < 3; ++n){
		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++){
				printf("mem[%d] = ", n * cols * rows + i * cols + j);
				print_fixed32(mem[n * cols * rows + i * cols + j], FX_IL);
				// printf("%d", mem[n * cols * rows + i * cols + j]);
				printf("\n");
			}
		}
	}
}

// static void dump_3mem(token_t *mem){
// 	printf("=============== dump memory =============\n");
// 	for (int n = 0; n < 3*3; ++n){
// 		for (int i = 0; i < rows; i++){
// 			for (int j = 0; j < cols; j++){
// 				printf("total_mem[%d] = ", n * cols * rows + i * cols + j);
// 				print_fixed32(mem[n * cols * rows + i * cols + j], FX_IL);
// 				// printf("%d", mem[n * cols * rows + i * cols + j]);
// 				printf("\n");
// 			}
// 		}
// 	}
// }

static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;

	for (i = 0; i < rows; i++){
		for (j = 0; j < cols; j++){
			if (gold[i * cols + j] != out[i * cols + j]){
				// float tmp1 = fixed32_to_float(gold[i * cols + j], FX_IL);
				// float tmp2 = fixed32_to_float(out[i * cols + j], FX_IL);

				// int fixed1_5 = float_to_fixed32(1.56, FX_IL);
				// printf("fixed1_5 int: %d\n", (fixed1_5 >> 20));
				// print_fixed32(fixed1_5, FX_IL);
				// printf("\n");

				///////////////// please use this part //////////////
				// printf("gold[%d] = ", i * cols + j);
				// print_fixed32(gold[i * cols + j], FX_IL);
				// printf(", out[%d] = ", i * cols + j);
				// print_fixed32(out[i * cols + j], FX_IL);
				// printf("\n");
				////////////////// please use the above part ///////////////


				// printf(stderr, "gold[%d] = %f, ", i * cols + j, tmp1);
				
				// printf(stderr, "out[%d] = %f\n", i * cols + j, tmp2);
				errors++;
			}
		}
	}

	return errors;
}


/* User-defined code */
static void init_buf (token_t *inA, token_t *inB, token_t * gold)
{
	// initialize input

	///////////// converting functions ///////////////
	// static inline int float_to_fixed32(float value, int n_int_bits)
	// static inline float fixed32_to_float(int value, int n_int_bits)
	//////////////////////////////////////////////////

	// inA[0] = 1; inA[1] = 5; inA[2] = 9; inA[3] = 13;
    // inA[4] = 2; inA[5] = 6; inA[6] = 10; inA[7] = 14; 
	// inA[8] = 3; inA[9] = 7; inA[10] = 11; inA[11] = 15; 
	// inA[12] = 4; inA[13] = 8; inA[14] = 12; inA[15] = 16;

	// inB[0] = 0+1; inB[1] = 1+1; inB[2] = 2+1; inB[3] = 3+1;
    // inB[4] = 4+1; inB[5] = 5+1; inB[6] = 6+1; inB[7] = 7+1;
    // inB[8] = 8+1; inB[9] = 9+1; inB[10] = 10+1; inB[11] = 11+1;
    // inB[12] = 12+1; inB[13] = 13+1; inB[14] = 14+1; inB[15] = 15+1;

	inA[0] = float_to_fixed32(1.0, FX_IL);
    inA[1] = float_to_fixed32(5.0, FX_IL);
    inA[2] = float_to_fixed32(9.0, FX_IL);
    inA[3] = float_to_fixed32(13.0, FX_IL);
    inA[4] = float_to_fixed32(2.0, FX_IL);
    inA[5] = float_to_fixed32(6.0, FX_IL);
    inA[6] = float_to_fixed32(10.0, FX_IL);
    inA[7] = float_to_fixed32(14.0, FX_IL);
    inA[8] = float_to_fixed32(3.0, FX_IL);
    inA[9] = float_to_fixed32(7.0, FX_IL);
    inA[10] = float_to_fixed32(11.0, FX_IL);
    inA[11] = float_to_fixed32(15.0, FX_IL);
    inA[12] = float_to_fixed32(4.0, FX_IL);
    inA[13] = float_to_fixed32(8.0, FX_IL);
    inA[14] = float_to_fixed32(12.0, FX_IL);
    inA[15] = float_to_fixed32(16.0, FX_IL);

	inB[0] = float_to_fixed32(0.0+1.0, FX_IL);
    inB[1] = float_to_fixed32(1.0+1.0, FX_IL);
    inB[2] = float_to_fixed32(2.0+1.0, FX_IL);
    inB[3] = float_to_fixed32(3.0+1.0, FX_IL);
    inB[4] = float_to_fixed32(4.0+1.0, FX_IL);
    inB[5] = float_to_fixed32(5.0+1.0, FX_IL);
    inB[6] = float_to_fixed32(6.0+1.0, FX_IL);
    inB[7] = float_to_fixed32(7.0+1.0, FX_IL);
    inB[8] = float_to_fixed32(8.0+1.0, FX_IL);
    inB[9] = float_to_fixed32(9.0+1.0, FX_IL);
    inB[10] = float_to_fixed32(10.0+1.0, FX_IL);
    inB[11] = float_to_fixed32(11.0+1.0, FX_IL);
    inB[12] = float_to_fixed32(12.0+1.0, FX_IL);
    inB[13] = float_to_fixed32(13.0+1.0, FX_IL);
    inB[14] = float_to_fixed32(14.0+1.0, FX_IL);
    inB[15] = float_to_fixed32(15.0+1.0, FX_IL);


	// Compute golden output

	// gold[0] = 90.0;
    // gold[1] = 100.0;
    // gold[2] = 110.0;
    // gold[3] = 120.0;
    // gold[4] = 202.0;
    // gold[5] = 228.0;
    // gold[6] = 254.0;
    // gold[7] = 280.0;
    // gold[8] = 314.0;
    // gold[9] = 356.0;
    // gold[10] = 398.0;
    // gold[11] = 440.0;
    // gold[12] = 426.0;
    // gold[13] = 484.0;
    // gold[14] = 542.0;
    // gold[15] = 600.0;

	gold[0] = float_to_fixed32(90.0, FX_IL);
    gold[1] = float_to_fixed32(100.0, FX_IL);
    gold[2] = float_to_fixed32(110.0, FX_IL);
    gold[3] = float_to_fixed32(120.0, FX_IL);
    gold[4] = float_to_fixed32(202.0, FX_IL);
    gold[5] = float_to_fixed32(228.0, FX_IL);
    gold[6] = float_to_fixed32(254.0, FX_IL);
    gold[7] = float_to_fixed32(280.0, FX_IL);
    gold[8] = float_to_fixed32(314.0, FX_IL);
    gold[9] = float_to_fixed32(356.0, FX_IL);
    gold[10] = float_to_fixed32(398.0, FX_IL);
    gold[11] = float_to_fixed32(440.0, FX_IL);
    gold[12] = float_to_fixed32(426.0, FX_IL);
    gold[13] = float_to_fixed32(484.0, FX_IL);
    gold[14] = float_to_fixed32(542.0, FX_IL);
    gold[15] = float_to_fixed32(600.0, FX_IL);
}

static void init_buf_all_1 (token_t *inA, token_t *inB, token_t * gold)
{
	// initialize input

	///////////// converting functions ///////////////
	// static inline int float_to_fixed32(float value, int n_int_bits)
	// static inline float fixed32_to_float(int value, int n_int_bits)
	//////////////////////////////////////////////////

	// inA[0] = 1; inA[1] = 5; inA[2] = 9; inA[3] = 13;
    // inA[4] = 2; inA[5] = 6; inA[6] = 10; inA[7] = 14; 
	// inA[8] = 3; inA[9] = 7; inA[10] = 11; inA[11] = 15; 
	// inA[12] = 4; inA[13] = 8; inA[14] = 12; inA[15] = 16;

	// inB[0] = 0+1; inB[1] = 1+1; inB[2] = 2+1; inB[3] = 3+1;
    // inB[4] = 4+1; inB[5] = 5+1; inB[6] = 6+1; inB[7] = 7+1;
    // inB[8] = 8+1; inB[9] = 9+1; inB[10] = 10+1; inB[11] = 11+1;
    // inB[12] = 12+1; inB[13] = 13+1; inB[14] = 14+1; inB[15] = 15+1;

	inA[0] = float_to_fixed32(1.0, FX_IL);
    inA[1] = float_to_fixed32(0.0, FX_IL);
    inA[2] = float_to_fixed32(0.0, FX_IL);
    inA[3] = float_to_fixed32(0.0, FX_IL);
    inA[4] = float_to_fixed32(0.0, FX_IL);
    inA[5] = float_to_fixed32(1.0, FX_IL);
    inA[6] = float_to_fixed32(0.0, FX_IL);
    inA[7] = float_to_fixed32(0.0, FX_IL);
    inA[8] = float_to_fixed32(0.0, FX_IL);
    inA[9] = float_to_fixed32(0.0, FX_IL);
    inA[10] = float_to_fixed32(1.0, FX_IL);
    inA[11] = float_to_fixed32(0.0, FX_IL);
    inA[12] = float_to_fixed32(0.0, FX_IL);
    inA[13] = float_to_fixed32(0.0, FX_IL);
    inA[14] = float_to_fixed32(0.0, FX_IL);
    inA[15] = float_to_fixed32(1.0, FX_IL);

	inB[0] = float_to_fixed32(1.0, FX_IL);
    inB[1] = float_to_fixed32(0.0, FX_IL);
    inB[2] = float_to_fixed32(0.0, FX_IL);
    inB[3] = float_to_fixed32(0.0, FX_IL);
    inB[4] = float_to_fixed32(0.0, FX_IL);
    inB[5] = float_to_fixed32(1.0, FX_IL);
    inB[6] = float_to_fixed32(0.0, FX_IL);
    inB[7] = float_to_fixed32(0.0, FX_IL);
    inB[8] = float_to_fixed32(0.0, FX_IL);
    inB[9] = float_to_fixed32(0.0, FX_IL);
    inB[10] = float_to_fixed32(1.0, FX_IL);
    inB[11] = float_to_fixed32(0.0, FX_IL);
    inB[12] = float_to_fixed32(0.0, FX_IL);
    inB[13] = float_to_fixed32(0.0, FX_IL);
    inB[14] = float_to_fixed32(0.0, FX_IL);
    inB[15] = float_to_fixed32(1.0, FX_IL);


	// Compute golden output

	// gold[0] = 90.0;
    // gold[1] = 100.0;
    // gold[2] = 110.0;
    // gold[3] = 120.0;
    // gold[4] = 202.0;
    // gold[5] = 228.0;
    // gold[6] = 254.0;
    // gold[7] = 280.0;
    // gold[8] = 314.0;
    // gold[9] = 356.0;
    // gold[10] = 398.0;
    // gold[11] = 440.0;
    // gold[12] = 426.0;
    // gold[13] = 484.0;
    // gold[14] = 542.0;
    // gold[15] = 600.0;

	gold[0] = float_to_fixed32(90.0, FX_IL);
    gold[1] = float_to_fixed32(100.0, FX_IL);
    gold[2] = float_to_fixed32(110.0, FX_IL);
    gold[3] = float_to_fixed32(120.0, FX_IL);
    gold[4] = float_to_fixed32(202.0, FX_IL);
    gold[5] = float_to_fixed32(228.0, FX_IL);
    gold[6] = float_to_fixed32(254.0, FX_IL);
    gold[7] = float_to_fixed32(280.0, FX_IL);
    gold[8] = float_to_fixed32(314.0, FX_IL);
    gold[9] = float_to_fixed32(356.0, FX_IL);
    gold[10] = float_to_fixed32(398.0, FX_IL);
    gold[11] = float_to_fixed32(440.0, FX_IL);
    gold[12] = float_to_fixed32(426.0, FX_IL);
    gold[13] = float_to_fixed32(484.0, FX_IL);
    gold[14] = float_to_fixed32(542.0, FX_IL);
    gold[15] = float_to_fixed32(600.0, FX_IL);

	// gold[0] = float_to_fixed32(1.0, FX_IL);
    // gold[1] = float_to_fixed32(5.0, FX_IL);
    // gold[2] = float_to_fixed32(9.0, FX_IL);
    // gold[3] = float_to_fixed32(13.0, FX_IL);
    // gold[4] = float_to_fixed32(2.0, FX_IL);
    // gold[5] = float_to_fixed32(6.0, FX_IL);
    // gold[6] = float_to_fixed32(10.0, FX_IL);
    // gold[7] = float_to_fixed32(14.0, FX_IL);
    // gold[8] = float_to_fixed32(3.0, FX_IL);
    // gold[9] = float_to_fixed32(7.0, FX_IL);
    // gold[10] = float_to_fixed32(11.0, FX_IL);
    // gold[11] = float_to_fixed32(15.0, FX_IL);
    // gold[12] = float_to_fixed32(4.0, FX_IL);
    // gold[13] = float_to_fixed32(8.0, FX_IL);
    // gold[14] = float_to_fixed32(12.0, FX_IL);
    // gold[15] = float_to_fixed32(16.0, FX_IL);
}


static void zero_mem_fixed32(token_t *mem1)
{
	for(int k = 0; k < rows*cols + rows*loaded_cols + loaded_cols*cols; k++){
		mem1[k] = float_to_fixed32(0.0, FX_IL);
	}
}

static void zero_mem_float32(float *mem1)
{
	for(int k = 0; k < rows*cols + rows*loaded_cols + loaded_cols*cols; k++){
		mem1[k] = 0.0;
	}
}


static void standard_input_initialization_fixed32(token_t* A, token_t* B) {
	A[0] = float_to_fixed32(1.0, FX_IL);
    A[1] = float_to_fixed32(5.0, FX_IL);
    A[2] = float_to_fixed32(9.0, FX_IL);
    A[3] = float_to_fixed32(13.0, FX_IL);
    A[4] = float_to_fixed32(2.0, FX_IL);
    A[5] = float_to_fixed32(6.0, FX_IL);
    A[6] = float_to_fixed32(10.0, FX_IL);
    A[7] = float_to_fixed32(14.0, FX_IL);
    A[8] = float_to_fixed32(3.0, FX_IL);
    A[9] = float_to_fixed32(7.0, FX_IL);
    A[10] = float_to_fixed32(11.0, FX_IL);
    A[11] = float_to_fixed32(15.0, FX_IL);
    A[12] = float_to_fixed32(4.0, FX_IL);
    A[13] = float_to_fixed32(8.0, FX_IL);
    A[14] = float_to_fixed32(12.0, FX_IL);
    A[15] = float_to_fixed32(16.0, FX_IL);

	B[0] = float_to_fixed32(0.0+1.0, FX_IL);
    B[1] = float_to_fixed32(1.0+1.0, FX_IL);
    B[2] = float_to_fixed32(2.0+1.0, FX_IL);
    B[3] = float_to_fixed32(3.0+1.0, FX_IL);
    B[4] = float_to_fixed32(4.0+1.0, FX_IL);
    B[5] = float_to_fixed32(5.0+1.0, FX_IL);
    B[6] = float_to_fixed32(6.0+1.0, FX_IL);
    B[7] = float_to_fixed32(7.0+1.0, FX_IL);
    B[8] = float_to_fixed32(8.0+1.0, FX_IL);
    B[9] = float_to_fixed32(9.0+1.0, FX_IL);
    B[10] = float_to_fixed32(10.0+1.0, FX_IL);
    B[11] = float_to_fixed32(11.0+1.0, FX_IL);
    B[12] = float_to_fixed32(12.0+1.0, FX_IL);
    B[13] = float_to_fixed32(13.0+1.0, FX_IL);
    B[14] = float_to_fixed32(14.0+1.0, FX_IL);
    B[15] = float_to_fixed32(15.0+1.0, FX_IL);
}

static void standard_input_initialization_float32(float* A, float* B) {
	A[0] = 1.0;
    A[1] = 5.0;
    A[2] = 9.0;
    A[3] = 13.0;
    A[4] = 2.0;
    A[5] = 6.0;
    A[6] = 10.0;
    A[7] = 14.0;
    A[8] = 3.0;
    A[9] = 7.0;
    A[10] = 11.0;
    A[11] = 15.0;
    A[12] = 4.0;
    A[13] = 8.0;
    A[14] = 12.0;
    A[15] = 16.0;

	B[0] = 0.0+1.0;
    B[1] = 1.0+1.0;
    B[2] = 2.0+1.0;
    B[3] = 3.0+1.0;
    B[4] = 4.0+1.0;
    B[5] = 5.0+1.0;
    B[6] = 6.0+1.0;
    B[7] = 7.0+1.0;
    B[8] = 8.0+1.0;
    B[9] = 9.0+1.0;
    B[10] = 10.0+1.0;
    B[11] = 11.0+1.0;
    B[12] = 12.0+1.0;
    B[13] = 13.0+1.0;
    B[14] = 14.0+1.0;
    B[15] = 15.0+1.0;
}

static void standard_golden_initialization_float32(float* golden)
{
	golden[0] = 90.0;
    golden[1] = 100.0;
    golden[2] = 110.0;
    golden[3] = 120.0;
    golden[4] = 202.0;
    golden[5] = 228.0;
    golden[6] = 254.0;
    golden[7] = 280.0;
    golden[8] = 314.0;
    golden[9] = 356.0;
    golden[10] = 398.0;
    golden[11] = 440.0;
    golden[12] = 426.0;
    golden[13] = 484.0;
    golden[14] = 542.0;
    golden[15] = 600.0;
}

static void standard_golden_initialization_fixed32(token_t* golden) 
{
	golden[0] = float_to_fixed32(90.0, FX_IL);
    golden[1] = float_to_fixed32(100.0, FX_IL);
    golden[2] = float_to_fixed32(110.0, FX_IL);
    golden[3] = float_to_fixed32(120.0, FX_IL);
    golden[4] = float_to_fixed32(202.0, FX_IL);
    golden[5] = float_to_fixed32(228.0, FX_IL);
    golden[6] = float_to_fixed32(254.0, FX_IL);
    golden[7] = float_to_fixed32(280.0, FX_IL);
    golden[8] = float_to_fixed32(314.0, FX_IL);
    golden[9] = float_to_fixed32(356.0, FX_IL);
    golden[10] = float_to_fixed32(398.0, FX_IL);
    golden[11] = float_to_fixed32(440.0, FX_IL);
    golden[12] = float_to_fixed32(426.0, FX_IL);
    golden[13] = float_to_fixed32(484.0, FX_IL);
    golden[14] = float_to_fixed32(542.0, FX_IL);
    golden[15] = float_to_fixed32(600.0, FX_IL);
}




static void random_input_initialization_fixed32(token_t *A, token_t *B)
{
	for (int i = 0; i < rows*loaded_cols; ++i) {
		float tmp = i % 3;
		A[i] = float_to_fixed32(tmp, FX_IL);
	}
	for (int i = 0; i < loaded_cols*cols; ++i) {
		float tmp = i % 3;
		B[i] = float_to_fixed32(tmp, FX_IL);
	}
}

static void random_input_initialization_float32(float *A, float *B)
{
	for (int i = 0; i < rows*loaded_cols; ++i) {
		float tmp = i % 3;
		A[i] = tmp;
	}
	for (int i = 0; i < loaded_cols*cols; ++i) {
		float tmp = i % 3;
		B[i] = tmp;
	}
}

static void random_golden_initialization_float32(float* A, float* B, float* gold) {

	for (int j = 0; j < rows; ++j) {
		for (int k = 0; k < cols; ++k) {
			gold[j*cols+k] = 0;
		}	
	}
	
	for (int i = 0; i < loaded_cols; ++i) {
		for (int j = 0; j < rows; ++j) {
			for (int k = 0; k < cols; ++k) {
				float partial_sum = A[i*rows + j] * B[i*cols + k];
				gold[j*cols + k] += partial_sum;
			}
		}
	}
	return;
}

static void random_golden_initialization_fixed32(float* gold_cpu, token_t* gold_acc) {

	// for (int j = 0; j < rows; ++j) {
	// 	for (int k = 0; k < cols; ++k) {
	// 		gold_cpu[i*cols+j] = 0;
	// 	}	
	// }
	
	// for (int i = 0; i < loaded_cols; ++i) {
	// 	for (int j = 0; j < rows; ++j) {
	// 		for (int k = 0; k < cols; ++k) {
	// 			float partial_sum = A[i*rows + j] * B[i*cols + k];
	// 			gold_cpu[j*cols + k] += partial_sum;
	// 		}
	// 	}
	// }

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			gold_acc[i*cols + j] = float_to_fixed32(gold_cpu[i*cols + j], FX_IL);
		}
	}
	return;
}


static void validation_float32(float* out, float* gold) {
	for (int j = 0; j < rows; ++j) {
		for (int k = 0; k < cols; ++k) {
			if (gold[j*cols + k] != out[j*cols + k]) {
				printf("failed in float 32 (cpu) verification\n");
			}
		}
	}
}

static void validation_fixed32(token_t* out, token_t* gold) {
	for (int j = 0; j < rows; ++j) {
		for (int k = 0; k < cols; ++k) {
			if (gold[j*cols + k] != out[j*cols + k]) {
				printf("failed in fixed 32 (acc) verification\n");
			}
		}
	}
}

// static void zero_mems(token_t *mem1, token_t *mem2, token_t * mem3)
// {
// 	for(int k = 0; k < 48; k++){
// 		mem1[k] = float_to_fixed32(0.0, FX_IL);
// 		mem2[k] = float_to_fixed32(0.0, FX_IL);
// 		mem3[k] = float_to_fixed32(0.0, FX_IL);
// 	}
// }



/* User-defined code */
static void init_parameters()
{
	inA_len = rows*loaded_cols;
	inB_len = loaded_cols*cols;
	out_len = rows*cols;
	inA_size = inA_len * sizeof(token_t);
	inB_size = inB_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = inA_len + inB_len;
	mem_size = inA_size + inB_size + out_size;
	input_size = inA_size + inB_size;
}


int main(int argc, char **argv)
{
	int errors;

	// token_t *mem;
	token_t *mem_acc;
	float* mem_cpu;
	// token_t *mem2;
	// token_t *mem3;
	token_t* A_acc;
	float* A_cpu;
	token_t* B_acc;
	float* B_cpu;
	// token_t *Ap2;
	// token_t *Bp2;
	// token_t *Ap3;
	// token_t *Bp3;
	token_t *gold_acc;
	float* gold_cpu;
	// token_t *buf;
	unsigned long long time_s;
	struct timespec t_test_1, t_test_2;


	init_parameters();

	////////////// memory layout ///////////////////
	//
	// ---------------------------------------- mem1
	//               A1
	// ---------------------------------------- 
	//               B1
	// ---------------------------------------- mem2
	//               A2
	// ----------------------------------------
	//               B2
	// ---------------------------------------- mem3
	//               dummy
	// ---------------------------------------- 
	//               dummy
	// ---------------------------------------- 
	//               O
	// ----------------------------------------
	//
	////////////////////////////////////////////////

	// mem = (token_t *) esp_alloc(mem_size*3);
	// mem1 = mem;
	// mem2 = mem + mem_size;
	// mem3 = mem + 2*mem_size;

	mem_acc = (token_t *) esp_alloc(mem_size);
	mem_cpu = (float *) esp_alloc(mem_size);
	// mem2 = (token_t *) esp_alloc(mem_size);
	// mem3 = (token_t *) esp_alloc(mem_size);


	cfg_000[0].hw_buf = mem_acc;
	// cfg_000[1].hw_buf = mem2;
	// cfg_000[2].hw_buf = mem3;
    
	gold_acc = malloc(out_size);
	gold_cpu = malloc(out_size);

	A_acc = mem_acc;
	B_acc = mem_acc;
	B_acc += inA_len;

	A_cpu = mem_cpu;
	B_cpu = mem_cpu;
	B_cpu += inA_len;
	float* O_cpu = &mem_cpu[out_offset];

	int cpu_rows = rows;
	int cpu_cols = cols;
	int cpu_loaded_cols = loaded_cols;
	// float* gold_cpu = gold;

	if (argc > 1) {
		printf("this is cpu flow\n");
		zero_mem_float32(A_cpu);
		// standard_input_initialization_float32(A_cpu, B_cpu);
		// standard_golden_initialization_float32(gold_cpu);
		random_input_initialization_float32(A_cpu, B_cpu);
		random_golden_initialization_float32(A_cpu, B_cpu, gold_cpu);

		gettime(&t_test_1);
		
		for (int i = 0; i < cpu_loaded_cols; ++i) {
			for (int j = 0; j < cpu_rows; ++j) {
				for (int k = 0; k < cpu_cols; ++k) {

					float partial_sum = A_cpu[i*cpu_rows + j] * B_cpu[i*cpu_cols + k];
					// printf("=== (loaded_cols, rows, cols) =  (%d, %d, %d)\n", i, j, k);
					// printf("A[%d] = %f\n", i*cpu_rows + j, A_cpu[i*cpu_rows + j]);
					// printf("B[%d] = %f\n", i*cpu_cols + k, B_cpu[i*cpu_cols + k]);
					// printf("the before O[%d] = %f\n", j*cpu_cols + k, O_cpu[j*cpu_cols + k]);
					O_cpu[j*cpu_cols + k] += partial_sum;
					// printf("the O[%d] = %f\n", j*cpu_cols + k, O_cpu[j*cpu_cols + k]);
				}
			}
		}

		gettime(&t_test_2);
		time_s = ts_subtract(&t_test_1, &t_test_2);
		printf("-------------------------------------------------------\n");
		printf("Finish testing. time of test: %llu (ns)\n", time_s);
		printf("-------------------------------------------------------\n");


		// validation
		validation_float32(O_cpu, gold_cpu);
		// for (int j = 0; j < cpu_rows; ++j) {
		// 	for (int k = 0; k < cpu_cols; ++k) {
		// 		if (gold_cpu[j*cpu_cols + k] != O_cpu[j*cpu_cols + k]) {
		// 			printf("failed in cpu verification\n");
		// 		}
		// 	}
		// }

		// free the memory
		free(gold_acc);
		free(gold_cpu);
		esp_free(mem_acc);
		esp_free(mem_cpu);

		return 0;


	}

	zero_mem_fixed32(mem_acc);
	random_input_initialization_float32(A_cpu, B_cpu);
	random_golden_initialization_float32(A_cpu, B_cpu, gold_cpu);
	random_input_initialization_fixed32(A_acc, B_acc);
	random_golden_initialization_fixed32(gold_cpu, gold_acc);
	printf("really enter the acc flow\n");


	// #### this is the original flow ####
	// init_buf(Ap1, Bp1, gold);
	// ####################################


	// init_buf_all_1(Ap2, Bp2, gold);
	// init_buf(Ap3, Bp3, gold);
#ifdef DBG_APP
	dump_A_and_B(Ap1, Bp1);
	printf("=============== before calculation and validation ==================\n");
	printf("=============== this is A1:\n");
	dump_mem(mem1);
#endif
	// printf("=============== this is A2:\n");
	// dump_mem(mem2);
	// printf("=============== this is O:\n");
	// dump_mem(mem3);
	// printf("=============== this is the total memory layout:\n");
	// dump_3mem(mem);
	// printf("=============== this is O:\n");
	// dump_mem(mem3);

	// dump_mem(mem);

	// printf("\n====== %s ======\n\n", cfg_000[0].devname);
	// /* <<--print-params-->> */
	// printf("  .rows = %d\n", rows);
	// printf("  .cols = %d\n", cols);
	// printf("  .loaded_cols = %d\n", loaded_cols);
	// printf("\n  ** START **\n");

	// token_t * Op = &mem1[out_offset];
	// if (argc > 1) {
	// 	printf("this is cpu flow\n");
	// 	for (int i = 0; i < loaded_cols; ++i) {
	// 		for (int j = 0; j < rows; ++j) {
	// 			for (int k = 0; k < cols; ++k) {

	// 				int partial_sum = Ap1[i*rows + j] + Bp1[i*cols + k];
	// 				printf("(loaded_cols, rows, cols) =  (%d, %d, %d)\n");
	// 				printf("partial_sum = %d\n", partial_sum);
	// 				Op[j*cols + k] += partial_sum;
	// 				printf("the O[%d] = %d\n", j*cols + k, partial_sum);
	// 			}
	// 		}
	// 	}
	// } else {
	
	gettime(&t_test_1);
	esp_run(cfg_000, 1);
	gettime(&t_test_2);
	time_s = ts_subtract(&t_test_1, &t_test_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test: %llu (ns)\n", time_s);
    printf("-------------------------------------------------------\n");
	// }
	

	// printf("\n  ** DONE **\n");

	// errors = validate_buf(&mem3[out_offset], gold);
	// errors = validate_buf(&mem1[out_offset], gold);
	// errors = validate_buf(&mem2[out_offset], gold);
	errors = validate_buf(&mem_acc[out_offset], gold_acc);
	// // char ans[] = "answer";
	// // dump_Matrix(&mem3[out_offset], 16, ans);
	// printf("=============== after calculation and validation ==================\n");
	// printf("=============== this is A1:\n");
	// // dump_mem(mem1);
	// // // printf("=============== this is A2:\n");
	// // dump_mem(mem2);
	// // printf("=============== this is O:\n");
#ifdef DBG_APP
	dump_mem(mem_acc);
#endif

	free(gold_acc);
	free(gold_cpu);
	esp_free(mem_acc);
	esp_free(mem_cpu);
	// esp_free(mem2);
	// esp_free(mem3);
#ifdef DBG_APP
	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
#endif
	return errors;
}
