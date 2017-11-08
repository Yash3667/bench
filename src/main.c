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
    char *   path;
};

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
    args->path = argv[PATH];

    return 0;
}

int 
main(int argc, char *argv[])
{
    int ret;
    struct bench_args args_data;
    struct work_profile bench_profile;

    if (parse_args(argc, argv, &args_data)) {
        printf("Not Enough Args!\n");
        return -1;
    }

    bench_profile.rread_prob = args_data.prob[0];
    bench_profile.rwrite_prob = args_data.prob[1];
    bench_profile.sread_prob = args_data.prob[2];
    bench_profile.swrite_prob = args_data.prob[3];
    if (bench_profile.rread_prob > 0) {
        SET_PROFILE_FLAG(bench_profile, RREAD);
    }
    if (bench_profile.rwrite_prob > 0) {
        SET_PROFILE_FLAG(bench_profile, RWRITE);
    }
    if (bench_profile.sread_prob > 0) {
        SET_PROFILE_FLAG(bench_profile, SREAD);
    }
    if (bench_profile.swrite_prob > 0) {
        SET_PROFILE_FLAG(bench_profile, SWRITE);
    }
    ASSERT_PROFILE(bench_profile);

    bench_profile.rread_sz = args_data.sz[0];
    bench_profile.rwrite_sz = args_data.sz[1];
    bench_profile.sread_sz = args_data.sz[2];
    bench_profile.swrite_sz = args_data.sz[3];

    return 0;
}