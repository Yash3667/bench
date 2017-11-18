/**
 * Main source for running arbitrary benchmarks
 * on storage drives. Utilizes a workload profile
 * structure along with a producer consumer architecture.
 * 
 * Author: Yash Gupta <yash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 */
#include "expdistrib/expdistrib.h"
#include "cirq/cirq.h"
#include "work_profile.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <sys/vfs.h>

//
// Macros
//
// Workload Queue Count
#define QWL_MAX     64

//
// Enumerations
//
// Args
enum {
    NAME = 0,

    // Workload Probabilities.
    RREAD_PROB,
    RWRITE_PROB,
    SREAD_PROB,
    SWRITE_PROB,

    // Workload Sizes.
    RREAD_SZ,
    RWRITE_SZ,
    SREAD_SZ,
    SWRITE_SZ,

    // Miscellaneous.
    TIMER,
    LAMBDA,
    PATH,
    ARG_COUNT
};

//
// Structures
//
// All required args
struct bench_args {
    uint8_t  prob[4];
    uint64_t sz[4];
    double   timer;
    double   lambda;
    char *   path;
};

void*
cwork(void *args)
{
    struct thread_args_consumer *cargs = args;

    while (1);

    return NULL;
}

void*
pwork(void *args)
{
    struct thread_args_producer *pargs = args;

    while (1);

    return NULL;
}

void*
twork(void *args)
{
    struct thread_args_timer *targs = args;

    sleep(targs->timer);
    pthread_cancel(*targs->producer);
    pthread_cancel(*targs->consumer);

    return NULL;
}

int 
parse_args(int argc, char *argv[], struct bench_args *args)
{
    if (argc != ARG_COUNT) {
        return -1;
    }

    // Get Probabilities.
    args->prob[0] = atoi(argv[RREAD_PROB]);
    args->prob[1] = atoi(argv[RWRITE_PROB]);
    args->prob[2] = atoi(argv[SREAD_PROB]);
    args->prob[3] = atoi(argv[SWRITE_PROB]);

    // Get Sizes
    args->sz[0] = atoi(argv[RREAD_SZ]);
    args->sz[1] = atoi(argv[RWRITE_SZ]);
    args->sz[2] = atoi(argv[SREAD_SZ]);
    args->sz[3] = atoi(argv[SWRITE_SZ]);

    // Get Miscellaneous
    args->timer = atof(argv[TIMER]);
    args->lambda = atof(argv[LAMBDA]);
    args->path = argv[PATH];

    return 0;
}

int 
main(int argc, char *argv[])
{
    pthread_t producer, consumer, timer;
    cirq *qwl;
    struct bench_args args_data;
    struct work_profile bench_profile;
    struct thread_args_consumer cargs;
    struct thread_args_producer pargs;
    struct thread_args_timer targs;
    struct statfs fsb;
    int ret;

    if (parse_args(argc, argv, &args_data)) {
        printf("Not Enough Args!\n");
        return -1;
    }

    // Build a work profile out of the arguments provided. For
    // more information on how the probability distribution is layed
    // out, refer to "work_profile.h"
    bench_profile.flags = 0;
    if (args_data.prob[0] > 0) {
        SET_PROFILE_FLAG(bench_profile, RREAD);
    }
    if (args_data.prob[1] > 0) {
        SET_PROFILE_FLAG(bench_profile, RWRITE);
    }
    if (args_data.prob[2] > 0) {
        SET_PROFILE_FLAG(bench_profile, SREAD);
    }
    if (args_data.prob[3] > 0) {
        SET_PROFILE_FLAG(bench_profile, SWRITE);
    }
    bench_profile.rread_prob = 0 + args_data.prob[0];
    bench_profile.rwrite_prob = bench_profile.rread_prob + args_data.prob[1];
    bench_profile.sread_prob = bench_profile.rwrite_prob + args_data.prob[2];
    bench_profile.swrite_prob = bench_profile.sread_prob + args_data.prob[3];
    bench_profile.rread_sz = args_data.sz[0];
    bench_profile.rwrite_sz = args_data.sz[1];
    bench_profile.sread_sz = args_data.sz[2];
    bench_profile.swrite_sz = args_data.sz[3];
    ASSERT_PROFILE(bench_profile);

    // Create the circular queue shared amongst the producer
    // and the consumer.
    qwl = cirq_create(QWL_MAX, CIRQ_LOCKING_AND_BLOCKING);
    assert(qwl != NULL);

    // Acquire size of the disk drive
    statfs(args_data.path, &fsb);

    // Prepare the arguments for both producer and consumer.
    // The producer needs a pointer to the consumers thread,
    // hence, the consumer starts first.
    cargs.drive_path = args_data.path;
    cargs.workload = qwl;
    pargs.rate = 1 / args_data.lambda;
    pargs.workload = qwl;
    pargs.profile = &bench_profile;
    pargs.drive_size = fsb.f_blocks * fsb.f_bsize;

    targs.producer = &producer;
    targs.consumer = &consumer;
    targs.timer = args_data.timer;
    ret = pthread_create(&timer, NULL, twork, &targs);
    assert(ret == 0);

    ret = pthread_create(&consumer, NULL, cwork, &cargs);
    assert(ret == 0);

    ret = pthread_create(&producer, NULL, pwork, &pargs);
    assert(ret == 0);

    pthread_join(timer, NULL);
    return 0;
}