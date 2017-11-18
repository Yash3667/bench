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
#include <stdint.h>
#include <pthread.h>

#ifndef _MODEL_H_
#define _MODEL_H_

//
// Enumerations
//
// Task Identifier
enum iotask {
    IO_READ,
    IO_WRITE,
    IO_STOP
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

// Thread arguments
struct thread_args_consumer {
    /*
     * The architecture model we use means that we can
     * have a brain dead consumer whose job is to simply
     * dequeue an item and execute the task. All it requires
     * is the drive path and a link to the shared queue along
     * with all drive statistics.
     * 
     * TODO: Need statistics.
     */

    char *drive_path;
    cirq *workload;
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

#endif