// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "gemmBig.hpp"

// Optional application-specific helper functions

//
// Utility functions
//

inline void gemmBig::calculate_config(uint24_t ninputs,
				   uint24_t matrix_d1,
				   uint24_t matrix_d2,
				   uint24_t matrix_d3,
				   bool transpose,
				   uint32_t& size_matrix1,
				   uint32_t& size_matrix2,
				   uint32_t& size_matrix_out,
				   uint24_t& matrix_chk_in,
				   uint16_t& matrix_rem_in1,
				   uint16_t& matrix_rem_in2,
				   uint24_t& matrix_chk_out,
				   uint16_t& matrix_rem_out,
				   uint8_t& load_cfg,
				   uint16_t& loadable_rows,
				   uint16_t& loadable_chunk,
				   uint16_t& index_d1_incr,
				   uint16_t& m2_loop_iters,
				   uint16_t& m2_plm_incr)
{
    size_matrix1 = matrix_d1 * matrix_d2;
    size_matrix2 = matrix_d2 * matrix_d3;
    size_matrix_out = matrix_d1 * matrix_d3 * ninputs;

    m2_loop_iters = 1;
    m2_plm_incr = 1;

    bool d3_odd = matrix_d3 % 2;
    bool is_less_than_matrix2 = (size_matrix2 > DMA_CHUNK || !transpose);

    if ((matrix_d2 > DMA_CHUNK) || (is_less_than_matrix2 && d3_odd)) {
	load_cfg = LESS_THAN_ROW;
	loadable_rows = 1;
	loadable_chunk = DMA_CHUNK;
	calculate_chunks(matrix_chk_in, matrix_rem_in1, matrix_d2, 0);
	matrix_rem_in2 = matrix_rem_in1;
	index_d1_incr = matrix_d2;
    } else if (is_less_than_matrix2) {
	load_cfg = LESS_THAN_MATRIX2;
	if (size_matrix2 > DMA_CHUNK) {
	    loadable_rows = DMA_CHUNK / matrix_d2;
	    if (loadable_rows != 1)
		loadable_rows = (loadable_rows >> 1) << 1;
	} else {
	    loadable_rows = matrix_d3;
	}
	loadable_chunk = loadable_rows * matrix_d2;
	matrix_chk_in = 1;
	matrix_rem_in1 = size_matrix1 % loadable_chunk;
	matrix_rem_in2 = size_matrix2 % loadable_chunk;
	index_d1_incr = loadable_chunk;
	if (!transpose) {
	    m2_loop_iters = matrix_d2;
	    m2_plm_incr = matrix_d2;
	}
    } else {
	load_cfg = MORE_THAN_MATRIX2;
	loadable_rows = matrix_d3;
	loadable_chunk = size_matrix2;
	matrix_chk_in = 1;
	matrix_rem_in1 = size_matrix1 % loadable_chunk;
	matrix_rem_in2 = size_matrix2;
	index_d1_incr = loadable_chunk;
    }

    calculate_chunks(matrix_chk_out, matrix_rem_out, size_matrix_out, 1);
}

inline void gemmBig::calculate_chunks(uint24_t  &matrix_chk,
				   uint16_t &matrix_rem,
				   uint32_t matrix_d2,
				   bool in_or_out)
{
     uint32_t matrix_mul;
     {
        HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "calc-chunks");

	if (!in_or_out) {
	    // calculating the number of chunks (ceil)
	    matrix_chk = matrix_d2 >> DMA_CHUNK_LOG;
	    // calculating the number of cols (covered the by the chunks)
	    matrix_mul = matrix_chk << DMA_CHUNK_LOG;
	} else {
	    // calculating the number of chunks (ceil)
	    matrix_chk = matrix_d2 >> OUT_DMA_CHUNK_LOG;
	    // calculating the number of cols (covered the by the chunks)
	    matrix_mul = matrix_chk << OUT_DMA_CHUNK_LOG;
	}

        // calculating the remaining cols (size of the last chunk)
        matrix_rem = matrix_d2 - matrix_mul;

        // adding the last chunk if it is necessary
        if (matrix_rem != 0) { ++matrix_chk; }
    }
}

inline void gemmBig::sync_compute_store(uint16_t &count, uint16_t loaded_rows,
				     uint8_t load_cfg, uint16_t loadable_rows,
				     bool &pingpong)
{
    count++;
    if (load_cfg == LESS_THAN_MATRIX2 && loadable_rows != 1) {
	if (count == loaded_rows) {
            count = 0;
	    // ESP_REPORT_INFO("COMPUTE2: before store hs %u", (unsigned) count);
            // Call the store_output process
            compute_store_handshake();
	    // ESP_REPORT_INFO("COMPUTE2: after store hs %u", (unsigned) count);
	    pingpong = !pingpong;
	}
    } else {
        if (count == OUT_DMA_CHUNK) {
            count = 0;
	    // ESP_REPORT_INFO("COMPUTE: before store hs");
            // Call the store_output process
            compute_store_handshake();
	    // ESP_REPORT_INFO("COMPUTE: after store hs");
	    pingpong = !pingpong;
        }
    }
}

inline void gemmBig::load_compute_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("load-compute-cfg-handshake");

    load_compute_cfg_done.req.req();
}

inline void gemmBig::compute_load_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("compute-load-cfg-handshake");

    load_compute_cfg_done.ack.ack();
}

inline void gemmBig::load_store_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("load-store-cfg-handshake");

    load_store_cfg_done.req.req();
}

inline void gemmBig::store_load_cfg_handshake()
{
    HLS_DEFINE_PROTOCOL("store-load-cfg-handshake");

    load_store_cfg_done.ack.ack();
}

void gemmBig::compute_32_helper(uint16_t length, uint16_t plm_i_row, uint16_t plm_i_col, bool pingpong_m1, bool pingpong_m2)
{

    for (uint16_t k = 0; k < (length + PARALLELISM - 1) / PARALLELISM; ++k) {
        // HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "constrain-mac");
#ifdef FIXED_POINT

#if (PARALLELISM >= 32)
        HLS_PIPELINE_LOOP(HARD_STALL, 4, "pipeline-mac-fixed");
#else
        HLS_PIPELINE_LOOP(HARD_STALL, 1, "pipeline-mac-fixed");
#endif

#else
        HLS_PIPELINE_LOOP(HARD_STALL, 2, "pipeline-mac-float");

#endif
        HLS_BREAK_ARRAY_DEPENDENCY(input0);
        HLS_BREAK_ARRAY_DEPENDENCY(input1);
        HLS_BREAK_ARRAY_DEPENDENCY(input2);
        HLS_BREAK_ARRAY_DEPENDENCY(input3);

        if (pingpong_m1) {
            row32[0] = INT2FP32(input0[plm_i_row++]);
            row32[1] = INT2FP32(input0[plm_i_row++]);
#if (PARALLELISM >= 4)
            row32[2] = INT2FP32(input0[plm_i_row++]);
            row32[3] = INT2FP32(input0[plm_i_row++]);
#endif
#if (PARALLELISM >= 8)
            row32[4] = INT2FP32(input0[plm_i_row++]);
            row32[5] = INT2FP32(input0[plm_i_row++]);
            row32[6] = INT2FP32(input0[plm_i_row++]);
            row32[7] = INT2FP32(input0[plm_i_row++]);
#endif
#if (PARALLELISM >= 16)
            row32[8]  = INT2FP32(input0[plm_i_row++]);
            row32[9]  = INT2FP32(input0[plm_i_row++]);
            row32[10] = INT2FP32(input0[plm_i_row++]);
            row32[11] = INT2FP32(input0[plm_i_row++]);
            row32[12] = INT2FP32(input0[plm_i_row++]);
            row32[13] = INT2FP32(input0[plm_i_row++]);
            row32[14] = INT2FP32(input0[plm_i_row++]);
            row32[15] = INT2FP32(input0[plm_i_row++]);
#endif
#if (PARALLELISM >= 32)
            row32[16] = INT2FP32(input0[plm_i_row++]);
            row32[17] = INT2FP32(input0[plm_i_row++]);
            row32[18] = INT2FP32(input0[plm_i_row++]);
            row32[19] = INT2FP32(input0[plm_i_row++]);
            row32[20] = INT2FP32(input0[plm_i_row++]);
            row32[21] = INT2FP32(input0[plm_i_row++]);
            row32[22] = INT2FP32(input0[plm_i_row++]);
            row32[23] = INT2FP32(input0[plm_i_row++]);
            row32[24] = INT2FP32(input0[plm_i_row++]);
            row32[25] = INT2FP32(input0[plm_i_row++]);
            row32[26] = INT2FP32(input0[plm_i_row++]);
            row32[27] = INT2FP32(input0[plm_i_row++]);
            row32[28] = INT2FP32(input0[plm_i_row++]);
            row32[29] = INT2FP32(input0[plm_i_row++]);
            row32[30] = INT2FP32(input0[plm_i_row++]);
            row32[31] = INT2FP32(input0[plm_i_row++]);
#endif
        } else {
            row32[0] = INT2FP32(input1[plm_i_row++]);
            row32[1] = INT2FP32(input1[plm_i_row++]);
#if (PARALLELISM >= 4)
            row32[2] = INT2FP32(input1[plm_i_row++]);
            row32[3] = INT2FP32(input1[plm_i_row++]);
#endif
#if (PARALLELISM >= 8)
            row32[4] = INT2FP32(input1[plm_i_row++]);
            row32[5] = INT2FP32(input1[plm_i_row++]);
            row32[6] = INT2FP32(input1[plm_i_row++]);
            row32[7] = INT2FP32(input1[plm_i_row++]);
#endif
#if (PARALLELISM >= 16)
            row32[8]  = INT2FP32(input1[plm_i_row++]);
            row32[9]  = INT2FP32(input1[plm_i_row++]);
            row32[10] = INT2FP32(input1[plm_i_row++]);
            row32[11] = INT2FP32(input1[plm_i_row++]);
            row32[12] = INT2FP32(input1[plm_i_row++]);
            row32[13] = INT2FP32(input1[plm_i_row++]);
            row32[14] = INT2FP32(input1[plm_i_row++]);
            row32[15] = INT2FP32(input1[plm_i_row++]);
#endif
#if (PARALLELISM >= 32)
            row32[16] = INT2FP32(input1[plm_i_row++]);
            row32[17] = INT2FP32(input1[plm_i_row++]);
            row32[18] = INT2FP32(input1[plm_i_row++]);
            row32[19] = INT2FP32(input1[plm_i_row++]);
            row32[20] = INT2FP32(input1[plm_i_row++]);
            row32[21] = INT2FP32(input1[plm_i_row++]);
            row32[22] = INT2FP32(input1[plm_i_row++]);
            row32[23] = INT2FP32(input1[plm_i_row++]);
            row32[24] = INT2FP32(input1[plm_i_row++]);
            row32[25] = INT2FP32(input1[plm_i_row++]);
            row32[26] = INT2FP32(input1[plm_i_row++]);
            row32[27] = INT2FP32(input1[plm_i_row++]);
            row32[28] = INT2FP32(input1[plm_i_row++]);
            row32[29] = INT2FP32(input1[plm_i_row++]);
            row32[30] = INT2FP32(input1[plm_i_row++]);
            row32[31] = INT2FP32(input1[plm_i_row++]);
#endif
        }
        if (pingpong_m2) {
            col32[0] = INT2FP32(input2[plm_i_col++]);
            col32[1] = INT2FP32(input2[plm_i_col++]);
#if (PARALLELISM >= 4)
            col32[2] = INT2FP32(input2[plm_i_col++]);
            col32[3] = INT2FP32(input2[plm_i_col++]);
#endif
#if (PARALLELISM >= 8)
            col32[4] = INT2FP32(input2[plm_i_col++]);
            col32[5] = INT2FP32(input2[plm_i_col++]);
            col32[6] = INT2FP32(input2[plm_i_col++]);
            col32[7] = INT2FP32(input2[plm_i_col++]);
#endif
#if (PARALLELISM >= 16)
            col32[8]  = INT2FP32(input2[plm_i_col++]);
            col32[9]  = INT2FP32(input2[plm_i_col++]);
            col32[10] = INT2FP32(input2[plm_i_col++]);
            col32[11] = INT2FP32(input2[plm_i_col++]);
            col32[12] = INT2FP32(input2[plm_i_col++]);
            col32[13] = INT2FP32(input2[plm_i_col++]);
            col32[14] = INT2FP32(input2[plm_i_col++]);
            col32[15] = INT2FP32(input2[plm_i_col++]);
#endif
#if (PARALLELISM >= 32)
            col32[16] = INT2FP32(input2[plm_i_col++]);
            col32[17] = INT2FP32(input2[plm_i_col++]);
            col32[18] = INT2FP32(input2[plm_i_col++]);
            col32[19] = INT2FP32(input2[plm_i_col++]);
            col32[20] = INT2FP32(input2[plm_i_col++]);
            col32[21] = INT2FP32(input2[plm_i_col++]);
            col32[22] = INT2FP32(input2[plm_i_col++]);
            col32[23] = INT2FP32(input2[plm_i_col++]);
            col32[24] = INT2FP32(input2[plm_i_col++]);
            col32[25] = INT2FP32(input2[plm_i_col++]);
            col32[26] = INT2FP32(input2[plm_i_col++]);
            col32[27] = INT2FP32(input2[plm_i_col++]);
            col32[28] = INT2FP32(input2[plm_i_col++]);
            col32[29] = INT2FP32(input2[plm_i_col++]);
            col32[30] = INT2FP32(input2[plm_i_col++]);
            col32[31] = INT2FP32(input2[plm_i_col++]);
#endif
        } else {
            col32[0] = INT2FP32(input3[plm_i_col++]);
            col32[1] = INT2FP32(input3[plm_i_col++]);
#if (PARALLELISM >= 4)
            col32[2] = INT2FP32(input3[plm_i_col++]);
            col32[3] = INT2FP32(input3[plm_i_col++]);
#endif
#if (PARALLELISM >= 8)
            col32[4] = INT2FP32(input3[plm_i_col++]);
            col32[5] = INT2FP32(input3[plm_i_col++]);
            col32[6] = INT2FP32(input3[plm_i_col++]);
            col32[7] = INT2FP32(input3[plm_i_col++]);
#endif
#if (PARALLELISM >= 16)
            col32[8]  = INT2FP32(input3[plm_i_col++]);
            col32[9]  = INT2FP32(input3[plm_i_col++]);
            col32[10] = INT2FP32(input3[plm_i_col++]);
            col32[11] = INT2FP32(input3[plm_i_col++]);
            col32[12] = INT2FP32(input3[plm_i_col++]);
            col32[13] = INT2FP32(input3[plm_i_col++]);
            col32[14] = INT2FP32(input3[plm_i_col++]);
            col32[15] = INT2FP32(input3[plm_i_col++]);
#endif
#if (PARALLELISM >= 32)
            col32[16]  = INT2FP32(input3[plm_i_col++]);
            col32[17]  = INT2FP32(input3[plm_i_col++]);
            col32[18] = INT2FP32(input3[plm_i_col++]);
            col32[19] = INT2FP32(input3[plm_i_col++]);
            col32[20] = INT2FP32(input3[plm_i_col++]);
            col32[21] = INT2FP32(input3[plm_i_col++]);
            col32[22] = INT2FP32(input3[plm_i_col++]);
            col32[23] = INT2FP32(input3[plm_i_col++]);
            col32[24]  = INT2FP32(input3[plm_i_col++]);
            col32[25]  = INT2FP32(input3[plm_i_col++]);
            col32[26] = INT2FP32(input3[plm_i_col++]);
            col32[27] = INT2FP32(input3[plm_i_col++]);
            col32[28] = INT2FP32(input3[plm_i_col++]);
            col32[29] = INT2FP32(input3[plm_i_col++]);
            col32[30] = INT2FP32(input3[plm_i_col++]);
            col32[31] = INT2FP32(input3[plm_i_col++]);
#endif
        }

        uint16_t plm_i = k * PARALLELISM + 1;
        mult_out32[0]    = row32[0] * col32[0];
        if (plm_i < length)
            mult_out32[1] = row32[1] * col32[1];
        else
            mult_out32[1] = 0;
#if (PARALLELISM >= 4)
        if (plm_i + 1 < length)
            mult_out32[2] = row32[2] * col32[2];
        else
            mult_out32[2] = 0;
        if (plm_i + 2 < length)
            mult_out32[3] = row32[3] * col32[3];
        else
            mult_out32[3] = 0;
#endif
#if (PARALLELISM >= 8)
        if (plm_i + 3 < length)
            mult_out32[4] = row32[4] * col32[4];
        else
            mult_out32[4] = 0;
        if (plm_i + 4 < length)
            mult_out32[5] = row32[5] * col32[5];
        else
            mult_out32[5] = 0;
        if (plm_i + 5 < length)
            mult_out32[6] = row32[6] * col32[6];
        else
            mult_out32[6] = 0;
        if (plm_i + 6 < length)
            mult_out32[7] = row32[7] * col32[7];
        else
            mult_out32[7] = 0;
#endif
#if (PARALLELISM >= 16)
        if (plm_i + 7 < length)
            mult_out32[8] = row32[8] * col32[8];
        else
            mult_out32[8] = 0;
        if (plm_i + 8 < length)
            mult_out32[9] = row32[9] * col32[9];
        else
            mult_out32[9] = 0;
        if (plm_i + 9 < length)
            mult_out32[10] = row32[10] * col32[10];
        else
            mult_out32[10] = 0;
        if (plm_i + 10 < length)
            mult_out32[11] = row32[11] * col32[11];
        else
            mult_out32[11] = 0;
        if (plm_i + 11 < length)
            mult_out32[12] = row32[12] * col32[12];
        else
            mult_out32[12] = 0;
        if (plm_i + 12 < length)
            mult_out32[13] = row32[13] * col32[13];
        else
            mult_out32[13] = 0;
        if (plm_i + 13 < length)
            mult_out32[14] = row32[14] * col32[14];
        else
            mult_out32[14] = 0;
        if (plm_i + 14 < length)
            mult_out32[15] = row32[15] * col32[15];
        else
            mult_out32[15] = 0;
#endif
#if (PARALLELISM >= 32)
        if (plm_i + 15 < length)
            mult_out32[16] = row32[16] * col32[16];
        else
            mult_out32[16] = 0;
        if (plm_i + 16 < length)
            mult_out32[17] = row32[17] * col32[17];
        else
            mult_out32[17] = 0;
        if (plm_i + 17 < length)
            mult_out32[18] = row32[18] * col32[18];
        else
            mult_out32[18] = 0;
        if (plm_i + 18 < length)
            mult_out32[19] = row32[19] * col32[19];
        else
            mult_out32[19] = 0;
        if (plm_i + 19 < length)
            mult_out32[20] = row32[20] * col32[20];
        else
            mult_out32[20] = 0;
        if (plm_i + 20 < length)
            mult_out32[21] = row32[21] * col32[21];
        else
            mult_out32[21] = 0;
        if (plm_i + 21 < length)
            mult_out32[22] = row32[22] * col32[22];
        else
            mult_out32[22] = 0;
        if (plm_i + 22 < length)
            mult_out32[23] = row32[23] * col32[23];
        else
            mult_out32[23] = 0;
        if (plm_i + 23 < length)
            mult_out32[24] = row32[24] * col32[24];
        else
            mult_out32[24] = 0;
        if (plm_i + 24 < length)
            mult_out32[25] = row32[25] * col32[25];
        else
            mult_out32[25] = 0;
        if (plm_i + 25 < length)
            mult_out32[26] = row32[26] * col32[26];
        else
            mult_out32[26] = 0;
        if (plm_i + 26 < length)
            mult_out32[27] = row32[27] * col32[27];
        else
            mult_out32[27] = 0;
        if (plm_i + 27 < length)
            mult_out32[28] = row32[28] * col32[28];
        else
            mult_out32[28] = 0;
        if (plm_i + 28 < length)
            mult_out32[29] = row32[29] * col32[29];
        else
            mult_out32[29] = 0;
        if (plm_i + 29 < length)
            mult_out32[30] = row32[30] * col32[30];
        else
            mult_out32[30] = 0;
        if (plm_i + 30 < length)
            mult_out32[31] = row32[31] * col32[31];
        else
            mult_out32[31] = 0;
#endif

#if (PARALLELISM == 2)
        accumulator32 += mult_out32[0] + mult_out32[1];
#elif (PARALLELISM == 4)
        FPDATA32 add_tmp0 = mult_out32[0] + mult_out32[1];
        FPDATA32 add_tmp1 = mult_out32[2] + mult_out32[3];
        accumulator32 += add_tmp0 + add_tmp1;
#elif (PARALLELISM == 8)
        FPDATA32 add_tmp0 = mult_out32[0] + mult_out32[1];
        FPDATA32 add_tmp1 = mult_out32[2] + mult_out32[3];
        FPDATA32 add_tmp2 = mult_out32[4] + mult_out32[5];
        FPDATA32 add_tmp3 = mult_out32[6] + mult_out32[7];
        FPDATA32 add_tmp4 = add_tmp0 + add_tmp1;
        FPDATA32 add_tmp5 = add_tmp2 + add_tmp3;
        accumulator32 += add_tmp4 + add_tmp5;
#elif (PARALLELISM == 16)
        FPDATA32 add_tmp0  = mult_out32[0] + mult_out32[1];
        FPDATA32 add_tmp1  = mult_out32[2] + mult_out32[3];
        FPDATA32 add_tmp2  = mult_out32[4] + mult_out32[5];
        FPDATA32 add_tmp3  = mult_out32[6] + mult_out32[7];
        FPDATA32 add_tmp4  = mult_out32[8] + mult_out32[9];
        FPDATA32 add_tmp5  = mult_out32[10] + mult_out32[11];
        FPDATA32 add_tmp6  = mult_out32[12] + mult_out32[13];
        FPDATA32 add_tmp7  = mult_out32[14] + mult_out32[15];
        FPDATA32 add_tmp8  = add_tmp0 + add_tmp1;
        FPDATA32 add_tmp9  = add_tmp2 + add_tmp3;
        FPDATA32 add_tmp10 = add_tmp4 + add_tmp5;
        FPDATA32 add_tmp11 = add_tmp6 + add_tmp7;
        FPDATA32 add_tmp12 = add_tmp8 + add_tmp9;
        FPDATA32 add_tmp13 = add_tmp10 + add_tmp11;
        accumulator32 += add_tmp12 + add_tmp13;
#elif (PARALLELISM == 32)
        FPDATA32 add_tmp0  = mult_out32[0] + mult_out32[1];
        FPDATA32 add_tmp1  = mult_out32[2] + mult_out32[3];
        FPDATA32 add_tmp2  = mult_out32[4] + mult_out32[5];
        FPDATA32 add_tmp3  = mult_out32[6] + mult_out32[7];
        FPDATA32 add_tmp4  = mult_out32[8] + mult_out32[9];
        FPDATA32 add_tmp5  = mult_out32[10] + mult_out32[11];
        FPDATA32 add_tmp6  = mult_out32[12] + mult_out32[13];
        FPDATA32 add_tmp7  = mult_out32[14] + mult_out32[15];
        FPDATA32 add_tmp8  = mult_out32[16] + mult_out32[17];
        FPDATA32 add_tmp9  = mult_out32[18] + mult_out32[19];
        FPDATA32 add_tmp10 = mult_out32[20] + mult_out32[21];
        FPDATA32 add_tmp11 = mult_out32[22] + mult_out32[23];
        FPDATA32 add_tmp12 = mult_out32[24] + mult_out32[25];
        FPDATA32 add_tmp13 = mult_out32[26] + mult_out32[27];
        FPDATA32 add_tmp14  = mult_out32[28] + mult_out32[29];
        FPDATA32 add_tmp15  = mult_out32[30] + mult_out32[31];
        FPDATA32 add_tmp16  = add_tmp0 + add_tmp1;
        FPDATA32 add_tmp17  = add_tmp2 + add_tmp3;
        FPDATA32 add_tmp18  = add_tmp4 + add_tmp5;
        FPDATA32 add_tmp19  = add_tmp6 + add_tmp7;
        FPDATA32 add_tmp20  = add_tmp8 + add_tmp9;
        FPDATA32 add_tmp21  = add_tmp10 + add_tmp11;
        FPDATA32 add_tmp22  = add_tmp12 + add_tmp13;
        FPDATA32 add_tmp23  = add_tmp14 + add_tmp15;
        FPDATA32 add_tmp24 = add_tmp16 + add_tmp17;
        FPDATA32 add_tmp25 = add_tmp18 + add_tmp19;
        FPDATA32 add_tmp26 = add_tmp20 + add_tmp21;
        FPDATA32 add_tmp27 = add_tmp22 + add_tmp23;
        FPDATA32 add_tmp28 = add_tmp24 + add_tmp25;
        FPDATA32 add_tmp29 = add_tmp26 + add_tmp27;
        accumulator32 += add_tmp28 + add_tmp29;
#endif
    }//end loop

}

void gemmBig::compute_8_helper(uint16_t length, uint16_t plm_i_row, uint16_t plm_i_col, bool pingpong_m1, bool pingpong_m2)
{

    for (uint16_t k = 0; k < (length + PARALLELISM_8 - 1) / PARALLELISM_8; ++k) {
        // HLS_CONSTRAIN_LATENCY(0, HLS_ACHIEVABLE, "constrain-mac");
#ifdef FIXED_POINT

#if (PARALLELISM_8 >= 32)
        HLS_PIPELINE_LOOP(HARD_STALL, 4, "pipeline-mac-int8");
#elif (PARALLELISM_8 >= 16)
        HLS_PIPELINE_LOOP(HARD_STALL, 2, "pipeline-mac-int8");
#else
        HLS_PIPELINE_LOOP(HARD_STALL, 1, "pipeline-mac-int8");
#endif

#else
        HLS_PIPELINE_LOOP(HARD_STALL, 2, "pipeline-mac-float");

#endif

        HLS_BREAK_ARRAY_DEPENDENCY(input0);
        HLS_BREAK_ARRAY_DEPENDENCY(input1);
        HLS_BREAK_ARRAY_DEPENDENCY(input2);
        HLS_BREAK_ARRAY_DEPENDENCY(input3);

        if (pingpong_m1) {
            row8[0] = INT2FP8(input0[plm_i_row++]);
            row8[1] = INT2FP8(input0[plm_i_row++]);
#if (PARALLELISM_8 >= 4)
            row8[2] = INT2FP8(input0[plm_i_row++]);
            row8[3] = INT2FP8(input0[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 8)
            row8[4] = INT2FP8(input0[plm_i_row++]);
            row8[5] = INT2FP8(input0[plm_i_row++]);
            row8[6] = INT2FP8(input0[plm_i_row++]);
            row8[7] = INT2FP8(input0[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 16)
            row8[8]  = INT2FP8(input0[plm_i_row++]);
            row8[9]  = INT2FP8(input0[plm_i_row++]);
            row8[10] = INT2FP8(input0[plm_i_row++]);
            row8[11] = INT2FP8(input0[plm_i_row++]);
            row8[12] = INT2FP8(input0[plm_i_row++]);
            row8[13] = INT2FP8(input0[plm_i_row++]);
            row8[14] = INT2FP8(input0[plm_i_row++]);
            row8[15] = INT2FP8(input0[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 32)
            row8[16] = INT2FP8(input0[plm_i_row++]);
            row8[17] = INT2FP8(input0[plm_i_row++]);
            row8[18] = INT2FP8(input0[plm_i_row++]);
            row8[19] = INT2FP8(input0[plm_i_row++]);
            row8[20] = INT2FP8(input0[plm_i_row++]);
            row8[21] = INT2FP8(input0[plm_i_row++]);
            row8[22] = INT2FP8(input0[plm_i_row++]);
            row8[23] = INT2FP8(input0[plm_i_row++]);
            row8[24] = INT2FP8(input0[plm_i_row++]);
            row8[25] = INT2FP8(input0[plm_i_row++]);
            row8[26] = INT2FP8(input0[plm_i_row++]);
            row8[27] = INT2FP8(input0[plm_i_row++]);
            row8[28] = INT2FP8(input0[plm_i_row++]);
            row8[29] = INT2FP8(input0[plm_i_row++]);
            row8[30] = INT2FP8(input0[plm_i_row++]);
            row8[31] = INT2FP8(input0[plm_i_row++]);
#endif
        } else {
            row8[0] = INT2FP8(input1[plm_i_row++]);
            row8[1] = INT2FP8(input1[plm_i_row++]);
#if (PARALLELISM_8 >= 4)
            row8[2] = INT2FP8(input1[plm_i_row++]);
            row8[3] = INT2FP8(input1[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 8)
            row8[4] = INT2FP8(input1[plm_i_row++]);
            row8[5] = INT2FP8(input1[plm_i_row++]);
            row8[6] = INT2FP8(input1[plm_i_row++]);
            row8[7] = INT2FP8(input1[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 16)
            row8[8]  = INT2FP8(input1[plm_i_row++]);
            row8[9]  = INT2FP8(input1[plm_i_row++]);
            row8[10] = INT2FP8(input1[plm_i_row++]);
            row8[11] = INT2FP8(input1[plm_i_row++]);
            row8[12] = INT2FP8(input1[plm_i_row++]);
            row8[13] = INT2FP8(input1[plm_i_row++]);
            row8[14] = INT2FP8(input1[plm_i_row++]);
            row8[15] = INT2FP8(input1[plm_i_row++]);
#endif
#if (PARALLELISM_8 >= 32)
            row8[16] = INT2FP8(input1[plm_i_row++]);
            row8[17] = INT2FP8(input1[plm_i_row++]);
            row8[18] = INT2FP8(input1[plm_i_row++]);
            row8[19] = INT2FP8(input1[plm_i_row++]);
            row8[20] = INT2FP8(input1[plm_i_row++]);
            row8[21] = INT2FP8(input1[plm_i_row++]);
            row8[22] = INT2FP8(input1[plm_i_row++]);
            row8[23] = INT2FP8(input1[plm_i_row++]);
            row8[24] = INT2FP8(input1[plm_i_row++]);
            row8[25] = INT2FP8(input1[plm_i_row++]);
            row8[26] = INT2FP8(input1[plm_i_row++]);
            row8[27] = INT2FP8(input1[plm_i_row++]);
            row8[28] = INT2FP8(input1[plm_i_row++]);
            row8[29] = INT2FP8(input1[plm_i_row++]);
            row8[30] = INT2FP8(input1[plm_i_row++]);
            row8[31] = INT2FP8(input1[plm_i_row++]);
#endif
        }
        if (pingpong_m2) {
            col8[0] = INT2FP8(input2[plm_i_col++]);
            col8[1] = INT2FP8(input2[plm_i_col++]);
#if (PARALLELISM_8 >= 4)
            col8[2] = INT2FP8(input2[plm_i_col++]);
            col8[3] = INT2FP8(input2[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 8)
            col8[4] = INT2FP8(input2[plm_i_col++]);
            col8[5] = INT2FP8(input2[plm_i_col++]);
            col8[6] = INT2FP8(input2[plm_i_col++]);
            col8[7] = INT2FP8(input2[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 16)
            col8[8]  = INT2FP8(input2[plm_i_col++]);
            col8[9]  = INT2FP8(input2[plm_i_col++]);
            col8[10] = INT2FP8(input2[plm_i_col++]);
            col8[11] = INT2FP8(input2[plm_i_col++]);
            col8[12] = INT2FP8(input2[plm_i_col++]);
            col8[13] = INT2FP8(input2[plm_i_col++]);
            col8[14] = INT2FP8(input2[plm_i_col++]);
            col8[15] = INT2FP8(input2[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 32)
            col8[16] = INT2FP8(input2[plm_i_col++]);
            col8[17] = INT2FP8(input2[plm_i_col++]);
            col8[18] = INT2FP8(input2[plm_i_col++]);
            col8[19] = INT2FP8(input2[plm_i_col++]);
            col8[20] = INT2FP8(input2[plm_i_col++]);
            col8[21] = INT2FP8(input2[plm_i_col++]);
            col8[22] = INT2FP8(input2[plm_i_col++]);
            col8[23] = INT2FP8(input2[plm_i_col++]);
            col8[24] = INT2FP8(input2[plm_i_col++]);
            col8[25] = INT2FP8(input2[plm_i_col++]);
            col8[26] = INT2FP8(input2[plm_i_col++]);
            col8[27] = INT2FP8(input2[plm_i_col++]);
            col8[28] = INT2FP8(input2[plm_i_col++]);
            col8[29] = INT2FP8(input2[plm_i_col++]);
            col8[30] = INT2FP8(input2[plm_i_col++]);
            col8[31] = INT2FP8(input2[plm_i_col++]);
#endif
        } else {
            col8[0] = INT2FP8(input3[plm_i_col++]);
            col8[1] = INT2FP8(input3[plm_i_col++]);
#if (PARALLELISM_8 >= 4)
            col8[2] = INT2FP8(input3[plm_i_col++]);
            col8[3] = INT2FP8(input3[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 8)
            col8[4] = INT2FP8(input3[plm_i_col++]);
            col8[5] = INT2FP8(input3[plm_i_col++]);
            col8[6] = INT2FP8(input3[plm_i_col++]);
            col8[7] = INT2FP8(input3[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 16)
            col8[8]  = INT2FP8(input3[plm_i_col++]);
            col8[9]  = INT2FP8(input3[plm_i_col++]);
            col8[10] = INT2FP8(input3[plm_i_col++]);
            col8[11] = INT2FP8(input3[plm_i_col++]);
            col8[12] = INT2FP8(input3[plm_i_col++]);
            col8[13] = INT2FP8(input3[plm_i_col++]);
            col8[14] = INT2FP8(input3[plm_i_col++]);
            col8[15] = INT2FP8(input3[plm_i_col++]);
#endif
#if (PARALLELISM_8 >= 32)
            col8[16]  = INT2FP8(input3[plm_i_col++]);
            col8[17]  = INT2FP8(input3[plm_i_col++]);
            col8[18] = INT2FP8(input3[plm_i_col++]);
            col8[19] = INT2FP8(input3[plm_i_col++]);
            col8[20] = INT2FP8(input3[plm_i_col++]);
            col8[21] = INT2FP8(input3[plm_i_col++]);
            col8[22] = INT2FP8(input3[plm_i_col++]);
            col8[23] = INT2FP8(input3[plm_i_col++]);
            col8[24]  = INT2FP8(input3[plm_i_col++]);
            col8[25]  = INT2FP8(input3[plm_i_col++]);
            col8[26] = INT2FP8(input3[plm_i_col++]);
            col8[27] = INT2FP8(input3[plm_i_col++]);
            col8[28] = INT2FP8(input3[plm_i_col++]);
            col8[29] = INT2FP8(input3[plm_i_col++]);
            col8[30] = INT2FP8(input3[plm_i_col++]);
            col8[31] = INT2FP8(input3[plm_i_col++]);
#endif
        }

        uint16_t plm_i = k * PARALLELISM_8 + 1;
        mult_out8[0]    = row8[0] * col8[0];
        if (plm_i < length)
            mult_out8[1] = row8[1] * col8[1];
        else
            mult_out8[1] = 0;
#if (PARALLELISM_8 >= 4)
        if (plm_i + 1 < length)
            mult_out8[2] = row8[2] * col8[2];
        else
            mult_out8[2] = 0;
        if (plm_i + 2 < length)
            mult_out8[3] = row8[3] * col8[3];
        else
            mult_out8[3] = 0;
#endif
#if (PARALLELISM_8 >= 8)
        if (plm_i + 3 < length)
            mult_out8[4] = row8[4] * col8[4];
        else
            mult_out8[4] = 0;
        if (plm_i + 4 < length)
            mult_out8[5] = row8[5] * col8[5];
        else
            mult_out8[5] = 0;
        if (plm_i + 5 < length)
            mult_out8[6] = row8[6] * col8[6];
        else
            mult_out8[6] = 0;
        if (plm_i + 6 < length)
            mult_out8[7] = row8[7] * col8[7];
        else
            mult_out8[7] = 0;
#endif
#if (PARALLELISM_8 >= 16)
        if (plm_i + 7 < length)
            mult_out8[8] = row8[8] * col8[8];
        else
            mult_out8[8] = 0;
        if (plm_i + 8 < length)
            mult_out8[9] = row8[9] * col8[9];
        else
            mult_out8[9] = 0;
        if (plm_i + 9 < length)
            mult_out8[10] = row8[10] * col8[10];
        else
            mult_out8[10] = 0;
        if (plm_i + 10 < length)
            mult_out8[11] = row8[11] * col8[11];
        else
            mult_out8[11] = 0;
        if (plm_i + 11 < length)
            mult_out8[12] = row8[12] * col8[12];
        else
            mult_out8[12] = 0;
        if (plm_i + 12 < length)
            mult_out8[13] = row8[13] * col8[13];
        else
            mult_out8[13] = 0;
        if (plm_i + 13 < length)
            mult_out8[14] = row8[14] * col8[14];
        else
            mult_out8[14] = 0;
        if (plm_i + 14 < length)
            mult_out8[15] = row8[15] * col8[15];
        else
            mult_out8[15] = 0;
#endif
#if (PARALLELISM_8 >= 32)
        if (plm_i + 15 < length)
            mult_out8[16] = row8[16] * col8[16];
        else
            mult_out8[16] = 0;
        if (plm_i + 16 < length)
            mult_out8[17] = row8[17] * col8[17];
        else
            mult_out8[17] = 0;
        if (plm_i + 17 < length)
            mult_out8[18] = row8[18] * col8[18];
        else
            mult_out8[18] = 0;
        if (plm_i + 18 < length)
            mult_out8[19] = row8[19] * col8[19];
        else
            mult_out8[19] = 0;
        if (plm_i + 19 < length)
            mult_out8[20] = row8[20] * col8[20];
        else
            mult_out8[20] = 0;
        if (plm_i + 20 < length)
            mult_out8[21] = row8[21] * col8[21];
        else
            mult_out8[21] = 0;
        if (plm_i + 21 < length)
            mult_out8[22] = row8[22] * col8[22];
        else
            mult_out8[22] = 0;
        if (plm_i + 22 < length)
            mult_out8[23] = row8[23] * col8[23];
        else
            mult_out8[23] = 0;
        if (plm_i + 23 < length)
            mult_out8[24] = row8[24] * col8[24];
        else
            mult_out8[24] = 0;
        if (plm_i + 24 < length)
            mult_out8[25] = row8[25] * col8[25];
        else
            mult_out8[25] = 0;
        if (plm_i + 25 < length)
            mult_out8[26] = row8[26] * col8[26];
        else
            mult_out8[26] = 0;
        if (plm_i + 26 < length)
            mult_out8[27] = row8[27] * col8[27];
        else
            mult_out8[27] = 0;
        if (plm_i + 27 < length)
            mult_out8[28] = row8[28] * col8[28];
        else
            mult_out8[28] = 0;
        if (plm_i + 28 < length)
            mult_out8[29] = row8[29] * col8[29];
        else
            mult_out8[29] = 0;
        if (plm_i + 29 < length)
            mult_out8[30] = row8[30] * col8[30];
        else
            mult_out8[30] = 0;
        if (plm_i + 30 < length)
            mult_out8[31] = row8[31] * col8[31];
        else
            mult_out8[31] = 0;
#endif

#if (PARALLELISM_8 == 2)
        accumulator8 += mult_out8[0] + mult_out8[1];
#elif (PARALLELISM_8 == 4)
        FPDATA8 add_tmp0 = mult_out8[0] + mult_out8[1];
        FPDATA8 add_tmp1 = mult_out8[2] + mult_out8[3];
        accumulator8 += add_tmp0 + add_tmp1;
#elif (PARALLELISM_8 == 8)
        FPDATA8 add_tmp0 = mult_out8[0] + mult_out8[1];
        FPDATA8 add_tmp1 = mult_out8[2] + mult_out8[3];
        FPDATA8 add_tmp2 = mult_out8[4] + mult_out8[5];
        FPDATA8 add_tmp3 = mult_out8[6] + mult_out8[7];
        FPDATA8 add_tmp4 = add_tmp0 + add_tmp1;
        FPDATA8 add_tmp5 = add_tmp2 + add_tmp3;
        accumulator8 += add_tmp4 + add_tmp5;
#elif (PARALLELISM_8 == 16)
        FPDATA8 add_tmp0  = mult_out8[0] + mult_out8[1];
        FPDATA8 add_tmp1  = mult_out8[2] + mult_out8[3];
        FPDATA8 add_tmp2  = mult_out8[4] + mult_out8[5];
        FPDATA8 add_tmp3  = mult_out8[6] + mult_out8[7];
        FPDATA8 add_tmp4  = mult_out8[8] + mult_out8[9];
        FPDATA8 add_tmp5  = mult_out8[10] + mult_out8[11];
        FPDATA8 add_tmp6  = mult_out8[12] + mult_out8[13];
        FPDATA8 add_tmp7  = mult_out8[14] + mult_out8[15];
        FPDATA8 add_tmp8  = add_tmp0 + add_tmp1;
        FPDATA8 add_tmp9  = add_tmp2 + add_tmp3;
        FPDATA8 add_tmp10 = add_tmp4 + add_tmp5;
        FPDATA8 add_tmp11 = add_tmp6 + add_tmp7;
        FPDATA8 add_tmp12 = add_tmp8 + add_tmp9;
        FPDATA8 add_tmp13 = add_tmp10 + add_tmp11;
        accumulator8 += add_tmp12 + add_tmp13;
#elif (PARALLELISM_8 == 32)
        FPDATA8 add_tmp0  = mult_out8[0] + mult_out8[1];
        FPDATA8 add_tmp1  = mult_out8[2] + mult_out8[3];
        FPDATA8 add_tmp2  = mult_out8[4] + mult_out8[5];
        FPDATA8 add_tmp3  = mult_out8[6] + mult_out8[7];
        FPDATA8 add_tmp4  = mult_out8[8] + mult_out8[9];
        FPDATA8 add_tmp5  = mult_out8[10] + mult_out8[11];
        FPDATA8 add_tmp6  = mult_out8[12] + mult_out8[13];
        FPDATA8 add_tmp7  = mult_out8[14] + mult_out8[15];
        FPDATA8 add_tmp8  = mult_out8[16] + mult_out8[17];
        FPDATA8 add_tmp9  = mult_out8[18] + mult_out8[19];
        FPDATA8 add_tmp10 = mult_out8[20] + mult_out8[21];
        FPDATA8 add_tmp11 = mult_out8[22] + mult_out8[23];
        FPDATA8 add_tmp12 = mult_out8[24] + mult_out8[25];
        FPDATA8 add_tmp13 = mult_out8[26] + mult_out8[27];
        FPDATA8 add_tmp14  = mult_out8[28] + mult_out8[29];
        FPDATA8 add_tmp15  = mult_out8[30] + mult_out8[31];
        FPDATA8 add_tmp16  = add_tmp0 + add_tmp1;
        FPDATA8 add_tmp17  = add_tmp2 + add_tmp3;
        FPDATA8 add_tmp18  = add_tmp4 + add_tmp5;
        FPDATA8 add_tmp19  = add_tmp6 + add_tmp7;
        FPDATA8 add_tmp20  = add_tmp8 + add_tmp9;
        FPDATA8 add_tmp21  = add_tmp10 + add_tmp11;
        FPDATA8 add_tmp22  = add_tmp12 + add_tmp13;
        FPDATA8 add_tmp23  = add_tmp14 + add_tmp15;
        FPDATA8 add_tmp24 = add_tmp16 + add_tmp17;
        FPDATA8 add_tmp25 = add_tmp18 + add_tmp19;
        FPDATA8 add_tmp26 = add_tmp20 + add_tmp21;
        FPDATA8 add_tmp27 = add_tmp22 + add_tmp23;
        FPDATA8 add_tmp28 = add_tmp24 + add_tmp25;
        FPDATA8 add_tmp29 = add_tmp26 + add_tmp27;
        accumulator8 += add_tmp28 + add_tmp29;
#endif
    }//end loop
}
