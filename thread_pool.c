#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <thread_pool.h>

#include "queue.h"

/** Thread Pool **/
typedef struct thread_pool {
    pthread_t *worker_list; // array of pthreads
    queue_t *queue;         // queue of tasks
    size_t num_elements;    // number of elements
} thread_pool_t;

/** Work Struct **/
typedef struct work {
    work_function_t work_function; // pointer to work function
    void *aux;                     // pointer to argument
} work_t;

/** Helper Function **/
void *func(void *queue_ptr) {
    queue_t *queue = (queue_t *) queue_ptr;
    while (true) {
        void *element = queue_dequeue(queue);
        if (element == NULL) {
            return NULL;
        }
        work_t *work = (work_t *) element;
        (work->work_function)(work->aux);
        free(element);
    }
    return NULL;
}

/**
 * Creates a new heap-allocated thread pool
 *
 * @param num_worker_threads the number of threads in the pool
 * @return a pointer to the new thread pool
 */
thread_pool_t *thread_pool_init(size_t num_worker_threads) {
    pthread_t *ptr = (pthread_t *) calloc(num_worker_threads, sizeof(pthread_t));
    assert(ptr != NULL);

    thread_pool_t *thread_pool = (thread_pool_t *) malloc(sizeof(thread_pool_t));
    assert(thread_pool != NULL);

    thread_pool->queue = queue_init();

    thread_pool->worker_list = ptr;
    thread_pool->num_elements = num_worker_threads;

    for (size_t i = 0; i < num_worker_threads; i++) {
        pthread_create(&ptr[i], NULL, func, thread_pool->queue);
    }
    return thread_pool;
}

/**
 * Adds work to a thread pool
 *
 * @param pool the thread pool to perform the work
 * @param function the function to call on a thread in the thread pool
 * @param aux the argument to call the work function with
 */
void thread_pool_add_work(thread_pool_t *pool, work_function_t function, void *aux) {
    work_t *work = (work_t *) malloc(sizeof(work_t));
    if (work == NULL) {
        return;
    }
    work->work_function = function;
    work->aux = aux;
    queue_enqueue(pool->queue, (void *) work);
}

/**
 * Waits for all work added to a thread pool to finish,
 * then frees all resources associated with a heap-allocated thread pool.
 * You can enqueue a special value, e.g. NULL, to mark the end of the work.
 *
 * @param pool the thread pool to close
 */
void thread_pool_finish(thread_pool_t *pool) {
    for (size_t j = 0; j < pool->num_elements; j++) {
        queue_enqueue(pool->queue, NULL);
    }
    for (size_t i = 0; i < pool->num_elements; i++) {
        pthread_join(pool->worker_list[i], NULL);
    }
    queue_free(pool->queue);
    free(pool->worker_list);
    free(pool);
}
