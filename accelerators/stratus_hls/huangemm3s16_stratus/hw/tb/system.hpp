// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "huangemm3s16_conf_info.hpp"
#include "huangemm3s16_debug_info.hpp"
#include "huangemm3s16.hpp"
#include "huangemm3s16_directives.hpp"

#include "esp_templates.hpp"

const size_t MEM_SIZE = 4160000 / (DMA_WIDTH/8);

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
#include "huangemm3s16_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
public:

    // ACC instance
#ifdef CADENCE
    huangemm3s16_wrapper *acc;
#else
    huangemm3s16 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new huangemm3s16_wrapper("huangemm3s16_wrapper");
#else
        acc = new huangemm3s16("huangemm3s16_wrapper");
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
        // rows = 10;
        // cols = 1;
        // loaded_cols = 32678;

        rows = 4;
        cols = 4;
        loaded_cols = 4;

        // rows = 10;
        // cols = 1;
        // loaded_cols = 32678;
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
    int32_t rows;
    int32_t cols;
    int32_t loaded_cols;

    uint32_t A_size_adj;
    uint32_t B_size_adj;
    uint32_t O_size_adj;

    uint32_t A_size;
    uint32_t B_size;
    uint32_t O_size;

    FPDATA *inA;
    FPDATA *inB;
    FPDATA *outO;
    FPDATA *gold;
    
    // int32_t *inA;
    // int32_t *inB;
    // int32_t *outO;
    // int32_t *gold;

    // sc_dt::sc_int<DATA_WIDTH> *inA;
    // sc_dt::sc_int<DATA_WIDTH> *inB;
    // sc_dt::sc_int<DATA_WIDTH> *outO;
    // sc_dt::sc_int<DATA_WIDTH> *gold;

    // Other Functions
};

#endif // __SYSTEM_HPP__
