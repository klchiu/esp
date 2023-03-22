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

//--------------------------- below is for multi thread app -------------------
#include "kthreads.h"

#define KERNELS_NUM_MAX   16
#define KERNELS_NUM_MTM_A 6
#define FRAMES_NUM        1
#define QUEUE_CAPACITY    1

#ifndef FRAMES_NUM
    #warning "FRAMES_NUM is not defined! Set FRAMES_NUM to 3."
    #define FRAMES_NUM 3
#endif

#ifndef QUEUE_CAPACITY
    #warning "QUEUE_CAPACITY is not defined! Set QUEUE_CAPACITY to 1"
    #define QUEUE_CAPACITY 1
#endif

#define RAND        rand() % 100000
#define A_EXEC_TIME RAND
#define B_EXEC_TIME RAND
#define C_EXEC_TIME RAND
#define D_EXEC_TIME RAND
#define E_EXEC_TIME RAND
#define F_EXEC_TIME RAND
#define G_EXEC_TIME RAND
#define H_EXEC_TIME RAND

// ACC time (ms = ns/1000)
#define DEBAYER_EXEC_TIME 10119501 / 1000.0

FILE *log_file;

static kthread_t ktreads[KERNELS_NUM_MAX];
pthread_mutex_t  printf_mutex;
//--------------------------------------------------------------------------------------

#include "cfg_stm_a.h"
#include "cfg_mtm_a.h"
#include "cfg_p2p_a.h"
#include "cfg_stm_b.h"
#include "cfg_mtm_b.h"
#include "cfg_p2p_b.h"
#include "cfg_stm_c.h"
#include "cfg_mtm_c.h"
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
            printf("buf[%d] = %lld\n", y, buf[y]);
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
void run_synth3_0_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_0_mtm_a, 1); }
void run_synth3_1_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_1_mtm_a, 1); }
void run_synth3_2_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_2_mtm_a, 1); }
void run_synth3_3_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_3_mtm_a, 1); }
void run_synth3_4_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_4_mtm_a, 1); }
void run_synth3_5_mtm_a(void *ptr) { esp_run_no_print(cfg_ny_acc_5_mtm_a, 1); }

void run_synth3_0_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_0_mtm_b, 1); }
void run_synth3_1_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_1_mtm_b, 1); }
void run_synth3_2_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_2_mtm_b, 1); }
void run_synth3_3_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_3_mtm_b, 1); }
void run_synth3_4_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_4_mtm_b, 1); }
void run_synth3_5_mtm_b(void *ptr) { esp_run_no_print(cfg_ny_acc_5_mtm_b, 1); }

void run_synth3_0_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_0_mtm_c, 1); }
void run_synth3_1_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_1_mtm_c, 1); }
void run_synth3_2_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_2_mtm_c, 1); }
void run_synth3_3_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_3_mtm_c, 1); }
void run_synth3_4_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_4_mtm_c, 1); }
void run_synth3_5_mtm_c(void *ptr) { esp_run_no_print(cfg_ny_acc_5_mtm_c, 1); }

void test_mtm_a(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_mtm_a[0].hw_buf = buf;
    cfg_ny_acc_1_mtm_a[0].hw_buf = buf;
    cfg_ny_acc_2_mtm_a[0].hw_buf = buf;
    cfg_ny_acc_3_mtm_a[0].hw_buf = buf;
    cfg_ny_acc_4_mtm_a[0].hw_buf = buf;
    cfg_ny_acc_5_mtm_a[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_0_mtm_a[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */

    srand(0);

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].run_hw           = &run_synth3_0_mtm_a;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].run_hw           = &run_synth3_1_mtm_a;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 1; /* A */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].run_hw           = &run_synth3_2_mtm_a;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].run_hw           = &run_synth3_3_mtm_a;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* E */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].run_hw           = &run_synth3_4_mtm_a;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* F */
    ktreads[4].incoming         = 1; /* D */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3]};
    ktreads[4].feedback         = (unsigned[]){0};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].run_hw           = &run_synth3_5_mtm_a;
    ktreads[5].fifo.capacity    = FRAMES_NUM;
    ktreads[5].fifo.consumers   = 0; /* SINK! */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_MTM_A, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_MTM_A, 1);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

    dump_dgraph(ktreads, KERNELS_NUM_MTM_A, "graph_a.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_MTM_A)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_MTM_A);

    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[mtm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[mtm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_mtm_b(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_mtm_b[0].hw_buf = buf;
    cfg_ny_acc_1_mtm_b[0].hw_buf = buf;
    cfg_ny_acc_2_mtm_b[0].hw_buf = buf;
    cfg_ny_acc_3_mtm_b[0].hw_buf = buf;
    cfg_ny_acc_4_mtm_b[0].hw_buf = buf;
    cfg_ny_acc_5_mtm_b[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_0_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_1_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_2_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_3_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_4_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_5_mtm_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */

    srand(0);

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].run_hw           = &run_synth3_0_mtm_b;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].run_hw           = &run_synth3_1_mtm_b;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 2; /* A, E */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0], &ktreads[4]};
    ktreads[1].feedback         = (unsigned[]){0, 1};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].run_hw           = &run_synth3_2_mtm_b;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].run_hw           = &run_synth3_3_mtm_b;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* E */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].run_hw           = &run_synth3_4_mtm_b;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 2; /* B, F */
    ktreads[4].incoming         = 1; /* D */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3]};
    ktreads[4].feedback         = (unsigned[]){0};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].run_hw           = &run_synth3_5_mtm_b;
    ktreads[5].fifo.capacity    = FRAMES_NUM;
    ktreads[5].fifo.consumers   = 0; /* SINK! */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_MTM_A, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_MTM_A, 1);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

    dump_dgraph(ktreads, KERNELS_NUM_MTM_A, "graph_b.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_MTM_A)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_MTM_A);

    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[mtm_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[mtm_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_mtm_c(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_mtm_c[0].hw_buf = buf;
    cfg_ny_acc_1_mtm_c[0].hw_buf = buf;
    cfg_ny_acc_2_mtm_c[0].hw_buf = buf;
    cfg_ny_acc_3_mtm_c[0].hw_buf = buf;
    cfg_ny_acc_4_mtm_c[0].hw_buf = buf;
    cfg_ny_acc_5_mtm_c[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_0_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_1_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_2_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_3_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_4_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_5_mtm_c[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */

    srand(0);

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].run_hw           = &run_synth3_0_mtm_c;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].run_hw           = &run_synth3_1_mtm_c;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 2; /* C, D */
    ktreads[1].incoming         = 1; /* A */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].run_hw           = &run_synth3_2_mtm_c;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].run_hw           = &run_synth3_3_mtm_c;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* E */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].run_hw           = &run_synth3_4_mtm_c;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* F */
    ktreads[4].incoming         = 2; /* D */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[3]};
    ktreads[4].feedback         = (unsigned[]){0, 0};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].run_hw           = &run_synth3_5_mtm_c;
    ktreads[5].fifo.capacity    = FRAMES_NUM;
    ktreads[5].fifo.consumers   = 0; /* SINK! */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_MTM_A, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_MTM_A, 1);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }
    for (i = 0; i < KERNELS_NUM_MTM_A; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

    dump_dgraph(ktreads, KERNELS_NUM_MTM_A, "graph_c.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_MTM_A)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_MTM_A; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_MTM_A);

    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[mtm_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[mtm_c] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_stm_a(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_a[0].hw_buf = buf;
    cfg_ny_acc_1_stm_a[0].hw_buf = buf;
    cfg_ny_acc_2_stm_a[0].hw_buf = buf;
    cfg_ny_acc_3_stm_a[0].hw_buf = buf;
    cfg_ny_acc_4_stm_a[0].hw_buf = buf;
    cfg_ny_acc_5_stm_a[0].hw_buf = buf;

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
        esp_run_no_print(cfg_ny_acc_0_stm_a, 1);
        esp_run_no_print(cfg_ny_acc_1_stm_a, 1);
        esp_run_no_print(cfg_ny_acc_2_stm_a, 1);
        esp_run_no_print(cfg_ny_acc_3_stm_a, 1);
        esp_run_no_print(cfg_ny_acc_4_stm_a, 1);
        esp_run_no_print(cfg_ny_acc_5_stm_a, 1);
    }
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[stm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[stm_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_stm_b(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_b[0].hw_buf = buf;
    cfg_ny_acc_1_stm_b[0].hw_buf = buf;
    cfg_ny_acc_2_stm_b[0].hw_buf = buf;
    cfg_ny_acc_3_stm_b[0].hw_buf = buf;
    cfg_ny_acc_4_stm_b[0].hw_buf = buf;
    cfg_ny_acc_5_stm_b[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_1_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_2_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_3_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_4_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_5_stm_b[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_ny_acc_0_stm_b, 1);
        esp_run_no_print(cfg_ny_acc_1_stm_b, 1);
        esp_run_no_print(cfg_ny_acc_2_stm_b, 1);
        esp_run_no_print(cfg_ny_acc_3_stm_b, 1);
        esp_run_no_print(cfg_ny_acc_4_stm_b, 1);
        esp_run_no_print(cfg_ny_acc_5_stm_b, 1);
    }
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[stm_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[stm_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_stm_c(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_0_stm_c[0].hw_buf = buf;
    cfg_ny_acc_1_stm_c[0].hw_buf = buf;
    cfg_ny_acc_2_stm_c[0].hw_buf = buf;
    cfg_ny_acc_3_stm_c[0].hw_buf = buf;
    cfg_ny_acc_4_stm_c[0].hw_buf = buf;
    cfg_ny_acc_5_stm_c[0].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp = (struct ny_acc_stratus_access *)cfg_ny_acc_0_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_1_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row * 2;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_2_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_3_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_4_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_5_stm_c[0].esp_desc;
    // tmp->wami_num_img      = 100;    // test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_ny_acc_0_stm_c, 1);
        esp_run_no_print(cfg_ny_acc_1_stm_c, 1);
        esp_run_no_print(cfg_ny_acc_2_stm_c, 1);
        esp_run_no_print(cfg_ny_acc_3_stm_c, 1);
        esp_run_no_print(cfg_ny_acc_4_stm_c, 1);
        esp_run_no_print(cfg_ny_acc_5_stm_c, 1);
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
    cfg_ny_acc_p2p_a[4].hw_buf = buf;
    cfg_ny_acc_p2p_a[5].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    for (i = 0; i < 6; i++) {
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
    esp_run_no_print(cfg_ny_acc_p2p_a, 6);
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[p2p_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[p2p_a] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_p2p_b(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_p2p_b[0].hw_buf = buf;
    cfg_ny_acc_p2p_b[1].hw_buf = buf;
    cfg_ny_acc_p2p_b[2].hw_buf = buf;
    cfg_ny_acc_p2p_b[3].hw_buf = buf;
    cfg_ny_acc_p2p_b[4].hw_buf = buf;
    cfg_ny_acc_p2p_b[5].hw_buf = buf;

    struct ny_acc_stratus_access *tmp;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[1].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[2].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[3].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[4].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_b[5].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    esp_run_no_print(cfg_ny_acc_p2p_b, 6);
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    printf("[p2p_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row, delay,
           test_batch, time_s);
    fprintf(log_file, "[p2p_b] col_load: %d\trow_store: %d\tdelay: %d\tbatch: %d\ttime: %llu\n", num_col, num_row,
            delay, test_batch, time_s);
}

void test_p2p_c(token_t *buf, int num_col, int num_row, int delay, int test_batch)
{
    cfg_ny_acc_p2p_c[0].hw_buf = buf;
    cfg_ny_acc_p2p_c[1].hw_buf = buf;
    cfg_ny_acc_p2p_c[2].hw_buf = buf;
    cfg_ny_acc_p2p_c[3].hw_buf = buf;
    cfg_ny_acc_p2p_c[4].hw_buf = buf;
    cfg_ny_acc_p2p_c[5].hw_buf = buf;

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
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[4].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col * 2;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;
    tmp               = (struct ny_acc_stratus_access *)cfg_ny_acc_p2p_c[5].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp->wami_num_col = num_col;
    tmp->wami_num_row = num_row;
    tmp->wami_batch   = delay;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    esp_run_no_print(cfg_ny_acc_p2p_c, 6);
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
    buf = (token_t *)esp_alloc(5000000); // MEM_ONE_IMAGE_SIZE

    test_stm_b(buf, 128, 128, 16384, 10);
    test_mtm_b(buf, 128, 128, 16384, 10);
    test_p2p_b(buf, 128, 128, 16384, 10);

    test_stm_b(buf, 128, 128, 1048576, 10);
    test_mtm_b(buf, 128, 128, 1048576, 10);
    test_p2p_b(buf, 128, 128, 1048576, 10);

    test_stm_b(buf, 8192, 8192, 16384, 10);
    test_mtm_b(buf, 8192, 8192, 16384, 10);
    test_p2p_b(buf, 8192, 8192, 16384, 10);

    printf("------ b batch = 10 done\n");

    test_stm_b(buf, 128, 128, 16384, 100);
    test_mtm_b(buf, 128, 128, 16384, 100);
    test_p2p_b(buf, 128, 128, 16384, 100);

    test_stm_b(buf, 128, 128, 1048576, 100);
    test_mtm_b(buf, 128, 128, 1048576, 100);
    test_p2p_b(buf, 128, 128, 1048576, 100);

    test_stm_b(buf, 8192, 8192, 16384, 100);
    test_mtm_b(buf, 8192, 8192, 16384, 100);
    test_p2p_b(buf, 8192, 8192, 16384, 100);

    printf("------ b batch = 100 done\n");

    test_stm_b(buf, 128, 128, 16384, 1000);
    test_mtm_b(buf, 128, 128, 16384, 1000);
    test_p2p_b(buf, 128, 128, 16384, 1000);

    test_stm_b(buf, 128, 128, 1048576, 1000);
    test_mtm_b(buf, 128, 128, 1048576, 1000);
    test_p2p_b(buf, 128, 128, 1048576, 1000);

    test_stm_b(buf, 8192, 8192, 16384, 1000);
    test_mtm_b(buf, 8192, 8192, 16384, 1000);
    test_p2p_b(buf, 8192, 8192, 16384, 1000);

    printf("------ b batch = 1000 done\n");

    test_stm_a(buf, 128, 128, 16384, 10);
    test_mtm_a(buf, 128, 128, 16384, 10);
    test_p2p_a(buf, 128, 128, 16384, 10);

    test_stm_a(buf, 128, 128, 1048576, 10);
    test_mtm_a(buf, 128, 128, 1048576, 10);
    test_p2p_a(buf, 128, 128, 1048576, 10);

    test_stm_a(buf, 8192, 8192, 16384, 10);
    test_mtm_a(buf, 8192, 8192, 16384, 10);
    test_p2p_a(buf, 8192, 8192, 16384, 10);

    printf("------ a batch = 10 done\n");

    test_stm_a(buf, 128, 128, 16384, 100);
    test_mtm_a(buf, 128, 128, 16384, 100);
    test_p2p_a(buf, 128, 128, 16384, 100);

    test_stm_a(buf, 128, 128, 1048576, 100);
    test_mtm_a(buf, 128, 128, 1048576, 100);
    test_p2p_a(buf, 128, 128, 1048576, 100);

    test_stm_a(buf, 8192, 8192, 16384, 100);
    test_mtm_a(buf, 8192, 8192, 16384, 100);
    test_p2p_a(buf, 8192, 8192, 16384, 100);

    printf("------ a batch = 100 done\n");

    test_stm_a(buf, 128, 128, 16384, 1000);
    test_mtm_a(buf, 128, 128, 16384, 1000);
    test_p2p_a(buf, 128, 128, 16384, 1000);

    test_stm_a(buf, 128, 128, 1048576, 1000);
    test_mtm_a(buf, 128, 128, 1048576, 1000);
    test_p2p_a(buf, 128, 128, 1048576, 1000);

    test_stm_a(buf, 8192, 8192, 16384, 1000);
    test_mtm_a(buf, 8192, 8192, 16384, 1000);
    test_p2p_a(buf, 8192, 8192, 16384, 1000);

    printf("------ a batch = 1000 done\n");

CLEANUP:
    esp_free(buf);

    fclose(log_file);

    printf("-------------------------------------------------------\n");
    printf("-- WAMI RUN: FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
