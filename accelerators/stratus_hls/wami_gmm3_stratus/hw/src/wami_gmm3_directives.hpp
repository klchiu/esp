// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GMM3_DIRECTIVES_HPP__
#define __WAMI_GMM3_DIRECTIVES_HPP__

#define MAX_PXL_WIDTH     16
#define MAX_PXL_WIDTH_LOG 4
#define WORDS_PER_DMA     (DMA_WIDTH >> MAX_PXL_WIDTH_LOG)
#define PLM_HIST_SIZE     (1 << MAX_PXL_WIDTH)

typedef sc_uint<16> pixel_t;

#if defined(STRATUS_HLS)

    #define HLS_MAP_plm_A0_delta_p    HLS_MAP_TO_MEMORY(plm_A0_delta_p, "plm_reshape_6")
    #define HLS_MAP_plm_B0_sd_delta_p HLS_MAP_TO_MEMORY(plm_B0_sd_delta_p, "plm_reshape_6")

    #define HLS_MAP_plm_A0_gmm_img    HLS_MAP_TO_MEMORY(plm_A0_gmm_img, "plm_gmm_16384_16")
    #define HLS_MAP_plm_A0_mu         HLS_MAP_TO_MEMORY(plm_A0_mu, "plm_gmm_5x16384_64")
    #define HLS_MAP_plm_A0_sigma      HLS_MAP_TO_MEMORY(plm_A0_sigma, "plm_gmm_5x16384_64")
    #define HLS_MAP_plm_A0_weight     HLS_MAP_TO_MEMORY(plm_A0_weight, "plm_gmm_5x16384_64")
    #define HLS_MAP_plm_B0_foreground HLS_MAP_TO_MEMORY(plm_B0_foreground, "plm_gmm_16384_16")

    #define HLS_PROTO(_s) HLS_DEFINE_PROTOCOL(_s)

    #define HLS_FLAT(_a) HLS_FLATTEN_ARRAY(_a);

    #define HLS_BREAK_DEP(_a) HLS_BREAK_ARRAY_DEPENDENCY(_a)

    #define HLS_UNROLL_SIMPLE HLS_UNROLL_LOOP(ON)

    #if defined(HLS_DIRECTIVES_BASIC)

        #define HLS_PIPE_H1(_s)

        #define HLS_PIPE_H2(_s)

        #define HLS_DWT_XPOSE_CONSTR(_m, _s)

    #elif defined(HLS_DIRECTIVES_FAST)

        #define HLS_PIPE_H1(_s) HLS_PIPELINE_LOOP(HARD_STALL, 1, _s);

        #define HLS_PIPE_H2(_s) HLS_PIPELINE_LOOP(HARD_STALL, 2, _s);

        #define HLS_DWT_XPOSE_CONSTR(_m, _s) HLS_CONSTRAIN_ARRAY_MAX_DISTANCE(_m, 2, _s);

    #else

        #error Unsupported or undefined HLS configuration

    #endif /* HLS_DIRECTIVES_* */

#else /* !STRATUS_HLS */

    #define HLS_MAP_plm_A0_delta_p
    #define HLS_MAP_plm_B0_sd_delta_p

    #define HLS_MAP_plm_A0_gmm_img
    #define HLS_MAP_plm_A0_mu
    #define HLS_MAP_plm_A0_sigma
    #define HLS_MAP_plm_A0_weight
    #define HLS_MAP_plm_B0_foreground

    #define HLS_PROTO(_s)
    #define HLS_FLAT(_a)
    #define HLS_BREAK_DEP(_a)
    #define HLS_UNROLL_SIMPLE
    #define HLS_PIPE_H1(_s)
    #define HLS_PIPE_H2(_s)

    #define HLS_DWT_XPOSE_CONSTR(_m, _s)

#endif /* STRATUS_HLS */

#endif /* __WAMI_GMM3_DIRECTIVES_HPP_ */
