// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    printf("****start*****\n");

    /* <<--params-->> */
    const unsigned matrix_A_dim = 5;
    const unsigned matrix_B_dim = 3;
    const unsigned matrix_C_dim = 3;
    unsigned state_control = 0;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    in_words_adj  = round_up(matrix_A_dim * matrix_A_dim + matrix_B_dim * matrix_B_dim, VALUES_PER_WORD);
    out_words_adj = round_up(matrix_C_dim * matrix_C_dim, VALUES_PER_WORD);
    in_size       = in_words_adj * (1);
    out_size      = out_words_adj * (1);

    dma_in_size  = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size     = dma_in_size + dma_out_size;

    dma_word_t *mem          = (dma_word_t *)malloc(dma_size * sizeof(dma_word_t));
    word_t *    inbuff       = (word_t *)malloc(in_size * sizeof(word_t));
    word_t *    outbuff      = (word_t *)malloc(out_size * sizeof(word_t));
    word_t *    outbuff_gold = (word_t *)malloc(out_size * sizeof(word_t));
    dma_info_t  load;
    dma_info_t  store;


    uint32_t index_A = 0;
    uint32_t index_B = 0;
    // Prepare input data (matrix A)
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_A_dim * matrix_A_dim; j++) {
            index_A = i * in_words_adj + j;
            inbuff[index_A] = (word_t)(j + 1);
        }
    }
    index_A += 1;
    
    // Prepare input data (matrix B)
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_B_dim * matrix_B_dim; j++) {
            index_B = i * in_words_adj + j;
            inbuff[index_A+ index_B] = (word_t)(j + 1);
        }
    }

    for (unsigned i = 0; i < dma_in_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];
        }
    }

    // Set golden output
    // for (unsigned i = 0; i < 1; i++) {
    //     for (unsigned j = 0; j < matrix_C_dim * matrix_C_dim; j++) {
    //         outbuff_gold[i * out_words_adj + j] = (word_t)j;
    //     }
    // }
    outbuff_gold[0] = (word_t)219;
    outbuff_gold[1] = (word_t)264;
    outbuff_gold[2] = (word_t)309;
    outbuff_gold[3] = (word_t)444;
    outbuff_gold[4] = (word_t)489;
    outbuff_gold[5] = (word_t)534;
    outbuff_gold[6] = (word_t)669;
    outbuff_gold[7] = (word_t)714;
    outbuff_gold[8] = (word_t)759;
    printf("outbuff_gold[0]: %d\n", (int)outbuff_gold[0]);
    printf("outbuff_gold[1]: %d\n", (int)outbuff_gold[1]);
    printf("outbuff_gold[2]: %d\n", (int)outbuff_gold[2]);
    printf("outbuff_gold[3]: %d\n", (int)outbuff_gold[3]);
    printf("outbuff_gold[3]: %d\n", (int)outbuff_gold[3]);
    printf("outbuff_gold[4]: %d\n", (int)outbuff_gold[4]);
    printf("outbuff_gold[5]: %d\n", (int)outbuff_gold[5]);
    printf("outbuff_gold[6]: %d\n", (int)outbuff_gold[6]);
    printf("outbuff_gold[7]: %d\n", (int)outbuff_gold[7]);
    printf("outbuff_gold[8]: %d\n", (int)outbuff_gold[8]);

    // Call the TOP function
    state_control = 1; // only load
    top(mem, mem,
        /* <<--args-->> */
        matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, load, store);

    state_control = 2; // only compute and store
    top(mem, mem,
        /* <<--args-->> */
        matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, load, store);

    // state_control = 3; // only store
    // top(mem, mem,
    //     /* <<--args-->> */
    //     matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, load, store);


    // Validate
    uint32_t out_offset = dma_in_size;
    for (unsigned i = 0; i < dma_out_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];
        }
    }

    int errors = 0;
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_C_dim * matrix_C_dim; j++) {
            int index = i * out_words_adj + j;
            if (outbuff[index] != outbuff_gold[index]) {
                errors++;
            }
            printf("C[%d]: out: %d, \tgold: %d\n", index, (int)outbuff[index], (int)outbuff_gold[index]);
        }
    }

    if (errors) {
        std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    } else {
        std::cout << "Test PASSED." << std::endl;
    }

    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
