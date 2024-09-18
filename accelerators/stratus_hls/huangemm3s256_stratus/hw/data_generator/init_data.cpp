#ifndef __INIT_DATA_CPP__
#define __INIT_DATA_CPP__

void data_init(FPDATA* inA, FPDATA* inB, FPDATA* gold) {
    inA[0] = 3.0;
    inA[1] = 4.0;
    inA[2] = 3.0;
    inA[3] = 5.0;
    inA[4] = 5.0;
    inA[5] = 3.0;
    inA[6] = 4.0;
    inA[7] = 6.0;
    inB[0] = 4.0;
    inB[1] = 5.0;
    inB[2] = 1.0;
    inB[3] = 5.0;
    gold[0] = 52.0;
    gold[1] = 74.0;
}
#endif
