#include "expdistrib.h"

/**
 * Get a uniformly distributed number between 0 and 1.
 */
static double
get_uniform_variate(void)
{
    unsigned int random_number;
    double variate;

    random_number = rand();
    variate = (double)random_number / (double)RAND_MAX;

    return variate;
}

/**
 * Acquire a negative exponential distribution of
 * for a specified interval lambda upon a chain of 
 * calls to the function.
 */
double
get_exponential_variate(double rate)
{
    double exp_variate;
    double variate;

    variate = get_uniform_variate();
    exp_variate = log(variate) / -rate;

    return exp_variate;
}