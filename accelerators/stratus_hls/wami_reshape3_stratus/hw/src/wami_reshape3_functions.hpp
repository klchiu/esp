// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_RESHAPE3_FUNCTIONS_HPP__
#define __WAMI_RESHAPE3_FUNCTIONS_HPP__

#include "wami_reshape3.hpp"

#include "../../../wami_common/lib_math/div_func.hpp"
#include "../../../wami_common/lib_math/sqrt_func.hpp"
#include "../../../wami_common/lib_math/cordic/exp_cordic.hpp"


// ----------- exp ------------
template <int IN_W, int IN_I, int OUT_W, int OUT_I>
sc_fixed<OUT_W, OUT_I> exp_func(sc_fixed<IN_W, IN_I> x_in)
{

    sc_fixed<32, 16, SC_RND> x_out;
    x_out = exp_cordic_func<10, TRAIN64_WORDL, TRAIN_IWORDL>(x_in);

    cout << "in bin = " << x_in.to_bin() << "\t x_in = " << x_in << endl;
    cout << "in bin = " << x_out.to_bin() << "\t x_out = " << x_out << endl;


    return x_out;
}
// ----------- end of exp ------------


// Optional application-specific helper functions
uint32_t mod_op(uint32_t a, uint32_t b)
{
    FPDATA aa = FPDATA(a);
    FPDATA bb = FPDATA(b);
    int    f  = floor(sc_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(aa / bb)).to_uint();

    uint32_t mod = a - (b * f);

    return mod;
}

void wami_reshape3::kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols)
{
    uint32_t i, j, cur_index;

    // for(i = 0 ;i < 6; i++){
    //     FPDATA in = int2fp<FPDATA, FPDATA_WL>(plm_A0_delta_p[i]);
    //     fprintf(stderr, "plm_A0_delta_p[%d]: %f\n", i, (float)in );
    // }


/*
    sc_fixed<TRAIN64_WORDL, TRAIN_IWORDL, SC_RND> x = sc_fixed<TRAIN64_WORDL, TRAIN_IWORDL, SC_RND>(8);
    sc_fixed<TRAIN64_WORDL, TRAIN_IWORDL, SC_RND> y = sc_fixed<TRAIN64_WORDL, TRAIN_IWORDL, SC_RND>(3);

    sc_fixed<TRAIN64_WORDL, TRAIN_IWORDL, SC_RND> div_out;
    div_out = div_func<TRAIN64_WORDL, TRAIN_IWORDL, TRAIN64_WORDL, TRAIN_IWORDL, TRAIN64_WORDL, TRAIN_IWORDL>(x, y);

    fprintf(stderr, "[kernel_reshape TEST]: x = %f\n", (float)x);
    fprintf(stderr, "[kernel_reshape TEST]: y = %f\n", (float)y);
    fprintf(stderr, "[kernel_reshape TEST]: div_out = %f\n", (float)div_out);

    sc_fixed<64, 48, SC_RND> sqrt_out;
    sc_fixed<64, 48, SC_RND> z = sc_fixed<64, 48, SC_RND>(10);
    sqrt_out                   = sqrt_func<64, 48, 64, 48>(z);

    fprintf(stderr, "[kernel_reshape TEST]: z = %f\n", (float)z);
    fprintf(stderr, "[kernel_reshape TEST]: sqrt_out = %f\n", (float)sqrt_out);

    sc_fixed<32, 16, SC_RND> v = sc_fixed<32, 16, SC_RND>(10);
    sc_fixed<32, 16, SC_RND> sqrt_out2;
    sqrt_out2 = sqrt_func2<32, 16, 32, 16>(v);

    fprintf(stderr, "[kernel_reshape TEST]: v = %f\n", (float)v);
    fprintf(stderr, "[kernel_reshape TEST]: sqrt_out2 = %f\n", (float)sqrt_out2);


    sc_fixed<32, 16, SC_RND> x_in = sc_fixed<32, 16, SC_RND>(-3.125);
    sc_fixed<32, 16, SC_RND> x_out;
    fprintf(stderr, "[kernel_reshape TEST]: x_in = %f\n", (float)x_in);

    x_out = exp_func<32, 16, 32, 16>(x_in);
    fprintf(stderr, "[kernel_reshape TEST]: x_out = %f\n", (float)x_out);


    return;
*/


    for (cur_index = 0; cur_index < newRows * newCols; cur_index++) {

        uint32_t dst  = mod_op(cur_index, newRows) * newCols;
        uint32_t tmp1 = (cur_index / newRows);
        dst += tmp1;

        // fprintf(stderr, "dst[%d]: %d\n", cur_index, dst);

        uint32_t src = mod_op(cur_index, nRows) * nCols;

        FPDATA temp1 = FPDATA(cur_index);
        FPDATA temp2 = FPDATA(nRows);
        int    f     = floor(sc_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(temp1 / temp2)).to_uint();

        src += f;
        // fprintf(stderr, "src[%d]: %d\n", cur_index, src);

        FPDATA_WORD in_src = plm_A0_delta_p[src];

        plm_B0_sd_delta_p[dst] = in_src;
    }
}

#endif // __WAMI_RESHAPE3_FUNCTIONS_HPP__
