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


#ifndef _FLUID_RVOICE_EVENT_H
#define _FLUID_RVOICE_EVENT_H

#include "fluidsynth_priv.h"
#include "fluid_rvoice_mixer.h"
#include "fluid_ringbuffer.h"

typedef struct _fluid_rvoice_event_t fluid_rvoice_event_t;

struct _fluid_rvoice_event_t
{
    fluid_rvoice_function_t method;
    void *object;
    fluid_rvoice_param_t param[MAX_EVENT_PARAMS];
};

/*
 * Bridge between the renderer thread and the midi state thread.
 * fluid_rvoice_eventhandler_fetch_all() can be called in parallel
 * with fluid_rvoice_eventhandler_push/flush()
 */
struct _fluid_rvoice_eventhandler_t
{
    fluid_ringbuffer_t *queue; /**< List of fluid_rvoice_event_t */
    fluid_atomic_int_t queue_stored; /**< Extras pushed but not flushed */
    fluid_ringbuffer_t *finished_voices; /**< return queue from handler, list of fluid_rvoice_t* */
    fluid_rvoice_mixer_t *mixer;
};

fluid_rvoice_eventhandler_t *new_fluid_rvoice_eventhandler(
    int queuesize, int finished_voices_size, int bufs,
    int fx_bufs, int fx_units, fluid_real_t sample_rate_max, fluid_real_t sample_rate, int, int);

void delete_fluid_rvoice_eventhandler(fluid_rvoice_eventhandler_t *);

int fluid_rvoice_eventhandler_dispatch_all(fluid_rvoice_eventhandler_t *);
int fluid_rvoice_eventhandler_dispatch_count(fluid_rvoice_eventhandler_t *);
void fluid_rvoice_eventhandler_finished_voice_callback(fluid_rvoice_eventhandler_t *eventhandler,
        fluid_rvoice_t *rvoice);

static FLUID_INLINE void
fluid_rvoice_eventhandler_flush(fluid_rvoice_eventhandler_t *handler)
{
    int queue_stored = fluid_atomic_int_get(&handler->queue_stored);

    if(queue_stored > 0)
    {
        fluid_atomic_int_set(&handler->queue_stored, 0);
        fluid_ringbuffer_next_inptr(handler->queue, queue_stored);
    }
}

/**
 * @return next finished voice, or NULL if nothing in queue
 */
static FLUID_INLINE fluid_rvoice_t *
fluid_rvoice_eventhandler_get_finished_voice(fluid_rvoice_eventhandler_t *handler)
{
    void *result = fluid_ringbuffer_get_outptr(handler->finished_voices);

    if(result == NULL)
    {
        return NULL;
    }

    result = * (fluid_rvoice_t **) result;
    fluid_ringbuffer_next_outptr(handler->finished_voices);
    return result;
}


int fluid_rvoice_eventhandler_push_int_real(fluid_rvoice_eventhandler_t *handler,
        fluid_rvoice_function_t method, void *object, int intparam,
        fluid_real_t realparam);

int fluid_rvoice_eventhandler_push_ptr(fluid_rvoice_eventhandler_t *handler,
                                       fluid_rvoice_function_t method, void *object, void *ptr);

int fluid_rvoice_eventhandler_push(fluid_rvoice_eventhandler_t *handler,
                                   fluid_rvoice_function_t method, void *object,
                                   fluid_rvoice_param_t param[MAX_EVENT_PARAMS]);

static FLUID_INLINE void
fluid_rvoice_eventhandler_add_rvoice(fluid_rvoice_eventhandler_t *handler,
                                     fluid_rvoice_t *rvoice)
{
    fluid_rvoice_eventhandler_push_ptr(handler, fluid_rvoice_mixer_add_voice,
                                       handler->mixer, rvoice);
}



#endif
