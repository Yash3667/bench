/**
 * Source file for generic, thread safe circular
 * queue implementation in C.
 * 
 * Author: Yash Gupta <yash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 */
#include "cirq.h"

/**
 * Create a circular queue of of the specfied
 * size.
 * 
 * @param   count       Count of items in Queue.
 * @param   type        single or multi threaded queue.
 * 
 * @return  A circular queue.
 * @return  NULL        malloc failed.
 * @return  NULL        mutex init failed.
 * @return  NULL        cond init failed.
 */
cirq*
cirq_create(uint64_t count, uint8_t type)
{
    cirq *q;

    q = malloc(sizeof *q);
    if (!q) {
        return NULL;
    }

    q->head = q->tail = 0;
    q->len = count;
    q->store = malloc(sizeof(void*) * count);
    if (!q->store) {
        goto store_alloc_fail;
    }

    q->type = type;
    if (q->type > CIRQ_SINGLE_THREAD) {
        if (pthread_mutex_init(&q->lock, NULL)) {
            goto mutex_init_fail;
        }
    }

    if (q->type > CIRQ_LOCKING) {
        if (pthread_cond_init(&q->cond, NULL)) {
            goto cond_init_fail;
        }
    }

    return q;

cond_init_fail:
    pthread_mutex_destroy(&q->lock);

mutex_init_fail:
    free(q->store);

store_alloc_fail:
    free(q);

    return NULL;
}

/**
 * Deallocate space acquired by a circular queue.
 * 
 * @param   q   The queue to deallocate.
 */
void
cirq_free(cirq *q)
{
    if (q->type > CIRQ_SINGLE_THREAD) {
        pthread_mutex_destroy(&q->lock);
    }

    if (q->type > CIRQ_LOCKING) {
        pthread_cond_destroy(&q->cond);
    }

    free(q->store);
    free(q);
}

/**
 * Acquire the element at the head of the circular queue
 * in case the queue is not empty.
 * 
 * @param   q       The queue to dequeue from.
 * 
 * @return  item    The item at head of queue.
 * @return  NULL    q is NULL.
 * @return  NULL    q is empty and non blocking.
 * 
 * NOTE: This function blocks in case the queue is empty.
 */
void*
cirq_get(cirq *q)
{
    void *item = NULL;

    if (!q) {
        return NULL;
    }

    // Acquire mutex if required.
    if (q->type > CIRQ_SINGLE_THREAD) {
        pthread_mutex_lock(&q->lock);
    }

    if (q->head == q->tail) {
        if (q->type != CIRQ_LOCKING_AND_BLOCKING) {
            goto release_mutex;
        }
        
        // The consumer will go to sleep until a
        // signal wakes it. It needs to confirm on
        // wake that indeed the queue is no longer
        // empty.
        do {
            pthread_cond_wait(&q->cond, &q->lock);
        } while (q->head == q->tail);
    }

    item = q->store[q->head];
    q->head = (q->head + 1) % q->len;

    // Signal any producers waiting on this condition.
    // Has no effect if nothing is waiting. This signal
    // can be sent even though the current thread own the
    // mutex [pthread_cond_signal man page].
    if (q->type == CIRQ_LOCKING_AND_BLOCKING) {
        pthread_cond_signal(&q->cond);        
    }

release_mutex:
    if (q->type > CIRQ_SINGLE_THREAD) {
        pthread_mutex_unlock(&q->lock);
    }

    return item;
}

/**
 * Put an element at the tail of the queue in case the
 * queue is not full.
 * 
 * @param   q       The queue to enqueue into.
 * @param   item    The item to enqueue.
 * 
 * @return  0       Successfully enqueued item.
 * @return  -1      q is NULL.
 * @return  -1      q is full and non blocking.
 * 
 * NOTE: This function blocks in case the queue is full.
 */
int
cirq_put(cirq *q, void *item)
{
    int ret = -1;
    uint64_t q_full;

    if (!q) {
        return -1;
    }

    // Acquire mutex if required.
    if (q->type > CIRQ_SINGLE_THREAD) {
        pthread_mutex_lock(&q->lock);
    }

    q_full = (q->tail + 1) % q->len;
    q_full = (q_full == q->head)? 1: 0;

    if (q_full) {
        if (q->type != CIRQ_LOCKING_AND_BLOCKING) {
            goto release_mutex;
        }
        
        // The producer will go to sleep until a
        // signal wakes it. It needs to confirm on
        // wake that indeed the queue is no longer
        // full;
        do {
            pthread_cond_wait(&q->cond, &q->lock);
            q_full = (q->tail + 1) % q->len;
            q_full = (q_full == q->head)? 1: 0;
        } while (q_full);
    }

    q->store[q->tail] = item;
    q->tail = (q->tail + 1) % q->len;
    ret = 0;

    // Signal any consumers waiting on this condition.
    // Has no effect if nothing is waiting. This signal
    // can be sent even though the current thread own the
    // mutex [pthread_cond_signal man page].
    if (q->type == CIRQ_LOCKING_AND_BLOCKING) {
        pthread_cond_signal(&q->cond);        
    }

release_mutex:
    if (q->type > CIRQ_SINGLE_THREAD) {
        pthread_mutex_unlock(&q->lock);
    }

    return ret;
}