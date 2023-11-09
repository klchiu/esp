// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
	 const unsigned matrix_C_dim = 2;
	 const unsigned matrix_A_dim = 2;
	 const unsigned matrix_B_dim = 2;
     unsigned state_control = 0;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;


    in_words_adj = round_up(matrix_A_dim * matrix_A_dim * 2, VALUES_PER_WORD);
    out_words_adj = round_up(matrix_C_dim * matrix_C_dim, VALUES_PER_WORD);
    in_size = in_words_adj * (1);
    out_size = out_words_adj * (1);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff=(word_t*) malloc(in_size * sizeof(word_t));
    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));
    dma_info_t load;
    dma_info_t store;

    // Prepare input data
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < matrix_A_dim * matrix_A_dim * 2; j++)
            inbuff[i * in_words_adj + j] = (word_t) j;

    for(unsigned i = 0; i < dma_in_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];

    // Set golden output
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < matrix_C_dim * matrix_C_dim; j++)
            outbuff_gold[i * out_words_adj + j] = (word_t) j;


    // Call the TOP function
    state_control = 1; // only load
    top(mem, mem,
        /* <<--args-->> */
	 	 matrix_C_dim,
	 	 matrix_A_dim,
	 	 matrix_B_dim,
         state_control,
        load, store);

    state_control = 2; // compute and store
    top(mem, mem,
        /* <<--args-->> */
	 	 matrix_C_dim,
	 	 matrix_A_dim,
	 	 matrix_B_dim,
         state_control,
        load, store);


    // Validate
    uint32_t out_offset = dma_in_size;
    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

    int errors = 0;
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < matrix_C_dim * matrix_C_dim; j++)
	    if (outbuff[i * out_words_adj + j] != outbuff_gold[i * out_words_adj + j])
		errors++;

    if (errors)
	std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    else
	std::cout << "Test PASSED." << std::endl;

    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
