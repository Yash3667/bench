/**
 * Header file for generic, thread safe circular
 * queue implementation in C.
 * 
 * Author: Yash Gupta <yash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 */
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>  

#ifndef _CIRQ_H_
#define _CIRQ_H_

enum {
    CIRQ_SINGLE_THREAD,
    CIRQ_LOCKING,
    CIRQ_LOCKING_AND_BLOCKING
};

typedef struct cirq {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    uint64_t len, head, tail;
    uint8_t type;
    void **store;
} cirq;

/**
 * Create a circular queue of of the specfied
 * size.
 */
cirq*
cirq_create(uint64_t count, uint8_t type);

/**
 * Deallocate space acquired by a circular queue.
 */
void
cirq_free(cirq *q);

/**
 * Acquire the element at the head of the circular queue
 * in case the queue is not empty.
 * 
 * NOTE: This function blocks in case the queue is empty.
 */
void*
cirq_get(cirq *q);

/**
 * Put an element at the tail of the queue in case the
 * queue is not full.
 * 
 * NOTE: This function blocks in case the queue is full.
 */
int
cirq_put(cirq *q, void *item);

#endif