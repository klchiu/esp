// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned do_relu,
	 const unsigned stride,
	 const unsigned feature_map_width,
	 const unsigned n_channels,
	 const unsigned n_filters,
	 const unsigned batch_size,
	 const unsigned filter_dim,
	 const unsigned is_padded,
	 const unsigned pool_type,
	 const unsigned feature_map_height,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(feature_map_height * feature_map_width, VALUES_PER_WORD) / 1;
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
	 const unsigned do_relu,
	 const unsigned stride,
	 const unsigned feature_map_width,
	 const unsigned n_channels,
	 const unsigned n_filters,
	 const unsigned batch_size,
	 const unsigned filter_dim,
	 const unsigned is_padded,
	 const unsigned pool_type,
	 const unsigned feature_map_height,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(feature_map_height * feature_map_width, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(feature_map_height * feature_map_width, VALUES_PER_WORD) * 1;
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
	 const unsigned do_relu,
	 const unsigned stride,
	 const unsigned feature_map_width,
	 const unsigned n_channels,
	 const unsigned n_filters,
	 const unsigned batch_size,
	 const unsigned filter_dim,
	 const unsigned is_padded,
	 const unsigned pool_type,
	 const unsigned feature_map_height,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
    const unsigned length = round_up(feature_map_height * feature_map_width, VALUES_PER_WORD) / 1;

    for (int i = 0; i < length; i++)
        _outbuff[i] = _inbuff[i];
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_do_relu,
	 const unsigned conf_info_stride,
	 const unsigned conf_info_feature_map_width,
	 const unsigned conf_info_n_channels,
	 const unsigned conf_info_n_filters,
	 const unsigned conf_info_batch_size,
	 const unsigned conf_info_filter_dim,
	 const unsigned conf_info_is_padded,
	 const unsigned conf_info_pool_type,
	 const unsigned conf_info_feature_map_height,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned do_relu = conf_info_do_relu;
	 const unsigned stride = conf_info_stride;
	 const unsigned feature_map_width = conf_info_feature_map_width;
	 const unsigned n_channels = conf_info_n_channels;
	 const unsigned n_filters = conf_info_n_filters;
	 const unsigned batch_size = conf_info_batch_size;
	 const unsigned filter_dim = conf_info_filter_dim;
	 const unsigned is_padded = conf_info_is_padded;
	 const unsigned pool_type = conf_info_pool_type;
	 const unsigned feature_map_height = conf_info_feature_map_height;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 do_relu,
	 	 stride,
	 	 feature_map_width,
	 	 n_channels,
	 	 n_filters,
	 	 batch_size,
	 	 filter_dim,
	 	 is_padded,
	 	 pool_type,
	 	 feature_map_height,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 do_relu,
	 	 stride,
	 	 feature_map_width,
	 	 n_channels,
	 	 n_filters,
	 	 batch_size,
	 	 filter_dim,
	 	 is_padded,
	 	 pool_type,
	 	 feature_map_height,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 do_relu,
	 	 stride,
	 	 feature_map_width,
	 	 n_channels,
	 	 n_filters,
	 	 batch_size,
	 	 filter_dim,
	 	 is_padded,
	 	 pool_type,
	 	 feature_map_height,
                  store_ctrl, c, b);
        }
    }
}
