#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define N 5
#define STACK_DOUBLE_SIZE 10000


void* dowork(void* a) {
    pthread_attr_t* attr = (pthread_attr_t*) a;
    size_t stacksize;
    pthread_attr_getstacksize(attr, &stacksize);
    printf("stack size is %li\n", stacksize);
    pthread_exit(NULL);
}

int main() {
    pthread_attr_t attr;
    size_t stacksize;
    pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &stacksize);
    printf("main: stacksize is %li\n", stacksize);

    stacksize = stacksize + STACK_DOUBLE_SIZE * sizeof(double);
    pthread_attr_setstacksize(&attr, stacksize);
    pthread_t threads[N];
    int i = 0;
    for(i = 0; i < N; ++i) {
        if (pthread_create(&threads[i], &attr, dowork, (void*)&attr) == 0) {
            printf("created thread %d\n", i);
        } else {
            printf("error creating thread %d\n", i);
            exit(-1);
        }
    }
    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}