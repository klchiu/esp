// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_MULT_RESHAPE_ADD_FUNCTIONS_HPP__
#define __WAMI_MULT_RESHAPE_ADD_FUNCTIONS_HPP__

#include "wami_mult_reshape_add.hpp"

void wami_mult_reshape_add::multiplication(uint32_t nRows, uint32_t nCols, uint32_t nCommon)
{
    // nRows = 6
    // nCols = 1
    // nCommon = 6

    uint32_t x, y, i;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            A0B0_delta_p[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(FPDATA(0.0));
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

    /* most trivial implementation */
    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            for (i = 0; i < nCommon; i++) {

                FPDATA tmp1 = int2fp<FPDATA, FPDATA_WL>(A0_hess_inv[y * nCommon + i]);
                FPDATA tmp2 = int2fp<FPDATA, FPDATA_WL>(A0_sd_delta_p[i * nCols + x]);
                FPDATA tmp3 = tmp1 * tmp2;

                FPDATA tmp4 = int2fp<FPDATA, FPDATA_WL>(A0B0_delta_p[y * nCols + x]);
                FPDATA tmp5 = tmp4 + tmp3;

                // fprintf(stderr, "TB-tmp1 = %f\n", (float)tmp1);
                // fprintf(stderr, "TB-tmp2 = %f\n", (float)tmp2);
                // fprintf(stderr, "TB-tmp3 = %f\n", (float)tmp3);
                // fprintf(stderr, "TB-tmp4 = %f\n", (float)tmp4);
                // fprintf(stderr, "TB-tmp5 = %f\n", (float)tmp5);

                FPDATA_WORD temp05 = fp2bv<FPDATA, FPDATA_WL>(tmp5);

                A0B0_delta_p[y * nCols + x] = temp05;
                // printf("func: %d\t %f\t%f\t%f\n", y * nCols + x, (float)tmp1, (float)tmp2, (float)tmp4);
            }
        }
    }

    //  for (i = 0; i < 6; i++) {
    //      FPDATA      tmp111   = int2fp<FPDATA, FPDATA_WL>(A0B0_delta_p[i]);
    //      fprintf(stderr, "func B: %d\t %f\n", i, (float)tmp111);
    //  }

    // fprintf(stderr, "nRows: %d\n", nRows);
    // fprintf(stderr, "nCols: %d\n", nCols);
    // fprintf(stderr, "nCommon: %d\n", nCommon);
}

// Optional application-specific helper functions
uint32_t mod_op(uint32_t a, uint32_t b)
{
    FPDATA aa = FPDATA(a);
    FPDATA bb = FPDATA(b);
    int    f  = floor(cynw_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(aa / bb)).to_uint();

    uint32_t mod = a - (b * f);

    return mod;
}

void wami_mult_reshape_add::kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols)
{
    uint32_t i, j, cur_index;

    // for(i = 0 ;i < 6; i++){
    //     FPDATA in = int2fp<FPDATA, FPDATA_WL>(A0B0_delta_p[i]);
    //     fprintf(stderr, "A0B0_delta_p[%d]: %f\n", i, (float)in );
    // }

    for (cur_index = 0; cur_index < newRows * newCols; cur_index++) {

        uint32_t dst  = mod_op(cur_index, newRows) * newCols;
        uint32_t tmp1 = (cur_index / newRows);
        dst += tmp1;

        // fprintf(stderr, "dst[%d]: %d\n", cur_index, dst);

        uint32_t src = mod_op(cur_index, nRows) * nCols;

        FPDATA temp1 = FPDATA(cur_index);
        FPDATA temp2 = FPDATA(nRows);
        int    f     = floor(cynw_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(temp1 / temp2)).to_uint();

        src += f;
        // fprintf(stderr, "src[%d]: %d\n", cur_index, src);

        FPDATA_WORD in_src = A0B0_delta_p[src];

        A0B0_sd_delta_p[dst] = in_src;
    }
}

void wami_mult_reshape_add::addition(uint32_t nRows, uint32_t nCols)
{
    uint32_t i, j;

    for (i = 0; i < nRows; i++) {
        for (j = 0; j < nCols; j++) {
            HLS_PROTO("wami_mult_reshape_add_kernel");

            uint32_t index = i * nCols + j;
            wait();
            FPDATA_WORD tmp1 = A0_affine_params[index];
            wait();
            FPDATA_WORD tmp2 = A0B0_sd_delta_p[index];
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

#endif // __WAMI_MULT_RESHAPE_ADD_FUNCTIONS_HPP__
