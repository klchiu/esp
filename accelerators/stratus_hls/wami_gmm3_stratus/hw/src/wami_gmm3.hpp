// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GMM3_HPP__
#define __WAMI_GMM3_HPP__

// #include "wami_gmm3_data.hpp"
#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"


#include "wami_gmm3_debug_info.hpp"
#include "wami_gmm3_directives.hpp"


#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class wami_gmm3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(wami_gmm3);
    wami_gmm3(const sc_module_name &name)
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
        HLS_MAP_plm_A0_gmm_img;
        HLS_MAP_plm_A0_mu;
        HLS_MAP_plm_A0_sigma;
        HLS_MAP_plm_A0_weight;
        HLS_MAP_plm_B0_foreground;
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


    // Configure wami_gmm3
    esp_config_proc cfg;

    // Functions
    // void kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols);
    void kernel_gmm(uint32_t nRows, uint32_t nCols, uint32_t nModels);

    // -- Private local memories
    uint16_t plm_A0_gmm_img[WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];
    FPDATA_WORD plm_A0_mu[5*WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];
    FPDATA_WORD plm_A0_sigma[5*WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];
    FPDATA_WORD plm_A0_weight[5*WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];
    uint16_t plm_B0_foreground[WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];


    // -- Private state variables
};

inline void wami_gmm3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_gmm3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_GMM3_HPP__ */
