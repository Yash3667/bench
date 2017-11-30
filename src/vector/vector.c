/**
 * Source file for generic vector data structure. A vector
 * is essentially an array which grows as required. 
 * 
 * Auhtor: Yash Gupta
 * Copyright: Yash gupta
 * 
 * License: MIT Public License
 */
#include "vector.h"

/**
 * Create a vector with a specified initial size. This size
 * will grow as required.
 * 
 * @param   len     Required initial length of vector.
 * 
 * @return  vec     An initialized vector.
 * @return  NULL    malloc error.
 */
vector*
vector_create(uint64_t len)
{
    vector *vec;

    vec = malloc(sizeof *vec);
    if (!vec) {
        return NULL;
    }

    vec->index = 0;
    vec->len = len;
    vec->store = malloc(sizeof(*vec->store) * len);
    if (!vec->store) {
        return NULL;
    }

    return vec;
}

/**
 * Free the space being used by a vector instance.
 * 
 * @param   vec     The vector to free.
 */
void
vector_free(vector *vec)
{
    free(vec->store);
    free(vec);
}

/**
 * Append a new value to the vector.
 * 
 * @param   vec     The vector to append value to.
 * @param   item    The item to append.
 * 
 * @return  vec     The same vector which had an element appended.
 * @return  NULL    vec is NULL.
 * @return  NULL    realloc failed.
 */
vector* 
vector_append(vector *vec, void *item)
{
    void **temp;
    if (!vec) {
        return NULL;
    }

    // Resize the array if required. Array doubling works
    // great for this.
    if (vec->index == vec->len) {
        vec->len *= 2;

        temp = realloc(vec->store, vec->len * sizeof(*vec->store));
        if (!temp) {
            return NULL;
        }
        vec->store = temp;
    }

    vec->store[vec->index++] = item;
    return vec;
}

/**
 * Remove the last element which was appended
 * to the vector.
 * 
 * @param   vec     The vector to remove an element from.
 * 
 * @return  vec     The same vector from whic an element was removed.
 * @return  NULL    vec is NULL.
 * @return  NULL    vec->index is 0.
 */
inline vector*
vector_remove(vector *vec)
{
    return (vec && vec->index > 0)? vec->index -= 1, vec: NULL;
}

/**
 * Acquire an element in the vector at the specified
 * position.
 * 
 * @param   vec     The vector to get an element from.
 * @param   index   The index within that vector.
 * 
 * @return  item    The item at vec->store[index].
 * @return  NULL    vec is NULL.
 * @return  NULL    index > vec->index.
 */
inline void*
vector_get(vector *vec, uint64_t index)
{
    return (vec && vec->index > index)? vec->store[index]: NULL;
}

/**
 * Replace the value of an element at a specified
 * index in the vector.
 * 
 * @param   vec     The vector to replace an element in.
 * @param   index   The index to replace element at.
 * @param   item    The item to replce it with.
 * 
 * @return  vec     The vector in which the item is replaced.
 * @return  NULL    vec is NULL.
 * @return  NULL    index > vec->index.
 */
inline vector*
vector_replace(vector *vec, uint64_t index, void *item)
{
    return (vec && vec->index > index)? vec->store[index] = item, vec: NULL;
}

/**
 * Acquire the size (working index) of the vector. This
 * is the effectively the size of the "array".
 * 
 * @param   vec     The vector to find the size of.
 * 
 * @return  index   The working size of the vector.
 * @return  -1      vec is NULL.
 */
inline int64_t 
vector_size(vector *vec)
{
    return (vec)? (int64_t)vec->index: -1;
}