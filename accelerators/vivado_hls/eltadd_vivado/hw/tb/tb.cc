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
    const unsigned matrix_dim_x  = 24;
    const unsigned matrix_dim_y  = 8;
    unsigned       state_control = 0;

    uint32_t in_1_words_adj;
    uint32_t in_2_words_adj;
    uint32_t out_words_adj;
    uint32_t in_1_size;
    uint32_t in_2_size;
    uint32_t out_size;
    uint32_t dma_in_1_size;
    uint32_t dma_in_2_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    in_1_words_adj = round_up(matrix_dim_x * matrix_dim_y, VALUES_PER_WORD);
    in_2_words_adj = round_up(matrix_dim_x * matrix_dim_y, VALUES_PER_WORD);
    out_words_adj  = round_up(matrix_dim_x * matrix_dim_y, VALUES_PER_WORD);
    in_1_size      = in_1_words_adj * (1);
    in_2_size      = in_2_words_adj * (1);
    out_size       = out_words_adj * (1);

    dma_in_1_size = in_1_size / VALUES_PER_WORD;
    dma_in_2_size = in_2_size / VALUES_PER_WORD;
    dma_out_size  = out_size / VALUES_PER_WORD;
    dma_size      = dma_in_1_size + dma_in_2_size + dma_out_size;

    dma_word_t *mem          = (dma_word_t *)malloc(dma_size * sizeof(dma_word_t));
    word_t *    inbuff_1     = (word_t *)malloc(in_1_size * sizeof(word_t));
    word_t *    inbuff_2     = (word_t *)malloc(in_2_size * sizeof(word_t));
    word_t *    outbuff      = (word_t *)malloc(out_size * sizeof(word_t));
    word_t *    outbuff_gold = (word_t *)malloc(out_size * sizeof(word_t));
    dma_info_t  load;
    dma_info_t  store;

    // -- VALUES_PER_WORD = 2
    // printf(" ---- tb: VALUES_PER_WORD = %d\n", VALUES_PER_WORD);

    // Prepare input data
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            inbuff_1[i * in_1_words_adj + j] = (word_t)(j + 2);
        }

        for (unsigned j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            inbuff_2[i * in_2_words_adj + j] = (word_t)(j + 7);
        }
    }

    for (unsigned i = 0; i < dma_in_1_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            mem[i].word[k] = inbuff_1[i * VALUES_PER_WORD + k];
        }
    }

    for (unsigned i = 0; i < dma_in_2_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            mem[dma_in_1_size + i].word[k] = inbuff_2[i * VALUES_PER_WORD + k];
        }
    }

    printf("dma_in_1_size: %d\n", dma_in_1_size);
    printf("dma_in_2_size: %d\n", dma_in_2_size);

    // Set golden output
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            outbuff_gold[i * out_words_adj + j] =
                (word_t)((int)inbuff_1[i * in_1_words_adj + j] + (int)inbuff_2[i * in_2_words_adj + j]);
        }
    }
    // printf("outbuff_gold[0]: %d\n", (int)outbuff_gold[0]);
    // printf("outbuff_gold[1]: %d\n", (int)outbuff_gold[1]);
    // printf("outbuff_gold[2]: %d\n", (int)outbuff_gold[2]);
    // printf("outbuff_gold[3]: %d\n", (int)outbuff_gold[3]);

    // Call the TOP function
    state_control = 1; // only load
    top(mem, mem, mem,
        /* <<--args-->> */
        matrix_dim_x, matrix_dim_y, state_control, load, store);

    state_control = 2; // only compute and store
    top(mem, mem, mem,
        /* <<--args-->> */
        matrix_dim_x, matrix_dim_y, state_control, load, store);

    // Validate
    uint32_t out_offset = dma_in_1_size + dma_in_2_size;
    for (unsigned i = 0; i < dma_out_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];
        }
    }

    int errors = 0;
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < matrix_dim_x * matrix_dim_y; j++) {
            int index = i * out_words_adj + j;
            if (outbuff[index] != outbuff_gold[index]) {
                errors++;
            }
            if (errors < 10) {
                printf("validate: index: %d, \tout: %d, \tgold: %d\n", index, (int)outbuff[index],
                       (int)outbuff_gold[index]);
            }
        }
    }

    if (errors) {
        printf("Test FAILED with %d errors.\n", errors);
    } else {
        printf("Test PASSED.\n");
    }

    // Free memory

    free(mem);
    free(inbuff_1);
    free(inbuff_2);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
