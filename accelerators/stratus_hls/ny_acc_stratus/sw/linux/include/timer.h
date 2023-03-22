#ifndef _TIMER_H_
#define _TIMER_H_

#define _XOPEN_SOURCE 600
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void   init_timer(void);
double timer_getres(void);
void   tic(void);
double toc(void);

#define STATS_INIT()                                        \
    do {                                                    \
        init_timer();                                       \
        printf("{\n");                                      \
        printf("\t\"timer_res\": %20.15e", timer_getres()); \
    } while (0)

#define STATS_END()      \
    do {                 \
        printf("\n}\n"); \
    } while (0)

#define PRINT_STAT_INT(NAME_, VALUE_)             \
    do {                                          \
        printf(",\n\t\"%s\": %d", NAME_, VALUE_); \
    } while (0)

#define PRINT_STAT_INT64(NAME_, VALUE_)            \
    do {                                           \
        printf(",\n\t\"%s\": %ld", NAME_, VALUE_); \
    } while (0)

#define PRINT_STAT_INT64_ARRAY_AS_PAIR(NAME_, ARY_, LEN_)      \
    do {                                                       \
        printf(",\n\t\"%s\": [", NAME_);                       \
        for (uint64_t i = 0; i < LEN_ - 1; i++) {              \
            printf("\n\t\t[ %ld, %ld],", i, ARY_[i]);          \
        }                                                      \
        printf("\n\t\t[ %ld, %ld]", LEN_ - 1, ARY_[LEN_ - 1]); \
        printf("\n\t]");                                       \
    } while (0)

#define PRINT_STAT_DOUBLE(NAME_, VALUE_)               \
    do {                                               \
        printf(",\n\t\"%s\": %20.15e", NAME_, VALUE_); \
    } while (0)

#define PRINT_STAT_COUNT_DOUBLE(COUNT_, NAME_, VALUE_)             \
    do {                                                           \
        printf(",\n\t\"%4d-%s\": %20.15e", COUNT_, NAME_, VALUE_); \
    } while (0)

#define PRINT_STAT_HEX64(NAME_, VALUE_)                 \
    do {                                                \
        printf(",\n\t\"%s\":\"0x%lx\"", NAME_, VALUE_); \
    } while (0)

#define PRINT_STAT_STRING(NAME_, VALUE_)             \
    do {                                             \
        printf(",\n\t\"%s\":\"%s\"", NAME_, VALUE_); \
    } while (0)

#ifdef _OPENMP
    #include <omp.h>
#endif

#if defined(_OPENMP)
void init_timer(void)
{ /* Empty. */
}

double timer(void) { return omp_get_wtime(); }

double timer_getres(void) { return omp_get_wtick(); }

#elif defined(__MACH__)

    /* tallent: quick hack to support MacOS */

    #warning "MacOS timer is a nop"

void init_timer(void)
{ /* Empty. */
}

double timer(void) { return 0.0; }

double timer_getres(void) { return 0.0; }

#else /* Elsewhere */
static clockid_t clockid;

    #if defined(CLOCK_REALTIME_ID)
        #define CLKID     CLOCK_REALTIME_ID
        #define CLKIDNAME "CLOCK_REALTIME_ID"
    #elif defined(CLOCK_THREAD_CPUTIME_ID)
        #define CLKID     CLOCK_THREAD_CPUTIME_ID
        #define CLKIDNAME "CLOCK_THREAD_CPUTIME_ID"
    #elif defined(CLOCK_REALTIME_ID)
        #warning "Falling back to realtime clock."
        #define CLKID     CLOCK_REALTIME_ID
        #define CLKIDNAME "CLOCK_REALTIME_ID"
    #else
        #error "Cannot find a clock!"
    #endif

/**
 * @brief Initialize the system timer
 */
void init_timer(void)
{
    int err;
    err = clock_getcpuclockid(0, &clockid);
    if (err >= 0)
        return;
    fprintf(stderr, "Unable to find CPU clock, falling back to " CLKIDNAME "\n");
    clockid = CLKID;
}

double timer(void)
{
    struct timespec tp;
    clock_gettime(clockid, &tp);
    return (double)tp.tv_sec + 1.0e-9 * (double)tp.tv_nsec;
}

/**
 * @brief Get the resolution of the system clock
 *
 * @return Clock Resolution
 */
double timer_getres(void)
{
    struct timespec tp;
    clock_getres(clockid, &tp);
    return (double)tp.tv_sec + 1.0e-9 * (double)tp.tv_nsec;
}

#endif

static double last_tic = -1.0;

/**
 * @brief Start the timer
 */
void tic(void) { last_tic = timer(); }

/**
 * @brief Stop the timer and return the time taken
 *
 * @return Time since last tic()
 */
double toc(void)
{
    const double t   = timer();
    const double out = t - last_tic;
    last_tic         = t;
    return out;
}

#endif /* _TIMER_H_ */
