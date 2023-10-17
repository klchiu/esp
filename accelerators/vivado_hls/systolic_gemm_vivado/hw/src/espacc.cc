// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

// temp buffer for PEs
word_t tempC[4];

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
          const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim, dma_info_t &load_ctrl,
          int chunk, int batch)
{
load_data:

    const unsigned length = round_up(matrix_A_dim * matrix_A_dim * 2, VALUES_PER_WORD) / 1;
    const unsigned index  = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

    load_ctrl.index  = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size   = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
load_label0:
        for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
            _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
        }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim,
           dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length       = round_up(matrix_C_dim * matrix_C_dim, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(matrix_A_dim * matrix_A_dim * 2, VALUES_PER_WORD) * 1;
    const unsigned out_offset   = store_offset;
    const unsigned index        = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

    store_ctrl.index  = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size   = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
store_label1:
        for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
            out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
        }
    }
}

// /*
void compute_PE(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t *out_buff,
                int PE_index, int stream_length)
{
    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;

        printf("[humu]: computePE: input_north = %d\n", (int)(input_north[i]));
        printf("[humu]: computePE: input_west = %d\n", (int)(input_west[i]));
        printf("[humu]: computePE: temp1 = %d\n", (int)(temp1));
        printf("[humu]: computePE: temp2 = %d\n", (int)(temp2));
        printf("[humu]: computePE: tempC[%d] = %d\n", PE_index, (int)(tempC[PE_index]));
        *out_buff = tempC[PE_index];
    }

    // west to east, north to south
    if (PE_index == 11) {
        for (int i = 0; i < stream_length; i++) {
            output_east[i]  = input_west[i];
            output_south[i] = input_north[i];
        }
    }
    if (PE_index == 21) {
        for (int i = 0; i < stream_length; i++) {
            output_east[i] = input_west[i];
            // output_south[i] = input_north[i];
        }
    }
    if (PE_index == 12) {
        for (int i = 0; i < stream_length; i++) {
            // output_east[i]  = input_west[i];
            output_south[i] = input_north[i];
        }
    }
    // if (PE_index == 22) {
    //     for (int i = 0; i < stream_length; i++) {
    //         output_east[i]  = input_west[i];
    //         output_south[i] = input_north[i];
    //     }
    // }
}
// */

void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    word_t *input_A_from_west  = _inbuff;
    word_t *input_B_from_north = &_inbuff[matrix_A_dim * matrix_A_dim];

    for (int i = 0; i < 4; i++) {
        printf("[humu]: compute: input_A_from_west = %d\n", (int)input_A_from_west[i]);
        printf("[humu]: compute: input_B_from_north = %d\n", (int)input_B_from_north[i]);
    }

    // prepare input streams for PEs
    // word_t *PE_11_west  = (word_t *)malloc((matrix_A_dim + 1) * sizeof(word_t));
    // word_t *PE_12_west  = (word_t *)malloc((matrix_A_dim + 1) * sizeof(word_t));
    // word_t *PE_21_west  = (word_t *)malloc((matrix_A_dim + 1) * sizeof(word_t));
    // word_t *PE_22_west  = (word_t *)malloc((matrix_A_dim + 1) * sizeof(word_t));
    // word_t *PE_11_north = (word_t *)malloc((matrix_B_dim + 1) * sizeof(word_t));
    // word_t *PE_12_north = (word_t *)malloc((matrix_B_dim + 1) * sizeof(word_t));
    // word_t *PE_21_north = (word_t *)malloc((matrix_B_dim + 1) * sizeof(word_t));
    // word_t *PE_22_north = (word_t *)malloc((matrix_B_dim + 1) * sizeof(word_t));

    word_t PE_11_west[3];
    word_t PE_12_west[3];
    word_t PE_21_west[3];
    word_t PE_22_west[3];
    word_t PE_11_north[3];
    word_t PE_12_north[3];
    word_t PE_21_north[3];
    word_t PE_22_north[3];

    // -- PE_11
    for (int i = 0; i < matrix_A_dim; i++) {
        PE_11_west[i] = input_A_from_west[i];
    }
    PE_11_west[matrix_A_dim] = 0; // padd a 0 at the end

    PE_11_north[0]            = input_B_from_north[0];
    PE_11_north[1]            = input_B_from_north[2];
    PE_11_north[matrix_B_dim] = 0; // padd a 0 at the end

    for (int i = 0; i < 3; i++) {
        printf("[humu]: compute: PE_11_west = %d\n", (int)PE_11_west[i]);
        printf("[humu]: compute: PE_11_north = %d\n", (int)PE_11_north[i]);
    }

    // -- PE_21
    PE_21_west[0] = 0; // padd a 0 at the front
    for (int i = 0; i < matrix_A_dim; i++) {
        PE_21_west[i + 1] = input_A_from_west[i + matrix_A_dim + 1];
    }

    PE_21_north[0] = 0; // padd a 0 at the front

    // -- PE_12
    PE_12_west[0] = 0; // padd a 0 at the front

    PE_12_north[0] = 0; // padd a 0 at the front
    for (int i = 0; i < matrix_B_dim; i += 2) {
        PE_12_north[i + 1] = input_B_from_north[i + matrix_B_dim + 1];
    }

    // -- PE_22
    PE_22_west[0]  = 0; // padd a 0 at the front
    PE_22_north[0] = 0; // padd a 0 at the front

    compute_PE(PE_11_west, PE_11_north, PE_12_west, PE_21_north, &_outbuff[0], 11, 3); // (PE_index = 11)
    // compute_PE(PE_21_west, PE_21_north, PE_22_west, NULL, 21, 3);         // (PE_index = 21)
    // compute_PE(&PE_12_west[0], &PE_12_north[0], NULL, &PE_22_north[0], 12, 3);           // (PE_index = 12)
    // compute_PE(&PE_22_west[0], &PE_22_north[0], NULL, NULL, 22, 3);                   // (PE_index = 22)

    for (int i = 0; i < 4; i++) {
        printf("[humu]: compute: _outbuff = %d\n", (int)_outbuff[i]);
    }

    // compute_PE()

    // free(PE_11_west );
    // free(PE_12_west );
    // free(PE_21_west );
    // free(PE_22_west );
    // free(PE_11_north);
    // free(PE_12_north);
    // free(PE_21_north);
    // free(PE_22_north);
}

void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
         const unsigned conf_info_matrix_C_dim, const unsigned conf_info_matrix_A_dim,
         const unsigned conf_info_matrix_B_dim, dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
    const unsigned matrix_C_dim = conf_info_matrix_C_dim;
    const unsigned matrix_A_dim = conf_info_matrix_A_dim;
    const unsigned matrix_B_dim = conf_info_matrix_B_dim;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++) {
        // Chunking
go:
        for (int c = 0; c < 1; c++) {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            // init tempC to zeros
            for (int x = 0; x < SIZE_OUT_CHUNK_DATA; x++) {
                tempC[x] = 0;
            }

            load(_inbuff, in1,
                 /* <<--args-->> */
                 matrix_C_dim, matrix_A_dim, matrix_B_dim, load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
                    matrix_C_dim, matrix_A_dim, matrix_B_dim, _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
                  matrix_C_dim, matrix_A_dim, matrix_B_dim, store_ctrl, c, b);
        }
    }
}
