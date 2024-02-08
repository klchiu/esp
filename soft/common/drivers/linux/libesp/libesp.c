/*
 * Copyright (c) 2011-2023 Columbia University, System Level Design Group
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libesp.h"

buf2handle_node *head = NULL;

int lock_a_device2(char *devname_noid, char *puffinname)
{
    FILE * pFile;
    char   acc[16][40];
    char   acc_lock[16][40];
    int8_t i = 0;
    // int8_t dev_id      = -1;
    int8_t acc_num_max = 4; // assume maximum number of accelerator is 4 at the beginning
    int    ret_flock;

    // Check the available resources
    // for (int8_t i = 0; i < 4; i++) {

    while (true) {
        i = i % acc_num_max;

        sprintf(acc[i], "/dev/%s.%d", devname_noid, i);

        if (access(acc[i], F_OK) == 0) { // device exists
            // fprintf(stderr, "[%s]: Found --> %s\n", puffinname, acc[i]);

            sprintf(acc_lock[i], "/lock/%s.%d", devname_noid, i);

            if (access(acc_lock[i], F_OK) != 0) { // lock not exists
                // fprintf(stderr, "[%s]: Lock NOT exists --> %s\n", puffinname, acc_lock[i]);

                pFile = fopen(acc_lock[i], "w");
                if (pFile != NULL) {
                    fputs("This file serves as a lock for accelerator.\n", pFile);
                }
                fclose(pFile);
                // fprintf(stderr, "[%s]: Create lock file --> %s\n", puffinname, acc_lock[i]);
            } else { // lock exists
                // fprintf(stderr, "[%s]: Lock exists --> %s\n", puffinname, acc_lock[i]);
                i++;
                continue;
            }

            // if acc_lock file is unlocked:
            // [humu]: lock the acc_lock file.   flock(acc_lock[i], LOCK_EX);
            // else if acc_lock file is locked:
            // [humu]: check the next available one

            pFile = fopen(acc_lock[i], "r");

            ret_flock = flock(fileno(pFile), LOCK_EX | LOCK_NB);
            if (ret_flock == -1) { // fail to lock it
                if (errno == EWOULDBLOCK) {
                    // fprintf(stderr, "[%s]: lock file was locked, keep finding the next available accelerator\n",
                    // puffinname);
                    i++;
                    continue;
                } else {
                    fprintf(stderr, "[%s]: other lock error --> errno = %d\n", puffinname, errno);
                    return -1;
                }
            } else { // successfully lock it
                flock(fileno(pFile), LOCK_EX);
                // fprintf(stderr, "[%s]: lock file was unlocked, lock it: %s\n", puffinname, acc_lock[i]);
                return i;
            }
            fclose(pFile);
        }
        //  else { // device not exists
        //     fprintf(stderr, "[%s]: No found --> %s\n", puffinname, acc[i]);
        // }

        i++;
    }

    return -1;
}

int unlock_a_device2(char *devname, char *puffinname)
{
    FILE *pFile;
    char  acc_lock[40];
    int   ret_flock = -1;

    sprintf(acc_lock, "/lock/%s", devname);
    pFile = fopen(acc_lock, "r");

    // fprintf(stderr, "[%s]: unlock this: %s\n", puffinname, acc_lock);
    // unlock the acc_lock file
    ret_flock = flock(fileno(pFile), LOCK_UN);
    // fprintf(stderr, "[%s]: unlock_a_device2() ret_flock: %d\n", puffinname, ret_flock);

    fclose(pFile);

    // fprintf(stderr, "[%s]: Let's just remove the lock file: %s\n", puffinname, acc_lock);
    remove(acc_lock);

    return ret_flock;
}



void insert_buf(void *buf, contig_handle_t *handle, enum contig_alloc_policy policy)
{
    buf2handle_node *new = malloc(sizeof(buf2handle_node));
    new->buf             = buf;
    new->handle          = handle;
    new->policy          = policy;

    new->next = head;
    head      = new;
}

contig_handle_t *lookup_handle(void *buf, enum contig_alloc_policy *policy)
{
    buf2handle_node *cur = head;
    while (cur != NULL) {
        if (cur->buf == buf) {
            if (policy != NULL)
                *policy = cur->policy;
            return cur->handle;
        }
        cur = cur->next;
    }
    die("buf not in active allocations\n");
}

void remove_buf(void *buf)
{
    buf2handle_node *cur = head;
    if (cur->buf == buf) {
        head = cur->next;
        contig_free(*(cur->handle));
        free(cur);
        return;
    }

    buf2handle_node *prev;
    while (cur != NULL && cur->buf != buf) {
        prev = cur;
        cur  = cur->next;
    }

    if (cur == NULL)
        die("buf not in active allocations\n");

    prev->next = cur->next;
    contig_free(*(cur->handle));
    free(cur->handle);
    free(cur);
}

bool thread_is_p2p(esp_thread_info_t *thread)
{
    return ((thread->esp_desc)->p2p_store || (thread->esp_desc)->p2p_nsrcs);
}

unsigned DMA_WORD_PER_BEAT(unsigned _st) { return (sizeof(void *) / _st); }

void *accelerator_thread(void *ptr)
{
    esp_thread_info_t *info = (esp_thread_info_t *)ptr;
    struct timespec    th_start;
    struct timespec    th_end;
    int                rc = 0;

    gettime(&th_start);
    // fprintf(stderr, "[humu]: accelerator_thread: before ioctl, devname: %s\n", info->devname);
    rc = ioctl(info->fd, info->ioctl_req, info->esp_desc);
    gettime(&th_end);
    if (rc < 0) {
        perror("ioctl");
    }

    info->hw_ns = ts_subtract(&th_start, &th_end);

    return NULL;
}

void *accelerator_thread_p2p(void *ptr)
{
    struct thread_args *args   = (struct thread_args *)ptr;
    esp_thread_info_t * thread = args->info;
    unsigned            nacc   = args->nacc;
    int                 rc     = 0;
    int                 i;

    pthread_t *threads = malloc(nacc * sizeof(pthread_t));

    for (i = 0; i < nacc; i++) {
        esp_thread_info_t *info = thread + i;
        if (!info->run)
            continue;
        rc = pthread_create(&threads[i], NULL, accelerator_thread, (void *)info);
        if (rc != 0)
            perror("pthread_create");
    }

    for (i = 0; i < nacc; i++) {
        esp_thread_info_t *info = thread + i;
        if (!info->run)
            continue;
        rc = pthread_join(threads[i], NULL);
        if (rc != 0)
            perror("pthread_join");
        close(info->fd);
    }
    free(threads);
    free(ptr);
    return NULL;
}

void *accelerator_thread_serial(void *ptr)
{
    struct thread_args *args   = (struct thread_args *)ptr;
    esp_thread_info_t * thread = args->info;
    unsigned            nacc   = args->nacc;
    int                 i;
    // int                 dev_id = -1;
    int k = 0;
    k++;

    for (i = 0; i < nacc; i++) {

        struct timespec    th_start;
        struct timespec    th_end;
        int                rc   = 0;
        esp_thread_info_t *info = thread + i;

        if (!info->run)
            continue;

        // fprintf(stderr, "[humu]: accelerator_thread_serial: dev_id = %d\n", dev_id);
        // fprintf(stderr, "[humu]: accelerator_thread_serial: strlen(info->devname) = %ld\n", strlen(info->devname));

        gettime(&th_start);
        // fprintf(stderr, "[humu]: accelerator_thread_serial: before ioctl, ------- devname: %s\n", info->devname);
        // fprintf(stderr, "[humu]: fd: %d, ioctl_req: %d\n", info->fd, info->ioctl_req);
        // fprintf(stderr, "    contig->unused  = %d\n", info->esp_desc->contig->unused);
        // fprintf(stderr, "    run             = %d\n", info->esp_desc->run);
        // fprintf(stderr, "    p2p_store       = %d\n", info->esp_desc->p2p_store);
        // fprintf(stderr, "    p2p_nsrcs       = %d\n", info->esp_desc->p2p_nsrcs);
        // fprintf(stderr, "    coherence       = %d\n", info->esp_desc->coherence);
        // fprintf(stderr, "    footprint       = %d\n", info->esp_desc->footprint);
        // fprintf(stderr, "    alloc_policy    = %d\n", info->esp_desc->alloc_policy);
        // fprintf(stderr, "    ddr_node        = %d\n", info->esp_desc->ddr_node);
        // fprintf(stderr, "    in_place        = %d\n", info->esp_desc->in_place);
        // fprintf(stderr, "    reuse_factor    = %d\n", info->esp_desc->reuse_factor);

        rc = ioctl(info->fd, info->ioctl_req, info->esp_desc);
        // sleep(5);
        // for (k = 0 ; k < 200; k++){
        //     fprintf(stderr, "%s\t%d\n", info->devname, k);
        // }
        // fprintf(stderr, "[humu]: accelerator_thread_serial: after ioctl, ------- puffinname: %s\n",
        // info->puffinname);

        gettime(&th_end);
        if (rc < 0) {
            perror("ioctl");
        }

        info->hw_ns = ts_subtract(&th_start, &th_end);
        close(info->fd);
    }
    free(ptr);

    return NULL;
}

void *esp_alloc_policy(struct contig_alloc_params params, size_t size)
{
    contig_handle_t *handle     = malloc(sizeof(contig_handle_t));
    void *           contig_ptr = contig_alloc_policy(params, size, handle);
    insert_buf(contig_ptr, handle, params.policy);
    return contig_ptr;
}

void *esp_alloc(size_t size)
{
    contig_handle_t *handle     = malloc(sizeof(contig_handle_t));
    void *           contig_ptr = contig_alloc(size, handle);
    insert_buf(contig_ptr, handle, CONTIG_ALLOC_PREFERRED);
    return contig_ptr;
}

static void esp_config(esp_thread_info_t *cfg[], unsigned nthreads, unsigned *nacc)
{
    int i, j;
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *info = cfg[i] + j;
            if (!info->run)
                continue;

            enum contig_alloc_policy policy;
            contig_handle_t *        handle = lookup_handle(info->hw_buf, &policy);

            (info->esp_desc)->contig       = contig_to_khandle(*handle);
            (info->esp_desc)->ddr_node     = contig_to_most_allocated(*handle);
            (info->esp_desc)->alloc_policy = policy;
            (info->esp_desc)->run          = true;
        }
    }
}

static void print_time_info(esp_thread_info_t *info[], unsigned long long hw_ns, int nthreads, unsigned *nacc)
{
    int i, j;

    // fprintf(stderr, "[humu]: print_time_info\n");
    fprintf(stderr, "  > Test time: %llu ns\n", hw_ns);
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *cur = info[i] + j;
            if (cur->run)
                fprintf(stderr, "    - %s time: %llu ns\n", cur->devname, cur->hw_ns);
        }
    }
}

void esp_run(esp_thread_info_t cfg[], unsigned nacc)
{
    int i;
    int dev_id = -1;


    dev_id = lock_a_device2(cfg->devname_noid, cfg->puffinname);
    if (dev_id < 0) {
        fprintf(stderr, "Failed to find a device!\n");
        return;
    }

    // dev_id = 1;

    // [humu]: there should be a better way of changing the dev number in devname c string
    char *temp_name;
    if (dev_id < 10) {
        temp_name = malloc(sizeof(char) * (strlen(cfg->devname)));
        sprintf(temp_name, "%s.%d", cfg->devname_noid, dev_id);
    } else {
        temp_name = malloc(sizeof(char) * (strlen(cfg->devname) + 1));
        sprintf(temp_name, "%s.%d", cfg->devname_noid, dev_id);
    }

    // info->devname[strlen(info->devname)-1] = '2';// + 1; // dev_id;
    cfg->devname = temp_name;

    // fprintf(stderr, "[%s]: esp_run: after changing devname, ------- dev_id: %d, devname: %s\n", cfg->puffinname,
    // dev_id,
    //              cfg->devname);

    if (thread_is_p2p(&cfg[0])) {
        esp_thread_info_t *cfg_ptrs[1];
        cfg_ptrs[0] = cfg;

        esp_run_parallel(cfg_ptrs, 1, &nacc);
    } else {
        // fprintf(stderr, "[humu]: esp_run, before malloc 1\n");
        esp_thread_info_t **cfg_ptrs = malloc(sizeof(esp_thread_info_t *) * nacc);
        // fprintf(stderr, "[humu]: esp_run, before malloc 2\n");
        unsigned *nacc_arr = malloc(sizeof(unsigned) * nacc);

        for (i = 0; i < nacc; i++) {
            nacc_arr[i] = 1;
            cfg_ptrs[i] = &cfg[i];
        }
        // fprintf(stderr, "[humu]: esp_run, before esp_run_parallel\n");
        esp_run_parallel(cfg_ptrs, nacc, nacc_arr);
        free(nacc_arr);
        free(cfg_ptrs);
    }


    unlock_a_device2(cfg->devname, cfg->puffinname);
}

unsigned long long esp_run_parallel_no_print(esp_thread_info_t *cfg[], unsigned nthreads, unsigned *nacc)
{
    int                i, j;
    unsigned long long acc_time;
    struct timespec    th_start;
    struct timespec    th_end;
    pthread_t *        thread = malloc(nthreads * sizeof(pthread_t));
    int                rc     = 0;
    esp_config(cfg, nthreads, nacc);
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *info   = cfg[i] + j;
            const char *       prefix = "/dev/";
            char               path[70];

            if (strlen(info->devname) > 64) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                die("Error: device name %s exceeds maximum length of 64 characters\n", info->devname);
            }

            sprintf(path, "%s%s", prefix, info->devname);

            info->fd = open(path, O_RDWR, 0);
            if (info->fd < 0) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                die_errno("fopen failed\n");
            }
        }
    }

    gettime(&th_start);
    for (i = 0; i < nthreads; i++) {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        ;
        args->info = cfg[i];
        args->nacc = nacc[i];

        if (thread_is_p2p(cfg[i])) {
            if (nthreads == 1) // [humu]: no need to create a new thread
                accelerator_thread_p2p((void *)args);
            else
                rc = pthread_create(&thread[i], NULL, accelerator_thread_p2p, (void *)args);
        } else {
            if (nthreads == 1) // [humu]: no need to create a new thread
                accelerator_thread_serial((void *)args);
            else
                rc = pthread_create(&thread[i], NULL, accelerator_thread_serial, (void *)args);
        }
        if (rc != 0) {
            perror("pthread_create");
        }
    }

    for (i = 0; i < nthreads; i++) {
        if (nthreads > 1) // [humu]: no need to join the thread if nthreads == 1
            rc = pthread_join(thread[i], NULL);

        if (rc != 0) {
            perror("pthread_join");
        }
    }

    gettime(&th_end);
    free(thread);

    acc_time = cfg[0]->hw_ns;
    // print_time_info(cfg, ts_subtract(&th_start, &th_end), nthreads, nacc);

    return acc_time;
}

void esp_run_parallel(esp_thread_info_t *cfg[], unsigned nthreads, unsigned *nacc)
{
    int i, j;
    // unsigned long long acc_time;
    struct timespec th_start;
    struct timespec th_end;
    pthread_t *     thread = malloc(nthreads * sizeof(pthread_t));
    int             rc     = 0;
    // int            dev_id = -1;

    // fprintf(stderr, "[humu]: esp_run_parallel()\n");

    esp_config(cfg, nthreads, nacc);
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *info   = cfg[i] + j;
            const char *       prefix = "/dev/";
            char               path[70];

            // fprintf(stderr, "[%s]: esp_run_parallel: after changing devname, ------- devname: %s\n",
            // info->puffinname,
            //         info->devname);

            if (strlen(info->devname) > 64) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                die("Error: device name %s exceeds maximum length of 64 characters\n", info->devname);
            }

            // [humu]: we can try to check the available accelerators here
            sprintf(path, "%s%s", prefix, info->devname);
            // fprintf(stderr, "[%s]: esp_run_parallel: after changing path, ------- path: %s\n", info->puffinname,
            // path);

            info->fd = open(path, O_RDWR, 0);

            // fprintf(stderr, "[%s]: esp_run_parallel: after open path, ------- path: %s\n", info->puffinname, path);

            if (info->fd < 0) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                die_errno("fopen failed\n");
            }
            // fprintf(stderr, "[%s]: esp_run_parallel: after open path 2, ------- path: %s\n", info->puffinname, path);
        }
    }

    gettime(&th_start);
    for (i = 0; i < nthreads; i++) {
        struct thread_args *args = malloc(sizeof(struct thread_args));

        args->info = cfg[i];
        args->nacc = nacc[i];

        if (thread_is_p2p(cfg[i])) {
            if (nthreads == 1) // [humu]: no need to create a new thread
                accelerator_thread_p2p((void *)args);
            else
                rc = pthread_create(&thread[i], NULL, accelerator_thread_p2p, (void *)args);
        } else {
            if (nthreads == 1) // [humu]: no need to create a new thread
            {
                // fprintf(stderr, "[humu]: esp_run_parallel(), nthreads = %d, devname = %s\n", nthreads,
                // args->info->devname);
                accelerator_thread_serial((void *)args);
            } else {
                // fprintf(stderr, "[humu]: esp_run_parallel(), nthreads = %d, devname = %s\n", nthreads,
                // args->info->devname);
                rc = pthread_create(&thread[i], NULL, accelerator_thread_serial, (void *)args);
            }
        }
        if (rc != 0) {
            perror("pthread_create");
        }
    }

    for (i = 0; i < nthreads; i++) {
        if (nthreads > 1) // [humu]: no need to join the thread if nthreads == 1
            rc = pthread_join(thread[i], NULL);

        if (rc != 0) {
            perror("pthread_join");
        }
    }

    gettime(&th_end);
    free(thread);

    // fprintf(stderr, "[humu]: esp_run_parallel, before calling print_time_info\n");
    print_time_info(cfg, ts_subtract(&th_start, &th_end), nthreads, nacc);

    return;
}

void esp_run_1_no_thread(esp_thread_info_t cfg[], unsigned nacc)
{
    if (nacc != 1) {
        perror("nacc != 1");
        return;
    }

    struct timespec th_start;
    struct timespec th_end;

    int rc = 0;
    esp_config(&cfg, 1, &nacc);

    const char *prefix = "/dev/";
    char        path[70];

    if (strlen(cfg->devname) > 64) {
        contig_handle_t *handle = lookup_handle(cfg->hw_buf, NULL);
        contig_free(*handle);
        die("Error: device name %s exceeds maximum length of 64 characters\n", cfg->devname);
    }

    sprintf(path, "%s%s", prefix, cfg->devname);

    cfg->fd = open(path, O_RDWR, 0);
    if (cfg->fd < 0) {
        contig_handle_t *handle = lookup_handle(cfg->hw_buf, NULL);
        contig_free(*handle);
        die_errno("fopen failed\n");
    }

    gettime(&th_start);
    struct thread_args *args = malloc(sizeof(struct thread_args));
    ;
    args->info = cfg;
    args->nacc = nacc;

    accelerator_thread_serial((void *)args);

    if (rc != 0) {
        perror("pthread_create");
    }

    gettime(&th_end);
    print_time_info(&cfg, ts_subtract(&th_start, &th_end), 1, &nacc);
}

unsigned long long esp_run_no_print(esp_thread_info_t cfg[], unsigned nacc)
{
    // [humu]: this api should be the same as esp_run except no print and return the acc time instead

    int                i;
    unsigned long long acc_time;

    if (thread_is_p2p(&cfg[0])) {
        esp_thread_info_t *cfg_ptrs[1];
        cfg_ptrs[0] = cfg;

        acc_time = esp_run_parallel_no_print(cfg_ptrs, 1, &nacc);
    } else {
        esp_thread_info_t **cfg_ptrs = malloc(sizeof(esp_thread_info_t *) * nacc);
        unsigned *          nacc_arr = malloc(sizeof(unsigned) * nacc);

        for (i = 0; i < nacc; i++) {
            nacc_arr[i] = 1;
            cfg_ptrs[i] = &cfg[i];
        }
        acc_time = esp_run_parallel_no_print(cfg_ptrs, nacc, nacc_arr);
        free(nacc_arr);
        free(cfg_ptrs);
    }

    return acc_time;
}

void esp_free(void *buf) { remove_buf(buf); }

void esp_dummy(int dummy) { fprintf(stderr, "esp_dummy: %d\n", dummy); }
