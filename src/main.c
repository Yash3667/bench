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
#include "nano_time.h"
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
#include <fcntl.h>

//
// Macros
//
// Workload Queue Count
#define MAX_CIRQ_LEN    64

//
// Enumerations
//
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

enum consumer_state {
    CONSUMER_STATE_INIT = 0,
    CONSUMER_STATE_IN_LOOP,
    CONSUMER_STATE_EXIT_LOOP,
    CONSUMER_STATE_EXITED_LOOP
};

//
// Global Variables
//
// Exit variable for consumer thread.
volatile uint8_t global_cstate = CONSUMER_STATE_INIT;

//
// Structures
//
// All required arguments
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
    struct timespec ttoken;
    double tstamp;
    double total;
    double rwrite_total, swrite_total;
    void *buf;
    char *ofile_name;
    int fd;
    uint64_t ret;
    int64_t i;

    for (i = 0; i < MAX_DATA_POINTS; i++) {
        cargs->data[i].total_time_consumed = 0;
        cargs->data[i].total_operations = 0;
    }
    lseek(cargs->fd, 0, SEEK_SET);

    /*
     **********************************************************
     * A global variable is used to signal this thread to
     * exit its loop. This is because this thread needs to
     * output its statistics to a file. 
     * 
     * There is no need for a mutex as it is guarenteed that
     * until this thread exits the loop, the producer thread
     * will keep functioning. Hence, even if it spends another
     * iteration in the loop due to a race condition, it hurts
     * no one and avoids the overhead of a mutex.
     * 
     * It also signals the timer thread that it is indeed out
     * of the loop and so it is safe to cancel the producer
     * thread 
     **********************************************************
     */

    total = rwrite_total = swrite_total = 0;
    global_cstate = CONSUMER_STATE_IN_LOOP;
    while (global_cstate == CONSUMER_STATE_IN_LOOP) {
        item = cirq_get(cargs->workload);

        printf("%lu. O: %lu | L: %lu | T: %d\n",
        item->sequence, item->offset, item->length, item->task);

        buf = malloc(item->length);
        assert(buf != NULL);

        if (item->task == IO_RREAD || item->task == IO_SREAD) {
            INIT_TIME(&ttoken);
            ret = pread(cargs->fd, buf, item->length, item->offset);
            tstamp = GET_TIME(ttoken);
        } else {
            memset(buf, 49, item->length);
            INIT_TIME(&ttoken);
            ret = pwrite(cargs->fd, buf, item->length, item->offset);
            tstamp = GET_TIME(ttoken);

            if (item->task == IO_RWRITE) {
                rwrite_total += item->length;
            } else {
                swrite_total += item->length;
            }
        }
        assert(ret == item->length);

        cargs->data[item->task].total_time_consumed += tstamp;
        cargs->data[item->task].total_operations += 1;
        printf("Time Taken: %.8lf seconds\n\n", tstamp);

        free(item);
        free(buf);
    }
    global_cstate = CONSUMER_STATE_EXITED_LOOP;

    /*
     * We need to ensure that all the data written is flushed
     * to the drive otherwise the benchmark is not accurate.
     * Calling fsync after each write is dumb however. We call
     * fsync after the loop and divide the time it takes.
     */
    INIT_TIME(&ttoken);
    fsync(cargs->fd);
    tstamp = GET_TIME(ttoken);
    printf("Sync Time: %.8lf seconds\n", tstamp);

    total = rwrite_total + swrite_total;
    cargs->data[IO_RWRITE].total_time_consumed += (total)? ((rwrite_total/total) * tstamp): 0;
    cargs->data[IO_SWRITE].total_time_consumed += (total)? ((swrite_total/total) * tstamp): 0;

    /*
     * Append a ".bin" to the end of given file name. This
     * new file contains the statistical data. In case the
     * file name contains a '/', then extract from the last
     * slash onwards.
     * 
     * Reason I am using strdup is so that I can simply free
     * ofile_name and not care.
     */

    ofile_name = strrchr(cargs->file_name, '/');
    if (!ofile_name) {
        ofile_name = strdup("default_output.bin");
        assert(ofile_name);
    } else {
        char *temp = ofile_name + 1;
        ofile_name = malloc(sizeof(*ofile_name) * (strlen(temp) + 5));
        assert(ofile_name);
        strcpy(ofile_name, temp);
        strcat(ofile_name, ".bin");
    }
    fd = open(ofile_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    assert(fd != -1);
    free(ofile_name);

    for (i = 0; i < MAX_DATA_POINTS; i++) {
        if (cargs->data[i].total_operations > 0) {
            cargs->data[i].avg_time_consumed = cargs->data[i].total_time_consumed / cargs->data[i].total_operations;
        } else {
            cargs->data[i].avg_time_consumed = 0;
        }
        printf("%ld. Total Operations: %lu\n", i, cargs->data[i].total_operations);
        printf("%ld. Average Latency : %.8lf seconds\n", i, cargs->data[i].avg_time_consumed);
        printf("%ld. Total Time Taken: %.8lf seconds\n\n", i, cargs->data[i].total_time_consumed);

        /*
         * The format of the output binary file is simple.
         *  --> The total number of operations == X (8 bytes).
         *  --> The average time consumed by 1 operation (8 bytes).
         *  --> The total time consumed (8 bytes).
         */ 

        write(fd, &cargs->data[i].total_operations, sizeof(uint64_t));
        write(fd, &cargs->data[i].avg_time_consumed, sizeof(double));
        write(fd, &cargs->data[i].total_time_consumed, sizeof(double));
    }

    close(cargs->fd);
    close(fd);
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
    long int i;

    for (i = 0; i < targs->timer; i++) {
        sleep(1);
        //printf("%ld...", i+1);
        //fflush(stdout);
    }
    //printf("\n");

    /*
     ************************************************************
     * The timer thread cannot simply cancel the consumer
     * thread as the consumer needs to output its statistics
     * to a file. Hence, we use a global variable to signify to the
     * consumer that it needs to stop and output its statistics
     * to a file. As there is a single writer and single reader,
     * we do not need to use a mutex. A simple wait in a loop is
     * enough as we do not care that it waits in a loop one more
     * time than needed in case of a race condition.
     * 
     * It is crucial that the consumer is confirmed to be out
     * of its loop before the producer is cancelled as otherwise
     * a system state might be reached such that there is no
     * producer and the circular queue is empty and so the 
     * consumer goes to sleep, expecting some thread to wake it
     * up (hence never exiting its loop). By making sure the consumer 
     * is stopped first, this state is avoided.
     ************************************************************
     */

    global_cstate = CONSUMER_STATE_EXIT_LOOP;
    while (global_cstate != CONSUMER_STATE_EXITED_LOOP);

    pthread_cancel(*targs->producer);
    return NULL;
}

/**
 * Incredibly crappy function to parse arguments without any
 * sort of error checking.
 * 
 * The error checking is meant to take place in the pyton
 * front end for the arguments.
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
    qwl = cirq_create(MAX_CIRQ_LEN, CIRQ_LOCKING_AND_BLOCKING);
    assert(qwl != NULL);

    /* 
     * Create and deploy all the required threads, including filling up
     * their required parameters. The timer thread controls the execution
     * of the producer and consumer thread, so main simply waits on the
     * timer thread.
     * 
     * Also acquire the size of the disk drive.
     */

    fd = open(args_data.path, O_RDWR);
    assert(fd != -1);

    // Timer.
    targs.producer = &producer;
    targs.consumer = &consumer;
    targs.timer = args_data.timer;
    ret = pthread_create(&timer, NULL, twork, &targs);
    assert(ret == 0);

    // Consumer.
    cargs.file_name = args_data.path;
    cargs.fd = fd;
    cargs.workload = qwl;
    ret = pthread_create(&consumer, NULL, cwork, &cargs);
    assert(ret == 0);

    // Producer.
    pargs.rate = 1 / args_data.lambda;
    pargs.workload = qwl;
    pargs.profile = &bench_profile;
    pargs.drive_size = lseek(fd, 0L, SEEK_END);
    ret = pthread_create(&producer, NULL, pwork, &pargs);
    assert(ret == 0);

    pthread_join(timer, NULL);
    pthread_join(consumer, NULL);
    pthread_join(consumer, NULL);
    cirq_free(qwl);

    return 0;
}