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
*/

#include "fluid_event.h"
#include "fluid_sys.h"	// timer, threads, etc...
#include "fluid_list.h"

/***************************************************************
 *
 *                           SEQUENCER
 */

#define FLUID_SEQUENCER_EVENTS_MAX	1000

/* Private data for SEQUENCER */
struct _fluid_sequencer_t
{
    unsigned int startMs;
    fluid_atomic_int_t currentMs;
    int useSystemTimer;
    double scale; // ticks per second
    fluid_list_t *clients;
    fluid_seq_id_t clientsID;
    /* for queue + heap */
    fluid_evt_entry *preQueue;
    fluid_evt_entry *preQueueLast;
    fluid_timer_t *timer;
    int queue0StartTime;
    short prevCellNb;
    fluid_evt_entry *queue0[256][2];
    fluid_evt_entry *queue1[255][2];
    fluid_evt_entry *queueLater;
    fluid_evt_heap_t *heap;
    fluid_mutex_t mutex;
};

/* Private data for clients */
typedef struct _fluid_sequencer_client_t
{
    fluid_seq_id_t id;
    char *name;
    fluid_event_callback_t callback;
    void *data;
} fluid_sequencer_client_t;

/* prototypes */
static short _fluid_seq_queue_init(fluid_sequencer_t *seq, int nbEvents);
static void _fluid_seq_queue_end(fluid_sequencer_t *seq);
static short _fluid_seq_queue_pre_insert(fluid_sequencer_t *seq, fluid_event_t *evt);
static void _fluid_seq_queue_pre_remove(fluid_sequencer_t *seq, fluid_seq_id_t src, fluid_seq_id_t dest, int type);
static int _fluid_seq_queue_process(void *data, unsigned int msec); // callback from timer
static void _fluid_seq_queue_insert_entry(fluid_sequencer_t *seq, fluid_evt_entry *evtentry);
static void _fluid_seq_queue_remove_entries_matching(fluid_sequencer_t *seq, fluid_evt_entry *temp);
static void _fluid_seq_queue_send_queued_events(fluid_sequencer_t *seq);
static void _fluid_free_evt_queue(fluid_evt_entry **first, fluid_evt_entry **last);


/* API implementation */

/**
 * Create a new sequencer object which uses the system timer.  Use
 * new_fluid_sequencer2() to specify whether the system timer or
 * fluid_sequencer_process() is used to advance the sequencer.
 * @return New sequencer instance
 * @deprecated As of fluidsynth 2.1.1 the use of the system timer has been deprecated.
 */
fluid_sequencer_t *
new_fluid_sequencer(void)
{
    return new_fluid_sequencer2(TRUE);
}

/**
 * Create a new sequencer object.
 * @param use_system_timer If TRUE, sequencer will advance at the rate of the
 *   system clock. If FALSE, call fluid_sequencer_process() to advance
 *   the sequencer.
 * @return New sequencer instance
 * @since 1.1.0
 * @note As of fluidsynth 2.1.1 the use of the system timer has been deprecated.
 */
fluid_sequencer_t *
new_fluid_sequencer2(int use_system_timer)
{
    fluid_sequencer_t *seq;

    if(use_system_timer)
    {
        FLUID_LOG(FLUID_WARN, "sequencer: Usage of the system timer has been deprecated!");
    }

    seq = FLUID_NEW(fluid_sequencer_t);

    if(seq == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return NULL;
    }

    FLUID_MEMSET(seq, 0, sizeof(fluid_sequencer_t));

    seq->scale = 1000;	// default value
    seq->useSystemTimer = use_system_timer ? 1 : 0;
    seq->startMs = seq->useSystemTimer ? fluid_curtime() : 0;
    seq->clients = NULL;
    seq->clientsID = 0;

    if(-1 == _fluid_seq_queue_init(seq, FLUID_SEQUENCER_EVENTS_MAX))
    {
        FLUID_FREE(seq);
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return NULL;
    }

    return(seq);
}

/**
 * Free a sequencer object.
 * @note Before fluidsynth 2.1.1 registered sequencer clients may not be fully freed by this function.
 * @param seq Sequencer to delete
 */
void
delete_fluid_sequencer(fluid_sequencer_t *seq)
{
    fluid_return_if_fail(seq != NULL);

    /* cleanup clients */
    while(seq->clients)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)seq->clients->data;
        fluid_sequencer_unregister_client(seq, client->id);
    }

    _fluid_seq_queue_end(seq);

    FLUID_FREE(seq);
}

/**
 * Check if a sequencer is using the system timer or not.
 * @param seq Sequencer object
 * @return TRUE if system timer is being used, FALSE otherwise.
 * @since 1.1.0
 * @deprecated As of fluidsynth 2.1.1 the usage of the system timer has been deprecated.
 */
int
fluid_sequencer_get_use_system_timer(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, FALSE);

    return seq->useSystemTimer;
}


/* clients */

/**
 * Register a sequencer client.
 * @param seq Sequencer object
 * @param name Name of sequencer client
 * @param callback Sequencer client callback or NULL for a source client.
 * @param data User data to pass to the \a callback
 * @return Unique sequencer ID or #FLUID_FAILED on error
 *
 * Clients can be sources or destinations of events. Sources don't need to
 * register a callback.
 *
 * @note Implementations are encouraged to explicitly unregister any registered client with fluid_sequencer_unregister_client() before deleting the sequencer.
 */
fluid_seq_id_t
fluid_sequencer_register_client(fluid_sequencer_t *seq, const char *name,
                                fluid_event_callback_t callback, void *data)
{
    fluid_sequencer_client_t *client;
    char *nameCopy;

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);

    client = FLUID_NEW(fluid_sequencer_client_t);

    if(client == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return FLUID_FAILED;
    }

    nameCopy = FLUID_STRDUP(name);

    if(nameCopy == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        FLUID_FREE(client);
        return FLUID_FAILED;
    }

    seq->clientsID++;

    client->name = nameCopy;
    client->id = seq->clientsID;
    client->callback = callback;
    client->data = data;

    seq->clients = fluid_list_append(seq->clients, (void *)client);

    return (client->id);
}

/**
 * Unregister a previously registered client.
 *
 * The client's callback function will receive a FLUID_SEQ_UNREGISTERING event right before it is being unregistered.
 * @param seq Sequencer object
 * @param id Client ID as returned by fluid_sequencer_register_client().
 */
void
fluid_sequencer_unregister_client(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;
    fluid_event_t evt;
    unsigned int now = fluid_sequencer_get_tick(seq);

    fluid_return_if_fail(seq != NULL);

    fluid_event_clear(&evt);
    fluid_event_unregistering(&evt);
    fluid_event_set_dest(&evt, id);
    fluid_event_set_time(&evt, now);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            // client found, remove it from the list to avoid recursive call when calling callback
            seq->clients = fluid_list_remove_link(seq->clients, tmp);

            // call the callback (if any), to free underlying memory (e.g. seqbind structure)
            if (client->callback != NULL)
            {
                (client->callback)(now, &evt, seq, client->data);
            }

            if(client->name)
            {
                FLUID_FREE(client->name);
            }
            delete1_fluid_list(tmp);
            FLUID_FREE(client);
            return;
        }

        tmp = tmp->next;
    }

    return;
}

/**
 * Count a sequencers registered clients.
 * @param seq Sequencer object
 * @return Count of sequencer clients.
 */
int
fluid_sequencer_count_clients(fluid_sequencer_t *seq)
{
    if(seq == NULL || seq->clients == NULL)
    {
        return 0;
    }

    return fluid_list_size(seq->clients);
}

/**
 * Get a client ID from its index (order in which it was registered).
 * @param seq Sequencer object
 * @param index Index of register client
 * @return Client ID or #FLUID_FAILED if not found
 */
fluid_seq_id_t fluid_sequencer_get_client_id(fluid_sequencer_t *seq, int index)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(index >= 0, FLUID_FAILED);

    tmp = fluid_list_nth(seq->clients, index);

    if(tmp == NULL)
    {
        return FLUID_FAILED;
    }
    else
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;
        return client->id;
    }
}

/**
 * Get the name of a registered client.
 * @param seq Sequencer object
 * @param id Client ID
 * @return Client name or NULL if not found.  String is internal and should not
 *   be modified or freed.
 */
char *
fluid_sequencer_get_client_name(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, NULL);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            return client->name;
        }

        tmp = tmp->next;
    }

    return NULL;
}

/**
 * Check if a client is a destination client.
 * @param seq Sequencer object
 * @param id Client ID
 * @return TRUE if client is a destination client, FALSE otherwise or if not found
 */
int
fluid_sequencer_client_is_dest(fluid_sequencer_t *seq, fluid_seq_id_t id)
{
    fluid_list_t *tmp;

    fluid_return_val_if_fail(seq != NULL, FALSE);

    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *client = (fluid_sequencer_client_t *)tmp->data;

        if(client->id == id)
        {
            return (client->callback != NULL);
        }

        tmp = tmp->next;
    }

    return FALSE;
}

/**
 * Send an event immediately.
 * @param seq Sequencer object
 * @param evt Event to send (not copied, used directly)
 */
void
fluid_sequencer_send_now(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    fluid_seq_id_t destID;
    fluid_list_t *tmp;

    fluid_return_if_fail(seq != NULL);
    fluid_return_if_fail(evt != NULL);

    destID = fluid_event_get_dest(evt);

    /* find callback */
    tmp = seq->clients;

    while(tmp)
    {
        fluid_sequencer_client_t *dest = (fluid_sequencer_client_t *)tmp->data;

        if(dest->id == destID)
        {
            if(fluid_event_get_type(evt) == FLUID_SEQ_UNREGISTERING)
            {
                fluid_sequencer_unregister_client(seq, destID);
            }
            else
            {
                if(dest->callback)
                {
                    (dest->callback)(fluid_sequencer_get_tick(seq), evt, seq, dest->data);
                }
            }
            return;
        }

        tmp = tmp->next;
    }
}

/**
 * Schedule an event for sending at a later time.
 * @param seq Sequencer object
 * @param evt Event to send (will be copied into internal queue)
 * @param time Time value in ticks (in milliseconds with the default time scale of 1000).
 * @param absolute TRUE if \a time is absolute sequencer time (time since sequencer
 *   creation), FALSE if relative to current time.
 * @return #FLUID_OK on success, #FLUID_FAILED otherwise
 */
int
fluid_sequencer_send_at(fluid_sequencer_t *seq, fluid_event_t *evt,
                        unsigned int time, int absolute)
{
    unsigned int now = fluid_sequencer_get_tick(seq);

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(evt != NULL, FLUID_FAILED);

    /* set absolute */
    if(!absolute)
    {
        time = now + time;
    }

    /* time stamp event */
    fluid_event_set_time(evt, time);

    /* queue for processing later */
    return _fluid_seq_queue_pre_insert(seq, evt);
}

/**
 * Remove events from the event queue.
 * @param seq Sequencer object
 * @param source Source client ID to match or -1 for wildcard
 * @param dest Destination client ID to match or -1 for wildcard
 * @param type Event type to match or -1 for wildcard (#fluid_seq_event_type)
 */
void
fluid_sequencer_remove_events(fluid_sequencer_t *seq, fluid_seq_id_t source,
                              fluid_seq_id_t dest, int type)
{
    fluid_return_if_fail(seq != NULL);

    _fluid_seq_queue_pre_remove(seq, source, dest, type);
}


/*************************************
	time
**************************************/

/**
 * Get the current tick of a sequencer.
 * @param seq Sequencer object
 * @return Current tick value
 */
unsigned int
fluid_sequencer_get_tick(fluid_sequencer_t *seq)
{
    unsigned int absMs;
    double nowFloat;
    unsigned int now;

    fluid_return_val_if_fail(seq != NULL, 0u);

    absMs = seq->useSystemTimer ? (int) fluid_curtime() : fluid_atomic_int_get(&seq->currentMs);
    nowFloat = ((double)(absMs - seq->startMs)) * seq->scale / 1000.0f;
    now = nowFloat;
    return now;
}

/**
 * Set the time scale of a sequencer.
 * @param seq Sequencer object
 * @param scale Sequencer scale value in ticks per second
 *   (default is 1000 for 1 tick per millisecond, max is 1000.0)
 *
 * If there are already scheduled events in the sequencer and the scale is changed
 * the events are adjusted accordingly.
 */
void
fluid_sequencer_set_time_scale(fluid_sequencer_t *seq, double scale)
{
    fluid_return_if_fail(seq != NULL);

    if(scale <= 0)
    {
        FLUID_LOG(FLUID_WARN, "sequencer: scale <= 0 : %f\n", scale);
        return;
    }

    if(scale > 1000.0)
        // Otherwise : problems with the timer = 0ms...
    {
        scale = 1000.0;
    }

    if(seq->scale != scale)
    {
        double oldScale = seq->scale;

        // stop timer
        if(seq->timer)
        {
            delete_fluid_timer(seq->timer);
            seq->timer = NULL;
        }

        seq->scale = scale;

        // change start0 so that cellNb is preserved
        seq->queue0StartTime = (seq->queue0StartTime + seq->prevCellNb) * (seq->scale / oldScale) - seq->prevCellNb;

        // change all preQueue events for new scale
        {
            fluid_evt_entry *tmp;
            tmp = seq->preQueue;

            while(tmp)
            {
                if(tmp->entryType == FLUID_EVT_ENTRY_INSERT)
                {
                    tmp->evt.time = tmp->evt.time * seq->scale / oldScale;
                }

                tmp = tmp->next;
            }
        }

        /* re-start timer */
        if(seq->useSystemTimer)
        {
            seq->timer = new_fluid_timer((int)(1000 / seq->scale), _fluid_seq_queue_process, (void *)seq, TRUE, FALSE, TRUE);
        }
    }
}

/**
 * Get a sequencer's time scale.
 * @param seq Sequencer object.
 * @return Time scale value in ticks per second.
 */
double
fluid_sequencer_get_time_scale(fluid_sequencer_t *seq)
{
    fluid_return_val_if_fail(seq != NULL, 0);
    return seq->scale;
}


/**********************

      the queue

**********************/

/*
  The queue stores all future events to be processed.

  Data structures

  There is a heap, allocated at init time, for managing a pool
  of event entries, that is description of an event, its time,
  and whether it is a normal event or a removal command.

  The queue is separated in two arrays, and a list.  The first
  array 'queue0' corresponds to the events to be sent in the
  next 256 ticks (0 to 255), the second array 'queue1' contains
  the events to be send from now+256 to now+65535. The list
  called 'queueLater' contains the events to be sent later than
  that. In each array, one cell contains a list of events having
  the same time (in the queue0 array), or the same time/256 (in
  the queue1 array), and a pointer to the last event in the list
  of the cell so as to be able to insert fast at the end of the
  list (i.e. a cell = 2 pointers).  The 'queueLater' list is
  ordered by time and by post time.  This way, inserting 'soon'
  events is fast (below 65535 ticks, that is about 1 minute if 1
  tick=1ms).  Inserting later events is more slow, but this is a
  realtime engine, isn't it ?

  The queue0 starts at queue0StartTime.  When 256 ticks have
  elapsed, the queue0 array is emptied, and the first cell of
  the queue1 array is expanded in the queue0 array, according to
  the time of each event. The queue1 array is shifted to the
  left, and the first events of the queueLater list are inserte
  in the last cell of the queue1 array.

  We remember the previously managed cell in queue0 in the
  prevCellNb variable. When processing the current cell, we
  process the events in between (late events).

  Functions

  The main thread functions first get an event entry from the
  heap, and copy the given event into it, then merely enqueue it
  in a preQueue. This is in order to protect the data structure:
  everything is managed in the callback (thread or interrupt,
  depending on the architecture).

  All queue data structure management is done in a timer
  callback: '_fluid_seq_queue_process'.  The
  _fluid_seq_queue_process function first process the preQueue,
  inserting or removing event entries from the queue, then
  processes the queue, by sending events ready to be sent at the
  current time.

  Critical sections between the main thread (or app) and the
  sequencer thread (or interrupt) are:

  - the heap management (if two threads get a free event at the
  same time)
  - the preQueue access.

  These are really small and fast sections (merely a pointer or
  two changing value). They are not protected by a mutex for now
  (August 2002). Waiting for crossplatform mutex solutions. When
  changing this code, beware that the
  _fluid_seq_queue_pre_insert function may be called by the
  callback of the queue thread (ex : a note event inserts a
  noteoff event).

*/


/********************/
/*       API        */
/********************/

static short
_fluid_seq_queue_init(fluid_sequencer_t *seq, int maxEvents)
{
    seq->heap = _fluid_evt_heap_init(maxEvents);

    if(seq->heap == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return -1;
    }

    seq->preQueue = NULL;
    seq->preQueueLast = NULL;

    FLUID_MEMSET(seq->queue0, 0, 2 * 256 * sizeof(fluid_evt_entry *));
    FLUID_MEMSET(seq->queue1, 0, 2 * 255 * sizeof(fluid_evt_entry *));

    seq->queueLater = NULL;
    seq->queue0StartTime = fluid_sequencer_get_tick(seq);
    seq->prevCellNb = -1;

    fluid_mutex_init(seq->mutex);

    /* start timer */
    if(seq->useSystemTimer)
    {
        seq->timer = new_fluid_timer((int)(1000 / seq->scale), _fluid_seq_queue_process,
                                     (void *)seq, TRUE, FALSE, TRUE);
    }

    return (0);
}

static void
_fluid_seq_queue_end(fluid_sequencer_t *seq)
{
    int i;

    /* free all remaining events */
    _fluid_free_evt_queue(&seq->preQueue, &seq->preQueueLast);

    for(i = 0; i < 256; i++)
    {
        _fluid_free_evt_queue(&(seq->queue0[i][0]), &(seq->queue0[i][1]));
    }

    for(i = 0; i < 255; i++)
    {
        _fluid_free_evt_queue(&(seq->queue1[i][0]), &(seq->queue1[i][1]));
    }

    _fluid_free_evt_queue(&seq->queueLater, NULL);


    if(seq->timer)
    {
        delete_fluid_timer(seq->timer);
        seq->timer = NULL;
    }

    if(seq->heap)
    {
        _fluid_evt_heap_free(seq->heap);
        seq->heap = NULL;
    }

    fluid_mutex_destroy(seq->mutex);
}



/********************/
/* queue management */
/********************/

/* Create event_entry and append to the preQueue.
 * May be called from the main thread (usually) but also recursively
 * from the queue thread, when a callback itself does an insert... */
static short
_fluid_seq_queue_pre_insert(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    fluid_evt_entry *evtentry = _fluid_seq_heap_get_free(seq->heap);

    if(evtentry == NULL)
    {
        /* should not happen */
        FLUID_LOG(FLUID_PANIC, "sequencer: no more free events\n");
        return -1;
    }

    evtentry->next = NULL;
    evtentry->entryType = FLUID_EVT_ENTRY_INSERT;
    FLUID_MEMCPY(&(evtentry->evt), evt, sizeof(fluid_event_t));

    fluid_mutex_lock(seq->mutex);

    /* append to preQueue */
    if(seq->preQueueLast)
    {
        seq->preQueueLast->next = evtentry;
    }
    else
    {
        seq->preQueue = evtentry;
    }

    seq->preQueueLast = evtentry;

    fluid_mutex_unlock(seq->mutex);

    return (0);
}

/* Create event_entry and append to the preQueue.
 * May be called from the main thread (usually) but also recursively
 * from the queue thread, when a callback itself does an insert... */
static void
_fluid_seq_queue_pre_remove(fluid_sequencer_t *seq, fluid_seq_id_t src, fluid_seq_id_t dest, int type)
{
    fluid_evt_entry *evtentry = _fluid_seq_heap_get_free(seq->heap);

    if(evtentry == NULL)
    {
        /* should not happen */
        FLUID_LOG(FLUID_PANIC, "sequencer: no more free events\n");
        return;
    }

    evtentry->next = NULL;
    evtentry->entryType = FLUID_EVT_ENTRY_REMOVE;
    {
        fluid_event_t *evt = &(evtentry->evt);
        fluid_event_set_source(evt, src);
        fluid_event_set_source(evt, src);
        fluid_event_set_dest(evt, dest);
        evt->type = type;
    }

    fluid_mutex_lock(seq->mutex);

    /* append to preQueue */
    if(seq->preQueueLast)
    {
        seq->preQueueLast->next = evtentry;
    }
    else
    {
        seq->preQueue = evtentry;
    }

    seq->preQueueLast = evtentry;

    fluid_mutex_unlock(seq->mutex);
    return;
}

static void
_fluid_free_evt_queue(fluid_evt_entry **first, fluid_evt_entry **last)
{
    fluid_evt_entry *tmp2;
    fluid_evt_entry *tmp = *first;

    while(tmp != NULL)
    {
        tmp2 = tmp->next;
        FLUID_FREE(tmp);
        tmp = tmp2;
    }

    *first = NULL;

    if(last != NULL)
    {
        *last = NULL;
    }
}

/* Callback from timer (may be in a different thread, or in an interrupt) */
static int
_fluid_seq_queue_process(void *data, unsigned int msec)
{
    fluid_sequencer_t *seq = (fluid_sequencer_t *)data;
    fluid_sequencer_process(seq, msec);
    /* continue timer */
    return 1;
}

/**
 * Advance a sequencer.
 *
 * If you have registered the synthesizer as client (fluid_sequencer_register_fluidsynth()), the synth
 * will take care of calling fluid_sequencer_process(). Otherwise it is up to the user to
 * advance the sequencer manually.
 * @param seq Sequencer object
 * @param msec Time to advance sequencer to (absolute time since sequencer start).
 * @since 1.1.0
 */
void
fluid_sequencer_process(fluid_sequencer_t *seq, unsigned int msec)
{

    /* process prequeue */
    fluid_evt_entry *tmp;
    fluid_evt_entry *next;

    fluid_mutex_lock(seq->mutex);

    /* get the preQueue */
    tmp = seq->preQueue;
    seq->preQueue = NULL;
    seq->preQueueLast = NULL;

    fluid_mutex_unlock(seq->mutex);

    /* walk all the preQueue and process them in order : inserts and removes */
    while(tmp)
    {
        next = tmp->next;

        if(tmp->entryType == FLUID_EVT_ENTRY_REMOVE)
        {
            _fluid_seq_queue_remove_entries_matching(seq, tmp);
        }
        else
        {
            _fluid_seq_queue_insert_entry(seq, tmp);
        }

        tmp = next;
    }

    /* send queued events */
    fluid_atomic_int_set(&seq->currentMs, msec);
    _fluid_seq_queue_send_queued_events(seq);

}

#if 0
static void
_fluid_seq_queue_print_later(fluid_sequencer_t *seq)
{
    int count = 0;
    fluid_evt_entry *tmp = seq->queueLater;

    printf("queueLater:\n");

    while(tmp)
    {
        unsigned int delay = tmp->evt.time - seq->queue0StartTime;
        printf("queueLater: Delay = %i\n", delay);
        tmp = tmp->next;
        count++;
    }

    printf("queueLater: Total of %i events\n", count);
}
#endif

static void
_fluid_seq_queue_insert_queue0(fluid_sequencer_t *seq, fluid_evt_entry *tmp, int cell)
{
    if(seq->queue0[cell][1] == NULL)
    {
        seq->queue0[cell][1] = seq->queue0[cell][0] = tmp;
    }
    else
    {
        seq->queue0[cell][1]->next = tmp;
        seq->queue0[cell][1] = tmp;
    }

    tmp->next = NULL;
}

static void
_fluid_seq_queue_insert_queue1(fluid_sequencer_t *seq, fluid_evt_entry *tmp, int cell)
{
    if(seq->queue1[cell][1] == NULL)
    {
        seq->queue1[cell][1] = seq->queue1[cell][0] = tmp;
    }
    else
    {
        seq->queue1[cell][1]->next = tmp;
        seq->queue1[cell][1] = tmp;
    }

    tmp->next = NULL;
}

static void
_fluid_seq_queue_insert_queue_later(fluid_sequencer_t *seq, fluid_evt_entry *evtentry)
{
    fluid_evt_entry *prev;
    fluid_evt_entry *tmp;
    unsigned int time = evtentry->evt.time;

    /* insert in 'queueLater', after the ones that have the same
     * time */

    /* first? */
    if((seq->queueLater == NULL)
            || (seq->queueLater->evt.time > time))
    {
        evtentry->next = seq->queueLater;
        seq->queueLater = evtentry;
        return;
    }

    /* walk queueLater */
    /* this is the only slow thing : if the event is more
       than 65535 ticks after the current time */

    prev = seq->queueLater;
    tmp = prev->next;

    while(tmp)
    {
        if(tmp->evt.time > time)
        {
            /* insert before tmp */
            evtentry->next = tmp;
            prev->next = evtentry;
            return;
        }

        prev = tmp;
        tmp = prev->next;
    }

    /* last */
    evtentry->next = NULL;
    prev->next = evtentry;
}

static void
_fluid_seq_queue_insert_entry(fluid_sequencer_t *seq, fluid_evt_entry *evtentry)
{
    /* time is relative to seq origin, in ticks */
    fluid_event_t *evt = &(evtentry->evt);
    unsigned int time = evt->time;
    unsigned int delay;

    if(seq->queue0StartTime > 0)
    {
        /* queue0StartTime could be < 0 if the scale changed a
           lot early, breaking the following comparison
        */
        if(time < (unsigned int)seq->queue0StartTime)
        {
            /* we are late, send now */
            fluid_sequencer_send_now(seq, evt);

            _fluid_seq_heap_set_free(seq->heap, evtentry);
            return;
        }
    }

    if(seq->prevCellNb >= 0)
    {
        /* prevCellNb could be -1 is seq was just started - unlikely */
        /* prevCellNb can also be -1 if cellNb was reset to 0 in
           _fluid_seq_queue_send_queued_events() */
        if(time <= (unsigned int)(seq->queue0StartTime + seq->prevCellNb))
        {
            /* we are late, send now */
            fluid_sequencer_send_now(seq, evt);

            _fluid_seq_heap_set_free(seq->heap, evtentry);
            return;
        }
    }

    delay = time - seq->queue0StartTime;

    if(delay > 65535)
    {
        _fluid_seq_queue_insert_queue_later(seq, evtentry);

    }
    else if(delay > 255)
    {
        _fluid_seq_queue_insert_queue1(seq, evtentry, delay / 256 - 1);

    }
    else
    {
        _fluid_seq_queue_insert_queue0(seq, evtentry, delay);
    }
}

static int
_fluid_seq_queue_matchevent(fluid_event_t *evt, int templType, fluid_seq_id_t templSrc, fluid_seq_id_t templDest)
{
    int eventType;

    if(templSrc != -1 && templSrc != fluid_event_get_source(evt))
    {
        return 0;
    }

    if(templDest != -1 && templDest != fluid_event_get_dest(evt))
    {
        return 0;
    }

    if(templType == -1)
    {
        return 1;
    }

    eventType = fluid_event_get_type(evt);

    if(templType == eventType)
    {
        return 1;
    }

    if(templType == FLUID_SEQ_ANYCONTROLCHANGE)
    {
        if(eventType == FLUID_SEQ_PITCHBEND ||
                eventType == FLUID_SEQ_MODULATION ||
                eventType == FLUID_SEQ_SUSTAIN ||
                eventType == FLUID_SEQ_PAN ||
                eventType == FLUID_SEQ_VOLUME ||
                eventType == FLUID_SEQ_REVERBSEND ||
                eventType == FLUID_SEQ_CONTROLCHANGE ||
                eventType == FLUID_SEQ_CHORUSSEND)
        {
            return 1;
        }
    }

    return 0;
}

static void
_fluid_seq_queue_remove_entries_matching(fluid_sequencer_t *seq, fluid_evt_entry *templ)
{
    /* we walk everything : this is slow, but that is life */
    int i, type;
    fluid_seq_id_t src, dest;

    src = templ->evt.src;
    dest = templ->evt.dest;
    type = templ->evt.type;

    /* we can set it free now */
    _fluid_seq_heap_set_free(seq->heap, templ);

    /* queue0 */
    for(i = 0 ; i < 256 ; i++)
    {
        fluid_evt_entry *tmp = seq->queue0[i][0];
        fluid_evt_entry *prev = NULL;

        while(tmp)
        {
            /* remove and/or walk */
            if(_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest))
            {
                /* remove */
                if(prev)
                {
                    prev->next = tmp->next;

                    if(tmp == seq->queue0[i][1])  // last one in list
                    {
                        seq->queue0[i][1] = prev;
                    }

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = prev->next;
                }
                else
                {
                    /* first one in list */
                    seq->queue0[i][0] = tmp->next;

                    if(tmp == seq->queue0[i][1])  // last one in list
                    {
                        seq->queue0[i][1] = NULL;
                    }

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = seq->queue0[i][0];
                }
            }
            else
            {
                prev = tmp;
                tmp = prev->next;
            }
        }
    }

    /* queue1 */
    for(i = 0 ; i < 255 ; i++)
    {
        fluid_evt_entry *tmp = seq->queue1[i][0];
        fluid_evt_entry *prev = NULL;

        while(tmp)
        {
            if(_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest))
            {
                /* remove */
                if(prev)
                {
                    prev->next = tmp->next;

                    if(tmp == seq->queue1[i][1])  // last one in list
                    {
                        seq->queue1[i][1] = prev;
                    }

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = prev->next;
                }
                else
                {
                    /* first one in list */
                    seq->queue1[i][0] = tmp->next;

                    if(tmp == seq->queue1[i][1])  // last one in list
                    {
                        seq->queue1[i][1] = NULL;
                    }

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = seq->queue1[i][0];
                }
            }
            else
            {
                prev = tmp;
                tmp = prev->next;
            }
        }
    }

    /* queueLater */
    {
        fluid_evt_entry *tmp = seq->queueLater;
        fluid_evt_entry *prev = NULL;

        while(tmp)
        {
            if(_fluid_seq_queue_matchevent((&tmp->evt), type, src, dest))
            {
                /* remove */
                if(prev)
                {
                    prev->next = tmp->next;

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = prev->next;
                }
                else
                {
                    seq->queueLater = tmp->next;

                    _fluid_seq_heap_set_free(seq->heap, tmp);
                    tmp = seq->queueLater;
                }
            }
            else
            {
                prev = tmp;
                tmp = prev->next;
            }
        }
    }
}

static void
_fluid_seq_queue_send_cell_events(fluid_sequencer_t *seq, int cellNb)
{
    fluid_evt_entry *next;
    fluid_evt_entry *tmp;

    tmp = seq->queue0[cellNb][0];

    while(tmp)
    {
        fluid_sequencer_send_now(seq, &(tmp->evt));

        next = tmp->next;

        _fluid_seq_heap_set_free(seq->heap, tmp);
        tmp = next;
    }

    seq->queue0[cellNb][0] = NULL;
    seq->queue0[cellNb][1] = NULL;
}

static void
_fluid_seq_queue_slide(fluid_sequencer_t *seq)
{
    short i;
    fluid_evt_entry *next;
    fluid_evt_entry *tmp;
    int count = 0;

    /* do the slide */
    seq->queue0StartTime += 256;

    /* sort all queue1[0] into queue0 according to new queue0StartTime */
    tmp = seq->queue1[0][0];

    while(tmp)
    {
        unsigned int delay = tmp->evt.time - seq->queue0StartTime;
        next = tmp->next;

        if(delay > 255)
        {
            /* should not happen !! */
            /* append it to queue1[1] */
            _fluid_seq_queue_insert_queue1(seq, tmp, 1);
        }
        else
        {
            _fluid_seq_queue_insert_queue0(seq, tmp, delay);
        }

        tmp = next;
        count++;
    }

    /* slide all queue1[i] into queue1[i-1] */
    for(i = 1 ; i < 255 ; i++)
    {
        seq->queue1[i - 1][0] = seq->queue1[i][0];
        seq->queue1[i - 1][1] = seq->queue1[i][1];
    }

    seq->queue1[254][0] = NULL;
    seq->queue1[254][1] = NULL;


    /* append queueLater to queue1[254] */
    count = 0;
    tmp = seq->queueLater;

    while(tmp)
    {
        unsigned int delay = tmp->evt.time - seq->queue0StartTime;

        if(delay > 65535)
        {
            break;
        }

        next = tmp->next;

        /* append it */
        _fluid_seq_queue_insert_queue1(seq, tmp, 254);
        tmp = next;
        count++;
    }

    seq->queueLater = tmp;
}

static void
_fluid_seq_queue_send_queued_events(fluid_sequencer_t *seq)
{
    unsigned int nowTicks = fluid_sequencer_get_tick(seq);
    short cellNb;

    cellNb = seq->prevCellNb + 1;

    while(cellNb <= (int)(nowTicks - seq->queue0StartTime))
    {
        if(cellNb == 256)
        {
            cellNb = 0;
            _fluid_seq_queue_slide(seq);
        } /* slide */

        /* process queue0[cellNb] */
        _fluid_seq_queue_send_cell_events(seq, cellNb);

        /* the current scale may have changed through a callback event */
        nowTicks = fluid_sequencer_get_tick(seq);

        /* next cell */
        cellNb++;
    }

    seq->prevCellNb = cellNb - 1;
}
