/*
 * This is a small example showing the thread-based approach. Each thread wraps
 * a ioctl to an accelerator (I hereby use a sleep). The threads are forever
 * alive. They are interconnected as a graph.
 *
 *
 * For running it "on bertha":
 *
 * gcc -std=gnu99 -O0 -g kthreads.c test_05.c -lpthread -o kthreads
 * sparc-leon3-linux-gcc kthreads.c test_05.c -lpthread -std=c99 -o kthreads
 */

#include "kthreads.h"

#define KERNELS_NUM 8

#ifndef FRAMES_NUM
#warning "FRAMES_NUM is not defined! Set FRAMES_NUM to 3."
#define FRAMES_NUM 3
#endif

#ifndef QUEUE_CAPACITY
#warning "QUEUE_CAPACITY is not defined! Set QUEUE_CAPACITY to 1"
#define QUEUE_CAPACITY 1
#endif

#define A_EXEC_TIME 10000.000
#define B_EXEC_TIME 10000.000
#define C_EXEC_TIME 10000.000
#define D_EXEC_TIME 10000.000
#define E_EXEC_TIME 10000.000
#define F_EXEC_TIME 10000.000
#define G_EXEC_TIME 10000.000
#define H_EXEC_TIME 10000.000

static kthread_t ktreads[KERNELS_NUM];
pthread_mutex_t printf_mutex;

int main(int argc, char **argv)
{
  unsigned i;

  pthread_mutex_init(&printf_mutex, NULL);

  setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
  printf("Frame count # %d\n", FRAMES_NUM);
  printf("Queue capacity # %d\n", QUEUE_CAPACITY);
  printf("Thread number # %d\n", KERNELS_NUM);

  srand(0);

  printf("KERNEL SIZE %zu\n", sizeof(ktreads));
  memset(ktreads, 0, sizeof(ktreads));

  /*       
   *      v---------,    v---------,
   * A -> B -> C -> D -> E -> F -> G -> H
   *   
   */

  ktreads[0].id = "A";
  ktreads[0].execution_time = A_EXEC_TIME;
  ktreads[0].fifo.capacity = QUEUE_CAPACITY;
  ktreads[0].fifo.consumers = 1; /* B */
  ktreads[0].incoming = 0;       /* SOURCE! */
  ktreads[0].incoming_threads = (kthread_t*[]) { };
  ktreads[0].feedback = (unsigned[]) {};

  ktreads[1].id = "B";
  ktreads[1].execution_time = B_EXEC_TIME;
  ktreads[1].fifo.capacity = QUEUE_CAPACITY;
  ktreads[1].fifo.consumers = 1; /* C */
  ktreads[1].incoming = 2;       /* A, D */
  ktreads[1].incoming_threads = (kthread_t*[]) { &ktreads[0], &ktreads[3] };
  ktreads[1].feedback = (unsigned[]) {0, 1};

  ktreads[2].id = "C";
  ktreads[2].execution_time = C_EXEC_TIME;
  ktreads[2].fifo.capacity = QUEUE_CAPACITY;
  ktreads[2].fifo.consumers = 1; /* D */
  ktreads[2].incoming = 1;       /* B */
  ktreads[2].incoming_threads = (kthread_t*[]) { &ktreads[1] };
  ktreads[2].feedback = (unsigned[]) {0};

  ktreads[3].id = "D";
  ktreads[3].execution_time = D_EXEC_TIME;
  ktreads[3].fifo.capacity = QUEUE_CAPACITY;
  ktreads[3].fifo.consumers = 2; /* E, B */
  ktreads[3].incoming = 1;       /* C */
  ktreads[3].incoming_threads = (kthread_t*[]) { &ktreads[2] };
  ktreads[3].feedback = (unsigned[]) {0};

  ktreads[4].id = "E";
  ktreads[4].execution_time = E_EXEC_TIME;
  ktreads[4].fifo.capacity = QUEUE_CAPACITY;
  ktreads[4].fifo.consumers = 1; /* F */
  ktreads[4].incoming = 2;       /* D, G */
  ktreads[4].incoming_threads = (kthread_t*[]) { &ktreads[3], &ktreads[6] };
  ktreads[4].feedback = (unsigned[]) {0, 1};

  ktreads[5].id = "F";
  ktreads[5].execution_time = F_EXEC_TIME;
  ktreads[5].fifo.capacity = QUEUE_CAPACITY;
  ktreads[5].fifo.consumers = 1; /* G */
  ktreads[5].incoming = 1;       /* E */
  ktreads[5].incoming_threads = (kthread_t*[]) { &ktreads[4] };
  ktreads[5].feedback = (unsigned[]) {0};

  ktreads[6].id = "G";
  ktreads[6].execution_time = G_EXEC_TIME;
  ktreads[6].fifo.capacity = QUEUE_CAPACITY;
  ktreads[6].fifo.consumers = 2; /* H, E */
  ktreads[6].incoming = 1;       /* F */
  ktreads[6].incoming_threads = (kthread_t*[]) { &ktreads[5] };
  ktreads[6].feedback = (unsigned[]) {0};

  ktreads[7].id = "H";
  ktreads[7].execution_time = H_EXEC_TIME;
  ktreads[7].fifo.capacity = FRAMES_NUM;
  ktreads[7].fifo.consumers = 0; /* SINK! */
  ktreads[7].incoming = 1;       /* G */
  ktreads[7].incoming_threads = (kthread_t*[]) { &ktreads[6] };
  ktreads[7].feedback = (unsigned[]) {0};



  /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

  /* init reset kernel */ 
  set_reset_kernel(ktreads, KERNELS_NUM, 0);

  /* init number of iterations */
  set_iteration_counters(ktreads, KERNELS_NUM, FRAMES_NUM);

  /* init counters, mutexes and conditional variables */
  for (i = 0; i < KERNELS_NUM; i++)
  {
    int err;
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
//  for (i = 0; i < KERNELS_NUM; i++)
//  { init_feedback(&ktreads[i]); }
  tcount = 0;
  pthread_mutex_init(&tcount_mutex, NULL);
  pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM);
  return 0;
#endif

  dump_dgraph(ktreads, KERNELS_NUM, "graph.dot");

  /* create threads */
  for (i = 0; i < KERNELS_NUM; i++)
    pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

  /* wait that all of the threads are up and running */
  pthread_mutex_lock(&tcount_mutex);
  while (tcount < KERNELS_NUM) 
    pthread_cond_wait(&tcount_ready, &tcount_mutex);
  tcount = 0;
  pthread_mutex_unlock(&tcount_mutex);

  /* run! */
  start(ktreads, 0);

  /* wait and dispose of all of the threads */
  for (i = 0; i < KERNELS_NUM; i++)
    pthread_join(ktreads[i].handle, NULL);

  wrap_up(ktreads, KERNELS_NUM);

  return 0;
}
