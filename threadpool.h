#ifndef THREAD_POOL_H_HEADER_INCLUDED
#define THREAD_POOL_H_HEADER_INCLUDED
#include <pthread.h>

// ------------------------------ Types ---------------------------------------
typedef void (*task_fun_t)(void *);

// TODO use tagged union for pool with different arg types
enum ARG_TYPE { HEAP, STACK };

typedef struct arg_ {
  enum ARG_TYPE arg_type;
  union {
    void *ptr;       // arg resides on the heap
    long long value; // arg on stack
  };
} arg_t;

typedef struct thread_task {
  task_fun_t func;
  void *arg;
} task_t;

typedef struct thread_pool {
  // synchronization
  pthread_mutex_t queue_lock;
  pthread_cond_t queue_not_empty;
  pthread_cond_t queue_not_full;
  pthread_mutex_t working_lock;

  // threads info
  pthread_t *workers; // array of threads
  pthread_t admin;
  size_t working_thread_num;
  size_t max_thread_num;
  size_t min_thread_num;

  // circle list as task queue
  int heap_args;
  task_t *task_queue;
  size_t max_queue_size;
  size_t queue_size;
  size_t head; // linked list of work submitted to the pool
  size_t tail;

  // shutdown
  int shutdown;
} thread_pool_t;

// --------------------------- pool routines ----------------------------------
thread_pool_t *thread_pool_create(size_t thread_num, size_t max_task_num,
                                  int heap_args);
int thread_pool_submit(thread_pool_t *pool, task_fun_t func, void *arg);
int thread_pool_destroy(thread_pool_t *pool);

#endif // THREAD_POOL_H_HEADER_INCLUDED
