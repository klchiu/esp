// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_SYNTH4_FUNCTIONS_HPP__
#define __WAMI_SYNTH4_FUNCTIONS_HPP__

#include "wami_synth4.hpp"


void wami_synth4::synth(uint32_t delay)
{
    // Read once from PLM and write once to PLM
    // Don't care the values

    int      i;
    uint64_t x = 0;
    // fprintf(stderr, "compute func debug 0, delay_A = %d, delay_B = %d\n");

    for (x = 0; x < 16; x++) {
        FPDATA_WORD temp    = A0B0_synth_in[x];
        FPDATA      fp_temp = int2fp<FPDATA, FPDATA_WL>(temp);
        // fprintf(stderr, "compute func debug 2\n");
        fp_temp = fp_temp + FPDATA(1.0);
        // fprintf(stderr, "compute func debug 3\n");
        FPDATA_WORD temp2 = fp2bv<FPDATA, FPDATA_WL>(fp_temp);
        // fprintf(stderr, "compute func debug 4\n");
        A0B0_synth_out[x] = temp2;
        // fprintf(stderr, "compute func debug 5\n");
        wait();
    }

    for (i = 0; i < delay; i++) {
            wait();
    }
}
#endif // __WAMI_SYNTH4_FUNCTIONS_HPP__