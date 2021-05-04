# A thread pool implementation in C

Modernized version of the thread pool used in a [internship project in 2015](https://github.com/tonyxwz/internship-neu-2015)

## Recap

- why thread pool? 
    1. the original project was for running a hospital signup server on an embedded system. It has limited resources. A number of the maximum concurrent thread must be set.
    2. In the meantime, having a few readily-created thread waiting for incoming jobs instead of creating a new thread for every connection is beneficial.

## Design

- `threadpool.c`
- `threadpool.h`

### API

```c
thread_pool_t *pool = thread_pool_create(4, 100, 0);
thread_pool_submit(pool, compute, (void *)(i));
// ...
thread_pool_destroy(pool);
```

### Status
Used to be a TCP server talking in json to destop Qt clients. I am extending it into a http server.
