// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_WARP_XY_FUNCTIONS_HPP__
#define __WAMI_WARP_XY_FUNCTIONS_HPP__

#include "wami_warp_xy.hpp"

// Optional application-specific helper functions

// Compute the 1D x and y gradients of the input matrix / image

// floor function
FPDATA wami_warp_xy::floor(FPDATA pixel_fp)
{
    sc_dt::sc_bv<FPDATA_WL> pixel_bv = fp2bv<FPDATA, FPDATA_WL>(pixel_fp);

    // sc_dt::sc_bv<FPDATA_WL> pixel_bv;

    // to_sc_bv<FPDATA_WORD, FPDATA_WL>(pixel_bv, pixel_fx);
    // pixel_bv.range(63, 0) = pixel_fx;

    const uint32_t   ML = FPDATA_WL - FPDATA_IL;
    sc_dt::sc_bv<ML> pixel_ml_bv(0);
    pixel_bv.range(ML - 1, 0) = pixel_ml_bv;

    // to_sc_fx<FPDATA_WORD, WARP64_WORDL>(pixel_fx_out, pixel_bv);
    // FPDATA_WORD pixel_fx_out = pixel_bv.to_uint();  // [humu]: there may be an error here

    FPDATA pixel_fp_out = bv2fp<FPDATA, FPDATA_WL>(pixel_bv);

    return pixel_fp_out;
}

FPDATA wami_warp_xy::interpolate(FPDATA Tlocalx, FPDATA Tlocaly, int nCols, int nRows)
{
    /* Linear interpolation variables */
    int    xBas0, xBas1, yBas0, yBas1;
    FPDATA perc[4] = {0, 0, 0, 0};
    FPDATA xCom, yCom, xComi, yComi;
    FPDATA color[4] = {0, 0, 0, 0};
    FPDATA result;
    /* Rounded location */
    FPDATA fTlocalx, fTlocaly;

    fTlocalx = floor(Tlocalx);
    fTlocaly = floor(Tlocaly);

    sc_dt::sc_bv<FPDATA_WL> ppx = fp2bv<FPDATA, FPDATA_WL>(fTlocalx);
    sc_dt::sc_bv<FPDATA_WL> ppy = fp2bv<FPDATA, FPDATA_WL>(fTlocaly);

    xBas0 = ppx.range(FPDATA_WL - 1, FPDATA_PL).to_int();
    yBas0 = ppy.range(FPDATA_WL - 1, FPDATA_PL).to_int();

    xBas1 = xBas0 + 1;
    yBas1 = yBas0 + 1;

    xCom    = Tlocalx - fTlocalx;
    yCom    = Tlocaly - fTlocaly;
    xComi   = (1 - xCom);
    yComi   = (1 - yCom);
    perc[0] = xComi * yComi;
    perc[1] = xComi * yCom;
    perc[2] = xCom * yComi;
    perc[3] = xCom * yCom;
    // fprintf(stderr, "---- Tlocalx: %f\tfTlocalx: %f\n", (float)Tlocalx, (float)fTlocalx);
    // fprintf(stderr, "---- Tlocaly: %f\tfTlocaly: %f\n", (float)Tlocaly, (float)fTlocaly);
    // fprintf(stderr, " xBas0 = %d\n", xBas0);
    // fprintf(stderr, " yBas0 = %d\n", yBas0);
    // fprintf(stderr, " xBas1 = %d\n", xBas1);
    // fprintf(stderr, " yBas1 = %d\n", yBas1);
    /* if all pixels are outside image, return 0, else in-image pixels are interpolated with 0 */
    if (xBas0 < -1) {
        return 0;
    }

    if (yBas0 < -1) {
        return 0;
    }

    if (xBas1 > nCols) {
        return 0;
    }

    if (yBas1 > nRows) {
        return 0;
    }

    /* sample values from the image, fill with 0 if the sample is just outside the edge */
    if (yBas0 < 0 || xBas0 < 0) {
        color[0] = FPDATA(0);
        // printf("-- 0.0\n");
    } else {
        FPDATA_WORD tmp_fpw_0 = A0_img[yBas0 * nCols + xBas0];
        FPDATA      tmp0      = int2fp<FPDATA, FPDATA_WL>(tmp_fpw_0);
        color[0]              = tmp0;
        // printf("-- 0.1:\t%f\n", (float)tmp0);
    }
    if (yBas1 > (nRows - 1) || xBas0 < 0) {
        color[1] = FPDATA(0);
        // printf("-- 1.0\n");
    } else {
        FPDATA_WORD tmp_fpw_1 = A0_img[yBas1 * nCols + xBas0];
        FPDATA      tmp1      = int2fp<FPDATA, FPDATA_WL>(tmp_fpw_1);
        color[1]              = tmp1;
        // printf("-- 1.1:\t%f\n", (float)tmp1);
    }
    if (yBas0 < 0 || xBas1 > (nCols - 1)) {
        color[2] = FPDATA(0);
        // printf("-- 2.0\n");
    } else {
        FPDATA_WORD tmp_fpw_2 = A0_img[yBas0 * nCols + xBas1];
        FPDATA      tmp2      = int2fp<FPDATA, FPDATA_WL>(tmp_fpw_2);
        color[2]              = tmp2;
        // printf("-- 2.1:\t%f\n", (float)tmp2);
    }
    if (yBas1 > (nRows - 1) || xBas1 > (nCols - 1)) {
        color[3] = FPDATA(0);
        // printf("-- 3.0\n");
    } else {
        FPDATA_WORD tmp_fpw_3 = A0_img[yBas1 * nCols + xBas1];
        FPDATA      tmp3      = int2fp<FPDATA, FPDATA_WL>(tmp_fpw_3);
        color[3]              = tmp3;
        // printf("-- 3.1:\t%f\n", (float)tmp3);
    }

    /* *** THIS WAR S A BUG! *** */
    /* color[3] = (yBas1 > (nRows-1) || xBas1 > (nCols-1)) ? 0 : Iin[yBas1 * nCols + xBas1]; */

    result = color[0] * perc[0] + color[1] * perc[1] + color[2] * perc[2] + color[3] * perc[3];

    // fprintf(stderr, "color[0]: %f\n", (float)color[0]);
    // fprintf(stderr, "color[1]: %f\n", (float)color[1]);
    // fprintf(stderr, "color[2]: %f\n", (float)color[2]);
    // fprintf(stderr, "color[3]: %f\n", (float)color[3]);
    // fprintf(stderr, "perc[0]: %f\n", (float)perc[0]);
    // fprintf(stderr, "perc[1]: %f\n", (float)perc[1]);
    // fprintf(stderr, "perc[2]: %f\n", (float)perc[2]);
    // fprintf(stderr, "perc[3]: %f\n", (float)perc[3]);
    // fprintf(stderr, "result: %f\n", (float)result);

    return result;
}

void wami_warp_xy::warp_image(uint32_t nCols, uint32_t nRows)
{
    int    x, y;
    FPDATA Tlocalx, Tlocaly;
    FPDATA compa0, compa1, compb0, compb1;
    int    index = 0;
    // convert input W_xp to FPDATA
    FPDATA w_xp_fp0 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[0]);
    FPDATA w_xp_fp1 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[1]);
    FPDATA w_xp_fp2 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[2]);
    FPDATA w_xp_fp3 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[3]);
    FPDATA w_xp_fp4 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[4]);
    FPDATA w_xp_fp5 = int2fp<FPDATA, FPDATA_WL>(A0_W_xp[5]);
    //-- fprintf(stderr, "[warp_kernel]: debug 1, nCols: %d, nRows: %d\n", nCols, nRows);

    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[0].to_uint64(), (float)w_xp_fp0);
    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[1].to_uint64(), (float)w_xp_fp1);
    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[2].to_uint64(), (float)w_xp_fp2);
    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[3].to_uint64(), (float)w_xp_fp3);
    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[4].to_uint64(), (float)w_xp_fp4);
    // fprintf(stderr, "[warp_kernel]: debug 2,  %d\t%f\n", A0_W_xp[5].to_uint64(), (float)w_xp_fp5);
// 
    // for (int i = 0; i < 128; i++) {
    //     FPDATA tmpFP = int2fp<FPDATA, FPDATA_WL>(A0_img[i]);
    //     fprintf(stderr, "A0_img[%d]:\t%f\n", i, (float)tmpFP);
    // }

    compb0 = w_xp_fp2;
    compb1 = w_xp_fp5;

    for (y = 0; y < nRows; y++) {
        // fprintf(stderr, "[row =  %d] %f, %f, %f, %f\n", y, (float)compa0, (float)compa1, (float)compb0, (float)compb1);

        compa0 = w_xp_fp1 * FPDATA(y) + compb0;
        compa1 = (FPDATA(1.0) + w_xp_fp4) * FPDATA(y) + compb1;

        for (x = 0; x < nCols; x++) {
            Tlocalx = (FPDATA(1.0) + w_xp_fp0) * FPDATA(x) + compa0;
            Tlocaly = w_xp_fp3 * FPDATA(x) + compa1;

            FPDATA tmp = interpolate(Tlocalx, Tlocaly, nCols, nRows);
            // convert output back to FPDATA_WORD
            B0_img[index] = fp2bv<FPDATA, FPDATA_WL>(tmp);

            index++;
        }
    }
}

#endif // __WAMI_WARP_XY_FUNCTIONS_HPP__
