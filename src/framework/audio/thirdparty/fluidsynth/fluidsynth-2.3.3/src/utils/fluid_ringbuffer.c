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

/*
 * Josh Green <josh@resonance.org>
 * 2009-05-28
 */

#include "fluid_ringbuffer.h"
#include "fluid_sys.h"


/**
 * Create a lock free queue with a fixed maximum count and size of elements.
 * @param count Count of elements in queue (fixed max number of queued elements)
 * @return New lock free queue or NULL if out of memory (error message logged)
 *
 * Lockless FIFO queues don't use any locking mechanisms and can therefore be
 * advantageous in certain situations, such as passing data between a lower
 * priority thread and a higher "real time" thread, without potential lock
 * contention which could stall the high priority thread.  Note that there may
 * only be one producer thread and one consumer thread.
 */
fluid_ringbuffer_t *
new_fluid_ringbuffer(int count, size_t elementsize)
{
    fluid_ringbuffer_t *queue;

    fluid_return_val_if_fail(count > 0, NULL);

    queue = FLUID_NEW(fluid_ringbuffer_t);

    if(!queue)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    queue->array = FLUID_MALLOC(elementsize * count);

    if(!queue->array)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        delete_fluid_ringbuffer(queue);
        return NULL;
    }

    /* Clear array, in case dynamic pointer reclaiming is being done */
    FLUID_MEMSET(queue->array, 0, elementsize * count);

    queue->totalcount = count;
    queue->elementsize = elementsize;
    fluid_atomic_int_set(&queue->count, 0);
    queue->in = 0;
    queue->out = 0;

    return (queue);
}

/**
 * Free an event queue.
 * @param queue Lockless queue instance
 *
 * Care must be taken when freeing a queue, to ensure that the consumer and
 * producer threads will no longer access it.
 */
void
delete_fluid_ringbuffer(fluid_ringbuffer_t *queue)
{
    fluid_return_if_fail(queue != NULL);
    FLUID_FREE(queue->array);
    FLUID_FREE(queue);
}
