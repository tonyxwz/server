#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5

void *PrintHello(void *threadid)
{
    long *tid = (long *)threadid;
    printf("Hello thread %ld\n", *tid);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[N];
    int rc;
    long i;
    for (i = 0; i < N; ++i)
    {
        printf("main : creating thread %ld\n", i);
        rc = pthread_create(threads + i, NULL, PrintHello, (void *)&i);
        if (rc)
        {
            printf("error creating thread %ld\n", i);
            exit(-1);
        }
    }
    pthread_exit(NULL);
}