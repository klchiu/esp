// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _in1buff[SIZE_IN1_CHUNK_DATA], dma_word_t *in1, word_t _in2buff[SIZE_IN2_CHUNK_DATA], dma_word_t *in2,
          /* <<--compute-params-->> */
          const unsigned m3_offset, const unsigned d3, const unsigned d2, const unsigned d1, const unsigned m2_offset,
          const unsigned m1_offset, dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    // unsigned length;
    // unsigned index;

    unsigned dma_length;
    unsigned dma_index;

    // length = round_up(m1_offset, VALUES_PER_WORD) / 1;
    // index  = length * (batch * 1 + chunk);

    // dma_length = d1 * d2 / VALUES_PER_WORD;
    // dma_index  = m1_offset;
    // printf("[load]: length = %d\n", length);
    // printf("[load]: index = %d\n", index);
    // printf("[load]: dma_length = %d\n", dma_length);
    // printf("[load]: dma_index = %d\n", dma_index);

    // for (int i = 0; i < 64 * 3; i++) {
    //     word_t temp = in1[i].word[0];
    //     printf("[load]: mem[%d] = %d\n", i, (int)temp);
    // }

    // load matrix 1
    dma_index  = m1_offset;
    dma_length = d1 * d2;

    load_ctrl.index  = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size   = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
load_label1:
        for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
            _in1buff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
            // printf("[load]: _in1buff[%d] = %d\n", i * VALUES_PER_WORD + j, (int)_in1buff[i * VALUES_PER_WORD + j]);
        }
    }

    // load matrix 2
    dma_index  = m2_offset;
    dma_length = d2 * d3;

    load_ctrl.index  = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size   = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
load_label2:
        for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
            word_t x                          = in1[dma_index + i].word[j];
            _in2buff[i * VALUES_PER_WORD + j] = x;
            // printf("[load]: _in2buff[%d] = %d\n", i * VALUES_PER_WORD + j, (int)_in2buff[i * VALUES_PER_WORD + j]);
        }
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned m3_offset, const unsigned d3, const unsigned d2, const unsigned d1, const unsigned m2_offset,
           const unsigned m1_offset, dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    // const unsigned length       = round_up(m1_offset, VALUES_PER_WORD) / 1;
    // const unsigned store_offset = round_up(m2_offset, VALUES_PER_WORD) * 1;
    // const unsigned out_offset   = store_offset;
    // const unsigned index        = out_offset + length * (batch * 1 + chunk);

    // unsigned dma_length = length / VALUES_PER_WORD;
    // unsigned dma_index  = index / VALUES_PER_WORD;

	unsigned dma_length = d1 * d3;
    unsigned dma_index = m3_offset;

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

void compute(word_t _in1buff[SIZE_IN1_CHUNK_DATA], word_t _in2buff[SIZE_IN2_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned m3_offset, const unsigned d3, const unsigned d2, const unsigned d1,
             const unsigned m2_offset, const unsigned m1_offset, word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // // TODO implement compute functionality
    // const unsigned length = round_up(m2_offset, VALUES_PER_WORD) / 1;

    // for (int i = 0; i < length; i++)
    //     _outbuff[i] = _in1buff[i];


  for (int i = 0; i < d1; i++) {
        for (int k = 0; k < d3; k++) {
            for (int j = 0; j < d2; j++) {
                _outbuff[i * d3 + k] += _in1buff[i * d2 + j] * _in2buff[j * d3 + k];
                // printf("m1_data[%d] = %d\t", i*d2+j, m1_data[i*d2+j]);
                // printf("m2_data[%d] = %d\t", j*d3+k, m2_data[j*d3+k]);
                // printf("m3_data[%d] = %d\n", i*d3+k, m3_data[i*d3+k]);
            }
        }
    }


}

void top(dma_word_t *out, dma_word_t *in1, dma_word_t *in2,
         /* <<--params-->> */
         const unsigned conf_info_m3_offset, const unsigned conf_info_d3, const unsigned conf_info_d2,
         const unsigned conf_info_d1, const unsigned conf_info_m2_offset, const unsigned conf_info_m1_offset,
         dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
    const unsigned m3_offset = conf_info_m3_offset;
    const unsigned d3        = conf_info_d3;
    const unsigned d2        = conf_info_d2;
    const unsigned d1        = conf_info_d1;
    const unsigned m2_offset = conf_info_m2_offset;
    const unsigned m1_offset = conf_info_m1_offset;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++) {
        // Chunking
go:
        for (int c = 0; c < 1; c++) {
            word_t _in1buff[SIZE_IN1_CHUNK_DATA];
            word_t _in2buff[SIZE_IN2_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_in1buff, in1, _in2buff, in2,
                 /* <<--args-->> */
                 m3_offset, d3, d2, d1, m2_offset, m1_offset, load_ctrl, c, b);
            compute(_in1buff, _in2buff,
                    /* <<--args-->> */
                    m3_offset, d3, d2, d1, m2_offset, m1_offset, _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
                  m3_offset, d3, d2, d1, m2_offset, m1_offset, store_ctrl, c, b);
        }
    }
}
