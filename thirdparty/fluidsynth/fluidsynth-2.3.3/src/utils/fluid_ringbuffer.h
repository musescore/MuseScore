/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#ifndef _FLUID_RINGBUFFER_H
#define _FLUID_RINGBUFFER_H

#include "fluid_sys.h"

/*
 * Lockless event queue instance.
 */
struct _fluid_ringbuffer_t
{
    char *array;  /**< Queue array of arbitrary size elements */
    int totalcount;       /**< Total count of elements in array */
    fluid_atomic_int_t count;            /**< Current count of elements */
    int in;               /**< Index in queue to store next pushed element */
    int out;              /**< Index in queue of next popped element */
    size_t elementsize;          /**< Size of each element */
    void *userdata;
};

typedef struct _fluid_ringbuffer_t fluid_ringbuffer_t;


fluid_ringbuffer_t *new_fluid_ringbuffer(int count, size_t elementsize);
void delete_fluid_ringbuffer(fluid_ringbuffer_t *queue);

/**
 * Get pointer to next input array element in queue.
 * @param queue Lockless queue instance
 * @param offset Normally zero, or more if you need to push several items at once
 * @return Pointer to array element in queue to store data to or NULL if queue is full
 *
 * This function along with fluid_ringbuffer_next_inptr() form a queue "push"
 * operation and is split into 2 functions to avoid an element copy.  Note that
 * the returned array element pointer may contain the data of a previous element
 * if the queue has wrapped around.  This can be used to reclaim pointers to
 * allocated memory, etc.
 */
static FLUID_INLINE void *
fluid_ringbuffer_get_inptr(fluid_ringbuffer_t *queue, int offset)
{
    return fluid_atomic_int_get(&queue->count) + offset >= queue->totalcount ? NULL
           : queue->array + queue->elementsize * ((queue->in + offset) % queue->totalcount);
}

/**
 * Advance the input queue index to complete a "push" operation.
 * @param queue Lockless queue instance
 * @param count Normally one, or more if you need to push several items at once
 *
 * This function along with fluid_ringbuffer_get_inptr() form a queue "push"
 * operation and is split into 2 functions to avoid element copy.
 */
static FLUID_INLINE void
fluid_ringbuffer_next_inptr(fluid_ringbuffer_t *queue, int count)
{
    fluid_atomic_int_add(&queue->count, count);

    queue->in += count;

    if(queue->in >= queue->totalcount)
    {
        queue->in -= queue->totalcount;
    }
}

/**
 * Get amount of items currently in queue
 * @param queue Lockless queue instance
 * @return amount of items currently in queue
 */
static FLUID_INLINE int
fluid_ringbuffer_get_count(fluid_ringbuffer_t *queue)
{
    return fluid_atomic_int_get(&queue->count);
}


/**
 * Get pointer to next output array element in queue.
 * @param queue Lockless queue instance
 * @return Pointer to array element data in the queue or NULL if empty, can only
 *   be used up until fluid_ringbuffer_next_outptr() is called.
 *
 * This function along with fluid_ringbuffer_next_outptr() form a queue "pop"
 * operation and is split into 2 functions to avoid an element copy.
 */
static FLUID_INLINE void *
fluid_ringbuffer_get_outptr(fluid_ringbuffer_t *queue)
{
    return fluid_ringbuffer_get_count(queue) == 0 ? NULL
           : queue->array + queue->elementsize * queue->out;
}


/**
 * Advance the output queue index to complete a "pop" operation.
 * @param queue Lockless queue instance
 *
 * This function along with fluid_ringbuffer_get_outptr() form a queue "pop"
 * operation and is split into 2 functions to avoid an element copy.
 */
static FLUID_INLINE void
fluid_ringbuffer_next_outptr(fluid_ringbuffer_t *queue)
{
    fluid_atomic_int_add(&queue->count, -1);

    if(++queue->out == queue->totalcount)
    {
        queue->out = 0;
    }
}

#endif /* _FLUID_ringbuffer_H */
