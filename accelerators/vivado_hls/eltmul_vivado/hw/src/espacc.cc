// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

word_t status_load;
word_t status_compute;
word_t status_store;

void load(word_t _inbuff_1[SIZE_IN_CHUNK_DATA], word_t _inbuff_2[SIZE_IN_CHUNK_DATA], dma_word_t *in1, dma_word_t *in2,
          /* <<--compute-params-->> */
          const unsigned matrix_dim_x, const unsigned matrix_dim_y, const unsigned state_control, dma_info_t &load_ctrl,
          int chunk, int batch)
{
load_data:

    const unsigned length = round_up(matrix_dim_x * matrix_dim_y, VALUES_PER_WORD) / 1;
    const unsigned index  = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

    if (state_control == 1) {

        load_ctrl.index  = dma_index;
        load_ctrl.length = dma_length;
        load_ctrl.size   = SIZE_WORD_T;

#ifndef __SYNTHESIS__
        printf("[load]: dma_index = %d\n", dma_index);
        printf("[load]: dma_length = %d\n", dma_length);
        printf("[load]: SIZE_WORD_T = %d\n", SIZE_WORD_T);
        printf("[load]: VALUES_PER_WORD = %d\n", VALUES_PER_WORD);
#endif

        for (unsigned i = 0; i < dma_length; i++) {
load_label0:
            for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
                _inbuff_1[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
                // #ifndef __SYNTHESIS__
                //                 printf("[load]: _inbuff_1[%d] = %d\n", i * VALUES_PER_WORD + j,
                //                        (int)_inbuff_1[i * VALUES_PER_WORD + j]);
                // #endif
            }
            //         }

            //         for (unsigned i = 0; i < dma_length; i++) {
            // load_label1:
            for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
                _inbuff_2[i * VALUES_PER_WORD + j] = in1[dma_length + dma_index + i].word[j];
#ifndef __SYNTHESIS__
                printf("[load]: _inbuff_2[%d] = %d\n", i * VALUES_PER_WORD + j,
                       (int)_inbuff_2[i * VALUES_PER_WORD + j]);
#endif
            }
        }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned matrix_dim_x, const unsigned matrix_dim_y, const unsigned state_control,
           dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length       = round_up(matrix_dim_x * matrix_dim_y, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(matrix_dim_x * matrix_dim_y * 2, VALUES_PER_WORD) * 1;
    const unsigned out_offset   = store_offset;
    const unsigned index        = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

#ifndef __SYNTHESIS__
    printf("[store]: dma_index = %d\n", dma_index);
    printf("[store]: dma_length = %d\n", dma_length);
    printf("[store]: SIZE_WORD_T = %d\n", SIZE_WORD_T);
    printf("[store]: VALUES_PER_WORD = %d\n", VALUES_PER_WORD);
#endif

    if (state_control == 2) {

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
}

void compute(word_t _inbuff_1[SIZE_IN_CHUNK_DATA], word_t _inbuff_2[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned matrix_dim_x, const unsigned matrix_dim_y, const unsigned state_control,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    // prepare input streams for PEs

    if (state_control == 2) {

        for (unsigned i = 0; i < SIZE_IN_CHUNK_DATA; i++) {
compute_label0:
            _outbuff[i] = _inbuff_1[i] * _inbuff_2[i];
#ifndef __SYNTHESIS__
            printf("[compute]: index = %d, in1 = %d, in2 = %d, out = %d\n", i, (int)_inbuff_1[i], (int)_inbuff_2[i],
                   (int)_outbuff[i]);
#endif
        }
    }
}

void top(dma_word_t *out, dma_word_t *in1, dma_word_t *in2,
         /* <<--params-->> */
         const unsigned conf_info_matrix_dim_x, const unsigned conf_info_matrix_dim_y,
         const unsigned conf_info_state_control, dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++) {
        // Chunking
go:
        for (int c = 0; c < 1; c++) {

            const unsigned matrix_dim_x  = conf_info_matrix_dim_x;
            const unsigned matrix_dim_y  = conf_info_matrix_dim_y;
            const unsigned state_control = conf_info_state_control;

            word_t _inbuff_1[SIZE_IN_CHUNK_DATA];
            word_t _inbuff_2[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];
#ifndef __SYNTHESIS__
            printf("[top]: SIZE_IN_CHUNK_DATA = %d\n", SIZE_IN_CHUNK_DATA);
            printf("[top]: SIZE_OUT_CHUNK_DATA = %d\n", SIZE_OUT_CHUNK_DATA);
#endif

            load(_inbuff_1, _inbuff_2, in1, in2,
                 /* <<--args-->> */
                 matrix_dim_x, matrix_dim_y, state_control, load_ctrl, c, b);

#ifndef __SYNTHESIS__
            printf("Finished load \n");
#endif

            compute(_inbuff_1, _inbuff_2,
                    /* <<--args-->> */
                    matrix_dim_x, matrix_dim_y, state_control, _outbuff);

#ifndef __SYNTHESIS__
            printf("Finished compute \n");
#endif

            store(_outbuff, out,
                  /* <<--args-->> */
                  matrix_dim_x, matrix_dim_y, state_control, store_ctrl, c, b);

#ifndef __SYNTHESIS__
            printf("Finished store \n");
#endif
        }
    }
}
