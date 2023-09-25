// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_ADD3_FUNCTIONS_HPP__
#define __WAMI_ADD3_FUNCTIONS_HPP__

#include "wami_add3.hpp"

void wami_add3::addition(uint32_t nRows, uint32_t nCols)
{
    uint32_t i, j;

    for (i = 0; i < nRows; i++) {
        for (j = 0; j < nCols; j++) {
            HLS_PROTO("wami_add3_kernel");

            uint32_t index = i * nCols + j;
            wait();
            FPDATA_WORD tmp1 = A0_affine_params[index];
            wait();
            FPDATA_WORD tmp2 = A0_sd_delta_p[index];
            wait();

            FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
            FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

            FPDATA      x    = in_fp_1 + in_fp_2;
            FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(x);

            B0_affine_params[index] = tmp3;
            wait();
        }
    }
}

#endif // __WAMI_ADD3_FUNCTIONS_HPP__
