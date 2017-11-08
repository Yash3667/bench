/**
 * Header for mainting work profiles used to model
 * arbitrary workloads. Provides an assertion to confirm
 * validity of profile along with set and deset macros.
 * 
 * Auhtor: Yash Gupta <yash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 */
#include <stdint.h>
#include <assert.h>

#ifndef _WORK_PROFILE_H_
#define _WORK_PROFILE_H_

//
// Macros
//
// Max Flags Value
#define MAX_FLAGS_VALUE     (1 << FLAG_END)

// Assert correct profile
#define ASSERT_PROFILE(p)                                   \
    do {                                                    \
        assert(p.flags > 0 && p.flags < MAX_FLAGS_VALUE);   \
        uint8_t prob = p.rread_prob + p.rwrite_prob;        \
        prob += p.sread_prob + p.swrite_prob;               \
        if (prob != 100) {                                  \
            assert(1 == 0);                                 \
        }                                                   \
        /* TODO: Add assert for sizes */                    \
    } while (0)

// Set and unset flags
#define SET_PROFILE_FLAG(p, f)      (p.flags |= 1 << f)
#define UNSET_PROFILE_FLAG(p, f)    (p.flags &= ~(1 << f))

//
// Enumerations
//
// Profile Flags
enum profile_flags {
    RREAD = 0,
    RWRITE,
    SREAD,
    SWRITE,
    FLAG_END
};

// 
// Structures
//
// Profile Structure
struct work_profile {
    uint8_t flags;

    /*
     * Using a probability distribution and specific
     * sizes for these fields, we should be able to
     * benchmark any arbitrary workload. The only 
     * limitation being that real time specific workloads
     * which work on absolute nature cannot be accurately
     * modeled.
     * 
     * Eg: A workload which ALWAYS performs 2 reads followed
     * by a write and so on. 
     * --> Such a workload does have a distribution but it is also 
     * very much absolute in how the work is given.
     * 
     * Legend:
     * --> r: random
     * --> s: sequential
     */
    uint64_t rread_sz, rwrite_sz;
    uint64_t sread_sz, swrite_sz;
    uint8_t rread_prob, rwrite_prob;
    uint8_t sread_prob, swrite_prob;
};

#endif