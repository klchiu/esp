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

#include "../include/fileio.h"

#include "cfg_stm_a.h"
#include "cfg_p2p_a.h"
#include "cfg_stm_b.h"
#include "cfg_p2p_b.h"

#include "monitors.h"

FILE *log_file;
FILE *log_monitor;

void load_buf(token_t *buf)
{
    int x, y;
    for (x = 0; x < 128 * 128; x++) {
        y      = 128 * 128 + x;
        buf[y] = x;
    }
}

void reset_buf_to_zero(token_t *buf)
{
    int x;
    for (x = 0; x < 128 * 128 * 2; x++) {
        buf[x] = 0;
    }
}

void validate_buf(token_t *buf)
{
    int x, y;
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
}

//--------------------------------------------------------------------------------------

unsigned long long test_stm_a(token_t *buf, int num_load, int num_store, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_a[0].hw_buf = buf;
    cfg_ny_acc_1_stm_a[0].hw_buf = buf;
    cfg_ny_acc_2_stm_a[0].hw_buf = buf;
    cfg_ny_acc_3_stm_a[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_a[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_1_stm_a[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_2_stm_a[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_3_stm_a[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;

    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run(cfg_ny_acc_0_stm_a, 1);
        esp_run(cfg_ny_acc_1_stm_a, 1);
        esp_run(cfg_ny_acc_2_stm_a, 1);
        esp_run(cfg_ny_acc_3_stm_a, 1);
    }
    gettime(&t_test_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    return time_s;
}

unsigned long long test_stm_b(token_t *buf, int num_load, int num_store, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_b[0].hw_buf = buf;
    cfg_ny_acc_1_stm_b[0].hw_buf = buf;
    cfg_ny_acc_2_stm_b[0].hw_buf = buf;
    cfg_ny_acc_3_stm_b[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_b[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store * 2;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_1_stm_b[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_2_stm_b[0].esp_desc;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_3_stm_b[0].esp_desc;
    tmp->num_load  = num_load * 2;
    tmp->num_store = num_store;
    tmp->delay     = delay;

    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run(cfg_ny_acc_0_stm_b, 1);
        esp_run(cfg_ny_acc_1_stm_b, 1);
        esp_run(cfg_ny_acc_2_stm_b, 1);
        esp_run(cfg_ny_acc_3_stm_b, 1);
    }
    gettime(&t_test_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    return time_s;
}

unsigned long long test_p2p_a(token_t *buf, int num_load, int num_store, int delay, int test_batch)
{
    cfg_ny_acc_p2p_a[0].hw_buf = buf;
    cfg_ny_acc_p2p_a[1].hw_buf = buf;
    cfg_ny_acc_p2p_a[2].hw_buf = buf;
    cfg_ny_acc_p2p_a[3].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    for (i = 0; i < 4; i++) {
        tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_a[i].esp_desc;
        tmp->num_img   = test_batch;
        tmp->num_load  = num_load;
        tmp->num_store = num_store;
        tmp->delay     = delay;
    }

    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    esp_run(cfg_ny_acc_p2p_a, 4);
    gettime(&t_test_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    return time_s;
}

unsigned long long test_p2p_b(token_t *buf, int num_load, int num_store, int delay, int test_batch)
{
    cfg_ny_acc_p2p_b[0].hw_buf = buf;
    cfg_ny_acc_p2p_b[1].hw_buf = buf;
    cfg_ny_acc_p2p_b[2].hw_buf = buf;
    cfg_ny_acc_p2p_b[3].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[0].esp_desc;
    tmp->num_img   = test_batch;
    tmp->num_load  = num_load;
    tmp->num_store = num_store * 2;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[1].esp_desc;
    tmp->num_img   = test_batch;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[2].esp_desc;
    tmp->num_img   = test_batch;
    tmp->num_load  = num_load;
    tmp->num_store = num_store;
    tmp->delay     = delay;
    tmp            = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[3].esp_desc;
    tmp->num_img   = test_batch;
    tmp->num_load  = num_load * 2;
    tmp->num_store = num_store;
    tmp->delay     = delay;

    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    esp_run(cfg_ny_acc_p2p_b, 4);
    gettime(&t_test_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    return time_s;
}

int main(int argc, char **argv)
{
    printf("-------------------------------------------------------\n");
    printf("-- NY_ACC RUN: Start! \n");
    printf("-------------------------------------------------------\n");

    log_file    = fopen("log.txt", "w");
    log_monitor = fopen("log_monitor.txt", "w");

    unsigned long long time_stm;
    unsigned long long time_p2p;

    token_t *buf;
    buf = (token_t *)esp_alloc(5000000);

    // demonstrate p2p
    time_stm = test_stm_a(buf, 128, 128, 16384, 10);
    time_p2p = test_p2p_a(buf, 128, 128, 16384, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 128, 128, 16384,
            (float)time_stm / time_p2p);

    time_stm = test_stm_a(buf, 128, 128, 1048576, 10);
    time_p2p = test_p2p_a(buf, 128, 128, 1048576, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 128, 128, 1048576,
            (float)time_stm / time_p2p);

    time_stm = test_stm_a(buf, 8192, 8192, 16384, 10);
    time_p2p = test_p2p_a(buf, 8192, 8192, 16384, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 8192, 8192, 16384,
            (float)time_stm / time_p2p);

    printf("------ a batch = 10 done\n");

    time_stm = test_stm_b(buf, 128, 128, 16384, 10);
    time_p2p = test_p2p_b(buf, 128, 128, 16384, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 128, 128, 16384,
            (float)time_stm / time_p2p);

    time_stm = test_stm_b(buf, 128, 128, 1048576, 10);
    time_p2p = test_p2p_b(buf, 128, 128, 1048576, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 128, 128, 1048576,
            (float)time_stm / time_p2p);

    time_stm = test_stm_b(buf, 8192, 8192, 16384, 10);
    time_p2p = test_p2p_b(buf, 8192, 8192, 16384, 10);
    fprintf(log_file, "load: %d, store: %d, compute: %d, p2p/stm speedup: %.2f\n", 8192, 8192, 16384,
            (float)time_stm / time_p2p);

    printf("------ b batch = 10 done\n");

    // demonstrate monitor
    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;

    mon_args.read_mode = ESP_MON_READ_ALL;

    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_ny_acc_0_stm_a, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print(mon_args, vals_diff, log_monitor);


    esp_free(buf);

    fclose(log_file);
    fclose(log_monitor);

    printf("-------------------------------------------------------\n");
    printf("-- FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
