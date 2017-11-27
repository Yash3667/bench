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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

// Need to define GNU Source to have access to
// O_DIRECT flag for open().
#define __USE_GNU
#include <fcntl.h>

// Workload Queue Count
#define QWL_MAX     64

// Argument Enumeration
enum bench_args_enum {
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

// Structure encompassing all required arguments
struct bench_args {
    uint8_t     prob[4];
    uint64_t    sz[4];
    long int    timer;
    double      lambda;
    char *      path;
};

/**
 * The consumer work function is a brain dead work function
 * which performs the actual I/O to the disk drive. It acquires
 * a workload item from a circular queue and executes that
 * workload on a specified drive, collecting statistics.
 * 
 * @param   args    Consumer specific arguments.
 * @return  NULL
 */
void*
cwork(void *args)
{
    struct thread_args_consumer *cargs = args;
    struct work_item *item;
    uint64_t ret;
    void *buf;

    lseek(cargs->fd, 0L, SEEK_SET);
    while (1) {
        item = cirq_get(cargs->workload);

        printf("%lu. O: %lu | L: %lu | T: %d\n",
        item->sequence, item->offset, item->length, item->task);

        buf = malloc(item->length);
        assert(buf != NULL);

        if (item->task == IO_READ) {
            ret = pread(cargs->fd, buf, item->length, item->offset);
        } else {
            ret = pwrite(cargs->fd, buf, item->length, item->offset);
        }
        assert(ret == item->length);
        free(item);
    }

    close(cargs->fd);
    return NULL;
}

/**
 * The producer work function is used to generate workloads for
 * a circular queue depending on a user defined work profile. It
 * generates a workload based on an exponential distribution to emulate
 * a periodic work interval.
 * 
 * @param   args    Producer specific arguments.
 * @return  NULL
 */
void*
pwork(void *args)
{
    struct thread_args_producer *pargs = args;
    struct work_item *item;
    double sleep_time;

    while (1) {
        sleep_time = get_exponential_variate(pargs->rate) * 1000000;
        usleep(sleep_time);

        item = _generate_work_item(pargs->profile, pargs->drive_size);
        cirq_put(pargs->workload, item);
    }

    return NULL;
}

/**
 * The timer work function is another brain dead function whose
 * sole job is to sleep for a set amount of time and then kill the
 * producer and consumer threads.
 * 
 * @param   args    Timer specific arguments.
 * @return  NULL
 */
void*
twork(void *args)
{
    struct thread_args_timer *targs = args;

    sleep(targs->timer);
    pthread_cancel(*targs->producer);
    pthread_cancel(*targs->consumer);

    return NULL;
}

/**
 * Incredibly crappy function to parse arguments without any
 * sort of error checking.
 */
int 
parse_args(int argc, char *argv[], struct bench_args *args)
{
    if (argc != ARG_COUNT) {
        return -1;
    }

    // Get Probabilities
    args->prob[0] = atoi(argv[RREAD_PROB]);
    args->prob[1] = atoi(argv[RWRITE_PROB]);
    args->prob[2] = atoi(argv[SREAD_PROB]);
    args->prob[3] = atoi(argv[SWRITE_PROB]);

    // Get Sizes
    args->sz[0] = atoll(argv[RREAD_SZ]);
    args->sz[1] = atoll(argv[RWRITE_SZ]);
    args->sz[2] = atoll(argv[SREAD_SZ]);
    args->sz[3] = atoll(argv[SWRITE_SZ]);

    // Get Miscellaneous
    args->timer = atol(argv[TIMER]);
    args->lambda = atof(argv[LAMBDA]);
    args->path = argv[PATH];

    return 0;
}

/**
 * Main function is used to setup the threads and execute them.
 * From there on, it basically waits until the timer returns, after
 * which it frees up the used memory and exits.
 */
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
    int ret, fd;

    if (parse_args(argc, argv, &args_data)) {
        printf("Not Enough Args!\n");
        return -1;
    }
    srand(time(0));

    /* Build a work profile out of the arguments provided. For
     * more information on how the probability distribution is layed
     * out, refer to "work_profile.h".
     */

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

    /* 
     * Create and deploy all the required threads, including filling up
     * their required parameters. The timer thread controls the execution
     * of the producer and consumer thread, so main simply waits on the
     * timer thread.
     * 
     * Also acquire the size of the disk drive.
     */
    fd = open(args_data.path, O_RDWR | O_SYNC);
    assert(fd != -1);

    pargs.rate = 1 / args_data.lambda;
    pargs.workload = qwl;
    pargs.profile = &bench_profile;
    pargs.drive_size = lseek(fd, 0L, SEEK_END);

    cargs.fd = fd;
    cargs.workload = qwl;

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
    cirq_free(qwl);

    return 0;
}