/**
 * Header to export a stateless exponential distribution
 * function which can be used to obtain distributions for
 * Poisson Processes for any arbitrary work.
 * 
 * Author: Yash Gupta <ash_gupta12@live.com>
 * Copyright: Yash Gupta
 * 
 * License: MIT Public License
 * 
 * Note: The algorithm for obtaining an exponential distribution
 * was obtained from Wikipedia.
 */
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef _EXPDISTRIB_H_
#define _EXPDISTRIB_H_

/**
 * Acquire a negative exponential distribution of
 * for a specified interval lambda upon subsequent
 * calls to the function.
 */
double 
get_exponential_variate(double lambda);

#endif