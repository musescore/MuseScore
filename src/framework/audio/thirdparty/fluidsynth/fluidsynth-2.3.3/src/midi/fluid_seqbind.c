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

#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_event.h"
#include "fluid_seqbind_notes.h"

/***************************************************************
*
*                           SEQUENCER BINDING
*/

struct _fluid_seqbind_t
{
    fluid_synth_t *synth;
    fluid_sequencer_t *seq;
    fluid_sample_timer_t *sample_timer;
    fluid_seq_id_t client_id;
    void* note_container;
};
typedef struct _fluid_seqbind_t fluid_seqbind_t;

extern void fluid_sequencer_invalidate_note(fluid_sequencer_t *seq, fluid_seq_id_t dest, fluid_note_id_t id);

int fluid_seqbind_timer_callback(void *data, unsigned int msec);
void fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data);

/* Proper cleanup of the seqbind struct. */
void
delete_fluid_seqbind(fluid_seqbind_t *seqbind)
{
    fluid_return_if_fail(seqbind != NULL);

    if((seqbind->client_id != -1) && (seqbind->seq != NULL))
    {
        fluid_sequencer_unregister_client(seqbind->seq, seqbind->client_id);
        seqbind->client_id = -1;
    }

    if((seqbind->sample_timer != NULL) && (seqbind->synth != NULL))
    {
        delete_fluid_sample_timer(seqbind->synth, seqbind->sample_timer);
        seqbind->sample_timer = NULL;
    }

    delete_fluid_note_container(seqbind->note_container);
    FLUID_FREE(seqbind);
}

/**
 * Registers a synthesizer as a destination client of the given sequencer.
 *
 * @param seq Sequencer instance
 * @param synth Synthesizer instance
 * @returns Sequencer client ID, or #FLUID_FAILED on error.
 *
 * A convenience wrapper function around fluid_sequencer_register_client(), that allows you to
 * easily process and render enqueued sequencer events with fluidsynth's synthesizer.
 * The client being registered will be named @c fluidsynth.
 *
 * @note Implementations are encouraged to explicitly unregister this client either by calling
 * fluid_sequencer_unregister_client() or by sending an unregistering event to the sequencer. Before
 * fluidsynth 2.1.1 this was mandatory to avoid memory leaks.
 *
 * @code{.cpp}
 * fluid_seq_id_t seqid = fluid_sequencer_register_fluidsynth(seq, synth);
 *
 * // ... do work
 *
 * fluid_event_t* evt = new_fluid_event();
 * fluid_event_set_source(evt, -1);
 * fluid_event_set_dest(evt, seqid);
 * fluid_event_unregistering(evt);
 *
 * // unregister the "fluidsynth" client immediately
 * fluid_sequencer_send_now(seq, evt);
 * delete_fluid_event(evt);
 * delete_fluid_synth(synth);
 * delete_fluid_sequencer(seq);
 * @endcode
 */
fluid_seq_id_t
fluid_sequencer_register_fluidsynth(fluid_sequencer_t *seq, fluid_synth_t *synth)
{
    fluid_seqbind_t *seqbind;

    fluid_return_val_if_fail(seq != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(synth != NULL, FLUID_FAILED);

    seqbind = FLUID_NEW(fluid_seqbind_t);

    if(seqbind == NULL)
    {
        FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
        return FLUID_FAILED;
    }

    FLUID_MEMSET(seqbind, 0, sizeof(*seqbind));

    seqbind->client_id = -1;
    seqbind->synth = synth;
    seqbind->seq = seq;

    /* set up the sample timer */
    if(!fluid_sequencer_get_use_system_timer(seq))
    {
        seqbind->sample_timer =
            new_fluid_sample_timer(synth, fluid_seqbind_timer_callback, (void *) seqbind);

        if(seqbind->sample_timer == NULL)
        {
            FLUID_LOG(FLUID_PANIC, "sequencer: Out of memory\n");
            FLUID_FREE(seqbind);
            return FLUID_FAILED;
        }
    }

    seqbind->note_container = new_fluid_note_container();
    if(seqbind->note_container == NULL)
    {
        delete_fluid_sample_timer(seqbind->synth, seqbind->sample_timer);
        FLUID_FREE(seqbind);
        return FLUID_FAILED;
    }

    /* register fluidsynth itself */
    seqbind->client_id =
        fluid_sequencer_register_client(seq, "fluidsynth", fluid_seq_fluidsynth_callback, (void *)seqbind);

    if(seqbind->client_id == FLUID_FAILED)
    {
        delete_fluid_note_container(seqbind->note_container);
        delete_fluid_sample_timer(seqbind->synth, seqbind->sample_timer);
        FLUID_FREE(seqbind);
        return FLUID_FAILED;
    }

    return seqbind->client_id;
}

/* Callback for sample timer */
int
fluid_seqbind_timer_callback(void *data, unsigned int msec)
{
    fluid_seqbind_t *seqbind = (fluid_seqbind_t *) data;
    fluid_sequencer_process(seqbind->seq, msec);
    return 1;
}

/* Callback for midi events */
void
fluid_seq_fluidsynth_callback(unsigned int time, fluid_event_t *evt, fluid_sequencer_t *seq, void *data)
{
    fluid_synth_t *synth;
    fluid_seqbind_t *seqbind = (fluid_seqbind_t *) data;
    synth = seqbind->synth;

    switch(fluid_event_get_type(evt))
    {
    case FLUID_SEQ_NOTEON:
        fluid_synth_noteon(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt), fluid_event_get_velocity(evt));
        break;

    case FLUID_SEQ_NOTEOFF:
    {
        fluid_note_id_t id = fluid_event_get_id(evt);
        if(id != -1)
        {
            fluid_note_container_remove(seqbind->note_container, id);
        }
        fluid_synth_noteoff(synth, fluid_event_get_channel(evt), fluid_event_get_key(evt));
    }
    break;

    case FLUID_SEQ_NOTE:
    {
        unsigned int dur = fluid_event_get_duration(evt);
        short vel = fluid_event_get_velocity(evt);
        short key = fluid_event_get_key(evt);
        int chan = fluid_event_get_channel(evt);

        fluid_note_id_t id = fluid_note_compute_id(chan, key);

        int res = fluid_note_container_insert(seqbind->note_container, id);
        if(res == FLUID_FAILED)
        {
            goto err;
        }
        else if(res)
        {
            // Note is already playing ATM, the following call to fluid_synth_noteon() will kill that note.
            // Thus, we need to remove its noteoff from the queue
            fluid_sequencer_invalidate_note(seqbind->seq, seqbind->client_id, id);
        }
        else
        {
            // Note not playing, all good.
        }

        fluid_event_noteoff(evt, chan, key);
        fluid_event_set_id(evt, id);

        res = fluid_sequencer_send_at(seq, evt, dur, 0);
        if(res == FLUID_FAILED)
        {
            err:
            FLUID_LOG(FLUID_ERR, "seqbind: Unable to process FLUID_SEQ_NOTE event, something went horribly wrong");
            return;
        }

        fluid_synth_noteon(synth, chan, key, vel);
    }
    break;

    case FLUID_SEQ_ALLSOUNDSOFF:
        fluid_note_container_clear(seqbind->note_container);
        fluid_synth_all_sounds_off(synth, fluid_event_get_channel(evt));
        break;

    case FLUID_SEQ_ALLNOTESOFF:
        fluid_note_container_clear(seqbind->note_container);
        fluid_synth_all_notes_off(synth, fluid_event_get_channel(evt));
        break;

    case FLUID_SEQ_BANKSELECT:
        fluid_synth_bank_select(synth, fluid_event_get_channel(evt), fluid_event_get_bank(evt));
        break;

    case FLUID_SEQ_PROGRAMCHANGE:
        fluid_synth_program_change(synth, fluid_event_get_channel(evt), fluid_event_get_program(evt));
        break;

    case FLUID_SEQ_PROGRAMSELECT:
        fluid_synth_program_select(synth,
                                   fluid_event_get_channel(evt),
                                   fluid_event_get_sfont_id(evt),
                                   fluid_event_get_bank(evt),
                                   fluid_event_get_program(evt));
        break;

    case FLUID_SEQ_PITCHBEND:
        fluid_synth_pitch_bend(synth, fluid_event_get_channel(evt), fluid_event_get_pitch(evt));
        break;

    case FLUID_SEQ_PITCHWHEELSENS:
        fluid_synth_pitch_wheel_sens(synth, fluid_event_get_channel(evt), fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_CONTROLCHANGE:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), fluid_event_get_control(evt), fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_MODULATION:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), MODULATION_MSB, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_SUSTAIN:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), SUSTAIN_SWITCH, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_PAN:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), PAN_MSB, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_VOLUME:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), VOLUME_MSB, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_REVERBSEND:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), EFFECTS_DEPTH1, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_CHORUSSEND:
        fluid_synth_cc(synth, fluid_event_get_channel(evt), EFFECTS_DEPTH3, fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_CHANNELPRESSURE:
        fluid_synth_channel_pressure(synth, fluid_event_get_channel(evt), fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_KEYPRESSURE:
        fluid_synth_key_pressure(synth,
                                 fluid_event_get_channel(evt),
                                 fluid_event_get_key(evt),
                                 fluid_event_get_value(evt));
        break;

    case FLUID_SEQ_SYSTEMRESET:
        fluid_synth_system_reset(synth);
        break;

    case FLUID_SEQ_UNREGISTERING: /* free ourselves */
        delete_fluid_seqbind(seqbind);
        break;

    case FLUID_SEQ_TIMER:
        /* nothing in fluidsynth */
        break;

    case FLUID_SEQ_SCALE:
        fluid_sequencer_set_time_scale(seq, fluid_event_get_scale(evt));
        break;

    default:
        break;
    }
}

static fluid_seq_id_t get_fluidsynth_dest(fluid_sequencer_t *seq)
{
    int i;
    fluid_seq_id_t id;
    char *name;
    int j = fluid_sequencer_count_clients(seq);

    for(i = 0; i < j; i++)
    {
        id = fluid_sequencer_get_client_id(seq, i);
        name = fluid_sequencer_get_client_name(seq, id);

        if(name && (FLUID_STRCMP(name, "fluidsynth") == 0))
        {
            return id;
        }
    }

    return -1;
}

/**
 * Transforms an incoming MIDI event (from a MIDI driver or MIDI router) to a
 * sequencer event and adds it to the sequencer queue for sending as soon as possible.
 *
 * @param data The sequencer, must be a valid #fluid_sequencer_t
 * @param event MIDI event
 * @return #FLUID_OK or #FLUID_FAILED
 *
 * The signature of this function is of type #handle_midi_event_func_t.
 *
 * @since 1.1.0
 */
int fluid_sequencer_add_midi_event_to_buffer(void *data, fluid_midi_event_t *event)
{
    fluid_event_t evt;
    fluid_sequencer_t *seq;

    fluid_return_val_if_fail(data != NULL, FLUID_FAILED);
    fluid_return_val_if_fail(event != NULL, FLUID_FAILED);

    seq = (fluid_sequencer_t *)data;

    fluid_event_clear(&evt);
    fluid_event_from_midi_event(&evt, event);
    fluid_event_set_dest(&evt, get_fluidsynth_dest(seq));

    /* Schedule for sending at next call to fluid_sequencer_process */
    return fluid_sequencer_send_at(seq, &evt, 0, 0);
}
