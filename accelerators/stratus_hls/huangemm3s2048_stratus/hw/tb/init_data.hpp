#ifndef __INIT_DATA_HPP__
#define __INIT_DATA_HPP__

#include "system.hpp"

void data_init(FPDATA* inA, FPDATA* inB, FPDATA* gold) {
    inA[0] = -1.0;
    inA[1] = -1.0;
    inA[2] = 3.0;
    inA[3] = 1.0;
    inA[4] = -3.0;
    inA[5] = -2.0;
    inA[6] = -2.0;
    inA[7] = -1.0;
    inB[0] = -3.0;
    inB[1] = -1.0;
    inB[2] = 3.0;
    inB[3] = 1.0;
    gold[0] = -11.0;
    gold[1] = -5.0;
}
#endif
