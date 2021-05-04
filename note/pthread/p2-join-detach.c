#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 5
void *HeavyJob(void *t)
{
    double result = 0.0;
    int i;
    long *pl_tid = (long*) t;
    printf("worker thread %ld\n", *pl_tid);
    for (i = 0; i < 100000; i++)
    {
        // result += sin(i) * tan(i);
        result += 100;
        result = result / 1000;
    }
    // int* program_break = sbrk(0);
    // printf("break: %p\tt:%p\n", program_break, t);
    printf("result of worker %ld: %e\n", *pl_tid, result);
    pthread_exit(t);
}

int main()
{
    pthread_t pthread[N];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // good practice to explicitly declare attr to be joinable
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    long t = 0;
    for (t = 0; t < N; ++t)
    {
        int rc = pthread_create(&pthread[t], &attr, HeavyJob, (void *)&t);
        if (rc != 0)
        {
            printf("Error creating thread %ld\n", t);
            exit(-1);
        }
    }
    pthread_attr_destroy(&attr);
    void* status;
    for(t = 0; t < N; ++t) {
        int rc = pthread_join(pthread[t], &status);
        if (rc) {
            printf("Error joining thread %ld\n", t);
            exit(-1);
        } else {
            // using the retval fron pthread_join
            printf("thread %ld joined\n", *((long*)status));
        }
    }
    pthread_exit(NULL);
}