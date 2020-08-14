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
 2002 : API design by Peter Hanappe and Antoine Schmitt
 August 2002 : Implementation by Antoine Schmitt as@gratin.org
               as part of the infiniteCD author project
               http://www.infiniteCD.org/
 Oct4.2002 : AS : corrected bug in heap allocation, that caused a crash during sequencer free.
*/


#include "fluid_event.h"
#include "fluidsynth_priv.h"

/***************************************************************
 *
 *                           SEQUENCER EVENTS
 */

/* Event alloc/free */

void
fluid_event_clear(fluid_event_t *evt)
{
    FLUID_MEMSET(evt, 0, sizeof(fluid_event_t));

    // by default, no type
    evt->dest = -1;
    evt->src = -1;
    evt->type = -1;
}

/**
 * Create a new sequencer event structure.
 * @return New sequencer event structure or NULL if out of memory
 */
fluid_event_t *
new_fluid_event()
{
    fluid_event_t *evt;

    evt = FLUID_NEW(fluid_event_t);

    if(evt == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "event: Out of memory\n");
        return NULL;
    }

    fluid_event_clear(evt);

    return(evt);
}

/**
 * Delete a sequencer event structure.
 * @param evt Sequencer event structure created by new_fluid_event().
 */
void
delete_fluid_event(fluid_event_t *evt)
{
    fluid_return_if_fail(evt != NULL);

    FLUID_FREE(evt);
}

/**
 * Set the time field of a sequencer event.
 * @internal
 * @param evt Sequencer event structure
 * @param time Time value to assign
 */
void
fluid_event_set_time(fluid_event_t *evt, unsigned int time)
{
    evt->time = time;
}

/**
 * Set source of a sequencer event. \c src must be a unique sequencer ID or -1 if not set.
 * @param evt Sequencer event structure
 * @param src Unique sequencer ID
 */
void
fluid_event_set_source(fluid_event_t *evt, fluid_seq_id_t src)
{
    evt->src = src;
}

/**
 * Set destination of this sequencer event, i.e. the sequencer client this event will be sent to. \c dest must be a unique sequencer ID.
 * @param evt Sequencer event structure
 * @param dest The destination unique sequencer ID
 */
void
fluid_event_set_dest(fluid_event_t *evt, fluid_seq_id_t dest)
{
    evt->dest = dest;
}

/**
 * Set a sequencer event to be a timer event.
 * @param evt Sequencer event structure
 * @param data User supplied data pointer
 */
void
fluid_event_timer(fluid_event_t *evt, void *data)
{
    evt->type = FLUID_SEQ_TIMER;
    evt->data = data;
}

/**
 * Set a sequencer event to be a note on event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param key MIDI note number (0-127)
 * @param vel MIDI velocity value (0-127)
 */
void
fluid_event_noteon(fluid_event_t *evt, int channel, short key, short vel)
{
    evt->type = FLUID_SEQ_NOTEON;
    evt->channel = channel;
    evt->key = key;
    evt->vel = vel;
}

/**
 * Set a sequencer event to be a note off event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param key MIDI note number (0-127)
 */
void
fluid_event_noteoff(fluid_event_t *evt, int channel, short key)
{
    evt->type = FLUID_SEQ_NOTEOFF;
    evt->channel = channel;
    evt->key = key;
}

/**
 * Set a sequencer event to be a note duration event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param key MIDI note number (0-127)
 * @param vel MIDI velocity value (0-127)
 * @param duration Duration of note in the time scale used by the sequencer (by default milliseconds)
 */
void
fluid_event_note(fluid_event_t *evt, int channel, short key, short vel, unsigned int duration)
{
    evt->type = FLUID_SEQ_NOTE;
    evt->channel = channel;
    evt->key = key;
    evt->vel = vel;
    evt->duration = duration;
}

/**
 * Set a sequencer event to be an all sounds off event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 */
void
fluid_event_all_sounds_off(fluid_event_t *evt, int channel)
{
    evt->type = FLUID_SEQ_ALLSOUNDSOFF;
    evt->channel = channel;
}

/**
 * Set a sequencer event to be a all notes off event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 */
void
fluid_event_all_notes_off(fluid_event_t *evt, int channel)
{
    evt->type = FLUID_SEQ_ALLNOTESOFF;
    evt->channel = channel;
}

/**
 * Set a sequencer event to be a bank select event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param bank_num MIDI bank number (0-16383)
 */
void
fluid_event_bank_select(fluid_event_t *evt, int channel, short bank_num)
{
    evt->type = FLUID_SEQ_BANKSELECT;
    evt->channel = channel;
    evt->control = bank_num;
}

/**
 * Set a sequencer event to be a program change event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val MIDI program number (0-127)
 */
void
fluid_event_program_change(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_PROGRAMCHANGE;
    evt->channel = channel;
    evt->value = val;
}

/**
 * Set a sequencer event to be a program select event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param sfont_id SoundFont ID number
 * @param bank_num MIDI bank number (0-16383)
 * @param preset_num MIDI preset number (0-127)
 */
void
fluid_event_program_select(fluid_event_t *evt, int channel,
                           unsigned int sfont_id, short bank_num, short preset_num)
{
    evt->type = FLUID_SEQ_PROGRAMSELECT;
    evt->channel = channel;
    evt->duration = sfont_id;
    evt->value = preset_num;
    evt->control = bank_num;
}

/**
 * Set a sequencer event to be an any control change event (for internal use).
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 */
void
fluid_event_any_control_change(fluid_event_t *evt, int channel)
{
    evt->type = FLUID_SEQ_ANYCONTROLCHANGE;
    evt->channel = channel;
}

/**
 * Set a sequencer event to be a pitch bend event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param pitch MIDI pitch bend value (0-16383, 8192 = no bend)
 */
void
fluid_event_pitch_bend(fluid_event_t *evt, int channel, int pitch)
{
    evt->type = FLUID_SEQ_PITCHBEND;
    evt->channel = channel;

    if(pitch < 0)
    {
        pitch = 0;
    }

    if(pitch > 16383)
    {
        pitch = 16383;
    }

    evt->pitch = pitch;
}

/**
 * Set a sequencer event to be a pitch wheel sensitivity event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param value MIDI pitch wheel sensitivity value in semitones
 */
void
fluid_event_pitch_wheelsens(fluid_event_t *evt, int channel, short value)
{
    evt->type = FLUID_SEQ_PITCHWHEELSENS;
    evt->channel = channel;
    evt->value = value;
}

/**
 * Set a sequencer event to be a modulation event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val MIDI modulation value (0-127)
 */
void
fluid_event_modulation(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_MODULATION;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a MIDI sustain event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val MIDI sustain value (0-127)
 */
void
fluid_event_sustain(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_SUSTAIN;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a MIDI control change event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param control MIDI control number (0-127)
 * @param val MIDI control value (0-127)
 */
void
fluid_event_control_change(fluid_event_t *evt, int channel, short control, short val)
{
    evt->type = FLUID_SEQ_CONTROLCHANGE;
    evt->channel = channel;
    evt->control = control;
    evt->value = val;
}

/**
 * Set a sequencer event to be a stereo pan event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val MIDI panning value (0-127, 0=left, 64 = middle, 127 = right)
 */
void
fluid_event_pan(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_PAN;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a volume event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val Volume value (0-127)
 */
void
fluid_event_volume(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_VOLUME;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a reverb send event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val Reverb amount (0-127)
 */
void
fluid_event_reverb_send(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_REVERBSEND;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a chorus send event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val Chorus amount (0-127)
 */
void
fluid_event_chorus_send(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_CHORUSSEND;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}


/**
 * Set a sequencer event to be an unregistering event.
 * @param evt Sequencer event structure
 * @since 1.1.0
 */
void
fluid_event_unregistering(fluid_event_t *evt)
{
    evt->type = FLUID_SEQ_UNREGISTERING;
}

/**
 * Set a sequencer event to be a channel-wide aftertouch event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param val Aftertouch amount (0-127)
 * @since 1.1.0
 */
void
fluid_event_channel_pressure(fluid_event_t *evt, int channel, short val)
{
    evt->type = FLUID_SEQ_CHANNELPRESSURE;
    evt->channel = channel;

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->value = val;
}

/**
 * Set a sequencer event to be a polyphonic aftertouch event.
 * @param evt Sequencer event structure
 * @param channel MIDI channel number
 * @param key MIDI note number (0-127)
 * @param val Aftertouch amount (0-127)
 * @since 2.0.0
 */
void
fluid_event_key_pressure(fluid_event_t *evt, int channel, short key, short val)
{
    evt->type = FLUID_SEQ_KEYPRESSURE;
    evt->channel = channel;

    if(key < 0)
    {
        key = 0;
    }

    if(key > 127)
    {
        key = 127;
    }

    if(val < 0)
    {
        val = 0;
    }

    if(val > 127)
    {
        val = 127;
    }

    evt->key = key;
    evt->value = val;
}

/**
 * Set a sequencer event to be a midi system reset event.
 * @param evt Sequencer event structure
 * @since 1.1.0
 */
void
fluid_event_system_reset(fluid_event_t *evt)
{
    evt->type = FLUID_SEQ_SYSTEMRESET;
}



/*
 * Accessing event data
 */

/**
 * Get the event type (#fluid_seq_event_type) field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return Event type (#fluid_seq_event_type).
 */
int fluid_event_get_type(fluid_event_t *evt)
{
    return evt->type;
}

/**
 * @internal
 * Get the time field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return Time value
 */
unsigned int fluid_event_get_time(fluid_event_t *evt)
{
    return evt->time;
}

/**
 * Get the source sequencer client from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return source field of the sequencer event
 */
fluid_seq_id_t fluid_event_get_source(fluid_event_t *evt)
{
    return evt->src;
}

/**
 * Get the dest sequencer client from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return dest field of the sequencer event
 */
fluid_seq_id_t fluid_event_get_dest(fluid_event_t *evt)
{
    return evt->dest;
}

/**
 * Get the MIDI channel field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI zero-based channel number
 */
int fluid_event_get_channel(fluid_event_t *evt)
{
    return evt->channel;
}

/**
 * Get the MIDI note field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI note number (0-127)
 */
short fluid_event_get_key(fluid_event_t *evt)
{
    return evt->key;
}

/**
 * Get the MIDI velocity field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI velocity value (0-127)
 */
short fluid_event_get_velocity(fluid_event_t *evt)

{
    return evt->vel;
}

/**
 * Get the MIDI control number field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI control number (0-127)
 */
short fluid_event_get_control(fluid_event_t *evt)
{
    return evt->control;
}

/**
 * Get the value field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return Value field of event.
 *
 * The Value field is used by the following event types:
 * #FLUID_SEQ_PROGRAMCHANGE, #FLUID_SEQ_PROGRAMSELECT (preset_num),
 * #FLUID_SEQ_PITCHWHEELSENS, #FLUID_SEQ_MODULATION, #FLUID_SEQ_SUSTAIN,
 * #FLUID_SEQ_CONTROLCHANGE, #FLUID_SEQ_PAN, #FLUID_SEQ_VOLUME,
 * #FLUID_SEQ_REVERBSEND, #FLUID_SEQ_CHORUSSEND.
 */
short fluid_event_get_value(fluid_event_t *evt)
{
    return evt->value;
}

/**
 * Get the data field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return Data field of event.
 *
 * Used by the #FLUID_SEQ_TIMER event type.
 */
void *fluid_event_get_data(fluid_event_t *evt)
{
    return evt->data;
}

/**
 * Get the duration field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return Note duration value in the time scale used by the sequencer (by default milliseconds)
 *
 * Used by the #FLUID_SEQ_NOTE event type.
 */
unsigned int fluid_event_get_duration(fluid_event_t *evt)
{
    return evt->duration;
}

/**
 * Get the MIDI bank field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI bank number (0-16383)
 *
 * Used by the #FLUID_SEQ_BANKSELECT and #FLUID_SEQ_PROGRAMSELECT
 * event types.
 */
short fluid_event_get_bank(fluid_event_t *evt)
{
    return evt->control;
}

/**
 * Get the pitch field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI pitch bend pitch value (0-16383, 8192 = no bend)
 *
 * Used by the #FLUID_SEQ_PITCHBEND event type.
 */
int fluid_event_get_pitch(fluid_event_t *evt)
{
    return evt->pitch;
}

/**
 * Get the MIDI program field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return MIDI program number (0-127)
 *
 * Used by the #FLUID_SEQ_PROGRAMCHANGE and #FLUID_SEQ_PROGRAMSELECT
 * event types.
 */
short
fluid_event_get_program(fluid_event_t *evt)
{
    return evt->value;
}

/**
 * Get the SoundFont ID field from a sequencer event structure.
 * @param evt Sequencer event structure
 * @return SoundFont identifier value.
 *
 * Used by the #FLUID_SEQ_PROGRAMSELECT event type.
 */
unsigned int
fluid_event_get_sfont_id(fluid_event_t *evt)
{
    return evt->duration;
}



/********************/
/* heap management  */
/********************/

fluid_evt_heap_t *
_fluid_evt_heap_init(int nbEvents)
{
#ifdef HEAP_WITH_DYNALLOC

    int i;
    fluid_evt_heap_t *heap;
    fluid_evt_entry *tmp;

    heap = FLUID_NEW(fluid_evt_heap_t);

    if(heap == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return NULL;
    }

    heap->freelist = NULL;
    fluid_mutex_init(heap->mutex);

    /* LOCK */
    fluid_mutex_lock(heap->mutex);

    /* Allocate the event entries */
    for(i = 0; i < nbEvents; i++)
    {
        tmp = FLUID_NEW(fluid_evt_entry);
        tmp->next = heap->freelist;
        heap->freelist = tmp;
    }

    /* UNLOCK */
    fluid_mutex_unlock(heap->mutex);


#else
    int i;
    fluid_evt_heap_t *heap;
    int siz = 2 * sizeof(fluid_evt_entry *) + sizeof(fluid_evt_entry) * nbEvents;

    heap = (fluid_evt_heap_t *)FLUID_MALLOC(siz);

    if(heap == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return NULL;
    }

    FLUID_MEMSET(heap, 0, siz);

    /* link all heap events */
    {
        fluid_evt_entry *tmp = &(heap->pool);

        for(i = 0 ; i < nbEvents - 1 ; i++)
        {
            tmp[i].next = &(tmp[i + 1]);
        }

        tmp[nbEvents - 1].next = NULL;

        /* set head & tail */
        heap->tail = &(tmp[nbEvents - 1]);
        heap->head = &(heap->pool);
    }
#endif
    return (heap);
}

void
_fluid_evt_heap_free(fluid_evt_heap_t *heap)
{
#ifdef HEAP_WITH_DYNALLOC
    fluid_evt_entry *tmp, *next;

    /* LOCK */
    fluid_mutex_lock(heap->mutex);

    tmp = heap->freelist;

    while(tmp)
    {
        next = tmp->next;
        FLUID_FREE(tmp);
        tmp = next;
    }

    /* UNLOCK */
    fluid_mutex_unlock(heap->mutex);
    fluid_mutex_destroy(heap->mutex);

    FLUID_FREE(heap);

#else
    FLUID_FREE(heap);
#endif
}

fluid_evt_entry *
_fluid_seq_heap_get_free(fluid_evt_heap_t *heap)
{
#ifdef HEAP_WITH_DYNALLOC
    fluid_evt_entry *evt = NULL;

    /* LOCK */
    fluid_mutex_lock(heap->mutex);

#if !defined(MACOS9)

    if(heap->freelist == NULL)
    {
        heap->freelist = FLUID_NEW(fluid_evt_entry);

        if(heap->freelist != NULL)
        {
            heap->freelist->next = NULL;
        }
    }

#endif

    evt = heap->freelist;

    if(evt != NULL)
    {
        heap->freelist = heap->freelist->next;
        evt->next = NULL;
    }

    /* UNLOCK */
    fluid_mutex_unlock(heap->mutex);

    return evt;

#else
    fluid_evt_entry *evt;

    if(heap->head == NULL)
    {
        return NULL;
    }

    /* take from head of the heap */
    /* critical - should threadlock ? */
    evt = heap->head;
    heap->head = heap->head->next;

    return evt;
#endif
}

void
_fluid_seq_heap_set_free(fluid_evt_heap_t *heap, fluid_evt_entry *evt)
{
#ifdef HEAP_WITH_DYNALLOC

    /* LOCK */
    fluid_mutex_lock(heap->mutex);

    evt->next = heap->freelist;
    heap->freelist = evt;

    /* UNLOCK */
    fluid_mutex_unlock(heap->mutex);

#else
    /* append to the end of the heap */
    /* critical - should threadlock ? */
    heap->tail->next = evt;
    heap->tail = evt;
    evt->next = NULL;
#endif
}
