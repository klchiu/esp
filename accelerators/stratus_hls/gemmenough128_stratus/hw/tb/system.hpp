// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "gemmenough128_conf_info.hpp"
#include "gemmenough128_debug_info.hpp"
#include "gemmenough128.hpp"
#include "gemmenough128_directives.hpp"

#include "esp_templates.hpp"

// const size_t MEM_SIZE = 1024 / (DMA_WIDTH/8);
const size_t MEM_SIZE = 50000000;

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
#include "gemmenough128_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
public:

    // ACC instance
#ifdef CADENCE
    gemmenough128_wrapper *acc;
#else
    gemmenough128 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new gemmenough128_wrapper("gemmenough128_wrapper");
#else
        acc = new gemmenough128("gemmenough128_wrapper");
#endif
        // Binding ACC
        acc->clk(clk);
        acc->rst(acc_rst);
        acc->dma_read_ctrl(dma_read_ctrl);
        acc->dma_write_ctrl(dma_write_ctrl);
        acc->dma_read_chnl(dma_read_chnl);
        acc->dma_write_chnl(dma_write_chnl);
        acc->conf_info(conf_info);
        acc->conf_done(conf_done);
        acc->acc_done(acc_done);
        acc->debug(debug);

        /* <<--params-default-->> */
        ninputs = 1;
        d3 = 128;
        d2 = 128;
        d1 = 128;
        st_offset = 32768;
        ld_offset1 = 0;
        ld_offset2 = 16384;
    }

    // Processes

    // Configure accelerator
    void config_proc();

    // Load internal memory
    void load_memory();

    // Dump internal memory
    void dump_memory();

    // Validate accelerator results
    int validate();

    // Accelerator-specific data
    /* <<--params-->> */
    int32_t ninputs;
    int32_t d3;
    int32_t d2;
    int32_t d1;
    int32_t st_offset;
    int32_t ld_offset1;
    int32_t ld_offset2;

    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    int32_t *in;
    int32_t *out;
    int32_t *gold;

    // Other Functions
};

#endif // __SYSTEM_HPP__
