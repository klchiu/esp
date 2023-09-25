#ifndef KTHREADS_H
#define KTHREADS_H

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */

#define BILLION 1000000000L

#define RESET 1
#define bool unsigned

#define eid_t    long long
#define uint64_t unsigned long long

/*
 * Keep count of the threads. Start the computation, as soon as all of them are
 * created, up and running.
 */
pthread_mutex_t tcount_mutex;
pthread_cond_t  tcount_ready;
unsigned        tcount;

/*
 * The FIFO has
 * - a tail (PUT-index) where you push elements in and
 * - a head (GET-index) where you pop elements out;
 * the FIFO is implemented as a circular buffer, this requires modulus
 * operations (PUT-/GET-index); each element in the FIFO has associated an
 * unique-incremental element id (e.g. n, n+1, ...);
 *
 * <------------ capacity ------------->
 *
 * |-----|-----|-----|-----|-----|-----|
 * |     |XXXXX|XXXXX|XXXXX|XXXXX|     |
 * |     | n+3 | n+2 | n+1 |  n  |     | element_ids
 * |     |XXXXX|XXXXX|XXXXX|XXXXX|     |
 * |-----|-----|-----|-----|-----|-----|
 *    ^                       v
 *   PUT                     GET
 *   in                      out
 *        <-------  size ------->
 */

/* Maximum capacity of the FIFO */
//#define FIFO_MAX_SIZE 128
#define FIFO_MAX_SIZE 3000

/* This FIFO has one producer and many consumers; an element is popped out of
 * the FIFO only when all of the consumers have consumed (get) it. */
typedef struct fifo_t {
    /* The element-id mechanism avoids to get the same element multiple times. I
     * use an extra counter for simplicity; for the same purpose I can access to
     * the previous element and increment that but this would require modulus
     * operations etc. */
    eid_t element_ids[FIFO_MAX_SIZE];
    eid_t element_id_counter;

    unsigned in;        /* where to put (PUT-in cursor) */
    unsigned out;       /* where to get (GET-out cursor) */
    unsigned capacity;  /* maximum size of the FIFO */
    unsigned size;      /* current size */
    unsigned consumers; /* total number of FIFO consumers */
    unsigned consumed;  /* how many elements have been currently consumed (consumed <= consumers) */

    pthread_mutex_t mutex;       /* to avoid race conditons */
    pthread_cond_t  cond_full;   /* queue full - threads waiting to put on a full queue */
    pthread_cond_t  cond_empty;  /* queue empty - threads waiting to get from an empty queue*/
    pthread_cond_t  cond_update; /* queue update - threads waiting to get a new element from a queue */
} fifo_t;

/* fifo_t helpers */
void     lock_fifo(fifo_t *fifo);              /* acquire mutex lock */
void     unlock_fifo(fifo_t *fifo);            /* release mutex lock */
bool     is_full(fifo_t *fifo);                /* size == capacity */
bool     is_empty(fifo_t *fifo);               /* size == 0 */
bool     has_consumers(fifo_t *fifo);          /* consumers != 0 */
void     cond_wait_full(fifo_t *fifo);         /* if FIFO is_full wait */
void     cond_wait_empty(fifo_t *fifo);        /* if FIFO is_empty wait */
void     cond_wait_update(fifo_t *fifo);       /* if thread is waiting for a new element under the GET-out cursor */
void     wake_up_empty(fifo_t *fifo);          /* wake up all of the threads waiting on a previously empty FIFO */
void     wake_up_full(fifo_t *fifo);           /* wake up all of the threads waiting on a previously full FIFO */
void     wake_up_update(fifo_t *fifo);         /* wake up all of the threads waiting for a new element */
unsigned get_current_element_id(fifo_t *fifo); /* return the element id under the GET-out cursor */
#if 0
void set_current_element_id(fifo_t *fifo, unsigned element_id);
#endif

/* Maximum number of threads that provide an input to each thread (incoming threads) */
//#define INCOMING_MAX_COUNT 128
#define INCOMING_MAX_COUNT 3000

typedef struct kthread_t {
    pthread_t       handle;
    pthread_mutex_t mutex; /* to avoid race conditions */
    pthread_cond_t  ready; /* TODO */

    unsigned           incoming;         /* incoming thread count */
    struct kthread_t **incoming_threads; /* pointer to incoming threads */
    unsigned          *feedback;         /* which of the incoming threads is feedback */
    bool               reset;            /* mark it as the first thread in the pipeline */
    unsigned           iter_max;         /* maximum number of iterations */
    unsigned           iter_count;       /* iteration counter */

    fifo_t fifo; /* kernel FIFO */

    unsigned fifo_get_index[FIFO_MAX_SIZE];
    unsigned fifo_put_index;
    eid_t    fifo_prev_element_ids[FIFO_MAX_SIZE];

    unsigned int execution_time;
    char        *id;
#if 1
  void (*run_hw)(void *);

  unsigned long long hw_ns;
  unsigned long long sw_ns;
  unsigned n_errors;
  unsigned n_imgs;
#endif
} kthread_t;

/* kthread_t helpers */
void     lock_kernel(kthread_t *kernel);                             /* acquire mutex lock */
void     unlock_kernel(kthread_t *kernel);                           /* release mutex lock */
void     enable_feedback(kthread_t *kernel, unsigned index);         /* mark a thread as a feedback */
void     disable_feedback(kthread_t *kernel, unsigned index);        /* unmark a feedback thread */
unsigned is_feedback(kthread_t *kernel, unsigned index);             /* check if a thread is a feedback */
unsigned get_previous_element_id(kthread_t *kernel, unsigned index); /* return the just gotten element id */
void     set_previous_element_id(kthread_t *kernel, unsigned index, unsigned element_id);
void     dump_topology(kthread_t *kernels, unsigned num_kernels);
void     dump_dgraph(kthread_t *kernels, unsigned num_kernels, char *filename);

/*
 * The function returns the PUT-index (i.e. first available slot) as soon as
 * there is an available slot (i.e. it may wait for that).
 */
unsigned can_put(kthread_t *kernel);
void     put(kthread_t *kernel);

/*
 * The function returns the GET-index (i.e. first available item) as soon as
 * there is an item available (i.e. it may wait for that).
 */
unsigned can_get(kthread_t *source, kthread_t *kernel, unsigned index);
void     get(kthread_t *source, kthread_t *kernel, unsigned index);

/*
 * It encapsulates the hardware.
 */
void *thread_function(void *kernel_ptr);

/*
 * Other thread helpers.
 */
void start(kthread_t *kernel, unsigned reset_index);
void assert_reset_kernel(kthread_t *kernel, unsigned kernels_num);
void set_reset_kernel(kthread_t *kernels, unsigned kernels_num, unsigned reset_index);
void set_iteration_counters(kthread_t *kernel, unsigned kernels_num, unsigned count);
void reset_fifo_prev_element_ids(kthread_t *kernel);
void init_fifo(kthread_t *kernel);
void init_feedback(kthread_t *kernel);
void wrap_up(kthread_t *kernels, unsigned kernels_num);

extern pthread_mutex_t printf_mutex;

/* fifo_t helpers */
void lock_fifo(fifo_t *fifo)
{
    assert(fifo);
    int ret = pthread_mutex_lock(&fifo->mutex);
    assert(ret == 0);
}

void unlock_fifo(fifo_t *fifo)
{
    assert(fifo);
    int ret = pthread_mutex_unlock(&fifo->mutex);
    assert(ret == 0);
}

bool is_full(fifo_t *fifo) { return fifo->size == fifo->capacity; }

bool is_empty(fifo_t *fifo) { return fifo->size == 0; }

bool has_consumers(fifo_t *fifo) { return (fifo->consumers > 0); }

void cond_wait_full(fifo_t *fifo) { pthread_cond_wait(&fifo->cond_full, &fifo->mutex); }

void cond_wait_empty(fifo_t *fifo) { pthread_cond_wait(&fifo->cond_empty, &fifo->mutex); }

void cond_wait_update(fifo_t *fifo) { pthread_cond_wait(&fifo->cond_update, &fifo->mutex); }

void wake_up_empty(fifo_t *fifo) { pthread_cond_broadcast(&fifo->cond_empty); }

void wake_up_update(fifo_t *fifo) { pthread_cond_broadcast(&fifo->cond_update); }

void wake_up_full(fifo_t *fifo) { pthread_cond_broadcast(&fifo->cond_full); }

unsigned get_current_element_id(fifo_t *fifo) { return fifo->element_ids[fifo->out]; }

#if 0
void set_current_element_id(fifo_t *fifo, unsigned element_id)
{ 
  fifo->element_ids[fifo->out] = element_id;
  fifo->element_id_counter = element_id;
}
#endif

/* kthread_t helpers */

void lock_kernel(kthread_t *kernel) { pthread_mutex_lock(&kernel->mutex); }

void unlock_kernel(kthread_t *kernel) { pthread_mutex_unlock(&kernel->mutex); }

void enable_feedback(kthread_t *kernel, unsigned index) { kernel->feedback[index] = 1; }

void disable_feedback(kthread_t *kernel, unsigned index) { kernel->feedback[index] = 0; }

unsigned is_feedback(kthread_t *kernel, unsigned index) { return kernel->feedback[index]; }

unsigned get_previous_element_id(kthread_t *kernel, unsigned index) { return kernel->fifo_prev_element_ids[index]; }

void set_previous_element_id(kthread_t *kernel, unsigned index, unsigned element_id)
{
    kernel->fifo_prev_element_ids[index] = element_id;
}

void dump_topology(kthread_t *ktreads, unsigned num_ktreads)
{
    unsigned i, j;
    for (i = 0; i < num_ktreads; i++) {
        printf("[%s]\n", ktreads[i].id);
        printf("   first    %d\n", ktreads[i].reset);
        printf("   incoming %d {", ktreads[i].incoming);
        for (j = 0; j < ktreads[i].incoming; j++) {
            printf("%s (%d), ", ktreads[i].incoming_threads[j]->id, ktreads[i].feedback[j]);
        }
        printf("}\n   fifo\n");
        printf("     capacity %d\n", ktreads[i].fifo.capacity);
        printf("     size %d\n", ktreads[i].fifo.size);
        printf("     outgoing %d\n", ktreads[i].fifo.consumers);
        printf("     in (PUT) %d\n", ktreads[i].fifo.in);
        printf("     out (GET) %d\n", ktreads[i].fifo.out);
    }
}

void dump_dgraph(kthread_t *ktreads, unsigned num_ktreads, char *filename)
{
    unsigned i, j;

    FILE *fp = fopen(filename, "w");
    fprintf(fp, "digraph G {\n");

    for (i = 0; i < num_ktreads; i++) {
        for (j = 0; j < ktreads[i].incoming; j++) {
            fprintf(fp, "  %s -> %s", ktreads[i].incoming_threads[j]->id, ktreads[i].id);
            if (ktreads[i].feedback[j])
                fprintf(fp, " [color=red]");
            fprintf(fp, ";\n");
        }
    }
    fprintf(fp, "}\n");

    fclose(fp);
}

static inline unsigned long long tick()
{
    unsigned long long d;
    __asm__ __volatile__("rdtsc" : "=A"(d));
    return d;
}

/*
 * The function returns the PUT-index (i.e. first available slot) as soon as
 * there is an available slot (i.e. it may wait for that).
 */
unsigned can_put(kthread_t *kernel)
{
    fifo_t *kernel_fifo = &kernel->fifo;

    lock_fifo(kernel_fifo);

    /* If full, wait for a get! */
    while (is_full(kernel_fifo))
        cond_wait_full(kernel_fifo);

    /* Get the PUT-index (where the data should be stored). */
    unsigned put_index = kernel_fifo->in;

    unlock_fifo(kernel_fifo);

    return put_index;
}

void put(kthread_t *kernel)
{
    fifo_t *kernel_fifo = &kernel->fifo;

    lock_fifo(kernel_fifo);

    /* FIFO size has to be less or equal the capacity! */
    assert(kernel_fifo->size <= kernel_fifo->capacity);
    assert(kernel_fifo->in < kernel_fifo->capacity);
    assert(kernel_fifo->out < kernel_fifo->capacity);

    /* Store the element-id. */
    kernel_fifo->element_ids[kernel_fifo->in] = ++kernel_fifo->element_id_counter;

    /* Update the PUT-index of the FIFO (circular buffer). */
    ++(kernel_fifo->in);
    kernel_fifo->in %= kernel_fifo->capacity;

    /* Update the FIFO size. */
    ++(kernel_fifo->size);

    /* Wake-up all of the waiting processes. */
    pthread_cond_broadcast(&kernel_fifo->cond_update);

    /* Wake-up all of the ktreads waiting for a element to get (can_get). */
    wake_up_empty(kernel_fifo);

    unlock_fifo(kernel_fifo);
}

/*
 * The function returns the GET-index (i.e. first available item) as soon as
 * there is an item available (i.e. it may wait for that).
 */
unsigned can_get(kthread_t *source, kthread_t *kernel, unsigned index)
{
    fifo_t *source_fifo = &source->fifo;
    fifo_t *kernel_fifo = &kernel->fifo;
    // fprintf(stderr, "** in can_get, debug 0\n");

    lock_kernel(kernel);
    unsigned feedback = is_feedback(kernel, index);
    unlock_kernel(kernel);
    // fprintf(stderr, "** in can_get, debug 1\n");

    lock_fifo(source_fifo);

    //  assert (kernel_fifo->in < kernel_fifo->capacity);
    //  assert (kernel_fifo->out < kernel_fifo->capacity);

    /* If there is an element (at least) but it is already consumed, wait for a get! */
    while ((!is_empty(source_fifo)) && get_previous_element_id(kernel, index) == get_current_element_id(source_fifo))
        cond_wait_update(source_fifo);
    // fprintf(stderr, "** in can_get, debug 2\n");
    /* If empty, wait for a put! */
    while (is_empty(source_fifo) && !feedback)
        cond_wait_empty(source_fifo);

    // fprintf(stderr, "** in can_get, debug 3\n");

    /* Return the GET-index (i.e. first available element in the FIFO - head). */
    unsigned get_index  = source_fifo->out;
    eid_t    element_id = get_current_element_id(source_fifo);

    // fprintf(stderr, "** in can_get, debug 4\n");

    unlock_fifo(source_fifo);
    // fprintf(stderr, "** in can_get, debug 5\n");
    lock_fifo(kernel_fifo);
    // fprintf(stderr, "** in can_get, debug 6\n");
    set_previous_element_id(kernel, index, element_id);
    // fprintf(stderr, "** in can_get, debug 7\n");
    unlock_fifo(kernel_fifo);
    // fprintf(stderr, "** in can_get, debug 8\n");

    //  /* One time only for feedback */
    //  lock_kernel(kernel);
    //  disable_feedback(kernel, index);
    //  unlock_kernel(kernel);
    //
    return get_index;
}

void get(kthread_t *source, kthread_t *kernel, unsigned index)
{
    fifo_t *source_fifo = &source->fifo;
    //  fifo_t *kernel_fifo = &kernel->fifo;

    /* One time only for feedback */
    lock_kernel(kernel);
    unsigned feedback = is_feedback(kernel, index);
    disable_feedback(kernel, index);
    unlock_kernel(kernel);

    if (feedback)
        return;

    lock_fifo(source_fifo);
    //  assert (kernel_fifo->in < kernel_fifo->capacity);
    //  assert (kernel_fifo->out < kernel_fifo->capacity);

    /* Update the number of times the element is consumed. */
    ++(source_fifo->consumed);

    /* Pop the element when all of the consumers have done. */
    if (source_fifo->consumed == source_fifo->consumers) {
        /* decrement the size of the FIFO */
        assert(source_fifo->size != 0);
        --(source_fifo->size);

        /* Reset consumer counter. */
        source_fifo->consumed = 0;

        /* Update the FIFO OUT-index (circular buffer). */
        ++(source_fifo->out);
        source_fifo->out %= source_fifo->capacity;

        /* Wake-up all of the ktreads waiting for the next element (can_get). */
        wake_up_update(source_fifo);

        /* Wake-up all of the ktreads waiting for a space to put (can_put) */
        wake_up_full(source_fifo);
    }

    unlock_fifo(source_fifo);
}

void *thread_function(void *kernel_ptr)
{
      // fprintf(stderr, "in thread_function: 0\n");

    unsigned        index;
    struct timespec zero;
    struct timespec start, end;
    uint64_t        start_diff, end_diff;

    kthread_t *kernel = (kthread_t *)kernel_ptr;
    assert(kernel != NULL);
    // fprintf(stderr, "in thread_function: 1 kern_id: %s\n", kernel->id);
    assert(kernel->id != NULL);
    // fprintf(stderr, "in thread_function: 2 kern_id: %s\n", kernel->id);

    /* Thread up and running! */
    pthread_mutex_lock(&tcount_mutex);
    tcount++;
    pthread_cond_signal(&tcount_ready);
    pthread_mutex_unlock(&tcount_mutex);

    /* First kernel wait for start. */
    pthread_mutex_lock(&kernel->mutex);
    while (kernel->reset) {
        pthread_cond_wait(&kernel->ready, &kernel->mutex);
        if (kernel->reset)
            break;
    }
    pthread_mutex_unlock(&kernel->mutex);
    // fprintf(stderr, "in thread_function: 3 kern_id: %s\n", kernel->id);

    clock_gettime(CLOCK_MONOTONIC, &zero); /* mark zero time */

    /* Alive for many iterations. */
    while (1) {
        /* At each iteration:
         * - for_each incoming-kernel queues can_get
         * - can_put on the local queue
         * - do something
         * - for_each incoming-kernel queues get
         * - put on the local queue
         */

        /* TODO: This works under the assumption that each accelerator consume the
         * same number of frames. */
        if (kernel->iter_count >= kernel->iter_max)
            break;
        // fprintf(stderr, "** in while loop, max = %d, iter_count = %d\n", kernel->iter_max, kernel->iter_count);

        for (index = 0; index < kernel->incoming; index++) {
            kthread_t *source = kernel->incoming_threads[index];
            assert(source != NULL);
            // fprintf(stderr, "** in while loop, debug 0.1\n");
            kernel->fifo_get_index[index] = can_get(source, kernel, index);
            // fprintf(stderr, "** in while loop, debug 0.2\n");
        }
        // fprintf(stderr, "** in while loop, debug 0\n");
        unsigned put_index = can_put(kernel);
        // fprintf(stderr, "** in while loop, debug 1\n");

        /* mark start time */
        clock_gettime(CLOCK_MONOTONIC, &start);
        start_diff = BILLION * (start.tv_sec - zero.tv_sec) + start.tv_nsec - zero.tv_nsec;

#ifdef PROFILING
        pthread_mutex_lock(&printf_mutex);
        printf("@ %llu %s BEG [ iter = %u ] in:{", start_diff, kernel->id, kernel->iter_count);
        for (index = 0; index < kernel->incoming; index++) {
            kthread_t *source = kernel->incoming_threads[index];
            printf(" [ %d ] = %lld ", kernel->fifo_get_index[index], source->fifo.element_ids[kernel->fifo.out]);
        }
        printf(" }\n");
        pthread_mutex_unlock(&printf_mutex);
#endif
    // fprintf(stderr, "in thread_function: 4 kern_id: %s\n", kernel->id);

        //usleep(kernel->execution_time);
        //printf("  > Test time: %llu ns\n", kernel->execution_time);
        //printf("    - %s time: %llu ns\n", kernel->id, kernel->execution_time);
	
             // fprintf(stderr, "** in while loop, debug 2, execution_time: %d\n", kernel->execution_time);
        // if (kernel->id == "add"){
            //fifo_t *kernel_fifo = &kernel->fifo;
            //fprintf(stderr, "in thread_function: 4.23481234981509778509127 kern_id: %s, fifo_size: %d\n", kernel->id, kernel_fifo->size);
          kernel->run_hw(kernel_ptr);
        //}
    // fprintf(stderr, "in thread_function: 5 kern_id: %s\n", kernel->id);


        /* mark the end time */
        clock_gettime(CLOCK_MONOTONIC, &end);
        end_diff = BILLION * (end.tv_sec - zero.tv_sec) + end.tv_nsec - zero.tv_nsec;

        for (index = 0; index < kernel->incoming; index++) {
            kthread_t *source = kernel->incoming_threads[index];
            get(source, kernel, index);
        }
        put(kernel);
        // fprintf(stderr, "** in while loop, debug 3, kernel id: %s\n", kernel->id);

        assert(kernel->fifo.element_ids[put_index] != 0 && "Wrong data in the FIFO!");

#ifdef PROFILING
        pthread_mutex_lock(&printf_mutex);
        printf("@ %llu %s END [ iter = %u ] in:{", end_diff, kernel->id, kernel->iter_count);
        printf(" } out:{ [ %d ] = ", put_index);
        printf("%lld ", kernel->fifo.element_ids[put_index]);
        printf("}\n");
        pthread_mutex_unlock(&printf_mutex);
#endif

        ++kernel->iter_count;
    }

    return NULL;
}

void start(kthread_t *kernel, unsigned reset_index)
{
    pthread_mutex_lock(&kernel[reset_index].mutex);
    pthread_cond_signal(&kernel[reset_index].ready);
    pthread_mutex_unlock(&kernel[reset_index].mutex);
}

void assert_reset_kernel(kthread_t *kernel, unsigned ktreads_num)
{
    unsigned index, rcount = 0;
    for (index = 0; index < ktreads_num; index++)
        if (kernel[index].reset)
            rcount++;
    assert(rcount == 1 && "One and only one reset kernel has to be set!");
}

void set_reset_kernel(kthread_t *ktreads, unsigned ktreads_num, unsigned reset_index)
{
    ktreads[reset_index].reset = 1;

    /* check that not more than one kernel is a reset kernel */
    assert_reset_kernel(ktreads, ktreads_num);
}

void set_iteration_counters(kthread_t *kernel, unsigned ktreads_num, unsigned count)
{
    unsigned index;
    for (index = 0; index < ktreads_num; index++)
        kernel[index].iter_max = count;
}

void reset_fifo_prev_element_ids(kthread_t *kernel)
{
    unsigned index;
    for (index = 0; index < INCOMING_MAX_COUNT; index++)
        kernel->fifo_prev_element_ids[index] = 0;
}

void init_fifo(kthread_t *kernel)
{
    kernel->fifo.size               = 0;
    kernel->fifo.in                 = 0;
    kernel->fifo.out                = 0;
    kernel->fifo.element_id_counter = 0;

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

    assert(kernel);
    int ret;
    ret = pthread_mutex_init(&kernel->fifo.mutex, &mta);
    assert(ret == 0);
    ret = pthread_cond_init(&kernel->fifo.cond_full, NULL);
    assert(ret == 0);
    ret = pthread_cond_init(&kernel->fifo.cond_empty, NULL);
    assert(ret == 0);
    ret = pthread_cond_init(&kernel->fifo.cond_update, NULL);
    assert(ret == 0);
    pthread_mutexattr_destroy(&mta);
}

void init_feedback(kthread_t *kernel)
{
    unsigned i;
    for (i = 0; i < kernel->incoming; i++) {
        if (kernel->feedback[i]) {
            //      set_current_element_id(&(kernel->incoming_threads[i]->fifo), 1);
            //      kernel->incoming_threads[i]->fifo.in = 1;
            //      kernel->incoming_threads[i]->fifo.in %= kernel->incoming_threads[i]->fifo.capacity;
        }
    }
}

void wrap_up(kthread_t *ktreads, unsigned ktreads_num)
{
    unsigned i, j;
    for (i = 0; i < ktreads_num; i++) {
        int ret;
        ret = pthread_mutex_destroy(&ktreads[i].mutex);
        assert(ret == 0);
        for (j = 0; j < ktreads[i].incoming; j++) {
            ret = pthread_mutex_destroy(&ktreads[i].incoming_threads[j]->fifo.mutex);
            assert(ret == 0);
        }
    }
}

#endif // KTHREADS_H