#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadpool.h"

// --------------------------- thread routines --------------------------------
static void *pool_worker(void *arg) {
  thread_pool_t *pool = (thread_pool_t *)arg;
  task_t task;
  while (1) {
    pthread_mutex_lock(&pool->queue_lock);
    while (pool->queue_size == 0 && !pool->shutdown) {
      pthread_cond_wait(&pool->queue_not_empty, &pool->queue_lock);
    }

    if (pool->shutdown) {
      pthread_mutex_unlock(&pool->queue_lock);
      pthread_exit(NULL);
    }

    task.func = pool->task_queue[pool->head].func;
    task.arg = pool->task_queue[pool->head].arg;

    // set extracted task to NULL immediate so that no other thread can gain
    // access to extracted args.

    // Tasks are freed in several places
    //
    // 1. after task is executed in worker thread
    // 2. after calling pool_destroy.
    // This causes complication, destroy function should wait until
    // all workers exits, then free the remaining non-NULL args.

    // A more reasonable & flexible approach is to ask the user to free `arg` in
    // the `func` they submitted.
    pool->task_queue[pool->head].arg = NULL;
    pool->task_queue[pool->head].func = NULL;

    pool->head = (pool->head + 1) % pool->max_queue_size;
    pool->queue_size -= 1;
    printf("thread id: %ld, remaining tasks: %ld\n",
           (unsigned long int)(pthread_self()), pool->queue_size);
    pthread_cond_broadcast(&pool->queue_not_full);
    pthread_mutex_unlock(&pool->queue_lock);

    // execute task
    pthread_mutex_lock(&pool->working_lock);
    pool->working_thread_num += 1;
    pthread_mutex_unlock(&pool->working_lock);

    // puts("1\n");
    (*(task.func))(task.arg);
    if (pool->heap_args)
      free(task.arg);
    // puts("2\n");

    pthread_mutex_lock(&pool->working_lock);
    pool->working_thread_num -= 1;
    pthread_mutex_unlock(&pool->working_lock);
  }
  return NULL;
}

static void *pool_admin(void *arg) {
  // TODO
  thread_pool_t *poll = (thread_pool_t *)arg;
  return NULL;
}

// --------------------------- pool routines ----------------------------------

static int thread_pool_free(thread_pool_t *pool) {
  if (pool == NULL)
    return 1;

  if (pool->task_queue != NULL)
    free(pool->task_queue);
  if (pool->workers != NULL) {
    free(pool->workers);
    pthread_mutex_lock(&pool->queue_lock);
    pthread_mutex_destroy(&pool->queue_lock);
    pthread_mutex_lock(&pool->working_lock);
    pthread_mutex_destroy(&pool->working_lock);
    pthread_cond_destroy(&pool->queue_not_empty);
    pthread_cond_destroy(&pool->queue_not_full);
  }

  free(pool);
  return 0;
}

thread_pool_t *thread_pool_create(size_t thread_num, size_t max_queue_size,
                                  int heap_args) {
  thread_pool_t *pool = NULL;
  int i;
  do {
    if ((pool = calloc(1, sizeof(thread_pool_t))) == NULL) {
      puts("Error allocating pool\n");
      break;
    }
    pool->max_thread_num = thread_num;
    // pool->min_thread_num = min_thread_num;
    pool->heap_args = heap_args;
    pool->queue_size = 0;
    pool->max_queue_size = max_queue_size;
    pool->head = 0;
    pool->tail = 0;
    pool->shutdown = 0;
    pool->working_thread_num = 0;

    pool->workers =
        (pthread_t *)calloc(pool->max_thread_num, sizeof(pthread_t));
    if (pool->workers == NULL) {
      puts("Error allocating pool->workers\n");
      break;
    }
    pool->task_queue = calloc(pool->max_queue_size, sizeof(task_t));
    if (pool->task_queue == NULL) {
      puts("Error allocating pool->task_queue\n");
      break;
    }

    if (pthread_mutex_init(&pool->queue_lock, NULL) != 0 ||
        pthread_cond_init(&pool->queue_not_empty, NULL) != 0 ||
        pthread_cond_init(&pool->queue_not_full, NULL) != 0 ||
        pthread_mutex_init(&pool->working_lock, NULL) != 0) {
      puts("Error creating mutex or condvar\n");
      break;
    }

    for (i = 0; i < pool->max_thread_num; ++i) {
      pthread_create(&(pool->workers[i]), NULL, &pool_worker, (void *)pool);
      printf("thread %d created\n", i);
    }
    // pthread_create(&pool->admin, NULL, pool_admin, (void *)pool);
    return pool;

  } while (0); // mimic goto with do ... while(0)

  thread_pool_free(pool);
  return NULL;
}

int thread_pool_submit(thread_pool_t *pool, task_fun_t func, void *arg) {
  pthread_mutex_lock(&pool->queue_lock);
  while (pool->queue_size == pool->max_queue_size && !pool->shutdown) {
    pthread_cond_wait(&pool->queue_not_full, &pool->queue_lock);
  }
  if (pool->shutdown) {
    pthread_mutex_unlock(&pool->queue_lock);
    return -1;
  }
  pool->task_queue[pool->tail].func = func;
  pool->task_queue[pool->tail].arg = arg;
  pool->queue_size += 1;
  pool->tail = (pool->tail + 1) % pool->max_queue_size;

  pthread_cond_signal(&pool->queue_not_empty);
  pthread_mutex_unlock(&pool->queue_lock);

  return 0;
}

int thread_pool_destroy(thread_pool_t *pool) {
  if (pool == NULL)
    return -1;
  pool->shutdown = 1;
  // pthread_join(pool->admin, NULL);
  pthread_cond_broadcast(&pool->queue_not_empty);

  int i = 0;
  for (i = 0; i < pool->max_thread_num; i++) {
    pthread_join(pool->workers[i], NULL);
    // printf("thread %d joined\n", i);
  }

  if (pool->heap_args) {
    for (i = 0; i < pool->max_queue_size; i++) {
      if (pool->task_queue[i].arg != NULL) {
        // all the worker threads are joined. If some task in queue still
        // exists, they are not used by any thread.
        // This is still unsafe because the user might submit a task and arg
        // just on the stack. A boolean arg `heap_arg` specifying the type of
        // arg passed is required when creating the pool
        free(pool->task_queue[i].arg);
      }
    }
  }
  thread_pool_free(pool);
  return 0;
}
