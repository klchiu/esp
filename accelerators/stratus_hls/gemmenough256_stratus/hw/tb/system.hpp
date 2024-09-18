// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "gemmenough256_conf_info.hpp"
#include "gemmenough256_debug_info.hpp"
#include "gemmenough256.hpp"
#include "gemmenough256_directives.hpp"

#include "esp_templates.hpp"

// const size_t MEM_SIZE = 1024 / (DMA_WIDTH/8);
const size_t MEM_SIZE = 50000000;

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
#include "gemmenough256_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
public:

    // ACC instance
#ifdef CADENCE
    gemmenough256_wrapper *acc;
#else
    gemmenough256 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new gemmenough256_wrapper("gemmenough256_wrapper");
#else
        acc = new gemmenough256("gemmenough256_wrapper");
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
        d3 = 256;
        d2 = 256;
        d1 = 256;
        st_offset = 131072;
        ld_offset1 = 0;
        ld_offset2 = 65536;
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
