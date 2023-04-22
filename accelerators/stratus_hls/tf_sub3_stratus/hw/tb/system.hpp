// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <iostream>
#include <string>


#include "tf_sub3_conf_info.hpp"
#include "tf_sub3_debug_info.hpp"
#include "tf_sub3.hpp"
#include "tf_sub3_directives.hpp"
#include "fpdata.hpp"

#include "esp_templates.hpp"

//#define DATA_WIDTH 64
#define MEM_SIZE   5000000 // TODO: [humu] right now it's oversized

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
    #include "tf_sub3_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
  public:
    // ACC instance
#ifdef CADENCE
    tf_sub3_wrapper *acc;
#else
    tf_sub3 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new tf_sub3_wrapper("tf_sub3_wrapper");
#else
        acc = new tf_sub3("tf_sub3_wrapper");
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

        length = 0;
        base_addr_0;
        base_addr_1;
        base_addr_2;
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

    // Optionally free resources (arrays)
    void malloc_arrays(int len);
    void free_arrays();
    void init_arrays(int len);

    void run_pv(int len);

    // Accelerator-specific data
    uint32_t length;
    uint32_t base_addr_0;
    uint32_t base_addr_1;
    uint32_t base_addr_2;
    uint32_t chunk_size;

    float *output_0;
    float *input_1;
    float *input_2;
    float *gold_0;
    

    // Other Functions
};

#endif // __SYSTEM_HPP__
