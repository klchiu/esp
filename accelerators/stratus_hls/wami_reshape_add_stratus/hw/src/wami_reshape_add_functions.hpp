// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_RESHAPE_ADD_FUNCTIONS_HPP__
#define __WAMI_RESHAPE_ADD_FUNCTIONS_HPP__

#include "wami_reshape_add.hpp"

// Optional application-specific helper functions
uint32_t mod_op(uint32_t a, uint32_t b)
{
    FPDATA aa = FPDATA(a);
    FPDATA bb = FPDATA(b);
    int    f  = floor(cynw_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(aa / bb)).to_uint();

    uint32_t mod = a - (b * f);

    return mod;
}

void wami_reshape_add::kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols)
{
    uint32_t i, j, cur_index;

    // for(i = 0 ;i < 6; i++){
    //     FPDATA in = int2fp<FPDATA, FPDATA_WL>(A0_delta_p[i]);
    //     fprintf(stderr, "A0_delta_p[%d]: %f\n", i, (float)in );
    // }

    for (cur_index = 0; cur_index < newRows * newCols; cur_index++) {

        uint32_t dst  = mod_op(cur_index, newRows) * newCols;
        uint32_t tmp1 = (cur_index / newRows);
        dst += tmp1;

        //fprintf(stderr, "dst[%d]: %d\n", cur_index, dst);

        uint32_t src = mod_op(cur_index, nRows) * nCols;

        FPDATA temp1 = FPDATA(cur_index);
        FPDATA temp2 = FPDATA(nRows);
        int    f     = floor(cynw_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(temp1 / temp2)).to_uint();

        src += f;
        // fprintf(stderr, "src[%d]: %d\n", cur_index, src);

        FPDATA_WORD in_src = A0_delta_p[src];

        A0_sd_delta_p[dst] = in_src;
    }
}



void wami_reshape_add::addition(uint32_t nRows, uint32_t nCols)
{
    uint32_t i, j;

    for (i = 0; i < nRows; i++) {
        for (j = 0; j < nCols; j++) {
            HLS_PROTO("wami_reshape_add_kernel");

            uint32_t index = i * nCols + j;
            wait();
            FPDATA_WORD tmp1 = A0_affine_params[index];
            wait();
            FPDATA_WORD tmp2 = A0_sd_delta_p[index];
            wait();

            FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
            FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

            FPDATA      x    = in_fp_1 + in_fp_2; // FPDATA(0.0);
            FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(x);

            B0_affine_params[index] = tmp3;
            wait();
        }
    }
}

#endif // __WAMI_RESHAPE_ADD_FUNCTIONS_HPP__
