#include "kthreads.h"

#include <assert.h>
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

extern pthread_mutex_t printf_mutex;

/* fifo_t helpers */
void lock_fifo(fifo_t *fifo)
{ 
  assert(fifo);
  int ret = pthread_mutex_lock(&fifo->mutex);
  assert(ret==0);
} 

void unlock_fifo(fifo_t *fifo)
{ 
  assert(fifo);
  int ret = pthread_mutex_unlock(&fifo->mutex);
  assert(ret==0);
} 

bool is_full(fifo_t *fifo)
{ return fifo->size == fifo->capacity; }

bool is_empty(fifo_t *fifo)
{ return fifo->size == 0; }

bool has_consumers(fifo_t *fifo) 
{ return (fifo->consumers > 0); }

void cond_wait_full(fifo_t *fifo)
{ pthread_cond_wait(&fifo->cond_full, &fifo->mutex); }

void cond_wait_empty(fifo_t *fifo)
{ pthread_cond_wait(&fifo->cond_empty, &fifo->mutex); }

void cond_wait_update(fifo_t *fifo)
{ pthread_cond_wait(&fifo->cond_update, &fifo->mutex); }

void wake_up_empty(fifo_t *fifo)
{ pthread_cond_broadcast(&fifo->cond_empty); }

void wake_up_update(fifo_t *fifo)
{ pthread_cond_broadcast(&fifo->cond_update); }

void wake_up_full(fifo_t *fifo)
{ pthread_cond_broadcast(&fifo->cond_full); }

unsigned get_current_element_id(fifo_t *fifo)
{ return fifo->element_ids[fifo->out]; }

#if 0
void set_current_element_id(fifo_t *fifo, unsigned element_id)
{ 
  fifo->element_ids[fifo->out] = element_id;
  fifo->element_id_counter = element_id;
}
#endif

/* kthread_t helpers */

void lock_kernel(kthread_t *kernel)
{  pthread_mutex_lock(&kernel->mutex); }

void unlock_kernel(kthread_t *kernel)
{ pthread_mutex_unlock(&kernel->mutex); }

void enable_feedback(kthread_t *kernel, unsigned index)
{ kernel->feedback[index] = 1; }

void disable_feedback(kthread_t *kernel, unsigned index)
{ kernel->feedback[index] = 0; }

unsigned is_feedback(kthread_t *kernel, unsigned index)
{ return kernel->feedback[index]; }

unsigned get_previous_element_id(kthread_t *kernel, unsigned index)
{ return kernel->fifo_prev_element_ids[index]; }

void set_previous_element_id(kthread_t *kernel, unsigned index, unsigned element_id)
{ kernel->fifo_prev_element_ids[index] = element_id; }

void dump_topology(kthread_t *ktreads, unsigned num_ktreads)
{
  unsigned i, j;
  for (i = 0; i < num_ktreads; i++)
  {
    printf ("[%s]\n", ktreads[i].id);
    printf ("   first    %d\n", ktreads[i].reset); 
    printf ("   incoming %d {", ktreads[i].incoming);
    for (j = 0; j < ktreads[i].incoming; j++)
    {
      printf ("%s (%d), ", ktreads[i].incoming_threads[j]->id, ktreads[i].feedback[j]);
    }
    printf ("}\n   fifo\n");
    printf ("     capacity %d\n", ktreads[i].fifo.capacity);
    printf ("     size %d\n", ktreads[i].fifo.size);
    printf ("     outgoing %d\n", ktreads[i].fifo.consumers);
    printf ("     in (PUT) %d\n", ktreads[i].fifo.in);
    printf ("     out (GET) %d\n", ktreads[i].fifo.out);
  }
}

void dump_dgraph(kthread_t *ktreads, unsigned num_ktreads, char *filename)
{
  unsigned i, j;

  FILE *fp = fopen(filename, "w");
  fprintf(fp, "digraph G {\n");

  for (i = 0; i < num_ktreads; i++)
  {
    for (j = 0; j < ktreads[i].incoming; j++)
    {
      fprintf(fp, "  %s -> %s", ktreads[i].incoming_threads[j]->id, ktreads[i].id);
      if (ktreads[i].feedback[j]) fprintf(fp, " [color=red]");
      fprintf(fp, ";\n");
    }
  }
  fprintf(fp, "}\n");

  fclose(fp);
}

static inline unsigned long long tick() 
{
  unsigned long long d;
  __asm__ __volatile__ ("rdtsc" : "=A" (d) );
  return d;
}

/* 
 * The function returns the PUT-index (i.e. first available slot) as soon as
 * there is an available slot (i.e. it may wait for that).
 */
unsigned can_put(kthread_t* kernel)
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

void put(kthread_t* kernel)
{
  fifo_t *kernel_fifo = &kernel->fifo;

  lock_fifo(kernel_fifo);

  /* FIFO size has to be less or equal the capacity! */
  assert (kernel_fifo->size <= kernel_fifo->capacity);
  assert (kernel_fifo->in < kernel_fifo->capacity);
  assert (kernel_fifo->out < kernel_fifo->capacity);

  /* Store the element-id. */
  kernel_fifo->element_ids[kernel_fifo->in] = ++kernel_fifo->element_id_counter;

  /* Update the PUT-index of the FIFO (circular buffer). */
  ++ (kernel_fifo->in);
  kernel_fifo->in %= kernel_fifo->capacity;

  /* Update the FIFO size. */
  ++ (kernel_fifo->size);

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
unsigned can_get(kthread_t* source, kthread_t* kernel, unsigned index)
{
  fifo_t *source_fifo = &source->fifo;
  fifo_t *kernel_fifo = &kernel->fifo;

  lock_kernel(kernel);
  unsigned feedback = is_feedback(kernel, index);
  unlock_kernel(kernel);

  lock_fifo(source_fifo);

//  assert (kernel_fifo->in < kernel_fifo->capacity);
//  assert (kernel_fifo->out < kernel_fifo->capacity);

  /* If there is an element (at least) but it is already consumed, wait for a get! */
  while ((!is_empty(source_fifo)) && get_previous_element_id(kernel, index) == get_current_element_id(source_fifo))
    cond_wait_update(source_fifo);
  /* If empty, wait for a put! */
  while (is_empty(source_fifo) && !feedback)
    cond_wait_empty(source_fifo);

  /* Return the GET-index (i.e. first available element in the FIFO - head). */ 
  unsigned get_index = source_fifo->out;
  eid_t element_id = get_current_element_id(source_fifo);

  unlock_fifo(source_fifo);

  lock_fifo(kernel_fifo);
  set_previous_element_id(kernel, index, element_id);
  unlock_fifo(kernel_fifo);

//  /* One time only for feedback */
//  lock_kernel(kernel);
//  disable_feedback(kernel, index);
//  unlock_kernel(kernel);
//
  return get_index;
}

void get(kthread_t* source, kthread_t* kernel, unsigned index)
{
  fifo_t *source_fifo = &source->fifo; 
//  fifo_t *kernel_fifo = &kernel->fifo;

  /* One time only for feedback */
  lock_kernel(kernel);
  unsigned feedback = is_feedback(kernel, index);
  disable_feedback(kernel, index);
  unlock_kernel(kernel);

  if (feedback) return;

  lock_fifo(source_fifo);
//  assert (kernel_fifo->in < kernel_fifo->capacity);
//  assert (kernel_fifo->out < kernel_fifo->capacity);

  /* Update the number of times the element is consumed. */
  ++ (source_fifo->consumed);

  /* Pop the element when all of the consumers have done. */
  if (source_fifo->consumed == source_fifo->consumers)
  {
    /* decrement the size of the FIFO */
    assert (source_fifo->size != 0);
    -- (source_fifo->size);

    /* Reset consumer counter. */
    source_fifo->consumed = 0;

    /* Update the FIFO OUT-index (circular buffer). */
    ++ (source_fifo->out);
    source_fifo->out %= source_fifo->capacity;

    /* Wake-up all of the ktreads waiting for the next element (can_get). */   
    wake_up_update(source_fifo);

    /* Wake-up all of the ktreads waiting for a space to put (can_put) */
    wake_up_full(source_fifo);
  }

  unlock_fifo(source_fifo);
}

void* thread_function(void* kernel_ptr)
{
  unsigned index;
  struct timespec zero;
  struct timespec start, end;
  uint64_t start_diff, end_diff;

  kthread_t* kernel = (kthread_t*) kernel_ptr;
  assert(kernel != NULL);
  assert(kernel->id != NULL);

  /* Thread up and running! */
  pthread_mutex_lock(&tcount_mutex);
  tcount++;
  pthread_cond_signal(&tcount_ready);
  pthread_mutex_unlock(&tcount_mutex);

  /* First kernel wait for start. */
  pthread_mutex_lock(&kernel->mutex);
  while (kernel->reset) 
  {
    pthread_cond_wait(&kernel->ready, &kernel->mutex);
    if (kernel->reset) break;
  }
  pthread_mutex_unlock(&kernel->mutex);

  clock_gettime(CLOCK_MONOTONIC, &zero); /* mark zero time */

  /* Alive for many iterations. */
  while (1)
  {
    /* At each iteration:
     * - for_each incoming-kernel queues can_get
     * - can_put on the local queue
     * - do something
     * - for_each incoming-kernel queues get
     * - put on the local queue
     */

    /* TODO: This works under the assumption that each accelerator consume the
     * same number of frames. */
    if (kernel->iter_count >= kernel->iter_max) break;

    for (index = 0; index < kernel->incoming; index++)
    {
      kthread_t* source = kernel->incoming_threads[index];
      assert(source != NULL);
      kernel->fifo_get_index[index] = can_get(source, kernel, index);
    }
    unsigned put_index = can_put(kernel);

    /* mark start time */
    clock_gettime(CLOCK_MONOTONIC, &start);	
    start_diff = BILLION * (start.tv_sec - zero.tv_sec) + start.tv_nsec - zero.tv_nsec;

#ifdef PROFILING
    pthread_mutex_lock(&printf_mutex);
    printf("@ %llu %s BEG [ iter = %u ] in:{", start_diff, kernel->id, kernel->iter_count);
    for (index = 0; index < kernel->incoming; index++)
    {
      kthread_t* source = kernel->incoming_threads[index];
      printf(" [ %d ] = %lld ", kernel->fifo_get_index[index], source->fifo.element_ids[kernel->fifo.out]);
    }
    printf(" }\n");
    pthread_mutex_unlock(&printf_mutex);
#endif
    usleep(kernel->execution_time);

    /* mark the end time */
    clock_gettime(CLOCK_MONOTONIC, &end);	
    end_diff = BILLION * (end.tv_sec - zero.tv_sec) + end.tv_nsec - zero.tv_nsec;

    for (index = 0; index < kernel->incoming; index++)
    {
      kthread_t* source = kernel->incoming_threads[index];
      get(source, kernel, index);
    }
    put(kernel);

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

void start(kthread_t* kernel, unsigned reset_index)
{
  pthread_mutex_lock(&kernel[reset_index].mutex);
  pthread_cond_signal(&kernel[reset_index].ready);
  pthread_mutex_unlock(&kernel[reset_index].mutex);
}

void assert_reset_kernel(kthread_t* kernel, unsigned ktreads_num)
{
  unsigned index, rcount = 0;
  for (index = 0; index < ktreads_num; index++)
    if (kernel[index].reset) rcount++;
  assert(rcount == 1 && "One and only one reset kernel has to be set!");
}

void set_reset_kernel(kthread_t* ktreads, unsigned ktreads_num, unsigned reset_index)
{
  ktreads[reset_index].reset = 1; 

  /* check that not more than one kernel is a reset kernel */
  assert_reset_kernel(ktreads, ktreads_num);
}

void set_iteration_counters(kthread_t* kernel, unsigned ktreads_num, unsigned count)
{
  unsigned index;
  for (index = 0; index < ktreads_num; index++)
    kernel[index].iter_max = count;
}

void reset_fifo_prev_element_ids(kthread_t* kernel)
{
  unsigned index;
  for (index = 0; index < INCOMING_MAX_COUNT; index++)
    kernel->fifo_prev_element_ids[index] = 0;
}

void init_fifo(kthread_t* kernel)
{
  kernel->fifo.size = 0;
  kernel->fifo.in = 0;
  kernel->fifo.out = 0;
  kernel->fifo.element_id_counter = 0;

  int err;
  pthread_mutexattr_t mta;

  err = pthread_mutexattr_init(&mta);
  if (err) 
  {
    exit(1);
  }
  err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
  if (err) {
    exit(1); 
  }

  assert(kernel);
  int ret;
  ret = pthread_mutex_init(&kernel->fifo.mutex, &mta);
  assert(ret==0);
  ret = pthread_cond_init(&kernel->fifo.cond_full, NULL);
  assert(ret==0);
  ret = pthread_cond_init(&kernel->fifo.cond_empty, NULL);
  assert(ret==0);
  ret = pthread_cond_init(&kernel->fifo.cond_update, NULL);
  assert(ret==0);
  pthread_mutexattr_destroy(&mta);
}

void init_feedback(kthread_t* kernel)
{
  unsigned i;
  for (i = 0; i < kernel->incoming; i++)
  {
    if (kernel->feedback[i])
    {
//      set_current_element_id(&(kernel->incoming_threads[i]->fifo), 1);
//      kernel->incoming_threads[i]->fifo.in = 1;
//      kernel->incoming_threads[i]->fifo.in %= kernel->incoming_threads[i]->fifo.capacity;
    }
  }
}

void wrap_up(kthread_t *ktreads, unsigned ktreads_num)
{
  unsigned i, j;
  for (i = 0; i < ktreads_num; i++)
  {
    int ret;
    ret = pthread_mutex_destroy(&ktreads[i].mutex);
    assert(ret==0);
    for (j = 0; j < ktreads[i].incoming; j++)
    {
      ret = pthread_mutex_destroy(&ktreads[i].incoming_threads[j]->fifo.mutex);
      assert(ret==0);
    }
  }
}
