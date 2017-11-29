/**
 * Header file for generic vector data structure. A vector
 * is essentially an array which grows as required. 
 * 
 * Auhtor: Yash Gupta
 * Copyright: Yash gupta
 * 
 * License: MIT Public License
 */
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#ifndef _VECTOR_H_
#define _VECTOR_H_

typedef struct vector {
    uint64_t len, index;
    void **store;
};

/**
 * Create a vector with a specified initial size. This size
 * will grow as required.
 */
vector*
vector_create(uint64_t len);

/**
 * Free the space being used by a vector instance.
 */
void
vector_free(vector *vec);

/**
 * Append a new value to the vector.
 */
int
vector_append(vector *vec, void *item);

/**
 * Remove the last element which was appended
 * to the vector.
 */
int 
vector_remove(vector *vec);

/**
 * Acquire an element in the vector at the specified
 * position.
 */
void*
vector_get(vector *vec, uint64_t index);

/**
 * Replace the value of an element at a specified
 * index in the vector.
 */
int
vector_replace(vector *vec, uint64_t index, void *item);

/**
 * Acquire the size (working index) of the vector. This
 * is the effectively the size of the "array".
 */
uint64_t 
vector_size(vector *vec);

#endif