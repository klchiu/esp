// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <iostream>
#include <string>

#include "../../../wami_common/wami_conf_info.hpp"
// #include "../../../wami_common/wami_C_data.hpp"

#include "../../../wami_common/wami_config_tb.hpp"
// #include "../../../wami_common/wami_utils.hpp"


#include "ny_acc_debug_info.hpp"
#include "ny_acc.hpp"
#include "ny_acc_directives.hpp"
#include "fpdata.hpp"

#include "esp_templates.hpp"

#define DATA_WIDTH 64
//#define MEM_SIZE   5000000 // TODO: [humu] right now it's oversized

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
    #include "ny_acc_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
  public:
    // ACC instance
#ifdef CADENCE
    ny_acc_wrapper *acc;
#else
    ny_acc *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new ny_acc_wrapper("ny_acc_wrapper");
#else
        acc = new ny_acc("ny_acc_wrapper");
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
    uint32_t num_img;
    uint32_t num_col;
    uint32_t num_row;
    uint32_t pad;
    uint32_t kern_id;
    uint32_t batch;
    uint32_t base_addr_0;
    uint32_t base_addr_1;
    uint32_t base_addr_2;
    uint32_t base_addr_3;
    uint32_t base_addr_4;
    uint32_t is_p2p;
    uint32_t p2p_config_0;
    uint32_t p2p_config_1;

  
  float output[65536];
    // Other Functions
};

#endif // __SYSTEM_HPP__
