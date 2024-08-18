// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "conv2d.hpp"

// Optional application-specific helper functions

inline void conv2d::compute_dimensions(
    const uint16_t height, const uint16_t width, const uint16_t n_channels,
    const bool is_padded, const uint16_t stride, const uint16_t filter_dim,
    const uint16_t n_filters, const uint16_t pool_type, const uint16_t batch_size,
    uint16_t *output_w, uint16_t *pad,
    uint16_t *feature_size, uint16_t *filter_size, uint32_t *filters_size, 
    uint16_t *max_cacheable_rows, uint16_t *max_cacheable_rows_init,
    uint16_t *max_cacheable_size,  uint16_t *max_cacheable_size_init,
    uint16_t *max_cacheable_filters, uint16_t *max_cacheable_filters_size,
    uint16_t *max_cacheable_bias_chunks, uint16_t *max_cacheable_bias_size,
    uint16_t *total_input_chunks, uint16_t *total_filters_chunks,
    uint16_t *feature_offset_incr, uint16_t *feature_offset_incr_init,
    uint16_t *channel_offset_incr, uint16_t *out_channel_offset_incr,
    uint16_t *out_channel_pool_offset_incr,
    uint32_t *filters_offset_start_base, uint32_t *bias_offset_start_base,
    uint32_t *feature_offset_start_base,
    uint16_t *loadable_chan, uint16_t *chan_iters, uint16_t *chan_rem,
    uint16_t *loadable_chan_sz, uint16_t *chan_rem_sz)
{
#ifndef STRATUS_HLS
    ESP_REPORT_INFO("[compute_dimensions]: height = %u", (unsigned)height);
    ESP_REPORT_INFO("[compute_dimensions]: width = %u", (unsigned)width);
    ESP_REPORT_INFO("[compute_dimensions]: n_channels = %u", (unsigned)n_channels);
    ESP_REPORT_INFO("[compute_dimensions]: is_padded = %u", (unsigned)is_padded);
    ESP_REPORT_INFO("[compute_dimensions]: stride = %u", stride);
    ESP_REPORT_INFO("[compute_dimensions]: filter_dim = %u", filter_dim);
    ESP_REPORT_INFO("[compute_dimensions]: n_filters = %u", (unsigned)n_filters);
    ESP_REPORT_INFO("[compute_dimensions]: pool_type = %u", pool_type);
    ESP_REPORT_INFO("[compute_dimensions]: batch_size = %u", (unsigned)batch_size);
    ESP_REPORT_INFO("[compute_dimensions]: *output_w = %u", (unsigned)*output_w);
    ESP_REPORT_INFO("[compute_dimensions]: *pad = %u", *pad);
    ESP_REPORT_INFO("[compute_dimensions]: *feature_size = %u", (unsigned)*feature_size);
    ESP_REPORT_INFO("[compute_dimensions]: *filter_size = %u", (unsigned)*filter_size);
    ESP_REPORT_INFO("[compute_dimensions]: *filters_size = %u", (unsigned)*filters_size);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_rows = %u", (unsigned)*max_cacheable_rows);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_rows_init = %u", (unsigned)*max_cacheable_rows_init);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_size = %u", (unsigned)*max_cacheable_size);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_size_init = %u", (unsigned)*max_cacheable_size_init);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_filters = %u", (unsigned)*max_cacheable_filters);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_filters_size = %u", (unsigned)*max_cacheable_filters_size);
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_bias_chunks  = %u", (unsigned)*max_cacheable_bias_chunks );
    ESP_REPORT_INFO("[compute_dimensions]: *max_cacheable_bias_size = %u", (unsigned)*max_cacheable_bias_size);
    ESP_REPORT_INFO("[compute_dimensions]: *total_input_chunks = %u", (unsigned)*total_input_chunks);
    ESP_REPORT_INFO("[compute_dimensions]: *total_filters_chunks = %u", (unsigned)*total_filters_chunks);
    ESP_REPORT_INFO("[compute_dimensions]: *feature_offset_incr = %u", (unsigned)*feature_offset_incr);
    ESP_REPORT_INFO("[compute_dimensions]: *feature_offset_incr_init = %u", (unsigned)*feature_offset_incr_init);
    ESP_REPORT_INFO("[compute_dimensions]: *channel_offset_incr = %u", (unsigned)*channel_offset_incr);
    ESP_REPORT_INFO("[compute_dimensions]: *out_channel_offset_incr = %u", (unsigned)*out_channel_offset_incr);
    ESP_REPORT_INFO("[compute_dimensions]: *out_channel_pool_offset_incr = %u", (unsigned)*out_channel_pool_offset_incr);
    ESP_REPORT_INFO("[compute_dimensions]: *filters_offset_start_base = %u", (unsigned)*filters_offset_start_base);
    ESP_REPORT_INFO("[compute_dimensions]: *bias_offset_start_base = %u", (unsigned)*bias_offset_start_base);
    ESP_REPORT_INFO("[compute_dimensions]: *feature_offset_start_base = %u", (unsigned)*feature_offset_start_base);
    ESP_REPORT_INFO("[compute_dimensions]: *loadable_chan = %u", *loadable_chan);
    ESP_REPORT_INFO("[compute_dimensions]: *chan_iters = %u", *chan_iters);
    ESP_REPORT_INFO("[compute_dimensions]: *chan_rem = %u", *chan_rem);
    ESP_REPORT_INFO("[compute_dimensions]: *loadable_chan_sz = %u", (unsigned)*loadable_chan_sz);
    ESP_REPORT_INFO("[compute_dimensions]: *chan_rem_sz = %u", (unsigned)*chan_rem_sz);
#endif

    uint8_t filter_dim2 = (uint8_t) filter_dim * filter_dim;
    /* Spatial dimensions of the output activation map */
    *pad = is_padded ? (filter_dim >> 1) : 0;
    *output_w = ((uint16_t) (width + 2 * *pad - filter_dim)) / stride + 1;

    /* Size (in number of words) of an input */
    *feature_size = height * width;

    /* Size (in number of weights) of each filter */
    *filter_size = n_channels * filter_dim2;
    *filters_size = *filter_size * n_filters;

    /* Max number of input rows cacheable in the input PLM */
    uint16_t max_io_channels = max(n_channels, n_filters);
    uint16_t io_channel_size = max_io_channels * width;
    *max_cacheable_rows = min(((uint16_t) INPUT_PLM_SIZE) / ((uint16_t) io_channel_size), height);

    *chan_iters = 1;
    *chan_rem = n_channels;
    *loadable_chan = n_channels;
    if (*max_cacheable_rows < filter_dim + 1) {
	*loadable_chan = ((uint16_t) INPUT_PLM_SIZE) / ((uint16_t) (width * (filter_dim + 1)));
	*chan_iters = ((uint16_t) (n_channels - 1)) / ((uint12_t) *loadable_chan) + 1;
	*chan_rem = n_channels - (*loadable_chan * (*chan_iters - 1));
	*max_cacheable_rows = filter_dim + 1;
    }
    *loadable_chan_sz = (uint16_t) *loadable_chan * filter_dim2;
    *chan_rem_sz = (uint16_t) *chan_rem * filter_dim2;

    *max_cacheable_rows_init = *max_cacheable_rows;

    /* Max number of input rows cacheable in the input PLM */
    // TODO optimize
    uint16_t cacheable_outputs = OUTPUT_PLM_SIZE / ((uint16_t) *output_w *
						    *max_cacheable_rows);
    uint16_t cacheable_filters = WEIGHTS_PLM_SIZE / (*filter_size);
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
    uint16_t max_cacheable_rows_norm = (*max_cacheable_rows) - filter_dim + 1;
    uint16_t max_cacheable_rows_norm_init = (*max_cacheable_rows_init) - filter_dim + 1;

    if (*max_cacheable_rows == height) {
    	*total_input_chunks = 1;
    } else {
    	*total_input_chunks = ((uint16_t) (height - *max_cacheable_rows_init - 1)) /
	    max_cacheable_rows_norm + 2;
    }

    /* Amount of filter chunks to be loaded in the filter PLM */
    *total_filters_chunks = ((uint16_t) (n_filters - 1)) / (*max_cacheable_filters) + 1;

    *max_cacheable_bias_chunks = BIAS_PLM_SIZE / *max_cacheable_filters;
    if (*max_cacheable_bias_chunks >= *total_filters_chunks)
    	*max_cacheable_bias_size = n_filters;
    else
    	*max_cacheable_bias_size = *max_cacheable_bias_chunks * *max_cacheable_filters;

    /* Load offsets */
    *channel_offset_incr = round_up(*feature_size, DMA_WORD_PER_BEAT);
    *out_channel_offset_incr = round_up(*output_w * *output_w, DMA_WORD_PER_BEAT);
    uint16_t output_pool_w = pool_type ? *output_w >> 1 : *output_w;
    *out_channel_pool_offset_incr = round_up(output_pool_w * output_pool_w, DMA_WORD_PER_BEAT);

    *feature_offset_incr = max_cacheable_rows_norm * width;
    *feature_offset_incr_init = max_cacheable_rows_norm_init * width;
    *filters_offset_start_base =  *channel_offset_incr * n_channels * batch_size;
    *bias_offset_start_base = *filters_offset_start_base + *filters_size;
    *feature_offset_start_base = *filters_offset_start_base + *filters_size + n_filters;
}

inline void conv2d::load_compute_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("load-compute-cfg-handshake");

    load_compute_cfg_done.req.req();
}

inline void conv2d::compute_load_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("compute-load-cfg-handshake");

    load_compute_cfg_done.ack.ack();
}

inline void conv2d::load_store_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("load-store-cfg-handshake");

    load_store_cfg_done.req.req();
}

inline void conv2d::store_load_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("store-load-cfg-handshake");

    load_store_cfg_done.ack.ack();
}
