// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __TF_MULT3_FUNCTIONS_HPP__
#define __TF_MULT3_FUNCTIONS_HPP__

#include "tf_mult3.hpp"


// Compute the 1D x and y gradients of the input matrix / image

void tf_mult3::mult(uint32_t length)
{
    // B0_out = A0_in1 - A0_in2
    uint32_t i;

    //    for (i = 0; i < 20; i++) {
    //        FPDATA_WORD temp0111 = A0_in1[i];
    //        FPDATA      tmp111   = int2fp<FPDATA, FPDATA_WL>(temp0111);
    //        FPDATA_WORD temp0222 = A0_in2[i];
    //        FPDATA      tmp222   = int2fp<FPDATA, FPDATA_WL>(temp0222);
    //
    //        printf("func B: %d\t %f\t%f\n", i, (float)tmp111, (float)tmp222);
    //    }

    // printf("nRows = %d\n", nRows);
    // printf("nCols = %d\n", nCols);
    for (i = 0; i < length; i++) {
            // uint32_t index = i * nCols + j;
            HLS_UNROLL_LOOP(COMPLETE, 4, "unroll-add-func");

            FPDATA_WORD tmp1 = A0_in1[i];
            FPDATA_WORD tmp2 = A0_in2[i];

            FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
            FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

            FPDATA      x    = in_fp_1 * in_fp_2; // FPDATA(0.0);
            FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(x);

            B0_out[i] = tmp3;

            // printf("func: %d\t %f\t%f\t%f\n", index, (float)in_fp_1, (float)in_fp_2, (float)x);
    }
}

#endif // __TF_MULT3_FUNCTIONS_HPP__
