// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned mac_vec,
	 const unsigned mac_len,
	 const unsigned mac_n,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(mac_len * mac_vec, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
    	}
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned mac_vec,
	 const unsigned mac_len,
	 const unsigned mac_n,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(mac_vec, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(mac_len * mac_vec, VALUES_PER_WORD) * mac_n;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}


void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
	 const unsigned mac_vec,
	 const unsigned mac_len,
	 const unsigned mac_n,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    // const unsigned length = round_up(mac_len * mac_vec, VALUES_PER_WORD) / 1;

    // for (int i = 0; i < length; i++)
    //     _outbuff[i] = _inbuff[i];
    

    // Compute
unsigned in_length = mac_len * mac_vec;
unsigned out_length = mac_vec;

unsigned vector_index = 0;
unsigned vector_number = 0;
int acc = 0;

for (int in_rem = in_length; in_rem > 0; in_rem -= SIZE_IN_CHUNK_DATA)
{

    unsigned in_len  = in_rem  > SIZE_IN_CHUNK_DATA  ? SIZE_IN_CHUNK_DATA  : in_rem;

    // Computing phase implementation
    for (int i = 0; i < in_len; i += 2) {

        // Multiply and accumulate
        acc += _inbuff[i] * _inbuff[i+1];

        vector_index += 2;

        // Write accumulated result
        if (vector_index == mac_len) {
            _outbuff[vector_number] = acc;

            acc = 0;
            vector_index = 0;
            vector_number++;
        }
    }
}
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_mac_vec,
	 const unsigned conf_info_mac_len,
	 const unsigned conf_info_mac_n,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned mac_vec = conf_info_mac_vec;
	 const unsigned mac_len = conf_info_mac_len;
	 const unsigned mac_n = conf_info_mac_n;

    // Batching
batching:
    for (unsigned b = 0; b < mac_n; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 mac_vec,
	 	 mac_len,
	 	 mac_n,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 mac_vec,
	 	 mac_len,
	 	 mac_n,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 mac_vec,
	 	 mac_len,
	 	 mac_n,
                  store_ctrl, c, b);
        }
    }
}
