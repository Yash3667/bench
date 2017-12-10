#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define INIT_TIME(x)    _initTime(x)
#define GET_TIME(x)     _getTime(x)

/**
 * Initialize a variable to hold the timestamp
 * accurate upto nanoseconds
 * @param start Address of variable which will hold the timestamp
 */
static inline void
_initTime(struct timespec *start)
{
    clock_gettime(CLOCK_REALTIME, start);
}

/**
 * Return the difference of time in nano seconds
 * passed between time of call and the time stamp
 * in variable
 * @param start The variablewhich holds previous time stamp
 * @return Difference in Nano Seconds
 */
static inline double
_getTime(struct timespec start)
{
    double time_passed;
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);
    time_passed = (int64_t)1000000000L * (int64_t)(now.tv_sec - start.tv_sec);
    time_passed += (int64_t)(now.tv_nsec - start.tv_nsec);

    return time_passed / 1000000000.0;
}
