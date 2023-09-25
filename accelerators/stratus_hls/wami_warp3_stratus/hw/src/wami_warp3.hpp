// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_WARP3_HPP__
#define __WAMI_WARP3_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "wami_warp3_debug_info.hpp"
#include "wami_warp3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class interp_args_t
{
  public:
    interp_args_t()
        : local_x(0.0)
        , local_y(0.0)
        , num_imgs(0)
    {
    }

    interp_args_t(FPDATA x, FPDATA y, uint32_t i)
        : local_x(x)
        , local_y(y)
        , num_imgs(i)
    {
    }

    bool operator==(const interp_args_t &rhs) const
    {
        return (rhs.local_x == local_x) && (rhs.local_y == local_y) && (rhs.num_imgs == num_imgs);
    }

    friend void sc_trace(sc_trace_file *tf, const interp_args_t &v,
                         const std::string &NAME) // VCD dumping function
    {
        ;
    }

    friend ostream &operator<<(ostream &os, interp_args_t const &conf_info) { return os; }

    FPDATA   local_x;
    FPDATA   local_y;
    uint32_t num_imgs;
};

class wami_warp3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(wami_warp3);
    wami_warp3(const sc_module_name &name)
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
        HLS_MAP_A0_W_xp;
        HLS_MAP_A0_img;
        HLS_MAP_B0_img;
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



    // Configure wami_warp3
    esp_config_proc cfg;

    /*
        b_get_initiator<interp_args_t> interpolation_args_get;
        b_put_initiator<interp_args_t> interpolation_args_put;

        b_put_initiator<FPDATA_WORD> interpolation_result_put;
        b_get_initiator<FPDATA_WORD> interpolation_result_get;

        // read and written by load_input
        put_get_channel<interp_args_t>     interpolation_args;
        put_get_channel<WARP64_CTOS_FLOAT> interpolation_result;
    */

    // Functions
    FPDATA floor(FPDATA pixel_fp);
    FPDATA interpolate(FPDATA Tlocalx, FPDATA Tlocaly, int nCols, int nRows);
    void   warp_image(uint32_t nCols, uint32_t nRows);

    FPDATA_WORD compb0, compb1;

    // -- Private local memories
    FPDATA_WORD A0_W_xp[6];
    FPDATA_WORD A0_img[WAMI_WARP_IMG_NUM_COLS * WAMI_WARP_IMG_NUM_ROWS]; // [128][128]
    FPDATA_WORD B0_img[WAMI_WARP_IMG_NUM_COLS * WAMI_WARP_IMG_NUM_ROWS]; // [128][128]

    // -- Private state variables
};

inline void wami_warp3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_warp3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_WARP3_HPP__ */
