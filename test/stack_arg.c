#include "../threadpool.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/signal.h>
#include <unistd.h>

void compute(void *arg) {
  // puts("3\n");
  long val = (long)(arg);

  for (long i = 0; i < 100000; i += 1) {
    val = (val + i) % 1000000;
  }
  printf("%ld\n", val);
}
int main() {
  thread_pool_t *pool = thread_pool_create(4, 100, 0);
  for (unsigned long i = 0; i < 200; ++i) {
    thread_pool_submit(pool, compute, (void *)(i));
    // printf("task %ld submitted\n", i);
  }
  sleep(2);
  thread_pool_destroy(pool);
  return 0;
}