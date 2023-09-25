/*
 * This is a small example showing the thread-based approach. Each thread wraps
 * a ioctl to an accelerator (I hereby use a sleep). The threads are forever
 * alive. They are interconnected as a graph.
 *
 *
 * For running it "on bertha":
 *
 * gcc -std=gnu99 -O0 -g kthreads.c test_wami.c -lpthread -o kthreads
 * sparc-leon3-linux-gcc kthreads.c test_wami.c -lpthread -std=c99 -o kthreads
 */

#include "kthreads.h"

#define KERNELS_NUM 16

#define FRAMES_NUM 1

#ifndef FRAMES_NUM
#warning "FRAMES_NUM is not defined! Set FRAMES_NUM to 3."
#define FRAMES_NUM 3
#endif

#ifndef QUEUE_CAPACITY
#warning "QUEUE_CAPACITY is not defined! Set QUEUE_CAPACITY to 1"
#define QUEUE_CAPACITY 1
#endif

#if 1
#if 1
/* These values are from RTL simulation */
// Test Time
/*
#define DEBAYER_EXEC_TIME 23720832
#define GRAYSCALE_EXEC_TIME 10409267
#define GRADIENT_EXEC_TIME 14102093
#define WARP_GRAYSCALE_EXEC_TIME 17884544
#define SUBTRACT_EXEC_TIME 11672627
#define WARP_DX_EXEC_TIME 16098125
#define WARP_DY_EXEC_TIME 18241715
#define STEEPEST_DESCENT_EXEC_TIME 20716467
#define HESSIAN_EXEC_TIME 35259930
#define INVERT_GAUSS_JORDAN_EXEC_TIME 8794931
#define SD_UPDATE_EXEC_TIME 20700186
#define MULT_EXEC_TIME 8372121
#define RESHAPE_EXEC_TIME 8579020
#define ADD_EXEC_TIME 8716416
#define WARP_IWXP_EXEC_TIME 13288064
#define CHANGE_DETECTION_EXEC_TIME 100
*/
// ACC time (ms = ns/1000)
#define DEBAYER_EXEC_TIME 10119501/1000.0
#define GRAYSCALE_EXEC_TIME 3165824/1000.0
#define GRADIENT_EXEC_TIME 4402023/1000.0
#define WARP_GRAYSCALE_EXEC_TIME 9491635/1000.0
#define SUBTRACT_EXEC_TIME 3662233/1000.0
#define WARP_DX_EXEC_TIME 8505293/1000.0
#define WARP_DY_EXEC_TIME 8576665/1000.0
#define STEEPEST_DESCENT_EXEC_TIME 12864921/1000.0
#define HESSIAN_EXEC_TIME 29555507/1000.0
#define INVERT_GAUSS_JORDAN_EXEC_TIME 1001088/1000.0
#define SD_UPDATE_EXEC_TIME 12812724/1000.0
#define MULT_EXEC_TIME 1021056/1000.0
#define RESHAPE_EXEC_TIME 760883/1000.0
#define ADD_EXEC_TIME 731034/1000.0
#define WARP_IWXP_EXEC_TIME 8521191/1000.0
#define CHANGE_DETECTION_EXEC_TIME 100
#elif 0
/* Random values */
#define RAND rand()%100000
#define DEBAYER_EXEC_TIME RAND
#define GRAYSCALE_EXEC_TIME RAND
#define GRADIENT_EXEC_TIME RAND
#define WARP_GRAYSCALE_EXEC_TIME RAND
#define SUBTRACT_EXEC_TIME RAND
#define WARP_DX_EXEC_TIME RAND
#define WARP_DY_EXEC_TIME RAND
#define STEEPEST_DESCENT_EXEC_TIME RAND
#define HESSIAN_EXEC_TIME RAND
#define INVERT_GAUSS_JORDAN_EXEC_TIME RAND
#define SD_UPDATE_EXEC_TIME RAND
#define MULT_EXEC_TIME RAND
#define RESHAPE_EXEC_TIME RAND
#define ADD_EXEC_TIME RAND
#define WARP_IWXP_EXEC_TIME RAND
#define CHANGE_DETECTION_EXEC_TIME RAND
#else
/* Constant values */
#define CONST 100000
#define DEBAYER_EXEC_TIME CONST
#define GRAYSCALE_EXEC_TIME CONST
#define GRADIENT_EXEC_TIME CONST
#define WARP_GRAYSCALE_EXEC_TIME CONST
#define SUBTRACT_EXEC_TIME CONST
#define WARP_DX_EXEC_TIME CONST
#define WARP_DY_EXEC_TIME CONST
#define STEEPEST_DESCENT_EXEC_TIME CONST
#define HESSIAN_EXEC_TIME CONST
#define INVERT_GAUSS_JORDAN_EXEC_TIME CONST
#define SD_UPDATE_EXEC_TIME CONST
#define MULT_EXEC_TIME CONST
#define RESHAPE_EXEC_TIME CONST
#define ADD_EXEC_TIME CONST
#define WARP_IWXP_EXEC_TIME CONST
#define CHANGE_DETECTION_EXEC_TIME CONST
#endif
#endif

static kthread_t ktreads[KERNELS_NUM];
pthread_mutex_t printf_mutex;

int main(int argc, char **argv)
{
  unsigned i;

#if 0
  if (argc != 13)
  { 
    fprintf(stderr, "Wrong number of parameters: %d (expected 12)\n", argc-1);
    return(1);
  }

  const float DIV_FACTOR = 10000;
  float DEBAYER_EXEC_TIME = atof(argv[1])/DIV_FACTOR;
  float GRAYSCALE_EXEC_TIME = atof(argv[2])/DIV_FACTOR;
  float WARP_GRAYSCALE_EXEC_TIME = atof(argv[3])/DIV_FACTOR;
  float GRADIENT_EXEC_TIME = atof(argv[4])/DIV_FACTOR;
  float SUBTRACT_EXEC_TIME = atof(argv[5])/DIV_FACTOR;
  float WARP_DX_EXEC_TIME = atof(argv[3])/DIV_FACTOR;
  float WARP_DY_EXEC_TIME = atof(argv[3])/DIV_FACTOR;
  float STEEPEST_DESCENT_EXEC_TIME = atof(argv[6])/DIV_FACTOR;
  float SD_UPDATE_EXEC_TIME = atof(argv[7])/DIV_FACTOR;
  float HESSIAN_EXEC_TIME = atof(argv[8])/DIV_FACTOR;
  float INVERT_GAUSS_JORDAN_EXEC_TIME = 355000/DIV_FACTOR;
  float MULT_EXEC_TIME = atof(argv[9])/DIV_FACTOR;
  float RESHAPE_EXEC_TIME = atof(argv[10])/DIV_FACTOR;
  float ADD_EXEC_TIME = atof(argv[11])/DIV_FACTOR;
  float WARP_IWXP_EXEC_TIME = atof(argv[3])/DIV_FACTOR;
  float CHANGE_DETECTION_EXEC_TIME = atof(argv[12])/DIV_FACTOR;
#endif 

  pthread_mutex_init(&printf_mutex, NULL);

  setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
  printf("Frame count # %d\n", FRAMES_NUM);
  printf("Queue capacity # %d\n", QUEUE_CAPACITY);
  printf("Thread number # %d\n", KERNELS_NUM);

  srand(time(NULL));

  printf("KERNEL SIZE %zu\n", sizeof(ktreads));
  memset(ktreads, 0, sizeof(ktreads));


  ktreads[0].id = "debayer";
  ktreads[0].execution_time = DEBAYER_EXEC_TIME;
  ktreads[0].fifo.capacity = QUEUE_CAPACITY;
  ktreads[0].fifo.consumers = 1; /* grayscale */
  ktreads[0].incoming = 0; /* SOURCE! */
  ktreads[0].incoming_threads = (kthread_t*[]) { };
  ktreads[0].feedback = (unsigned[]) {};

  ktreads[1].id = "grayscale";
  ktreads[1].execution_time = GRAYSCALE_EXEC_TIME;
  ktreads[1].fifo.capacity = QUEUE_CAPACITY;
  ktreads[1].fifo.consumers = 3; /* gradient, warp_grayscale, warp_iwxp */
  ktreads[1].incoming = 1; /* debayer */
  ktreads[1].incoming_threads = (kthread_t*[]) { &ktreads[0] };
  ktreads[1].feedback = (unsigned[]) {0};

  ktreads[2].id = "gradient";
  ktreads[2].execution_time = GRADIENT_EXEC_TIME;
  ktreads[2].fifo.capacity = QUEUE_CAPACITY;
  ktreads[2].fifo.consumers = 2; /* warp_dx, warp_dy */
  ktreads[2].incoming = 1; /* grayscale */
  ktreads[2].incoming_threads = (kthread_t*[]) { &ktreads[1] };
  ktreads[2].feedback = (unsigned[]) {0};

  ktreads[3].id = "warp_grayscale";
  ktreads[3].execution_time = WARP_GRAYSCALE_EXEC_TIME;
  ktreads[3].fifo.capacity = QUEUE_CAPACITY;
  ktreads[3].fifo.consumers = 1; /* subtract */
  ktreads[3].incoming = 2; /* grayscale, add */
  ktreads[3].incoming_threads = (kthread_t*[]) { &ktreads[1], &ktreads[13] };
  ktreads[3].feedback = (unsigned[]) {0, 1};

  ktreads[4].id = "subtract";
  ktreads[4].execution_time = SUBTRACT_EXEC_TIME;
  ktreads[4].fifo.capacity = QUEUE_CAPACITY;
  ktreads[4].fifo.consumers = 1; /* sd_update */
  ktreads[4].incoming = 2; /* warp_grayscale, warp_iwxp */
  ktreads[4].incoming_threads = (kthread_t*[]) { &ktreads[3], &ktreads[14] };
  ktreads[4].feedback = (unsigned[]) {0, 1};
 
  ktreads[5].id = "warp_dx";
  ktreads[5].execution_time = WARP_DX_EXEC_TIME;
  ktreads[5].fifo.capacity = QUEUE_CAPACITY;
  ktreads[5].fifo.consumers = 1; /* steepest_descent */
  ktreads[5].incoming = 2; /* gradient, add */
  ktreads[5].incoming_threads = (kthread_t*[]) { &ktreads[2], &ktreads[13] };
  ktreads[5].feedback = (unsigned[]) {0, 1};
 
  ktreads[6].id = "warp_dy";
  ktreads[6].execution_time = WARP_DY_EXEC_TIME;
  ktreads[6].fifo.capacity = QUEUE_CAPACITY;
  ktreads[6].fifo.consumers = 1; /* steepest_descent */
  ktreads[6].incoming = 2; /* gradient, add */
  ktreads[6].incoming_threads = (kthread_t*[]) { &ktreads[2], &ktreads[13] };
  ktreads[6].feedback = (unsigned[]) {0, 1};
 
  ktreads[7].id = "steepest_descent";
  ktreads[7].execution_time = STEEPEST_DESCENT_EXEC_TIME;
  ktreads[7].fifo.capacity = QUEUE_CAPACITY;
  ktreads[7].fifo.consumers = 2; /* sd_update, hessian */
  ktreads[7].incoming = 2; /* warp_dx, warp_dy */
  ktreads[7].incoming_threads = (kthread_t*[]) { &ktreads[5], &ktreads[6] };
  ktreads[7].feedback = (unsigned[]) {0, 0};
 
  ktreads[8].id = "sd_update";
  ktreads[8].execution_time = SD_UPDATE_EXEC_TIME;
  ktreads[8].fifo.capacity = QUEUE_CAPACITY;
  ktreads[8].fifo.consumers = 1; /* mult */
  ktreads[8].incoming = 2; /* subtract, steepest_descent */
  ktreads[8].incoming_threads = (kthread_t*[]) { &ktreads[4], &ktreads[7] };
  ktreads[8].feedback = (unsigned[]) {0, 0};
 
  ktreads[9].id = "hessian";
  ktreads[9].execution_time = HESSIAN_EXEC_TIME;
  ktreads[9].fifo.capacity = QUEUE_CAPACITY;
  ktreads[9].fifo.consumers = 1; /* invert_gauss_jordan */
  ktreads[9].incoming = 1; /* steepest_descent */
  ktreads[9].incoming_threads = (kthread_t*[]) { &ktreads[7] };
  ktreads[9].feedback = (unsigned[]) {0};
 
  ktreads[10].id = "invert_gauss_jordan";
  ktreads[10].execution_time = INVERT_GAUSS_JORDAN_EXEC_TIME;
  ktreads[10].fifo.capacity = QUEUE_CAPACITY;
  ktreads[10].fifo.consumers = 1; /* mult */
  ktreads[10].incoming = 1; /* hessian */
  ktreads[10].incoming_threads = (kthread_t*[]) { &ktreads[9] };
  ktreads[10].feedback = (unsigned[]) {0};
 
  ktreads[11].id = "mult";
  ktreads[11].execution_time = MULT_EXEC_TIME;
  ktreads[11].fifo.capacity = QUEUE_CAPACITY;
  ktreads[11].fifo.consumers = 1; /* reshape */
  ktreads[11].incoming = 2; /* sd_update, invert_gauss_jordan */
  ktreads[11].incoming_threads = (kthread_t*[]) { &ktreads[8], &ktreads[10] };
  ktreads[11].feedback = (unsigned[]) {0, 0};
 
  ktreads[12].id = "reshape";
  ktreads[12].execution_time = RESHAPE_EXEC_TIME;
  ktreads[12].fifo.capacity = QUEUE_CAPACITY;
  ktreads[12].fifo.consumers = 1; /* add */
  ktreads[12].incoming = 1; /* mult */
  ktreads[12].incoming_threads = (kthread_t*[]) { &ktreads[11] };
  ktreads[12].feedback = (unsigned[]) {0};
 
  ktreads[13].id = "add";
  ktreads[13].execution_time = ADD_EXEC_TIME;
  ktreads[13].fifo.capacity = QUEUE_CAPACITY;
  ktreads[13].fifo.consumers = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
  ktreads[13].incoming = 1; /* reshape */
  ktreads[13].incoming_threads = (kthread_t*[]) { &ktreads[12] };
  ktreads[13].feedback = (unsigned[]) {0};
 
  ktreads[14].id = "warp_iwxp";
  ktreads[14].execution_time = WARP_IWXP_EXEC_TIME;
  ktreads[14].fifo.capacity = QUEUE_CAPACITY;
  ktreads[14].fifo.consumers = 2; /* subtract, change_detection */
  ktreads[14].incoming = 2; /* grayscale, add */
  ktreads[14].incoming_threads = (kthread_t*[]) { &ktreads[1], &ktreads[13] };
  ktreads[14].feedback = (unsigned[]) {0, 0};

  ktreads[15].id = "change_detection";
  ktreads[15].execution_time = CHANGE_DETECTION_EXEC_TIME;
  ktreads[15].fifo.capacity = FRAMES_NUM;
  ktreads[15].fifo.consumers = 0; /* SINK! */
  ktreads[15].incoming = 1;
  ktreads[15].incoming_threads = (kthread_t*[]) { &ktreads[14] };
  ktreads[15].feedback = (unsigned[])  {0};

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
  for (i = 0; i < KERNELS_NUM; i++)
  { init_feedback(&ktreads[i]); }
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

  struct timespec time_start, time_end;
  uint64_t time_diff;
  clock_gettime(CLOCK_MONOTONIC, &time_start); /* mark start time */

  /* run! */
  start(ktreads, 0);

  /* wait and dispose of all of the threads */
  for (i = 0; i < KERNELS_NUM; i++)
    pthread_join(ktreads[i].handle, NULL);

  wrap_up(ktreads, KERNELS_NUM);

  clock_gettime(CLOCK_MONOTONIC, &time_end); /* mark the end time */
  time_diff = BILLION * (time_end.tv_sec - time_start.tv_sec) + time_end.tv_nsec - time_start.tv_nsec;
  fprintf(stderr, "multi-thread time: %llu\n", time_diff);

  return 0;
}
