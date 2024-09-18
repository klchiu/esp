/* Copyright (c) 2011-2024 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stdio.h>
#ifndef __riscv
#include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

typedef int32_t token_t;
#define FX_IL 12

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


#define SLD_HUANGEMM3S64 0x02c
#define DEV_NAME "sld,huangemm3s64_stratus"

/* <<--params-->> */
const int32_t rows = 4;
const int32_t cols = 4;
const int32_t loaded_cols = 4;

// const int32_t rows = 2;
// const int32_t cols = 1;
// const int32_t loaded_cols = 4;


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

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?		\
			(_sz / CHUNK_SIZE) :		\
			(_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */
// #define HUANGEMM3S64_HUANGEMM3S64_N_REG 0x48
// #define HUANGEMM3S64_HUANGEMM3S64_VEC_REG 0x44
// #define HUANGEMM3S64_HUANGEMM3S64_LEN_REG 0x40

#define HUANGEMM3S64_ROWS_REG 0x48
#define HUANGEMM3S64_COLS_REG 0x44
#define HUANGEMM3S64_LOADED_COLS_REG 0x40



static void print_fixed32(token_t fixed_num, int integer_bit) {

	int float_bit = 32 - integer_bit;
	// printf("float bit: %d\n", float_bit);
	// preserve the floating points
	int huan_mask = (1 << float_bit) - 1;

	int integer_part = fixed_num >> float_bit;
	long long frac_part = ((double)(fixed_num & huan_mask) / (double)(1 << float_bit)) * 100000 ;


	printf("%d.%lld", integer_part, frac_part);
}


static void dump_A_and_B(token_t *A, token_t *B){
	printf("=============== dump A and B =============\n");
	for (int i = 0; i < loaded_cols; i++){
		for (int j = 0; j < rows; j++){
			printf("A[%d] = ", i * rows + j);
			print_fixed32(A[i * rows + j], FX_IL);
			// printf("%d", A[i * cols + j]);
			printf("\n");
		}
	}

	for (int i = 0; i < loaded_cols; i++){
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

static int validate_buf(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;
	// float tmp1;
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
				printf("gold[%d] = ", i * cols + j);
				print_fixed32(gold[i * cols + j], FX_IL);
				printf(", out[%d] = ", i * cols + j);
				print_fixed32(out[i * cols + j], FX_IL);
				printf("\n");
				////////////////// please use the above part ///////////////


				// printf(stderr, "gold[%d] = %f, ", i * cols + j, tmp1);
				
				// printf(stderr, "out[%d] = %f\n", i * cols + j, tmp2);
				errors++;
			}
		}
	}
		
	return errors;
}


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


int main(int argc, char * argv[])
{
	int i;
	int n;
	int ndev;
	struct esp_device *espdevs;
	struct esp_device *dev;
	unsigned done;
	unsigned **ptable;
	token_t *mem;
	token_t *Ap;
	token_t *Bp;
	token_t *gold;
	unsigned errors = 0;
	unsigned coherence;

	// if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
	// 	in_words_adj = huangemm3s64_len*huangemm3s64_vec;
	// 	out_words_adj = huangemm3s64_vec;
	// } else {
	// 	in_words_adj = round_up(huangemm3s64_len*huangemm3s64_vec, DMA_WORD_PER_BEAT(sizeof(token_t)));
	// 	out_words_adj = round_up(huangemm3s64_vec, DMA_WORD_PER_BEAT(sizeof(token_t)));
	// }


	// in_len = in_words_adj * (huangemm3s64_n);
	// out_len = out_words_adj * (huangemm3s64_n);
	// in_size = in_len * sizeof(token_t);
	// out_size = out_len * sizeof(token_t);
	// out_offset  = in_len;

	inA_len = rows*loaded_cols;
	inB_len = loaded_cols*cols;
	out_len = rows*cols;
	inA_size = inA_len * sizeof(token_t);
	inB_size = inB_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = inA_len + inB_len;
	
	mem_size = inA_size + inB_size + out_size;


	// Search for the device
	printf("Scanning device tree... \n");

	ndev = probe(&espdevs, VENDOR_SLD, SLD_HUANGEMM3S64, DEV_NAME);
	if (ndev == 0) {
		printf("huangemm3s64 not found\n");
		return 0;
	}

	for (n = 0; n < ndev; n++) {

		printf("**************** %s.%d ****************\n", DEV_NAME, n);

		dev = &espdevs[n];

		// Check DMA capabilities
		if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
			printf("  -> scatter-gather DMA is disabled. Abort.\n");
			return 0;
		}

		if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
			printf("  -> Not enough TLB entries available. Abort.\n");
			return 0;
		}

		// Allocate memory
		gold = aligned_malloc(out_size);
		mem = aligned_malloc(mem_size);
		
		Ap = mem;
		printf("  Ap buffer base-address = %p\n", Ap);
		
		Bp = mem;
		
		Bp += inA_len;
		printf("  Bp buffer base-address = %p\n", Bp);
		printf("  memory buffer base-address = %p\n", mem);
		

		// Alocate and populate page table
		ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
		for (i = 0; i < NCHUNK(mem_size); i++)
			ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];

		printf("  ptable = %p\n", ptable);
		printf("  nchunk = %lu\n", NCHUNK(mem_size));

#ifndef __riscv
		for (coherence = ACC_COH_NONE; coherence <= ACC_COH_RECALL; coherence++) {
#else
		{
			/* TODO: Restore full test once ESP caches are integrated */
			coherence = ACC_COH_NONE;
#endif
			printf("  --------------------\n");
			printf("  Generate input...\n");
			init_buf(Ap, Bp, gold);
			printf("  Ap buffer base-address = %p\n", Ap);
			printf("  Bp buffer base-address = %p\n", Bp);
			printf("huananananana\n");
			dump_A_and_B(Ap, Bp);
			dump_mem(mem);
			printf("huananananana\n");

			// Pass common configuration parameters

			iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
			iowrite32(dev, COHERENCE_REG, coherence);

#ifndef __sparc
			iowrite32(dev, PT_ADDRESS_REG, (unsigned long long) ptable);
#else
			iowrite32(dev, PT_ADDRESS_REG, (unsigned) ptable);
#endif
			iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
			iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

			// Use the following if input and output data are not allocated at the default offsets
			iowrite32(dev, SRC_OFFSET_REG, 0x0);
			iowrite32(dev, DST_OFFSET_REG, 0x0);

			// Pass accelerator-specific configuration parameters
			/* <<--regs-config-->> */
			iowrite32(dev, HUANGEMM3S64_ROWS_REG, rows);
			iowrite32(dev, HUANGEMM3S64_COLS_REG, cols);
			iowrite32(dev, HUANGEMM3S64_LOADED_COLS_REG, loaded_cols);

			// Flush (customize coherence model here)
			esp_flush(coherence);

			// Start accelerators
			printf("  Start...\n");
			iowrite32(dev, CMD_REG, CMD_MASK_START);

			// Wait for completion
			done = 0;
			while (!done) {
				done = ioread32(dev, STATUS_REG);
				done &= STATUS_MASK_DONE;
			}
			iowrite32(dev, CMD_REG, 0x0);

			printf("  Done\n");
			printf("  validating...\n");

			/* Validation */
			dump_A_and_B(Ap, Bp);
			dump_mem(mem);
			errors = validate_buf(&mem[out_offset], gold);
			if (errors)
				printf("  ... FAIL\n");
			else
				printf("  ... PASS\n");
		}
		aligned_free(ptable);
		aligned_free(mem);
		aligned_free(gold);
	}

	return 0;
}
