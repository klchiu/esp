// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __TF_ADD3_FUNCTIONS_HPP__
#define __TF_ADD3_FUNCTIONS_HPP__

#include "tf_add3.hpp"


// Compute the 1D x and y gradients of the input matrix / image

void tf_add3::add(uint32_t length, bool pingpong)
{
    // B0_out = A0_in1 - A0_in2
    //uint32_t i;

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
    for (uint32_t i = 0; i < length; i+=UNROLL_IN) {
#if (PLM_ONLY == 1)
        HLS_PIPELINE_LOOP(SOFT_STALL, 1, "pipeline-add-loop");
#endif

        // uint32_t round_length = round_up(length, UNROLL_IN);
        // round_length round_length = round_length >> (UNROLL_PLM_LOG - 1);

#if (PLM_ONLY == 0)
#if (PLM_USED == 1)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nMove data from PLM to registers");
#endif

        //Move data from PLM to registers
        for(uint32_t d = 0; d < UNROLL_IN; d+=UNROLL_PLM){
            for(uint32_t j = 0; j < UNROLL_PLM; j++){
                HLS_UNROLL_LOOP(AGGRESSIVE, UNROLL_PLM, "unroll-plm-loop");

                uint32_t index_plm = i + j + d;
                uint32_t index_inter = j + d;

#ifndef STRATUS_HLS
                // ESP_REPORT_INFO("\nIndex_plm is: %d", index_plm);
                // ESP_REPORT_INFO("\nIndex_inter is: %d", index_inter);
#endif

                if(pingpong == true){
                    A0_inter_in1[index_inter] = A0_in1_ping[index_plm];
                    A0_inter_in2[index_inter] = A0_in2_ping[index_plm];
                }
                else{
                    A0_inter_in1[index_inter] = A0_in1_pong[index_plm];
                    A0_inter_in2[index_inter] = A0_in2_pong[index_plm];
                }

            }
        }
#endif
#endif

        //Run computation on elements in registers for unroll flexibility
        for(uint16_t k = 0; k < UNROLL_IN; k++){
            // uint32_t index = i * nCols + j;
            HLS_UNROLL_LOOP(AGGRESSIVE, UNROLL_IN, "unroll-add-loop");
            // HLS_BREAK_DEP(A0_in1_ping);
            // HLS_BREAK_DEP(A0_in1_pong);
            // HLS_BREAK_DEP(A0_in2_ping);
            // HLS_BREAK_DEP(A0_in2_pong);
            // HLS_BREAK_DEP(B0_out_ping);
            // HLS_BREAK_DEP(B0_out_pong);

            // if((k + i) >= length){}
            // else{

                FPDATA_WORD tmp1;
                FPDATA_WORD tmp2;
                uint32_t index_comp;// = k + i;

#if (PLM_ONLY == 1)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nCompute in PLM only case");
#endif

                index_comp = k + i;

                if(pingpong == true){
                    tmp1 = A0_in1_ping[index_comp];
                    tmp2 = A0_in2_ping[index_comp];
                }
                else{
                    tmp1 = A0_in1_pong[index_comp];
                    tmp2 = A0_in2_pong[index_comp];
                }
#endif
#if (PLM_ONLY == 0)
#if (PLM_USED == 1)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nCompute in PLM + registers case");
#endif

                index_comp = k;

                tmp1 = A0_inter_in1[index_comp];
                tmp2 = A0_inter_in2[index_comp];
#endif
#if (PLM_USED == 0)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nCompute in registers only case");
#endif

                index_comp = k + i;

                if(pingpong == true){
                    tmp1 = A0_in1_ping[index_comp];
                    tmp2 = A0_in2_ping[index_comp];
                }
                else{
                    tmp1 = A0_in1_pong[index_comp];
                    tmp2 = A0_in2_pong[index_comp];
                }
#endif
#endif

                FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
                // FPDATA in_fp_1;
                // cynw_interpret(tmp1, in_fp_1);
                FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);
                // FPDATA in_fp_2;
                // cynw_interpret(tmp2, in_fp_2);

                FPDATA x = in_fp_1 + in_fp_2; // FPDATA(0.0);
                FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(x);
                // FPDATA_WORD tmp3;
                // cynw_interpret(x, tmp3);

#if (PLM_ONLY == 1)
                if(pingpong == true)
                    B0_out_ping[index_comp] = tmp3;
                else
                    B0_out_pong[index_comp] = tmp3;
#endif

#if (PLM_ONLY == 0)
#if (PLM_USED == 1)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nCompute store output in PLM + registers case");
#endif
                B0_inter_out[index_comp] = tmp3;
#endif
#if (PLM_USED == 0)
                if(pingpong == true)
                    B0_out_ping[index_comp] = tmp3;
                else
                    B0_out_pong[index_comp] = tmp3;
#endif
#endif


#ifndef STRATUS_HLS
                // ESP_REPORT_INFO("\nIndex_comp is: %d", index_comp);
#endif

                // printf("func: %d\t %f\t%f\t%f\n", index, (float)in_fp_1,
                // (float)in_fp_2, (float)x);
            // }
        }

#if (PLM_ONLY == 0)
#if (PLM_USED == 1)

#ifndef STRATUS_HLS
        // ESP_REPORT_INFO("\nMove data from registers to PLM ");
#endif

        //Move data from registers to PLM
        for(uint32_t d = 0; d < UNROLL_IN; d+=UNROLL_PLM){
            for(uint32_t j = 0; j < UNROLL_PLM; j++){
                HLS_UNROLL_LOOP(AGGRESSIVE, UNROLL_PLM, "unroll-plm-out-loop");

                uint32_t index_plm = i + j + d;
                uint32_t index_inter = j + d;

#ifndef STRATUS_HLS
                // ESP_REPORT_INFO("\nOutput Index_plm is: %d", index_plm);
                // ESP_REPORT_INFO("\nOutput Index_inter is: %d", index_inter);
#endif

                if(pingpong == true){
                    B0_out_ping[index_plm] = B0_inter_out[index_inter];
                }
                else{
                    B0_out_pong[index_plm] = B0_inter_out[index_inter];
                }

            }
        }
#endif
#endif

    }
}

#endif // __TF_ADD3_FUNCTIONS_HPP__
