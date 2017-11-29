/**
 * Header for describing the architecture model
 * of the benchmark. Each process handles a single
 * producer and consumer working on a single drive.
 * 
 * Author: Yash Gupta <yash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 */
#include "cirq/cirq.h"
#include "work_profile.h"
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#ifndef _MODEL_H_
#define _MODEL_H_

//
// Enumerations
//
// Task Identifier
enum iotask {
    IO_RREAD = 0,
    IO_RWRITE,
    IO_SREAD,
    IO_SWRITE
};

// Index for statistical collection
enum stat_index {
    STAT_RREAD = 0,
    STAT_RWRITE,
    STAT_SREAD,
    STAT_SWRITE,
    STAT_TOTAL
};

//
// Structures
//
// Item for workload queue.
struct work_item {
    /*
     * Each work item describes a single task the consumer needs 
     * to perform. A task simply consists of whether the consumer 
     * needs to (read, write or stop), an offset to work on and
     * the length for the required task. A sequence number for the
     * task is also provided.
     */

    uint64_t sequence;
    uint64_t offset;
    uint64_t length;
    enum iotask task;
};

// Statistical Collection
struct statistical_collection {
    uint64_t total_time_consumed;
    uint64_t total_operations;
};

// Thread arguments
struct thread_args_consumer {
    /*
     * The architecture model we use means that we can
     * have a brain dead consumer whose job is to simply
     * dequeue an item and execute the task. All it requires
     * is the file descriptor and a link to the shared queue along
     * with all drive statistics.
     */

    int  fd;
    cirq *workload;
    struct statistical_collection statistics[STAT_TOTAL];
};

struct thread_args_producer {
    /*
     * The producer is far more complex than the consumer
     * in this architecture model. The producer is aware
     * of the workload semantics and accordingly needs to know
     * the distribution of the workload.
     * 
     * TODO: Need statistics.
     */

    double rate;
    long int drive_size;
    struct work_profile *profile;
    cirq *workload;
};

struct thread_args_timer {
    /*
     * The timer thread is used to time a certain benchmark
     * session. It conveniently kills the producer and the
     * consumer thread.
     */

    pthread_t *producer;
    pthread_t *consumer;
    long int timer;
};

/**
 * This function is used to generate a workload based on a 
 * workload profile. It uses stubs to generate the actual
 * workload. It is the callers responsibility to free this
 * workload.
 * 
 * @param profile       The profile to generate a workload on.
 * @param drive_size    The size of the drive.
 * 
 * @return A valid workitem
 * @return NULL         Malloc error
 */
static struct work_item*
_generate_work_item(struct work_profile *profile, uint64_t drive_size)
{
    static uint64_t sequence = 0;
    static uint64_t sread_offset = 0;
    static uint64_t swrite_offset = 0;

    struct work_item *item;
    long int task;

    item = malloc(sizeof *item);
    if (!item) {
        return NULL;
    }

    item->sequence = sequence++;
    task = (rand() % 100) + 1;

    /*
     * Assign a workload based on the cumulative probability
     * distribution of the workloads. In case the offset we get
     * is such that size of io > drive_size - offset, then we
     * trim the IO size until the end of the drive. This is the
     * simulation of most industry class workloads.
     */

    if (task <= profile->rread_prob) {
        item->task = IO_RREAD;
        item->length = profile->rread_sz;
        item->offset = rand() % drive_size;
    } else if (task <= profile->rwrite_prob) {
        item->task = IO_RWRITE;
        item->length = profile->rwrite_sz;
        item->offset = rand() % drive_size;
    } else if (task <= profile->sread_prob) {
        item->task = IO_SREAD;
        item->length = profile->sread_sz;
        item->offset = sread_offset;
        sread_offset = (sread_offset + item->length >= drive_size)? 0: sread_offset + item->length;
    } else if (task <= profile->swrite_prob) {
        item->task = IO_SWRITE;
        item->length = profile->swrite_sz;
        item->offset = swrite_offset;
        swrite_offset = (swrite_offset + item->length >= drive_size)? 0: swrite_offset + item->length;
    }

    if ((drive_size - item->offset) < item->length) {
        item->length = drive_size - item->offset;
    }

    return item;
}

#endif