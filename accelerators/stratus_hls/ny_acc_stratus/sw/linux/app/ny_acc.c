// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <my_stringify.h>
#include <test/test.h>
#include <test/time.h>
#include "ny_acc_stratus.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------

#include "wami_C_data.h"
#include "wami_config_tb.h"
#include "wami_utils.h"

#include "../include/fileio.h"


FILE *log_file;

//--------------------------------------------------------------------------------------

#include "cfg_stm_a.h"
#include "cfg_p2p_a.h"
#include "cfg_stm_c.h"
#include "cfg_p2p_c.h"

#include "monitors.h"

void load_buf(token_t *buf)
{
    int x, y;

    printf("------- load_buf, start, size = %d\n", 128);

    for (x = 0; x < 128 * 128; x++) {
        y      = 128 * 128 + x;
        buf[y] = x;
    }
    printf("------- load_buf, finish\n");
}

void reset_buf_to_zero(token_t *buf)
{
    int x;
    printf("------- reset_buf_to_zero, size = %d\n", 128);

    for (x = 0; x < 128 * 128 * 2; x++) {
        // printf("x = %d\n", x);
        buf[x] = 0;
    }
    printf("------- reset_buf_to_zero, finish\n");
}

void validate_buf(token_t *buf)
{
    int x, y;
    printf("------- validate_buf, size = %d\n", 128);

    int error_count = 0;
    for (x = 0; x < 128 * 128; x++) {
        y = 0 + x;
        if (buf[y] != (token_t)(x + 1)) {
            error_count++;
        }
        if (y < 10) {
            printf("buf[%d] = %ld\n", y, buf[y]);
        }
    }
    if (error_count > 0) {
        printf("ny_acc: error: %d\n", error_count);
    } else {
        printf("ny_acc: Correct!!!\n");
    }
    printf("------- validate_buf, finish\n");
}

//--------------------------------------------------------------------------------------


void test_stm_a(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_a[0].hw_buf = buf;
    cfg_ny_acc_1_stm_a[0].hw_buf = buf;
    cfg_ny_acc_2_stm_a[0].hw_buf = buf;
    cfg_ny_acc_3_stm_a[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_a[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run(cfg_ny_acc_0_stm_a, 1);
        esp_run(cfg_ny_acc_1_stm_a, 1);
        esp_run(cfg_ny_acc_2_stm_a, 1);
        esp_run(cfg_ny_acc_3_stm_a, 1);
    }
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[stm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[stm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_stm_c(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_c[0].hw_buf = buf;
    cfg_ny_acc_1_stm_c[0].hw_buf = buf;
    cfg_ny_acc_2_stm_c[0].hw_buf = buf;
    cfg_ny_acc_3_stm_c[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_c[0].esp_desc;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_1_stm_c[0].esp_desc;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_2_stm_c[0].esp_desc;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_3_stm_c[0].esp_desc;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run(cfg_ny_acc_0_stm_c, 1);
        esp_run(cfg_ny_acc_1_stm_c, 1);
        esp_run(cfg_ny_acc_2_stm_c, 1);
        esp_run(cfg_ny_acc_3_stm_c, 1);
    }
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[stm_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[stm_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_p2p_a(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_p2p_a[0].hw_buf = buf;
    cfg_ny_acc_p2p_a[1].hw_buf = buf;
    cfg_ny_acc_p2p_a[2].hw_buf = buf;
    cfg_ny_acc_p2p_a[3].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    for (i = 0; i < 4; i++) {
        tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_a[i].esp_desc;
        tmp->wami_num_img = test_batch;
        tmp->wami_num_col = num_col;
        tmp->wami_num_row = num_row;
        tmp->wami_batch   = delay;
    }

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    esp_run(cfg_ny_acc_p2p_a, 4);
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[p2p_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[p2p_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_p2p_c(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_p2p_c[0].hw_buf = buf;
    cfg_ny_acc_p2p_c[1].hw_buf = buf;
    cfg_ny_acc_p2p_c[2].hw_buf = buf;
    cfg_ny_acc_p2p_c[3].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[1].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[2].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[3].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    esp_run(cfg_ny_acc_p2p_c, 4);
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[p2p_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[p2p_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

int main(int argc, char **argv)
{
    printf("-------------------------------------------------------\n");
    printf("-- NY_ACC RUN: Start! \n");
    printf("-------------------------------------------------------\n");

    log_file = fopen("log.txt", "w");

    token_t *buf;
    buf = (token_t *)esp_alloc(5000000);

   
    test_stm_a(buf, 128, 128, 16384, 10);
    test_p2p_a(buf, 128, 128, 16384, 10);

    test_stm_a(buf, 128, 128, 1048576, 10);
    test_p2p_a(buf, 128, 128, 1048576, 10);

    test_stm_a(buf, 8192, 8192, 16384, 10);
    test_p2p_a(buf, 8192, 8192, 16384, 10);

    printf("------ a batch = 10 done\n");

    test_stm_a(buf, 128, 128, 16384, 100);
    test_p2p_a(buf, 128, 128, 16384, 100);

    test_stm_a(buf, 128, 128, 1048576, 100);
    test_p2p_a(buf, 128, 128, 1048576, 100);

    test_stm_a(buf, 8192, 8192, 16384, 100);
    test_p2p_a(buf, 8192, 8192, 16384, 100);

    printf("------ a batch = 100 done\n");

 

    esp_free(buf);

    fclose(log_file);

    printf("-------------------------------------------------------\n");
    printf("-- FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
