// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

// temp buffer for PEs
word_t tempC[9];
word_t status_load;
word_t status_compute;
word_t status_store;

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
          const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim,
          const unsigned state_control, dma_info_t &load_ctrl, int chunk,
          int batch) // , word_t *in_west_11, word_t *in_west_21, word_t *in_north_11, word_t *in_north_12)
{
load_data:

    const unsigned length = round_up(matrix_A_dim * matrix_A_dim + matrix_B_dim * matrix_B_dim, VALUES_PER_WORD) / 1;
    const unsigned index  = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

    status_load = 10;

    if (state_control == 1) {

        status_load = 11;

        load_ctrl.index  = dma_index;
        load_ctrl.length = dma_length;
        load_ctrl.size   = SIZE_WORD_T;

#ifndef __SYNTHESIS__
        printf("[load]: dma_index = %d\n", dma_index);
        printf("[load]: dma_length = %d\n", dma_length);
        printf("[load]: SIZE_WORD_T = %d\n", SIZE_WORD_T);
        printf("[load]: VALUES_PER_WORD = %d\n", VALUES_PER_WORD);
#endif

        for (unsigned i = 0; i < dma_length; i++) {
load_label0:
            for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
                _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
#ifndef __SYNTHESIS__
                printf("[load]: _inbuff[%d] = %d\n", i * VALUES_PER_WORD + j, (int)_inbuff[i * VALUES_PER_WORD + j]);
#endif
            }
        }

        // in_west_11[0]  = _inbuff[0];
        // in_west_11[1]  = _inbuff[1];
        // in_west_21[1]  = _inbuff[2];
        // in_west_21[2]  = _inbuff[3];
        // in_north_11[0] = _inbuff[4];
        // in_north_11[1] = _inbuff[6];
        // in_north_12[1] = _inbuff[5];
        // in_north_12[2] = _inbuff[7];
    }

    status_load = 12;
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
           /* <<--compute-params-->> */
           const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim,
           const unsigned state_control, dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length       = round_up(matrix_C_dim * matrix_C_dim, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(matrix_A_dim * matrix_A_dim + matrix_B_dim * matrix_B_dim, VALUES_PER_WORD) * 1;
    const unsigned out_offset   = store_offset;
    const unsigned index        = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index  = index / VALUES_PER_WORD;

#ifndef __SYNTHESIS__
    printf("[store]: tempC[0] = %d\n", (int)tempC[0]);
    printf("[store]: tempC[1] = %d\n", (int)tempC[1]);
    printf("[store]: tempC[2] = %d\n", (int)tempC[2]);
    printf("[store]: tempC[3] = %d\n", (int)tempC[3]);
    printf("[store]: tempC[4] = %d\n", (int)tempC[4]);
    printf("[store]: tempC[5] = %d\n", (int)tempC[5]);
    printf("[store]: tempC[6] = %d\n", (int)tempC[6]);
    printf("[store]: tempC[7] = %d\n", (int)tempC[7]);
    printf("[store]: tempC[8] = %d\n", (int)tempC[8]);
    printf("[store]: dma_index = %d\n", dma_index);
    printf("[store]: dma_length = %d\n", dma_length);
    printf("[store]: SIZE_WORD_T = %d\n", SIZE_WORD_T);
    printf("[store]: VALUES_PER_WORD = %d\n", VALUES_PER_WORD);
#endif

// #ifndef __SYNTHESIS__
    // for(unsigned x = 0 ; x < 4; x++){
    //     printf("[store]: tempC[%d] = %d\n", x, tempC[x]);
    // }
// #endif

    status_store = 10;

    if (state_control == 2) {
        status_store = 11;

        store_ctrl.index  = dma_index;
        store_ctrl.length = dma_length;
        store_ctrl.size   = SIZE_WORD_T;

        // _outbuff[0] = 100;
        // _outbuff[1] = 101;
        // _outbuff[2] = 102;
        // _outbuff[3] = 103;
        // _outbuff[4] = 104;
        // _outbuff[5] = 105;
        // _outbuff[6] = 106;
        // _outbuff[7] = 107;
        // _outbuff[8] = 108;


        for (unsigned i = 0; i < dma_length; i++) {
store_label1:
            for (unsigned j = 0; j < VALUES_PER_WORD; j++) {
                out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
                // out[dma_index + i].word[j] = tempC[i * VALUES_PER_WORD + j];

// #ifndef __SYNTHESIS__
//                 printf("[store]: dma_index + i = %d\n", dma_index + i);
//                 printf("[store]: j = %d\n", j);
//                 printf("[store]: i * VALUES_PER_WORD + j = %d\n", i * VALUES_PER_WORD + j);
//                 // printf("[store]: _outbuff = %d\n", _outbuff[i * VALUES_PER_WORD + j]);
// #endif
            }
        }
    }

    status_store = 12;
}

void compute_PE_2(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t &out_buff,
                  int PE_index, int stream_length)
{
#pragma HLS inline off

#pragma HLS function_instantiate variable = PE_index

    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    // printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;
#ifndef __SYNTHESIS__
        printf("[humu]: compute_PE_2: PE_index = %d, input_north = %d\n", PE_index, (int)(input_north[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, input_west = %d\n", PE_index, (int)(input_west[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, temp1 = %d\n", PE_index, (int)(temp1));
        printf("[humu]: compute_PE_2: PE_index = %d, temp2 = %d\n", PE_index, (int)(temp2));
        printf("[humu]: compute_PE_2: PE_index = %d, tempC[%d] = %d\n", PE_index, PE_index, (int)(tempC[PE_index]));
#endif
            out_buff = tempC[PE_index];
    }

    // pass west to east and pass north to south

    for (int i = 0; i < stream_length; i++) {
        output_east[i + 1]  = input_west[i];
        output_south[i + 1] = input_north[i];
    }
}

void compute_PE_3(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t &out_buff,
                  int PE_index, int stream_length)
{
#pragma HLS inline off

#pragma HLS function_instantiate variable = PE_index

    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    // printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;
#ifndef __SYNTHESIS__
        printf("[humu]: compute_PE_2: PE_index = %d, input_north = %d\n", PE_index, (int)(input_north[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, input_west = %d\n", PE_index, (int)(input_west[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, temp1 = %d\n", PE_index, (int)(temp1));
        printf("[humu]: compute_PE_2: PE_index = %d, temp2 = %d\n", PE_index, (int)(temp2));
        printf("[humu]: compute_PE_2: PE_index = %d, tempC[%d] = %d\n", PE_index, PE_index, (int)(tempC[PE_index]));
#endif
        out_buff = tempC[PE_index];
    }



    // pass west to east and pass north to south

    for (int i = 0; i < stream_length; i++) {
        output_east[i + 1]  = input_west[i];
        output_south[i + 1] = input_north[i];
    }
}

void compute_PE_4(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t &out_buff,
                  int PE_index, int stream_length)
{
#pragma HLS inline off

#pragma HLS function_instantiate variable = PE_index

    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    // printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;
#ifndef __SYNTHESIS__
        printf("[humu]: compute_PE_2: PE_index = %d, input_north = %d\n", PE_index, (int)(input_north[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, input_west = %d\n", PE_index, (int)(input_west[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, temp1 = %d\n", PE_index, (int)(temp1));
        printf("[humu]: compute_PE_2: PE_index = %d, temp2 = %d\n", PE_index, (int)(temp2));
        printf("[humu]: compute_PE_2: PE_index = %d, tempC[%d] = %d\n", PE_index, PE_index, (int)(tempC[PE_index]));
#endif
        out_buff = tempC[PE_index];
    }



    // pass west to east and pass north to south

    for (int i = 0; i < stream_length; i++) {
        output_east[i + 1]  = input_west[i];
        output_south[i + 1] = input_north[i];
    }
}

void compute_PE_5(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t &out_buff,
                  int PE_index, int stream_length)
{
#pragma HLS inline off

#pragma HLS function_instantiate variable = PE_index

    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    // printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;
#ifndef __SYNTHESIS__
        printf("[humu]: compute_PE_2: PE_index = %d, input_north = %d\n", PE_index, (int)(input_north[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, input_west = %d\n", PE_index, (int)(input_west[i]));
        printf("[humu]: compute_PE_2: PE_index = %d, temp1 = %d\n", PE_index, (int)(temp1));
        printf("[humu]: compute_PE_2: PE_index = %d, temp2 = %d\n", PE_index, (int)(temp2));
        printf("[humu]: compute_PE_2: PE_index = %d, tempC[%d] = %d\n", PE_index, PE_index, (int)(tempC[PE_index]));
#endif
        out_buff = tempC[PE_index];
    }



    // pass west to east and pass north to south

    for (int i = 0; i < stream_length; i++) {
        output_east[i + 1]  = input_west[i];
        output_south[i + 1] = input_north[i];
    }
}

/*
void compute_PE(word_t *input_west, word_t *input_north, word_t *output_east, word_t *output_south, word_t *out_buff,
                int PE_index, int stream_length)
{
    // *output = *output + ((*input_north) * (*input_west));
    word_t temp1;
    word_t temp2;

    // printf("[humu]: computePE: PE_index = %d, stream_length = %d\n", PE_index, stream_length);

    for (int i = 0; i < stream_length; i++) {
        temp1           = input_west[i] * input_north[i];
        temp2           = tempC[PE_index];
        tempC[PE_index] = temp1 + temp2;

        // printf("[humu]: computePE: PE_index = %d, input_north = %d\n", PE_index, (int)(input_north[i]));
        // printf("[humu]: computePE: PE_index = %d, input_west = %d\n", PE_index, (int)(input_west[i]));
        // printf("[humu]: computePE: PE_index = %d, temp1 = %d\n", PE_index, (int)(temp1));
        // printf("[humu]: computePE: PE_index = %d, temp2 = %d\n", PE_index, (int)(temp2));
        // printf("[humu]: computePE: PE_index = %d, tempC[%d] = %d\n", PE_index, PE_index, (int)(tempC[PE_index]));
        *out_buff = tempC[PE_index];
    }

    // pass west to east and pass north to south
    // if (PE_index == 11) {
    //     for (int i = 0; i < stream_length; i++) {
    //         output_east[i]  = input_west[i];
    //         output_south[i] = input_north[i];
    //     }
    // }
    // if (PE_index == 21) {
    //     for (int i = 0; i < stream_length; i++) {
    //         output_east[i] = input_west[i];
    //         // output_south[i] = input_north[i];
    //     }
    // }
    // if (PE_index == 12) {
    //     for (int i = 0; i < stream_length; i++) {
    //         // output_east[i]  = input_west[i];
    //         output_south[i] = input_north[i];
    //     }
    // }
    // if (PE_index == 22) {
    //     for (int i = 0; i < stream_length; i++) {
    //         output_east[i]  = input_west[i];
    //         output_south[i] = input_north[i];
    //     }
    // }
}
*/

void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
             const unsigned matrix_C_dim, const unsigned matrix_A_dim, const unsigned matrix_B_dim,
             const unsigned state_control, word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
    // prepare input streams for PEs

    status_compute = 10;

    if (state_control == 2) {
        status_compute = 11;

        word_t input_west_1[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_2[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_3[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_4[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_5[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_6[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_7[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_8[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_west_9[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        
        word_t input_north_1[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_2[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_3[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_4[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_5[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_6[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_7[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_8[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        word_t input_north_9[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        word_t output_east_1[17];
        word_t output_east_2[17];
        word_t output_east_3[17];
        word_t output_east_4[17];
        word_t output_east_5[17];
        word_t output_east_6[17];
        word_t output_east_7[17];
        word_t output_east_8[17];
        word_t output_east_9[17];

        word_t output_south_9[17];

        // input west PE_1
        input_west_1[0]= _inbuff[0];
        input_west_1[1]= _inbuff[1];
        input_west_1[2]= _inbuff[2];
        input_west_1[3]= _inbuff[5];
        input_west_1[4]= _inbuff[6];
        input_west_1[5]= _inbuff[7];
        input_west_1[6]= _inbuff[10];
        input_west_1[7]= _inbuff[11];
        input_west_1[8]= _inbuff[12];

        // input west PE_2
        input_west_2[1]= _inbuff[1];
        input_west_2[2]= _inbuff[2];
        input_west_2[3]= _inbuff[3];
        input_west_2[4]= _inbuff[6];
        input_west_2[5]= _inbuff[7];
        input_west_2[6]= _inbuff[8];
        input_west_2[7]= _inbuff[11];
        input_west_2[8]= _inbuff[12];
        input_west_2[9]= _inbuff[13];

        // input west PE_3
        input_west_3[2]= _inbuff[2];
        input_west_3[3]= _inbuff[3];
        input_west_3[4]= _inbuff[4];
        input_west_3[5]= _inbuff[7];
        input_west_3[6]= _inbuff[8];
        input_west_3[7]= _inbuff[9];
        input_west_3[8]= _inbuff[12];
        input_west_3[9]= _inbuff[13];
        input_west_3[10]= _inbuff[14];

        // input west PE_4
        input_west_4[3]= _inbuff[5];
        input_west_4[4]= _inbuff[6];
        input_west_4[5]= _inbuff[7];
        input_west_4[6]= _inbuff[10];
        input_west_4[7]= _inbuff[11];
        input_west_4[8]= _inbuff[12];
        input_west_4[9]= _inbuff[15];
        input_west_4[10]= _inbuff[16];
        input_west_4[11]= _inbuff[17];

        // input west PE_5
        input_west_5[4]= _inbuff[6];
        input_west_5[5]= _inbuff[7];
        input_west_5[6]= _inbuff[8];
        input_west_5[7]= _inbuff[11];
        input_west_5[8]= _inbuff[12];
        input_west_5[9]= _inbuff[13];
        input_west_5[10]= _inbuff[16];
        input_west_5[11]= _inbuff[17];
        input_west_5[12]= _inbuff[18];

        // input west PE_6
        input_west_6[5]= _inbuff[7];
        input_west_6[6]= _inbuff[8];
        input_west_6[7]= _inbuff[9];
        input_west_6[8]= _inbuff[12];
        input_west_6[9]= _inbuff[13];
        input_west_6[10]= _inbuff[14];
        input_west_6[11]= _inbuff[17];
        input_west_6[12]= _inbuff[18];
        input_west_6[13]= _inbuff[19];

        // input west PE_7
        input_west_7[6]= _inbuff[10];
        input_west_7[7]= _inbuff[11];
        input_west_7[8]= _inbuff[12];
        input_west_7[9]= _inbuff[15];
        input_west_7[10]= _inbuff[16];
        input_west_7[11]= _inbuff[17];
        input_west_7[12]= _inbuff[20];
        input_west_7[13]= _inbuff[21];
        input_west_7[14]= _inbuff[22];

        // input west PE_8
        input_west_8[7]= _inbuff[11];
        input_west_8[8]= _inbuff[12];
        input_west_8[9]= _inbuff[13];
        input_west_8[10]= _inbuff[16];
        input_west_8[11]= _inbuff[17];
        input_west_8[12]= _inbuff[18];
        input_west_8[13]= _inbuff[21];
        input_west_8[14]= _inbuff[22];
        input_west_8[15]= _inbuff[23];

        // input west PE_9
        input_west_9[8]= _inbuff[12];
        input_west_9[9]= _inbuff[13];
        input_west_9[10]= _inbuff[14];
        input_west_9[11]= _inbuff[17];
        input_west_9[12]= _inbuff[18];
        input_west_9[13]= _inbuff[19];
        input_west_9[14]= _inbuff[22];
        input_west_9[15]= _inbuff[23];
        input_west_9[16]= _inbuff[24];        

        // input north PE_1
        input_north_1[8] = _inbuff[25];
        input_north_1[7] = _inbuff[26];
        input_north_1[6] = _inbuff[27];
        input_north_1[5] = _inbuff[28];
        input_north_1[4] = _inbuff[29];
        input_north_1[3] = _inbuff[30];
        input_north_1[2] = _inbuff[31];
        input_north_1[1] = _inbuff[32];
        input_north_1[0] = _inbuff[33];


compute_dataflow:
        for (int a = 0; a < 1; a++) {
#pragma HLS inline off
            compute_PE_2(input_west_1, input_north_1, output_east_1, input_north_2, _outbuff[0], 0, 17);
            compute_PE_2(input_west_2, input_north_2, output_east_2, input_north_3, _outbuff[1], 1, 17);
            compute_PE_2(input_west_3, input_north_3, output_east_3, input_north_4, _outbuff[2], 2, 17);
            compute_PE_2(input_west_4, input_north_4, output_east_4, input_north_5, _outbuff[3], 3, 17);
            compute_PE_2(input_west_5, input_north_5, output_east_5, input_north_6, _outbuff[4], 4, 17);
            compute_PE_2(input_west_6, input_north_6, output_east_6, input_north_7, _outbuff[5], 5, 17);
            compute_PE_2(input_west_7, input_north_7, output_east_7, input_north_8, _outbuff[6], 6, 17);
            compute_PE_2(input_west_8, input_north_8, output_east_8, input_north_9, _outbuff[7], 7, 17);
            compute_PE_2(input_west_9, input_north_9, output_east_9, output_south_9, _outbuff[8], 8, 17);
            // compute_PE_4(input_west_21, input_north_21, input_west_22, output_south_21, _outbuff[2], 2, 4);
            // compute_PE_5(input_west_22, input_north_22, output_east_22, output_south_22, _outbuff[3], 3, 4);
        }
    }

    status_compute = 12;
}

void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
         const unsigned conf_info_matrix_C_dim, const unsigned conf_info_matrix_A_dim,
         const unsigned conf_info_matrix_B_dim, const unsigned conf_info_state_control, dma_info_t &load_ctrl,
         dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
    // const unsigned matrix_C_dim = conf_info_matrix_C_dim;
    // const unsigned matrix_A_dim = conf_info_matrix_A_dim;
    // const unsigned matrix_B_dim = conf_info_matrix_B_dim;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++) {
        // Chunking
go:
        for (int c = 0; c < 1; c++) {

            const unsigned matrix_C_dim  = conf_info_matrix_C_dim;
            const unsigned matrix_A_dim  = conf_info_matrix_A_dim;
            const unsigned matrix_B_dim  = conf_info_matrix_B_dim;
            const unsigned state_control = conf_info_state_control;

            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];
            // printf("[top]: SIZE_IN_CHUNK_DATA = %d\n", SIZE_IN_CHUNK_DATA);
            // printf("[top]: SIZE_OUT_CHUNK_DATA = %d\n", SIZE_OUT_CHUNK_DATA);

            // init tempC to zeros
            for (int x = 0; x < 9; x++) {
                tempC[x]    = 0;
                // _outbuff[x] = x + 7;
            }

            load(_inbuff, in1,
                 /* <<--args-->> */
                 matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, load_ctrl, c, b);
            // input_west_11, input_west_21,input_north_11, input_north_12);

#ifndef __SYNTHESIS__
            printf("Finished load \n");
#endif

            compute(_inbuff,
                    /* <<--args-->> */
                    matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, _outbuff);

#ifndef __SYNTHESIS__
            printf("Finished compute \n");
#endif

            // compute_PE_2(input_west_11, input_north_11, input_west_12, input_north_21, &_outbuff[0], 0, 4);
            // compute_PE_2(input_west_12, input_north_12, output_east_12, input_north_22, &_outbuff[1], 1, 4);
            // compute_PE_2(input_west_21, input_north_21, input_west_22, output_south_21, &_outbuff[2], 2, 4);
            // compute_PE_2(input_west_22, input_north_22, output_east_22, output_south_22, &_outbuff[3], 3, 4);

            store(_outbuff, out,
                  /* <<--args-->> */
                  matrix_C_dim, matrix_A_dim, matrix_B_dim, state_control, store_ctrl, c, b);

#ifndef __SYNTHESIS__
            printf("Finished store \n");
#endif
        }
    }
}
