// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_RESHAPE3_HPP__
#define __WAMI_RESHAPE3_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "wami_reshape3_debug_info.hpp"
#include "wami_reshape3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class wami_reshape3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(wami_reshape3);
    wami_reshape3(const sc_module_name &name)
        : esp_accelerator_3P<DMA_WIDTH>(name)
        , cfg("config")
        , accel_ready("accel_ready")
    {
        // SC_CTHREAD(config_kernel, clk.pos());
        // reset_signal_is(rst, false);

        // Signal binding
        cfg.bind_with(*this);
        accel_ready.bind_with(*this);

        // Clock binding for memories
        HLS_MAP_plm_A0_delta_p;
        HLS_MAP_plm_B0_sd_delta_p;
    }

    // Processes

    // void config_kernel();  // configuration
    void load_input();     // load input
    void compute_kernel(); // iterative Debayer
    void store_output();   // store output

    // Additional handshakes
    inline void store_load_handshake();
    inline void load_store_handshake();

    // Internal synchronization signals
    sc_signal<bool> init_done;


    // Configure wami_reshape3
    esp_config_proc cfg;

    // Functions
    void kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols);

    // -- Private local memories
    FPDATA_WORD plm_A0_delta_p[6];
    FPDATA_WORD plm_B0_sd_delta_p[6];

    // -- Private state variables
};

inline void wami_reshape3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_reshape3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_RESHAPE3_HPP__ */
