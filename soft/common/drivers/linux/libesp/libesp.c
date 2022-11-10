/*
 * Copyright (c) 2011-2022 Columbia University, System Level Design Group
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libesp.h"
#include <limits.h>

buf2handle_node *head = NULL;

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
    printf("buf not in active allocations\n");
    return NULL;
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
        printf("buf not in active allocations\n");

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
    esp_thread_info_t  *thread = args->info;
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
    esp_thread_info_t  *thread = args->info;
    unsigned            nacc   = args->nacc;
    int                 i;
    for (i = 0; i < nacc; i++) {

        struct timespec    th_start;
        struct timespec    th_end;
        int                rc   = 0;
        esp_thread_info_t *info = thread + i;

        if (!info->run)
            continue;

        gettime(&th_start);
        rc = ioctl(info->fd, info->ioctl_req, info->esp_desc);
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
    void            *contig_ptr = contig_alloc_policy(params, size, handle);
    insert_buf(contig_ptr, handle, params.policy);
    return contig_ptr;
}

void *esp_alloc(size_t size)
{
    contig_handle_t *handle     = malloc(sizeof(contig_handle_t));
    void            *contig_ptr = contig_alloc(size, handle);
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
            contig_handle_t         *handle = lookup_handle(info->hw_buf, &policy);

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

    printf("  > Test time: %llu ns\n", hw_ns);
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *cur = info[i] + j;
            if (cur->run)
                printf("	- %s time: %llu ns\n", cur->devname, cur->hw_ns);
        }
    }
}

void esp_run(esp_thread_info_t cfg[], unsigned nacc)
{
    int i;

    if (thread_is_p2p(&cfg[0])) {
        esp_thread_info_t *cfg_ptrs[1];
        cfg_ptrs[0] = cfg;

        esp_run_parallel(cfg_ptrs, 1, &nacc);
    } else {
        esp_thread_info_t **cfg_ptrs = malloc(sizeof(esp_thread_info_t *) * nacc);
        unsigned           *nacc_arr = malloc(sizeof(unsigned) * nacc);

        for (i = 0; i < nacc; i++) {
            nacc_arr[i] = 1;
            cfg_ptrs[i] = &cfg[i];
        }
        esp_run_parallel(cfg_ptrs, nacc, nacc_arr);
        free(nacc_arr);
        free(cfg_ptrs);
    }
}

void esp_run_parallel(esp_thread_info_t *cfg[], unsigned nthreads, unsigned *nacc)
{
    int             i, j;
    struct timespec th_start;
    struct timespec th_end;
    pthread_t      *thread = malloc(nthreads * sizeof(pthread_t));
    int             rc     = 0;
    esp_config(cfg, nthreads, nacc);
    for (i = 0; i < nthreads; i++) {
        unsigned len = nacc[i];
        for (j = 0; j < len; j++) {
            esp_thread_info_t *info   = cfg[i] + j;
            const char        *prefix = "/dev/";
            char               path[70];

            if (strlen(info->devname) > 64) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                printf("Error: device name %s exceeds maximum length of 64 characters\n", info->devname);
            }

            sprintf(path, "%s%s", prefix, info->devname);

            info->fd = open(path, O_RDWR, 0);
            if (info->fd < 0) {
                contig_handle_t *handle = lookup_handle(info->hw_buf, NULL);
                contig_free(*handle);
                printf("fopen failed\n");
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
            if (nthreads == 1)
                accelerator_thread_p2p((void *)args);
            else
                rc = pthread_create(&thread[i], NULL, accelerator_thread_p2p, (void *)args);
        } else {
            if (nthreads == 1)
                accelerator_thread_serial((void *)args);
            else
                rc = pthread_create(&thread[i], NULL, accelerator_thread_serial, (void *)args);
        }

        if (rc != 0) {
            perror("pthread_create");
        }
    }
    for (i = 0; i < nthreads; i++) {
        if (nthreads > 1)
            rc = pthread_join(thread[i], NULL);

        if (rc != 0) {
            perror("pthread_join");
        }
    }

    gettime(&th_end);
    print_time_info(cfg, ts_subtract(&th_start, &th_end), nthreads, nacc);

    free(thread);
}

void esp_free(void *buf) { remove_buf(buf); }

int esp_dummy(int x)
{
    printf("it's esp_dummy\n");
    return x + 7;
}

typedef unsigned long long token_t;
typedef double             native_t;
#define fx2float fixed64_to_double
#define float2fx double_to_fixed64
#define FX_IL    42

/* <<--params-def-->> */
#define LOGN_SAMPLES 6
//#define NUM_FFTS     46
#define NUM_FFTS 1
//#define LOGN_SAMPLES 12
//#define NUM_FFTS     13
/*#define NUM_SAMPLES (NUM_FFTS * (1 << LOGN_SAMPLES))*/
#define DO_INVERSE   0
#define DO_SHIFT     1
#define SCALE_FACTOR 0

/* <<--params-->> */
const int32_t logn_samples = LOGN_SAMPLES;
/*const int32_t num_samples = NUM_SAMPLES;*/
const int32_t num_ffts     = NUM_FFTS;
const int32_t do_inverse   = DO_INVERSE;
const int32_t do_shift     = DO_SHIFT;
const int32_t scale_factor = SCALE_FACTOR;

struct fft2_stratus_access {
    struct esp_access esp;
    /* <<--regs-->> */
    unsigned scale_factor;
    unsigned do_inverse;
    unsigned logn_samples;
    unsigned do_shift;
    unsigned num_ffts;
    unsigned src_offset;
    unsigned dst_offset;
};

#define FFT2_STRATUS_IOC_ACCESS _IOW('S', 0, struct fft2_stratus_access)

struct fft2_stratus_access fft2_cfg_000[] = {{
    /* <<--descriptor-->> */
    .logn_samples  = LOGN_SAMPLES,
    .num_ffts      = NUM_FFTS,
    .do_inverse    = DO_INVERSE,
    .do_shift      = DO_SHIFT,
    .scale_factor  = SCALE_FACTOR,
    .src_offset    = 0,
    .dst_offset    = 0,
    .esp.coherence = ACC_COH_NONE,
    .esp.p2p_store = 0,
    .esp.p2p_nsrcs = 0,
    .esp.p2p_srcs  = {"", "", "", ""},
}};
esp_thread_info_t          cfg_000[]      = {{
                  .run       = true,
                  .devname   = "fft2_stratus.0",
                  .ioctl_req = FFT2_STRATUS_IOC_ACCESS,
                  .esp_desc  = &(fft2_cfg_000[0].esp),
}};

//-- from tb/BAK/fft2_test.cpp
#define M_PI 3.14159265358979323846
unsigned int fft2_rev(unsigned int v)
{
    unsigned int r = v;
    int          s = sizeof(v) * CHAR_BIT - 1;

    for (v >>= 1; v; v >>= 1) {
        r <<= 1;
        r |= v & 1;
        s--;
    }
    r <<= s;
    return r;
}

void fft2_bit_reverse(float *w, unsigned int offset, unsigned int n, unsigned int bits)
{
    unsigned int i, s, shift;

    s     = sizeof(i) * CHAR_BIT - 1;
    shift = s - bits + 1;

    for (i = 0; i < n; i++) {
        unsigned int r;
        float        t_real, t_imag;

        r = fft2_rev(i);
        r >>= shift;

        if (i < r) {
            t_real                  = w[2 * (offset + i)];
            t_imag                  = w[2 * (offset + i) + 1];
            w[2 * (offset + i)]     = w[2 * (offset + r)];
            w[2 * (offset + i) + 1] = w[2 * (offset + r) + 1];
            w[2 * (offset + r)]     = t_real;
            w[2 * (offset + r) + 1] = t_imag;
        }
    }
}

void fft2_do_shift(float *A0, unsigned int offset, unsigned int num_samples, unsigned int bits)
{
    int md = (num_samples / 2);
    /* shift: */
    for (unsigned oi = 0; oi < md; oi++) {
        unsigned int iidx = 2 * (offset + oi);
        unsigned int midx = 2 * (offset + md + oi);
        // printf("TEST: DO_SHIFT: offset %u : iidx %u %u midx %u %u\n", offset, iidx, (iidx+1), midx, (midx+1));

        float swap_rl, swap_im;
        swap_rl      = A0[iidx];
        swap_im      = A0[iidx + 1];
        A0[iidx]     = A0[midx];
        A0[iidx + 1] = A0[midx + 1];
        A0[midx]     = swap_rl;
        A0[midx + 1] = swap_im;
    }
}
int fft2_comp(float *data, unsigned nffts, unsigned int n, unsigned int logn, int do_inverse, int do_shift)
{
    for (int nf = 0; nf < nffts; nf++) {
        unsigned int transform_length;
        unsigned int a, b, i, j, bit;
        float        theta, t_real, t_imag, w_real, w_imag, s, t, s2, z_real, z_imag;

        unsigned int offset = nf * n; // This is the offset for start of nf=th FFT
        int          sign;
        // printf("TEST : Doing computation of FFT %u : data[%u] = %.15g\n", nf, 2*offset, data[2*offset]);
        if (do_inverse) {
            sign = 1;
            if (do_shift) {
                // printf("TEST: Calling Inverse-Do-Shift\n");
                fft2_do_shift(data, offset, n, logn);
            }
        } else {
            sign = -1;
        }

        transform_length = 1;

        // Do the bit-reverse
        fft2_bit_reverse(data, offset, n, logn);

        /* calculation */
        for (bit = 0; bit < logn; bit++) {
            w_real = 1.0;
            w_imag = 0.0;

            theta = 1.0 * sign * M_PI / (float)transform_length;

            s  = sin(theta);
            t  = sin(0.5 * theta);
            s2 = 2.0 * t * t;

            for (a = 0; a < transform_length; a++) {
                for (b = 0; b < n; b += 2 * transform_length) {
                    i = offset + b + a;
                    j = offset + b + a + transform_length;

                    z_real = data[2 * j];
                    z_imag = data[2 * j + 1];

                    t_real = w_real * z_real - w_imag * z_imag;
                    t_imag = w_real * z_imag + w_imag * z_real;

                    /* write the result */
                    data[2 * j]     = data[2 * i] - t_real;
                    data[2 * j + 1] = data[2 * i + 1] - t_imag;
                    data[2 * i] += t_real;
                    data[2 * i + 1] += t_imag;
                } // for (b = 0 .. n by 2*transform_length

                /* adjust w */
                t_real = w_real - (s * w_imag + s2 * w_real);
                t_imag = w_imag + (s * w_real - s2 * w_imag);
                w_real = t_real;
                w_imag = t_imag;

            } // for (a = 0 .. transform_length)
            transform_length *= 2;
        } // for (bit = 0 .. logn )

        if ((!do_inverse) && do_shift) {
            // printf("TEST: Calling Non-Inverse Do-Shift\n");
            fft2_do_shift(data, offset, n, logn);
        }

    } // for (nf = 0 .. num_ffts)

    return 0;
}

// -- from fft2.c :

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned size;

const float ERR_TH = 0.05;

/* User-defined code */
static int validate_buffer(token_t *out, float *gold)
{
    int            j;
    unsigned       errors      = 0;
    const unsigned num_samples = 1 << logn_samples;

    for (j = 0; j < 2 * num_ffts * num_samples; j++) {
        native_t val = fx2float(out[j], FX_IL);

        if ((fabs(gold[j] - val) / fabs(gold[j])) > ERR_TH) {
            if (errors < 2) {
                printf(" GOLD[%u] = %f vs %f = out[%u]\n", j, gold[j], val, j);
            }
            errors++;
        }
    }
    printf("  + Relative error > %.02f for %d values out of %d\n", ERR_TH, errors, 2 * num_ffts * num_samples);

    return errors;
}

/* User-defined code */
static void init_buffer(token_t *in, float *gold)
{
    int            j;
    const float    LO          = -2.0;
    const float    HI          = 2.0;
    const unsigned num_samples = (1 << logn_samples);

    srand((unsigned int)time(NULL));

    for (j = 0; j < 2 * num_ffts * num_samples; j++) {
        float scaling_factor = (float)rand() / (float)RAND_MAX;
        gold[j]              = LO + scaling_factor * (HI - LO);
    }

    // convert input to fixed point
    for (j = 0; j < 2 * num_ffts * num_samples; j++) {
        in[j] = float2fx((native_t)gold[j], FX_IL);
    }

    // Compute golden output
    fft2_comp(gold, num_ffts, (1 << logn_samples), logn_samples, do_inverse, do_shift);
}

/* User-defined code */
static void init_parameters()
{
    const unsigned num_samples = (1 << logn_samples);

    if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
        in_words_adj  = 2 * num_ffts * num_samples;
        out_words_adj = 2 * num_ffts * num_samples;
    } else {
        in_words_adj  = round_up(2 * num_ffts * num_samples, DMA_WORD_PER_BEAT(sizeof(token_t)));
        out_words_adj = round_up(2 * num_ffts * num_samples, DMA_WORD_PER_BEAT(sizeof(token_t)));
    }
    in_len     = in_words_adj;
    out_len    = out_words_adj;
    in_size    = in_len * sizeof(token_t);
    out_size   = out_len * sizeof(token_t);
    out_offset = 0;
    size       = (out_offset * sizeof(token_t)) + out_size;
}

int esp_py_run(int x1, int x2)
{
    printf("esp_py_run: start\n");

    int errors;

    float   *gold;
    token_t *buf;

    const float    ERROR_COUNT_TH = 0.001;
    const unsigned num_samples    = (1 << logn_samples);

    init_parameters();

    buf               = (token_t *)esp_alloc(size);
    cfg_000[0].hw_buf = buf;
    gold              = malloc(out_len * sizeof(float));

    printf("\n====== %s ======\n\n", cfg_000[0].devname);
    /* <<--print-params-->> */
    printf("  .logn_samples = %d\n", logn_samples);
    printf("   num_samples  = %d\n", (1 << logn_samples));
    printf("  .num_ffts = %d\n", num_ffts);
    printf("  .do_inverse = %d\n", do_inverse);
    printf("  .do_shift = %d\n", do_shift);
    printf("  .scale_factor = %d\n", scale_factor);
    printf("\n  ** START **\n");
    init_buffer(buf, gold);

    esp_run(cfg_000, 1);

    printf("\n  ** DONE **\n");

    errors = validate_buffer(&buf[out_offset], gold);

    free(gold);
    esp_free(buf);

    if ((float)(errors / (float)(2.0 * (float)num_ffts * (float)num_samples)) > ERROR_COUNT_TH)
        printf("  + TEST FAIL: exceeding error count threshold\n");
    else
        printf("  + TEST PASS: not exceeding error count threshold\n");

    printf("\n====== %s ======\n\n", cfg_000[0].devname);

    printf("esp_py_run: end\n");
    return x1 + x2;
}