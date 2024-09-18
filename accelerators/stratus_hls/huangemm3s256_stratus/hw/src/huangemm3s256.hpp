// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __HUANGEMM3S256_HPP__
#define __HUANGEMM3S256_HPP__

#include "huangemm3s256_conf_info.hpp"
#include "huangemm3s256_debug_info.hpp"

#include "esp_templates.hpp"

#include "huangemm3s256_directives.hpp"

// supporting float calculation
#include "fpdata.hpp"

#define __round_mask(x, y) ((y)-1)
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
/* <<--defines-->> */
#define DATA_WIDTH 32
#define DMA_SIZE SIZE_WORD
#define PLM_OUT_WORD 256
// #define PLM_IN_WORD 326782
#define PLM_IN_WORD 256

class huangemm3s256 : public esp_accelerator_3P<DMA_WIDTH>
{
public:
    // Constructor
    SC_HAS_PROCESS(huangemm3s256);
    huangemm3s256(const sc_module_name& name)
    : esp_accelerator_3P<DMA_WIDTH>(name)
        , cfg("config")
    {
        // Signal binding
        cfg.bind_with(*this);

        // Map arrays to memories
        /* <<--plm-bind-->> */

        HLS_MAP_plm(O_pong, PLM_OUT_NAME);
        HLS_MAP_plm(O_ping, PLM_OUT_NAME);
        HLS_MAP_plm(A_ping, PLM_IN_NAME);
        HLS_MAP_plm(A_pong, PLM_IN_NAME);
        HLS_MAP_plm(B_ping, PLM_IN_NAME);
        HLS_MAP_plm(B_pong, PLM_IN_NAME);
    }

    // Processes

    // Load the input data
    void load_input();

    // Computation
    void compute_kernel();

    // Store the output data
    void store_output();

    // Configure huangemm3s256
    esp_config_proc cfg;

    // Functions

    // Private local memories
    sc_dt::sc_int<DATA_WIDTH> A_ping[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> A_pong[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> B_ping[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> B_pong[PLM_IN_WORD];
    sc_dt::sc_int<DATA_WIDTH> O_ping[PLM_OUT_WORD];
    sc_dt::sc_int<DATA_WIDTH> O_pong[PLM_OUT_WORD];


};


#endif /* __HUANGEMM3S256_HPP__ */
