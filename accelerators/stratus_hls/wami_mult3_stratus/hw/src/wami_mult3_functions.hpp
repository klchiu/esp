// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_MULT3_FUNCTIONS_HPP__
#define __WAMI_MULT3_FUNCTIONS_HPP__

#include "wami_mult3.hpp"

void wami_mult3::multiplication(uint32_t nRows, uint32_t nCols, uint32_t nCommon)
{
    // nRows = 6
    // nCols = 1
    // nCommon = 6

    uint32_t x, y, i;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            B0_delta_p[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(FPDATA(0.0));
            wait();
        }
        wait();
    }

    // for (i = 0; i < 36; i++) {
    //     FPDATA_WORD temp0111 = A0_hess_inv[i];
    //     FPDATA      tmp111   = int2fp<FPDATA, FPDATA_WL>(temp0111);
    //     fprintf(stderr, "A0_hess_inv: %d\t %f\n", i, (float)tmp111);
    // }
    // for (i = 0; i < 6; i++) {
    //     FPDATA_WORD temp0111 = A0_sd_delta_p[i];
    //     FPDATA      tmp111   = int2fp<FPDATA, FPDATA_WL>(temp0111);
    //     fprintf(stderr, "A0_sd_delta_p: %d\t %f\n", i, (float)tmp111);
    // }

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            for (i = 0; i < nCommon; i++) {

                FPDATA tmp1 = int2fp<FPDATA, FPDATA_WL>(A0_hess_inv[y * nCommon + i]);
                FPDATA tmp2 = int2fp<FPDATA, FPDATA_WL>(A0_sd_delta_p[i * nCols + x]);
                FPDATA tmp3 = tmp1 * tmp2;

                FPDATA tmp4 = int2fp<FPDATA, FPDATA_WL>(B0_delta_p[y * nCols + x]);
                FPDATA tmp5 = tmp4 + tmp3;

                // fprintf(stderr, "TB-tmp1 = %f\n", (float)tmp1);
                // fprintf(stderr, "TB-tmp2 = %f\n", (float)tmp2);
                // fprintf(stderr, "TB-tmp3 = %f\n", (float)tmp3);
                // fprintf(stderr, "TB-tmp4 = %f\n", (float)tmp4);
                // fprintf(stderr, "TB-tmp5 = %f\n", (float)tmp5);

                FPDATA_WORD temp05 = fp2bv<FPDATA, FPDATA_WL>(tmp5);

                B0_delta_p[y * nCols + x] = temp05;
                // printf("func: %d\t %f\t%f\t%f\n", y * nCols + x, (float)tmp1, (float)tmp2, (float)tmp4);
            }
        }
    }

    // for (i = 0; i < 6; i++) {
    //     FPDATA tmp111 = int2fp<FPDATA, FPDATA_WL>(B0_delta_p[i]);
    //     fprintf(stderr, "func B: %d\t %f\n", i, (float)tmp111);
    // }

}

#endif // __WAMI_MULT3_FUNCTIONS_HPP__
