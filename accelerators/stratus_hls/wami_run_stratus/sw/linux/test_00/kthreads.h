#ifndef KTHREADS_H
#define KTHREADS_H

#include <pthread.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>	/* for printf */
#include <stdint.h>	/* for uint64 definition */
#include <stdlib.h>	/* for exit() definition */
#include <time.h>	/* for clock_gettime */

#define BILLION 1000000000L

#define RESET 1
#define bool unsigned

#define eid_t long long
#define uint64_t unsigned long long

/*
 * Keep count of the threads. Start the computation, as soon as all of them are
 * created, up and running.
 */
pthread_mutex_t tcount_mutex;
pthread_cond_t tcount_ready;
unsigned tcount;

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
#define FIFO_MAX_SIZE 128

/* This FIFO has one producer and many consumers; an element is popped out of
 * the FIFO only when all of the consumers have consumed (get) it. */
typedef struct fifo_t
{
  /* The element-id mechanism avoids to get the same element multiple times. I
   * use an extra counter for simplicity; for the same purpose I can access to
   * the previous element and increment that but this would require modulus
   * operations etc. */
  eid_t element_ids[FIFO_MAX_SIZE];
  eid_t element_id_counter;

  unsigned in;  /* where to put (PUT-in cursor) */
  unsigned out; /* where to get (GET-out cursor) */
  unsigned capacity; /* maximum size of the FIFO */
  unsigned size; /* current size */
  unsigned consumers; /* total number of FIFO consumers */
  unsigned consumed; /* how many elements have been currently consumed (consumed <= consumers) */

  pthread_mutex_t mutex; /* to avoid race conditons */
  pthread_cond_t cond_full; /* queue full - threads waiting to put on a full queue */
  pthread_cond_t cond_empty; /* queue empty - threads waiting to get from an empty queue*/
  pthread_cond_t cond_update; /* queue update - threads waiting to get a new element from a queue */
} fifo_t;

/* fifo_t helpers */
void lock_fifo(fifo_t *fifo); /* acquire mutex lock */
void unlock_fifo(fifo_t *fifo); /* release mutex lock */
bool is_full(fifo_t *fifo); /* size == capacity */
bool is_empty(fifo_t *fifo); /* size == 0 */
bool has_consumers(fifo_t *fifo); /* consumers != 0 */
void cond_wait_full(fifo_t *fifo); /* if FIFO is_full wait */
void cond_wait_empty(fifo_t *fifo); /* if FIFO is_empty wait */
void cond_wait_update(fifo_t *fifo); /* if thread is waiting for a new element under the GET-out cursor */
void wake_up_empty(fifo_t *fifo); /* wake up all of the threads waiting on a previously empty FIFO */
void wake_up_full(fifo_t *fifo); /* wake up all of the threads waiting on a previously full FIFO */
void wake_up_update(fifo_t *fifo); /* wake up all of the threads waiting for a new element */
unsigned get_current_element_id(fifo_t *fifo); /* return the element id under the GET-out cursor */
#if 0
void set_current_element_id(fifo_t *fifo, unsigned element_id);
#endif

/* Maximum number of threads that provide an input to each thread (incoming threads) */
#define INCOMING_MAX_COUNT 128

typedef struct kthread_t
{
  pthread_t handle;
  pthread_mutex_t mutex; /* to avoid race conditions */
  pthread_cond_t ready; /* TODO */

  unsigned incoming; /* incoming thread count */
  struct kthread_t** incoming_threads; /* pointer to incoming threads */
  unsigned *feedback; /* which of the incoming threads is feedback */
  bool reset; /* mark it as the first thread in the pipeline */
  unsigned iter_max; /* maximum number of iterations */
  unsigned iter_count; /* iteration counter */

  fifo_t fifo; /* kernel FIFO */

  unsigned fifo_get_index[FIFO_MAX_SIZE];
  unsigned fifo_put_index;
  eid_t fifo_prev_element_ids[FIFO_MAX_SIZE];

  unsigned int execution_time;
  char* id;
#if 0
  void (*run_hw)(void *);

  unsigned long long hw_ns;
  unsigned long long sw_ns;
  unsigned n_errors;
  unsigned n_imgs;
#endif
} kthread_t;

/* kthread_t helpers */
void lock_kernel(kthread_t *kernel); /* acquire mutex lock */
void unlock_kernel(kthread_t *kernel); /* release mutex lock */
void enable_feedback(kthread_t *kernel, unsigned index); /* mark a thread as a feedback */
void disable_feedback(kthread_t *kernel, unsigned index); /* unmark a feedback thread */
unsigned is_feedback(kthread_t *kernel, unsigned index); /* check if a thread is a feedback */
unsigned get_previous_element_id(kthread_t *kernel, unsigned index); /* return the just gotten element id */
void set_previous_element_id(kthread_t *kernel, unsigned index, unsigned element_id);
void dump_topology(kthread_t *kernels, unsigned num_kernels);
void dump_dgraph(kthread_t *kernels, unsigned num_kernels, char *filename);

/*
 * The function returns the PUT-index (i.e. first available slot) as soon as
 * there is an available slot (i.e. it may wait for that).
 */
unsigned can_put(kthread_t* kernel);
void put(kthread_t* kernel);

/*
 * The function returns the GET-index (i.e. first available item) as soon as
 * there is an item available (i.e. it may wait for that).
 */
unsigned can_get(kthread_t* source, kthread_t* kernel, unsigned index);
void get(kthread_t* source, kthread_t* kernel, unsigned index);

/*
 * It encapsulates the hardware.
 */
void* thread_function(void* kernel_ptr);

/*
 * Other thread helpers.
 */
void start(kthread_t* kernel, unsigned reset_index);
void assert_reset_kernel(kthread_t* kernel, unsigned kernels_num);
void set_reset_kernel(kthread_t* kernels, unsigned kernels_num, unsigned reset_index);
void set_iteration_counters(kthread_t* kernel, unsigned kernels_num, unsigned count);
void reset_fifo_prev_element_ids(kthread_t* kernel);
void init_fifo(kthread_t* kernel);
void init_feedback(kthread_t* kernel);
void wrap_up(kthread_t *kernels, unsigned kernels_num);

#endif

