// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>




// (DMA_WIDTH == 64)
unsigned DMA_BEAT_PER_WORD = 1;
unsigned DMA_WORD_PER_BEAT = 2;
unsigned DMA_BEAT_PER_WORD_LOG2 = 0;
unsigned DMA_WORD_PER_BEAT_LOG2 = 1;


unsigned DATA_WIDTH = 32;
unsigned INPUT_PLM_SIZE = 2048;
unsigned WEIGHTS_PLM_SIZE = 2048;
unsigned BIAS_PLM_SIZE = 16;
unsigned OUTPUT_PLM_SIZE = 2048;
unsigned PATCH_PLM_SIZE = 512;
unsigned MAC_PLM_SIZE = 512;

inline int min(int a, int b){
    if (a < b)
		return a;
    else
		return b;
}

inline int max(int a, int b){
    if (a > b)
		return a;
    else
		return b;
}


inline void compute_dimensions(
    const unsigned height, const unsigned width, const unsigned n_channels,
    const bool is_padded, const unsigned stride, const unsigned filter_dim,
    const unsigned n_filters, const unsigned pool_type, const unsigned batch_size,
    unsigned *output_w, unsigned *pad,
    unsigned *feature_size, unsigned *filter_size, uint32_t *filters_size, 
    unsigned *max_cacheable_rows, unsigned *max_cacheable_rows_init,
    unsigned *max_cacheable_size,  unsigned *max_cacheable_size_init,
    unsigned *max_cacheable_filters, unsigned *max_cacheable_filters_size,
    unsigned *max_cacheable_bias_chunks, unsigned *max_cacheable_bias_size,
    unsigned *total_input_chunks, unsigned *total_filters_chunks,
    unsigned *feature_offset_incr, unsigned *feature_offset_incr_init,
    unsigned *channel_offset_incr, unsigned *out_channel_offset_incr,
    unsigned *out_channel_pool_offset_incr,
    uint32_t *filters_offset_start_base, uint32_t *bias_offset_start_base,
    uint32_t *feature_offset_start_base,
    unsigned *loadable_chan, unsigned *chan_iters, unsigned *chan_rem,
    unsigned *loadable_chan_sz, unsigned *chan_rem_sz)
{
#ifndef __SYNTHESIS__
 	printf("[compute_dimensions]: height = %u\n", height);
    printf("[compute_dimensions]: width = %u\n", width);
    printf("[compute_dimensions]: n_channels = %u\n", n_channels);
    printf("[compute_dimensions]: is_padded = %u\n", is_padded);
    printf("[compute_dimensions]: stride = %u\n", stride);
    printf("[compute_dimensions]: filter_dim = %u\n", filter_dim);
    printf("[compute_dimensions]: n_filters = %u\n", n_filters);
    printf("[compute_dimensions]: pool_type = %u\n", pool_type);
    printf("[compute_dimensions]: batch_size = %u\n", batch_size);
    printf("[compute_dimensions]: *output_w = %u\n", *output_w);
    printf("[compute_dimensions]: *pad = %u\n", *pad);
    printf("[compute_dimensions]: *feature_size = %u\n", *feature_size);
    printf("[compute_dimensions]: *filter_size = %u\n", *filter_size);
    printf("[compute_dimensions]: *filters_size = %u\n", *filters_size);
    printf("[compute_dimensions]: *max_cacheable_rows = %u\n", *max_cacheable_rows);
    printf("[compute_dimensions]: *max_cacheable_rows_init = %u\n", *max_cacheable_rows_init);
    printf("[compute_dimensions]: *max_cacheable_size = %u\n", *max_cacheable_size);
    printf("[compute_dimensions]: *max_cacheable_size_init = %u\n", *max_cacheable_size_init);
    printf("[compute_dimensions]: *max_cacheable_filters = %u\n", *max_cacheable_filters);
    printf("[compute_dimensions]: *max_cacheable_filters_size = %u\n", *max_cacheable_filters_size);
    printf("[compute_dimensions]: *max_cacheable_bias_chunks  = %u\n", *max_cacheable_bias_chunks );
    printf("[compute_dimensions]: *max_cacheable_bias_size = %u\n", *max_cacheable_bias_size);
    printf("[compute_dimensions]: *total_input_chunks = %u\n", *total_input_chunks);
    printf("[compute_dimensions]: *total_filters_chunks = %u\n", *total_filters_chunks);
    printf("[compute_dimensions]: *feature_offset_incr = %u\n", *feature_offset_incr);
    printf("[compute_dimensions]: *feature_offset_incr_init = %u\n", *feature_offset_incr_init);
    printf("[compute_dimensions]: *channel_offset_incr = %u\n", *channel_offset_incr);
    printf("[compute_dimensions]: *out_channel_offset_incr = %u\n", *out_channel_offset_incr);
    printf("[compute_dimensions]: *out_channel_pool_offset_incr = %u\n", *out_channel_pool_offset_incr);
    printf("[compute_dimensions]: *filters_offset_start_base = %u\n", *filters_offset_start_base);
    printf("[compute_dimensions]: *bias_offset_start_base = %u\n", *bias_offset_start_base);
    printf("[compute_dimensions]: *feature_offset_start_base = %u\n", *feature_offset_start_base);
    printf("[compute_dimensions]: *loadable_chan = %u\n", *loadable_chan);
    printf("[compute_dimensions]: *chan_iters = %u\n", *chan_iters);
    printf("[compute_dimensions]: *chan_rem = %u\n", *chan_rem);
    printf("[compute_dimensions]: *loadable_chan_sz = %u\n", *loadable_chan_sz);
    printf("[compute_dimensions]: *chan_rem_sz = %u\n", *chan_rem_sz);
#endif

    uint8_t filter_dim2 = (uint8_t) filter_dim * filter_dim;
    /* Spatial dimensions of the output activation map */
    *pad = is_padded ? (filter_dim >> 1) : 0;
    *output_w = ((unsigned) (width + 2 * *pad - filter_dim)) / stride + 1;

    /* Size (in number of words) of an input */
    *feature_size = height * width;

    /* Size (in number of weights) of each filter */
    *filter_size = n_channels * filter_dim2;
    *filters_size = *filter_size * n_filters;

    /* Max number of input rows cacheable in the input PLM */
    unsigned max_io_channels = max(n_channels, n_filters);
    unsigned io_channel_size = max_io_channels * width;
    *max_cacheable_rows = min(((unsigned) INPUT_PLM_SIZE) / ((unsigned) io_channel_size), height);

    *chan_iters = 1;
    *chan_rem = n_channels;
    *loadable_chan = n_channels;
    if (*max_cacheable_rows < filter_dim + 1) {
	*loadable_chan = ((unsigned) INPUT_PLM_SIZE) / ((unsigned) (width * (filter_dim + 1)));
	*chan_iters = ((unsigned) (n_channels - 1)) / ((unsigned) *loadable_chan) + 1;
	*chan_rem = n_channels - (*loadable_chan * (*chan_iters - 1));
	*max_cacheable_rows = filter_dim + 1;
    }
    *loadable_chan_sz = (unsigned) *loadable_chan * filter_dim2;
    *chan_rem_sz = (unsigned) *chan_rem * filter_dim2;

    *max_cacheable_rows_init = *max_cacheable_rows;

    /* Max number of input rows cacheable in the input PLM */
    // TODO optimize
    unsigned cacheable_outputs = OUTPUT_PLM_SIZE / ((unsigned) *output_w *
						    *max_cacheable_rows);
    unsigned cacheable_filters = WEIGHTS_PLM_SIZE / (*filter_size);
    *max_cacheable_filters = (min(min(cacheable_filters, n_filters),
    				  min(cacheable_outputs, BIAS_PLM_SIZE)) >> 1) << 1;
    *max_cacheable_filters_size = *filter_size * *max_cacheable_filters;

    if (*max_cacheable_rows < height) {
    	if ((*max_cacheable_rows & 1) == 1) {
    	    *max_cacheable_rows -= 1;
    	    if (!is_padded)
    		*max_cacheable_rows_init -= 1;
    	} else {
    	    if (is_padded && (filter_dim == 3))
    		*max_cacheable_rows_init -= 1;
    	}
    }

    *max_cacheable_size = *max_cacheable_rows * width;
    *max_cacheable_size_init = *max_cacheable_rows_init * width;

    /* Amount of input chunks to be loaded in the input PLM */
    unsigned max_cacheable_rows_norm = (*max_cacheable_rows) - filter_dim + 1;
    unsigned max_cacheable_rows_norm_init = (*max_cacheable_rows_init) - filter_dim + 1;

    if (*max_cacheable_rows == height) {
    	*total_input_chunks = 1;
    } else {
    	*total_input_chunks = ((unsigned) (height - *max_cacheable_rows_init - 1)) /
	    max_cacheable_rows_norm + 2;
    }

    /* Amount of filter chunks to be loaded in the filter PLM */
    *total_filters_chunks = ((unsigned) (n_filters - 1)) / (*max_cacheable_filters) + 1;

    *max_cacheable_bias_chunks = BIAS_PLM_SIZE / *max_cacheable_filters;
    if (*max_cacheable_bias_chunks >= *total_filters_chunks)
    	*max_cacheable_bias_size = n_filters;
    else
    	*max_cacheable_bias_size = *max_cacheable_bias_chunks * *max_cacheable_filters;

    /* Load offsets */
    *channel_offset_incr = round_up(*feature_size, DMA_WORD_PER_BEAT);
    *out_channel_offset_incr = round_up(*output_w * *output_w, DMA_WORD_PER_BEAT);
    unsigned output_pool_w = pool_type ? *output_w >> 1 : *output_w;
    *out_channel_pool_offset_incr = round_up(output_pool_w * output_pool_w, DMA_WORD_PER_BEAT);

    *feature_offset_incr = max_cacheable_rows_norm * width;
    *feature_offset_incr_init = max_cacheable_rows_norm_init * width;
    *filters_offset_start_base =  *channel_offset_incr * n_channels * batch_size;
    *bias_offset_start_base = *filters_offset_start_base + *filters_size;
    *feature_offset_start_base = *filters_offset_start_base + *filters_size + n_filters;
}




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
	// config parameters (to match the name with the stratus version)
 	unsigned height = feature_map_height;
    unsigned width = feature_map_width;

	// Precompute sizes
    bool ping_input = true;
    bool ping_weights = true;
    bool ping_bias = true;
    unsigned pad;
    unsigned output_w;
    unsigned feature_size;
    unsigned filter_size;
    unsigned filters_size;
    unsigned max_cacheable_rows;
    unsigned max_cacheable_rows_init;
    unsigned max_cacheable_size;
    unsigned max_cacheable_size_init;
    unsigned max_cacheable_filters;
    unsigned max_cacheable_filters_size;
    unsigned max_cacheable_bias_chunks;
    unsigned max_cacheable_bias_size;
    unsigned total_input_chunks;
    unsigned total_filters_chunks;
    unsigned feature_offset_incr;
    unsigned feature_offset_incr_init;
    unsigned channel_offset_incr;
    unsigned out_channel_offset_incr;
    unsigned out_channel_pool_offset_incr;
    unsigned filters_offset_start_base;
    unsigned feature_offset_start_base;
    unsigned bias_offset_start_base;
    unsigned loadable_chan, chan_iters, chan_rem;
    unsigned loadable_chan_sz, chan_rem_sz;


	compute_dimensions(height, width, n_channels, (bool) is_padded,
		       stride, filter_dim, n_filters, pool_type, batch_size,
		       &output_w, &pad, &feature_size, &filter_size, &filters_size,
		       &max_cacheable_rows, &max_cacheable_rows_init,
		       &max_cacheable_size, &max_cacheable_size_init,
		       &max_cacheable_filters,
		       &max_cacheable_filters_size, &max_cacheable_bias_chunks,
		       &max_cacheable_bias_size, &total_input_chunks,
		       &total_filters_chunks,
		       &feature_offset_incr, &feature_offset_incr_init,
		       &channel_offset_incr, &out_channel_offset_incr,
		       &out_channel_pool_offset_incr, &filters_offset_start_base,
		       &bias_offset_start_base, &feature_offset_start_base,
		       &loadable_chan, &chan_iters, &chan_rem,
		       &loadable_chan_sz, &chan_rem_sz);

#ifndef __SYNTHESIS__
	printf("[load] -- output_w %u\n", output_w);
	printf("[load] -- pad %u\n", (uint32_t) pad);
	printf("[load] -- feature_size %u\n", feature_size);
	printf("[load] -- filter_size %u\n", filter_size);
	printf("[load] -- filters_size %u\n", filters_size);
	printf("[load] -- max_cacheable_rows %u\n", max_cacheable_rows);
	printf("[load] -- max_cacheable_rows_init %u\n", max_cacheable_rows_init);
	printf("[load] -- max_cacheable_size %u\n", max_cacheable_size);
	printf("[load] -- max_cacheable_size_init %u\n", max_cacheable_size_init);
	printf("[load] -- max_cacheable_filters %u\n", max_cacheable_filters);
	printf("[load] -- max_cacheable_filters_size %u\n", max_cacheable_filters_size);
	printf("[load] -- max_cacheable_bias_chunks %u\n", max_cacheable_bias_chunks);
	printf("[load] -- max_cacheable_bias_size %u\n", max_cacheable_bias_size);
	printf("[load] -- total_input_chunks %u\n", total_input_chunks);
	printf("[load] -- total_filters_chunks %u\n", total_filters_chunks);
	printf("[load] -- feature_offset_incr %u\n", feature_offset_incr);
	printf("[load] -- feature_offset_incr_init %u\n", feature_offset_incr_init);
	printf("[load] -- channel_offset_incr %u\n", channel_offset_incr);
	printf("[load] -- filters_offset_start_base %u\n", filters_offset_start_base);
	printf("[load] -- bias_offset_start_base %u\n", bias_offset_start_base);
	printf("[load] -- feature_offset_start_base %u\n", feature_offset_start_base);
	printf("[load] -- loadable_chan %u\n", (unsigned) loadable_chan);
	printf("[load] -- chan_iters %u\n", (unsigned) chan_iters);
	printf("[load] -- chan_rem %u\n", (unsigned) chan_rem);
	printf("[load] -- loadable_chan_sz %u\n", (unsigned) loadable_chan_sz);
	printf("[load] -- chan_rem_sz %u\n", (unsigned) chan_rem_sz);
#endif






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



	// Chunking
	uint32_t infeature_offset_incr = channel_offset_incr * n_channels;
	bool single_chunk_done = false;
	uint32_t filters_offset_start_phys = filters_offset_start_base;
	uint32_t filters_offset_start_virt = 0;
	uint32_t bias_offset_start_phys = bias_offset_start_base;
	uint32_t bias_offset_start_virt = 0;
	uint16_t bias_chunk = 0;
	uint16_t plm_bias_i = 0;


	for (uint16_t filter_chunk = 0; filter_chunk < total_filters_chunks; filter_chunk++)
	{
	    uint16_t plm_weights_index = 0;
	    uint16_t n_words_to_load = min(filters_size - filters_offset_start_virt, max_cacheable_filters_size);
	    bool misaligned = filters_offset_start_phys & 1 & (DMA_WORD_PER_BEAT - 1);
	    uint16_t adj_words_to_load = n_words_to_load + misaligned;

	    dma_info_t dma_info(filters_offset_start_phys >> DMA_WORD_PER_BEAT_LOG2,
				(n_words_to_load + misaligned + DMA_WORD_PER_BEAT_LOG2) >>
				DMA_WORD_PER_BEAT_LOG2, DMA_SIZE);

#ifndef STRATUS_HLS
	    ESP_REPORT_INFO("load_input load filters dma_info. offset: %u len %u",
	    		filters_offset_start_phys, n_words_to_load);
#endif
	    this->dma_read_ctrl.put(dma_info);

	    for (uint16_t i = 0; i < adj_words_to_load; i += DMA_WORD_PER_BEAT)
	    {
		HLS_PROTO("load-dma-filters");
		HLS_BREAK_DEP(plm_weights_ping);
		HLS_BREAK_DEP(plm_weights_pong);

		sc_dt::sc_bv<DMA_WIDTH> dataBv;

		dataBv = this->dma_read_chnl.get();
		wait();

#if (DMA_WORD_PER_BEAT == 2)
		if (!(!i && misaligned)) {
		    if (ping_weights) {
			plm_weights_ping[plm_weights_index++] =
			    dataBv.range(31,0).to_int();
			if (i + 1 < adj_words_to_load)
			    plm_weights_ping[plm_weights_index++] =
				dataBv.range(63,32).to_int();
		    } else {
			plm_weights_pong[plm_weights_index++] =
			    dataBv.range(31,0).to_int();
			if (i + 1 < adj_words_to_load)
			    plm_weights_pong[plm_weights_index++] =
				dataBv.range(63,32).to_int();
		    }
		}
#else
                if (ping_weights) {
                    plm_weights_ping[plm_weights_index++] = dataBv.to_int();
                } else {
                    plm_weights_pong[plm_weights_index++] = dataBv.to_int();
                }
#endif
	    }

	    for (uint8_t i = 0; i < PARALLELISM; i += DMA_WORD_PER_BEAT) {
		if (ping_weights) {
		    plm_weights_ping[plm_weights_index++] = 0;
#if (DMA_WORD_PER_BEAT == 2)
		    plm_weights_ping[plm_weights_index++] = 0;
#endif
		} else {
		    plm_weights_pong[plm_weights_index++] = 0;
#if (DMA_WORD_PER_BEAT == 2)
		    plm_weights_pong[plm_weights_index++] = 0;
#endif
		}
	    }

	    filters_offset_start_phys += n_words_to_load;
	    filters_offset_start_virt += n_words_to_load;

	    ping_weights = !ping_weights;

	    if (!bias_chunk) {
		uint16_t n_words_to_load = min(n_filters - bias_offset_start_virt,
					       max_cacheable_bias_size);
		bool misaligned = bias_offset_start_phys & 1 & (DMA_WORD_PER_BEAT - 1);
		uint16_t adj_words_to_load = n_words_to_load + misaligned;

		dma_info_t dma_info(bias_offset_start_phys >> DMA_WORD_PER_BEAT_LOG2,
				    (n_words_to_load + misaligned + DMA_WORD_PER_BEAT_LOG2) >> DMA_WORD_PER_BEAT_LOG2,
				    DMA_SIZE);

#ifndef STRATUS_HLS
		ESP_REPORT_INFO("load_input load bias dma_info. offset: %u len %u",
				bias_offset_start_phys, n_words_to_load);
#endif
		this->dma_read_ctrl.put(dma_info);

		for (uint16_t i = 0; i < adj_words_to_load; i += DMA_WORD_PER_BEAT)
		{
		    HLS_PROTO("load-dma-biases");
		    HLS_BREAK_DEP(plm_bias_ping);
		    HLS_BREAK_DEP(plm_bias_pong);

		    sc_dt::sc_bv<DMA_WIDTH> dataBv;

		    dataBv = this->dma_read_chnl.get();
		    wait();

		    // Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
#if (DMA_WORD_PER_BEAT == 2)
		    if (!(!i && misaligned)) {
			if (ping_bias) {
			    plm_bias_ping[plm_bias_i++] =
				dataBv.range(31, 0).to_int();
			    if (i + 1 < adj_words_to_load)
				plm_bias_ping[plm_bias_i++] =
				    dataBv.range(63, 32).to_int();
			} else {
			    plm_bias_pong[plm_bias_i++] =
				dataBv.range(31, 0).to_int();
			    if (i + 1 < adj_words_to_load)
				plm_bias_pong[plm_bias_i++] =
				    dataBv.range(63, 32).to_int();
			}
		    }
#else
                    if (ping_bias) {
                        plm_bias_ping[plm_bias_i++] = dataBv.to_int();
                    } else {
                        plm_bias_pong[plm_bias_i++] = dataBv.to_int();
                    }
#endif
		}

		bias_offset_start_phys += n_words_to_load;
		bias_offset_start_virt += n_words_to_load;
	    }

	    bias_chunk++;
	    if (bias_chunk == max_cacheable_bias_chunks) {
		bias_chunk = 0;
		ping_bias = !ping_bias;
		plm_bias_i = 0;
	    }

	    // Batching
	    uint32_t infeature_offset_start_base = 0;
	    for (uint16_t b = 0; b < batch_size; b++)
	    {
		uint32_t infeature_offset_start_virt = 0;
		uint32_t infeature_offset_start = infeature_offset_start_base;
		for (uint16_t input_chunk = 0; input_chunk < total_input_chunks;
		     input_chunk++)
		{
		    uint32_t channel_offset = 0;
		    uint16_t max_cacheable_size_i;
		    uint16_t feature_offset_incr_i;

		    if (single_chunk_done) {
			this->load_compute_handshake();
			break;
		    }

		    if (total_input_chunks == 1 && batch_size == 1) {
			// optimize if multiple batches all fit in PLM
			single_chunk_done = true;
		    }

		    if (!input_chunk) {
			max_cacheable_size_i = max_cacheable_size_init;
			feature_offset_incr_i = feature_offset_incr_init;
		    } else {
			max_cacheable_size_i = max_cacheable_size;
			feature_offset_incr_i = feature_offset_incr;
		    }

		    for (uint12_t in_i = 0; in_i < chan_iters; in_i++)
		    {
			uint12_t channels;
			uint16_t plm_in_index = 0;

			if (in_i < chan_iters - 1)
			    channels = loadable_chan;
			else
			    channels = chan_rem;

			for (uint16_t ch = 0; ch < channels; ch++)
			{
			    wait();

			    // Configure DMA transaction
			    uint32_t offset_start = channel_offset + infeature_offset_start;
			    uint16_t n_words_to_load =
				min(feature_size - infeature_offset_start_virt,
				    max_cacheable_size_i);
			    bool misaligned = offset_start & 1 & (DMA_WORD_PER_BEAT - 1);
			    uint16_t adj_words_to_load = n_words_to_load + misaligned;

			    dma_info_t dma_info(offset_start >> DMA_WORD_PER_BEAT_LOG2,
						(n_words_to_load + DMA_WORD_PER_BEAT_LOG2) >>
						DMA_WORD_PER_BEAT_LOG2, DMA_SIZE);

#ifndef STRATUS_HLS
			    ESP_REPORT_INFO("load_input load features dma_info. offset: %u len %u", offset_start, n_words_to_load);
#endif
			    this->dma_read_ctrl.put(dma_info);

			    for (uint16_t i = 0; i < adj_words_to_load;
				 i += DMA_WORD_PER_BEAT)
			    {
				HLS_PROTO("load-dma-in-features");
				HLS_BREAK_DEP(plm_in_ping);
				HLS_BREAK_DEP(plm_in_pong);

				sc_dt::sc_bv<DMA_WIDTH> dataBv;

				dataBv = this->dma_read_chnl.get();
				wait();

				// Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
#if (DMA_WORD_PER_BEAT == 2)
				if (!(!i && misaligned)) {
				    if (ping_input) {
					plm_in_ping[plm_in_index++] =
					    dataBv.range(31, 0).to_int();
					if (i + 1 < adj_words_to_load)
					    plm_in_ping[plm_in_index++] =
						dataBv.range(63, 32).to_int();
				    } else {
					plm_in_pong[plm_in_index++] =
					    dataBv.range(31, 0).to_int();
					if (i + 1 < adj_words_to_load)
					    plm_in_pong[plm_in_index++] =
						dataBv.range(63, 32).to_int();
				    }
				}
#else
                                if (ping_input) {
                                    plm_in_ping[plm_in_index++] = dataBv.to_int();
                                } else {
                                    plm_in_pong[plm_in_index++] = dataBv.to_int();
                                }
#endif
			    }
			    channel_offset += channel_offset_incr;
			}

			ping_input = !ping_input;

			this->load_compute_handshake();
		    }
		    infeature_offset_start += feature_offset_incr_i;
		    infeature_offset_start_virt += feature_offset_incr_i;
		}
		infeature_offset_start_base += infeature_offset_incr;
	    }
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
