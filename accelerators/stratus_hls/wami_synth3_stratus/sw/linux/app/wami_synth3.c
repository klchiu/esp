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
#include "wami_synth3_stratus.h"

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

#define KERNELS_NUM_00     4  // test_00
#define KERNELS_NUM_09     8  // test_09
#define KERNELS_NUM        16 // test_mtsm
#define KERNELS_NUM_COMBO1 15
#define KERNELS_NUM_COMBO2 15
#define KERNELS_NUM_COMBO3 15
#define KERNELS_NUM_COMBO4 14
#define FRAMES_NUM         1
#define QUEUE_CAPACITY     1

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
#define DEBAYER_EXEC_TIME          10119501 / 1000.0
#define CHANGE_DETECTION_EXEC_TIME 100

unsigned long long time_debayer;

FILE *log_file;

static kthread_t ktreads[KERNELS_NUM]; // just use the max of all KERNELS_NUM here
pthread_mutex_t  printf_mutex;
//--------------------------------------------------------------------------------------

#include "cfg_independent.h"
// #include "cfg_independent_batch.h"
#include "cfg_p2p.h"
#include "cfg_multi_threads.h"

#include "monitors.h"

#define DEVNAME "/dev/wami_synth3_stratus.0"
#define NAME    "wami_synth3_stratus"

#define DBGMSG(MSG)                                         \
    {                                                       \
        printf("%s, line %d: %s", __FILE__, __LINE__, MSG); \
    }

#define CLIP_INRANGE(LOW, VAL, HIGH) ((VAL) < (LOW) ? (LOW) : ((VAL) > (HIGH) ? (HIGH) : (VAL)))

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
        printf("wami_synth3: error: %d\n", error_count);
    } else {
        printf("wami_synth3: Correct!!!\n");
    }
    printf("------- validate_buf, finish\n");
}

//--------------------------------------------------------------------------------------
int test_00()
{
    //
    // This is an example test for multi-thread shared memory
    //
    // A -> B -> C -> D
    //

    printf("-- test_00 start\n");

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", FRAMES_NUM);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_00);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));
    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 1; /* A */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].fifo.capacity    = FRAMES_NUM;
    ktreads[3].fifo.consumers   = 0; /* SINK! */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_00, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_00, FRAMES_NUM);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_00; i++) {
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
    //  for (unsigned i = 0; i < KERNELS_NUM_00; i++)
    //  { init_feedback(&ktreads[i]); }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_00);
  return 0;
#endif

    dump_dgraph(ktreads, KERNELS_NUM_00, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_00; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_00)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_00; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_00);

    printf("-- test_00 done\n");
    return 0;
}

int test_09()
{
    //
    // This is an example test with feedback loop for multi-thread shared memory
    //
    //      v---------,    v---------,
    // A -> B -> C -> D -> E -> F -> G -> H
    //
    //

    printf("-- test_09 start\n");

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", FRAMES_NUM);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_09);

    srand(0);

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));
    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 2; /* A, D */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0], &ktreads[3]};
    ktreads[1].feedback         = (unsigned[]){0, 1};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 2; /* E, B */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* F */
    ktreads[4].incoming         = 2; /* D, G */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[6]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* G */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    ktreads[6].id               = "G";
    ktreads[6].execution_time   = G_EXEC_TIME;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 2; /* H, E */
    ktreads[6].incoming         = 1; /* F */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[5]};
    ktreads[6].feedback         = (unsigned[]){0};

    ktreads[7].id               = "H";
    ktreads[7].execution_time   = H_EXEC_TIME;
    ktreads[7].fifo.capacity    = FRAMES_NUM;
    ktreads[7].fifo.consumers   = 0; /* SINK! */
    ktreads[7].incoming         = 1; /* G */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_09, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_09, FRAMES_NUM);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_09; i++) {
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
    //  for (i = 0; i < KERNELS_NUM_09; i++)
    //  { init_feedback(&ktreads[i]); }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_09);
  return 0;
#endif

    dump_dgraph(ktreads, KERNELS_NUM_09, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_09)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_09);

    printf("-- test_09 done\n");
    return 0;
}

//--------------------------------------------------------------------------------------
void run_synth3_0_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_0_mtm, 1); }
void run_synth3_1_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_1_mtm, 1); }
void run_synth3_2_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_2_mtm, 1); }
void run_synth3_3_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_3_mtm, 1); }
void run_synth3_4_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_4_mtm, 1); }
void run_synth3_5_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_5_mtm, 1); }
void run_synth3_6_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_6_mtm, 1); }
void run_synth3_7_mtm(void *ptr) { esp_run_no_print(cfg_wami_synth3_7_mtm, 1); }

void test_mtm(token_t *buf, int num_col, int num_row, int delay_A, int delay_B, int test_batch)
{
    cfg_wami_synth3_0_mtm[0].hw_buf = buf;
    cfg_wami_synth3_1_mtm[0].hw_buf = buf;
    cfg_wami_synth3_2_mtm[0].hw_buf = buf;
    cfg_wami_synth3_3_mtm[0].hw_buf = buf;
    cfg_wami_synth3_4_mtm[0].hw_buf = buf;
    cfg_wami_synth3_5_mtm[0].hw_buf = buf;
    cfg_wami_synth3_6_mtm[0].hw_buf = buf;
    cfg_wami_synth3_7_mtm[0].hw_buf = buf;

    struct wami_synth3_stratus_access *tmp;
    tmp                    = (struct wami_synth3_stratus_access *)cfg_wami_synth3_0_mtm[0].esp_desc;
    tmp->wami_num_img      = 100;
    tmp->wami_num_col      = num_col;
    tmp->wami_num_row      = num_row;
    tmp->wami_p2p_config_0 = delay_A;
    tmp->wami_p2p_config_1 = delay_B;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */

    srand(0);

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].run_hw           = &run_synth3_0_mtm;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].run_hw           = &run_synth3_1_mtm;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 1; /* A */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0, 1};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].run_hw           = &run_synth3_2_mtm;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].run_hw           = &run_synth3_3_mtm;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* E */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].run_hw           = &run_synth3_4_mtm;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* F */
    ktreads[4].incoming         = 1; /* D */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].run_hw           = &run_synth3_5_mtm;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* G */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    ktreads[6].id               = "G";
    ktreads[6].execution_time   = G_EXEC_TIME;
    ktreads[6].run_hw           = &run_synth3_6_mtm;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* H */
    ktreads[6].incoming         = 1; /* F */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[5]};
    ktreads[6].feedback         = (unsigned[]){0};

    ktreads[7].id               = "H";
    ktreads[7].execution_time   = H_EXEC_TIME;
    ktreads[7].run_hw           = &run_synth3_7_mtm;
    ktreads[7].fifo.capacity    = FRAMES_NUM;
    ktreads[7].fifo.consumers   = 0; /* SINK! */
    ktreads[7].incoming         = 1; /* G */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_09, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_09, 1);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_09; i++) {
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
    for (i = 0; i < KERNELS_NUM_09; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

    dump_dgraph(ktreads, KERNELS_NUM_09, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_09)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_09);

    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);

    // printf("-------------------------------------------------------\n");
    // printf("Finish testing. test_mtm: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_mtm, %d, %llu\n", 1, time_s);
    // printf("-------------------------------------------------------\n");

    printf("[mtm] col: %d\trow: %d\tdelay_A: %d\tdelay_B: %d\ttime: %llu\n", num_col, num_row, delay_A, delay_B,
           time_s);
}

void test_stm(token_t *buf, int num_col, int num_row, int delay_A, int delay_B, int test_batch)
{
    cfg_wami_synth3_0_indep[0].hw_buf = buf;
    cfg_wami_synth3_1_indep[0].hw_buf = buf;
    cfg_wami_synth3_2_indep[0].hw_buf = buf;
    cfg_wami_synth3_3_indep[0].hw_buf = buf;
    cfg_wami_synth3_4_indep[0].hw_buf = buf;
    cfg_wami_synth3_5_indep[0].hw_buf = buf;
    cfg_wami_synth3_6_indep[0].hw_buf = buf;
    cfg_wami_synth3_7_indep[0].hw_buf = buf;

    struct wami_synth3_stratus_access *tmp;
    tmp                    = (struct wami_synth3_stratus_access *)cfg_wami_synth3_0_indep[0].esp_desc;
    // tmp->wami_num_img      = 100;
    tmp->wami_num_col      = num_col;
    tmp->wami_num_row      = num_row;
    tmp->wami_p2p_config_0 = delay_A;
    tmp->wami_p2p_config_1 = delay_B;

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        // esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_1_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_2_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_3_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_4_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_5_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_6_indep, 1);
        // esp_run_no_print(cfg_wami_synth3_7_indep, 1);

        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
        esp_run_no_print(cfg_wami_synth3_0_indep, 1);
    }
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);
    // printf("-------------------------------------------------------\n");
    // printf("Finish testing. test_stm: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_stm, %d, %llu\n", 1, time_s);
    // printf("-------------------------------------------------------\n");

    printf("[stm] col: %d\trow: %d\tdelay_A: %d\tdelay_B: %d\ttime: %llu\n", num_col, num_row, delay_A, delay_B,
           time_s);
}

void test_p2p(token_t *buf, int num_col, int num_row, int delay_A, int delay_B, int test_batch)
{
    cfg_wami_synth3_p2p[0].hw_buf = buf;
    cfg_wami_synth3_p2p[1].hw_buf = buf;
    cfg_wami_synth3_p2p[2].hw_buf = buf;
    cfg_wami_synth3_p2p[3].hw_buf = buf;
    cfg_wami_synth3_p2p[4].hw_buf = buf;
    cfg_wami_synth3_p2p[5].hw_buf = buf;
    cfg_wami_synth3_p2p[6].hw_buf = buf;
    cfg_wami_synth3_p2p[7].hw_buf = buf;

    struct wami_synth3_stratus_access *tmp;
    for (i = 0; i < 8; i++) {
        tmp                    = (struct wami_synth3_stratus_access *)cfg_wami_synth3_p2p[i].esp_desc;
        tmp->wami_num_img      = test_batch;
        tmp->wami_num_col      = num_col;
        tmp->wami_num_row      = num_row;
        tmp->wami_p2p_config_0 = delay_A;
        tmp->wami_p2p_config_1 = delay_B;
    }

    // -- load inputs to the memory
    // reset_buf_to_zero(buf);
    // load_buf(buf);

    gettime(&t_test_1);
    // esp_run(cfg_wami_synth3_p2p, 8);
    esp_run_no_print(cfg_wami_synth3_p2p, 8);
    gettime(&t_test_2);

    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);
    // printf("-------------------------------------------------------\n");
    // printf("Finish testing. test_p2p: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_p2p, %d, %llu\n", 1, time_s);
    // printf("-------------------------------------------------------\n");
    printf("[p2p] time: %llu\n", time_s);
}

int main(int argc, char **argv)
{
    printf("-------------------------------------------------------\n");
    printf("-- WAMI SYNTH3 RUN: Start! \n");
    printf("-------------------------------------------------------\n");

    int x, y;

    log_file = fopen("log.txt", "w");

    // test_00();
    // test_09();

    token_t *buf;
    buf = (token_t *)esp_alloc(5000000); // MEM_ONE_IMAGE_SIZE

    if (argc > 1) {
        if (argc == 3) { // unit tests

            printf("unit test: argc = %d, argv = %s, validatema = %s\n", argc, argv[1], argv[2]);
        }
        goto CLEANUP;
    }

    printf("-------------------------------------------------------\n");

    //--  Each test includes reset_buf, load_buf, compute, and validate_buf
    test_stm(buf, 128, 128, 128, 128, 1000);
    // test_stm(buf, 128, 128, 128, 64);
    // test_stm(buf, 128, 128, 128, 32);
    // test_stm(buf, 128, 128, 128, 16);
    // test_stm(buf, 128, 128, 128, 8);
    // test_stm(buf, 128, 128, 128, 4);
    // test_stm(buf, 64, 64, 128, 128);
    // test_stm(buf, 32, 32, 128, 128);
    // test_stm(buf, 16, 16, 128, 128);
    // test_stm(buf, 8, 8, 128, 128);
    // test_stm(buf, 4, 4, 128, 128);

    test_p2p(buf, 128, 128, 128, 128, 1000);

    test_mtm(buf, 128, 128, 128, 128, 1000);

CLEANUP:
    esp_free(buf);

    fclose(log_file);

    printf("-------------------------------------------------------\n");
    printf("-- WAMI RUN: FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
