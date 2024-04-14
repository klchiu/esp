// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_matrix(int d1, int d2, int *m1_data)
{
    printf("--------------------------------------------\n");
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d2; j++) {
            printf("%d, ", m1_data[i * d2 + j]);
        }
        printf("\n");
    }
    printf("--------------------------------------------\n");
}

void init_m1(int d1, int d2, int *m1_data)
{
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d2; j++) {
            m1_data[i * d2 + j] = i * d2 + j;
        }
    }

    print_matrix(d1, d2, m1_data);
}

void init_m2(int d2, int d3, int *m2_data)
{
    for (int i = 0; i < d2; i++) {
        for (int j = 0; j < d3; j++) {
            m2_data[i * d3 + j] = i * d3 + j + 100;
        }
    }

    print_matrix(d2, d3, m2_data);
}

void gemm_pv(int d1, int d2, int d3, int *m1_data, int *m2_data, int *m3_data)
{
    for (int i = 0; i < d1; i++) {
        for (int k = 0; k < d3; k++) {
            for (int j = 0; j < d2; j++) {
                m3_data[i * d3 + k] += m1_data[i * d2 + j] * m2_data[j * d3 + k];
                // printf("m1_data[%d] = %d\t", i*d2+j, m1_data[i*d2+j]);
                // printf("m2_data[%d] = %d\t", j*d3+k, m2_data[j*d3+k]);
                // printf("m3_data[%d] = %d\n", i*d3+k, m3_data[i*d3+k]);
            }
        }
    }
}

int main(int argc, char **argv)
{

    printf("**** start *****\n");

    //  Memory allocation:
    //
    //  =========================   ^
    //  |     matrix3 (output)   |   |  (dma_out_size)
    //  =========================   ^
    //  |     matrix1 (input1)   |   |  (dma_in1_size)
    //  =========================   v
    //  |     matrix2 (input2)   |   |  (dma_in2_size)
    //  =========================   v

    /* <<--params-->> */
    // const unsigned d1        = 32;
    // const unsigned d2        = 32;
    // const unsigned d3        = 32;
    // const unsigned m3_offset = 0;
    // const unsigned m1_offset = 1024;
    // const unsigned m2_offset = 2048;

    const unsigned d1        = 8;
    const unsigned d2        = 8;
    const unsigned d3        = 8;
    const unsigned m3_offset = 0;
    const unsigned m1_offset = d1 * d3;
    const unsigned m2_offset = m1_offset + d1 * d2;

    // uint32_t in1_words_adj;
    // uint32_t in2_words_adj;
    // uint32_t out_words_adj;
    uint32_t in1_size = d1 * d2;
    uint32_t in2_size = d2 * d3;
    uint32_t out_size = d1 * d3;
    uint32_t dma_in1_size;
    uint32_t dma_in2_size;
    uint32_t dma_out_size;
    uint32_t dma_size;

    // in1_words_adj = round_up(m1_offset, VALUES_PER_WORD);
    // in2_words_adj = round_up(m2_offset, VALUES_PER_WORD);
    // out_words_adj = round_up(m3_offset, VALUES_PER_WORD);

    // in1_size = in1_words_adj * (1);
    // in2_size = in2_words_adj * (1);
    // out_size = out_words_adj * (1);

    dma_in1_size = in1_size / VALUES_PER_WORD;
    dma_in2_size = in2_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size     = dma_in1_size + dma_in1_size + dma_out_size;

    // std::cout << "[PRINT]: in1_words_adj = " << in1_words_adj << std::endl;
    // std::cout << "[PRINT]: in2_words_adj = " << in2_words_adj << std::endl;
    // std::cout << "[PRINT]: out_words_adj = " << out_words_adj << std::endl;
    std::cout << "[PRINT]: in1_size = " << in1_size << std::endl;
    std::cout << "[PRINT]: in2_size = " << in2_size << std::endl;
    std::cout << "[PRINT]: out_size = " << out_size << std::endl;
    std::cout << "[PRINT]: dma_in1_size = " << dma_in1_size << std::endl;
    std::cout << "[PRINT]: dma_in2_size = " << dma_in2_size << std::endl;
    std::cout << "[PRINT]: dma_out_size = " << dma_out_size << std::endl;
    std::cout << "[PRINT]: dma_size = " << dma_size << std::endl;

    dma_word_t *mem          = (dma_word_t *)malloc(dma_size * sizeof(dma_word_t));
    word_t *    in1buff      = (word_t *)malloc(in1_size * sizeof(word_t));
    word_t *    in2buff      = (word_t *)malloc(in2_size * sizeof(word_t));
    word_t *    outbuff      = (word_t *)malloc(out_size * sizeof(word_t));
    word_t *    outbuff_gold = (word_t *)malloc(out_size * sizeof(word_t));
    dma_info_t  load;
    dma_info_t  store;

    int *m1_data = (int *)malloc(d1 * d2 * sizeof(int));
    int *m2_data = (int *)malloc(d2 * d3 * sizeof(int));
    int *m3_data = (int *)malloc(d1 * d3 * sizeof(int));
    init_m1(d1, d2, m1_data);
    init_m2(d2, d3, m2_data);

    // Prepare input data (m1)
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < d1 * d2; j++) {
            // in1buff[i * in1_words_adj + j] = (word_t)m1_data[j];
            in1buff[j] = (word_t)m1_data[j];
        }
    }

    for (unsigned i = 0; i < dma_in1_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            mem[m1_offset + i].word[k] = in1buff[i * VALUES_PER_WORD + k];
        }
    }

    // Prepare input data (m2)
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < d2 * d3; j++) {
            // in2buff[i * in2_words_adj + j] = (word_t)m2_data[j];
            in2buff[j] = (word_t)m2_data[j];
        }
    }

    for (unsigned i = 0; i < dma_in2_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            mem[m2_offset + i].word[k] = in2buff[i * VALUES_PER_WORD + k];
        }
    }

    // Calculate golden output
    memset(m3_data, 0, d1 * d3 * sizeof(int));
    // print_matrix(d1, d3, m3_data);
    gemm_pv(d1, d2, d3, m1_data, m2_data, m3_data);
    print_matrix(d1, d3, m3_data);

    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < m1_offset; j++) {
            // outbuff_gold[i * out_words_adj + j] = (word_t)m3_data[j];
            outbuff_gold[j] = (word_t)m3_data[j];
        }
    }

    // Call the TOP function
    top(mem, mem, mem,
        /* <<--args-->> */
        m3_offset, d3, d2, d1, m2_offset, m1_offset, load, store);

    // Retrieve output
    for (unsigned i = 0; i < dma_out_size; i++) {
        for (unsigned k = 0; k < VALUES_PER_WORD; k++) {
            outbuff[i * VALUES_PER_WORD + k] = mem[m3_offset + i].word[k];
        }
    }

    // Validate
    int errors = 0;
    for (unsigned i = 0; i < 1; i++) {
        for (unsigned j = 0; j < d1 * d3; j++) {
            // if (outbuff[i * out_words_adj + j] != outbuff_gold[i * out_words_adj + j]) {
            if (outbuff[j] != outbuff_gold[j]) {
                errors++;
                printf("[%d]: outbuff = %d\tgold = %d\n", j, (int)outbuff[j], (int)outbuff_gold[j]);
            }
        }
    }

    if (errors) {
        std::cout << "Test FAILED with " << errors << " errors." << std::endl;
    } else {
        std::cout << "Test PASSED." << std::endl;
    }

    std::cout << "Alright! It's DONE!" << std::endl;

    // Free memory

    free(mem);
    free(in1buff);
    free(in2buff);
    free(outbuff);
    free(outbuff_gold);

    free(m1_data);
    free(m2_data);
    free(m3_data);

    return 0;
}
