// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_HESSIAN3_FUNCTIONS_HPP__
#define __WAMI_HESSIAN3_FUNCTIONS_HPP__

#include "wami_hessian3.hpp"


// Compute the 1D x and y gradients of the input matrix / image
void wami_hessian3::hess(uint32_t nCols, uint32_t nRows, uint32_t np)
{
    int i, j;
    int x, y;

    for (i = 0; i < np; i++) {
        for (j = 0; j < np; j++) {
            FPDATA      x    = FPDATA(0.0);
            FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(x);
            B0_hess[i * np + j]   = tmp3;
        }
    }

    for (y = 0; y < nRows; y++) {
        /* compare each image in the 6-wide I_steepest to each other image */
        for (i = 0; i < np; i++) {
            for (j = 0; j < np; j++) { /* sum the element-wise product of the images */
                FPDATA total = FPDATA(0.0);
                for (x = 0; x < nCols; x++) {
                    int         index1 = (np * y * nCols) + (nCols * i) + x;
                    int         index2 = (np * y * nCols) + (nCols * j) + x;
                    FPDATA_WORD tmp1   = A0_steepest_descent[index1];
                    FPDATA_WORD tmp2   = A0_steepest_descent[index2];

                    FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
                    FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

                    FPDATA temp03 = total + (in_fp_1 * in_fp_2);
                    total         = temp03;
                }

                FPDATA_WORD tmp0    = B0_hess[np * j + i];
                FPDATA      in_fp_0 = int2fp<FPDATA, FPDATA_WL>(tmp0);
                FPDATA      temp04  = total + in_fp_0;

                FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(temp04);
                wait();
                B0_hess[np * j + i] = tmp3;
            }
        }
    }
}

#endif // __WAMI_HESSIAN3_FUNCTIONS_HPP__
