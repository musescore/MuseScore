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

#include "fluid_synth.h"
#include "fluid_chan.h"
#include "fluid_defsfont.h"


/******************************************************************************
  The legato detector is composed as this,
  variables:
  - monolist: monophonic list variable.
  - prev_note: to store the most recent note before adding on noteon or before
               removing on noteoff.
  - FLUID_CHANNEL_LEGATO_PLAYING: legato/staccato state bit that informs on
               legato or staccato playing.
  functions:
  - fluid_channel_add_monolist(), for inserting a new note.
  - fluid_channel_search_monolist(), for searching the position of a note
    into the list.
  - fluid_channel_remove_monolist(), for removing a note from the list.

            The monophonic list
   +------------------------------------------------+
   |    +----+   +----+          +----+   +----+    |
   |    |note|   |note|          |note|   |note|    |
   +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
        +----+   +----+          +----+   +----+
         /|\                      /|\
          |                        |
       i_first                   i_last

  The list allows an easy automatic detection of a legato passage when it is
  played on a MIDI keyboard input device.
  It is useful also when the input device is an ewi (electronic wind instrument)
  or evi (electronic valve instrument) and these instruments are unable to send
  MIDI CC legato on/off.

  The list memorizes the notes in playing order.
  - (a) On noteOn n2, if a previous note n1 exists, there is a legato
     detection with n1 (with or without portamento from n1 to n2 See note below).
  - (b) On noteOff of the running note n2, if a previous note n1 exists,
     there is a legato detection from n2 to n1, allowing fast trills playing
     (with or without portamento from n2 to n1. See note below).

  Notes in the list are inserted to the end of the list that works like a
  circular buffer.The features are:

  1) It is always possible to play an infinite legato passage in
     direct order (n1_On,n2_On,n3_On,....).

  2) Playing legato in the reverse order (n10_Off, n9_Off,,...) helps in
     fast trills playing as the list memorizes 10 most recent notes.

  3) Playing an infinite lagato passage in ascendant or descendant order,
     without playing trills is always possible using the usual way like this:
      First we begin with an ascendant passage,
      n1On, (n2On,n1Off), (n3On,n2Off) , (n4On,n3Off), then
	  we continue with a descendant passage
      (n3On,n4off), (n2On,n3off), (n1On,n2off), n1Off...and so on

 Each MIDI channel have a legato detector.

 Note:
  Portamento is a feature independent of the legato detector. So
  portamento isn't part of the lagato detector. However portamento
  (when enabled) is triggered at noteOn (like legato). Like in legato
  situation it is usual to have a portamento from a note 'fromkey' to another
  note 'tokey'. Portamento fromkey note choice is determined at noteOn by
  fluid_synth_get_fromkey_portamento_legato() (see below).

  More information in FluidPolyMono-0004.pdf chapter 4 (Appendices).
******************************************************************************/


/*****************************************************************************
 Portamento related functions in Poly or Mono mode
******************************************************************************/

/**
 * fluid_synth_get_fromkey_portamento_legato returns two information:
 *    - fromkey note for portamento.
 *    - fromkey note for legato.
 *                                                 +-----> fromkey_portamento
 *                                           ______|________
 *                portamento modes >------->|               |
 *                                          | get_fromkey   |
 *  Porta.on/off >------------------------->|_______________|
 *  (PTC)                                          |
 *                                                 +-----> fromkey_legato
 *
 * The functions is intended to be call on noteOn mono
 * see fluid_synth_noteon_mono_staccato(), fluid_synth_noteon_monopoly_legato()
 * -------
 * 1)The function determines if a portamento must occur on next noteOn.
 * The value returned is 'fromkey portamento' which is the pitchstart key
 * of a portamento, as function of PTC or (default_fromkey, prev_note) both
 * if Portamento On. By order of precedence the result is:
 *  1.1) PTC have precedence over Portamento On.
 *       If CC PTC has been received, its value supersedes and any
 *       portamento pedal On, default_fromkey,prev_note or portamento mode.
 *  1.2) Otherwise ,when Portamento On the function takes the following value:
 *       - default_fromkey if valid
 *       - otherwise prev_note(prev_note is the note prior the most recent
 *         note played).
 *       Then portamento mode is applied to validate the value chosen.
 *       Where portamento mode is:
 *       - each note, a portamento occurs on each note.
 *       - legato only, portamento only on notes played legato.
 *       - staccato only, portamento only on notes played staccato.
 *  1.3) Otherwise, portamento is off,INVALID_NOTE is returned (portamento is disabled).
 * ------
 * 2)The function determines if a legato playing must occur on next noteOn.
 *  'fromkey legato note' is returned as a function of default_fromkey, PTC,
 *   current mono/poly mode,actual 'staccato/legato' playing state and prev_note.
 *   By order of precedence the result is:
 *   2.1) If valid, default_fromkey have precedence over any others values.
 *   2.2) Otherwise if CC PTC has been received its value is returned.
 *   2.3) Otherwise fromkey legato is determined from the mono/poly mode,
 *        the actual 'staccato/legato' playing state (FLUID_CHANNEL_LEGATO_PLAYING) and prev_note
 *        as this:
 *        - in (poly/Mono) staccato , INVALID_NOTE is returned.
 *        - in poly  legato , actually we don't want playing legato. So
 *          INVALID_NOTE is returned.
 *        - in mono legato , prev_note is returned.
 *
 * On input
 * @param chan  fluid_channel_t.
 * @param defaultFromkey, the default 'fromkey portamento' note or 'fromkey legato'
 *       note (see description above).
 *
 * @return
 *  1)'fromkey portamento' is returned in fluid_synth_t.fromkey_portamento.
 *  If valid,it means that portamento is enabled .
 *
 *  2)The 'fromkey legato' note is returned.
 *
 * Notes about usage:
 * The function is intended to be called when the following event occurs:
 * - On noteOn (Poly or Mono) after insertion in the monophonic list.
 * - On noteOff(mono legato playing). In this case, default_fromkey must be valid.
 *
 * Typical calling usage:
 * - In poly, default_fromkey must be INVALID_NOTE.
 * - In mono staccato playing,default_fromkey must be INVALID_NOTE.
 * - In mono legato playing,default_fromkey must be valid.
 */
static char fluid_synth_get_fromkey_portamento_legato(fluid_channel_t *chan,
        int default_fromkey)
{
    unsigned char ptc = fluid_channel_get_cc(chan, PORTAMENTO_CTRL);

    if(fluid_channel_is_valid_note(ptc))
    {
        /* CC PTC has been received */
        fluid_channel_clear_portamento(chan);	/* clears the CC PTC receive */
        chan->synth->fromkey_portamento =  ptc;/* returns fromkey portamento */

        /* returns fromkey legato */
        if(!fluid_channel_is_valid_note(default_fromkey))
        {
            default_fromkey = ptc;
        }
    }
    else
    {
        /* determines and returns fromkey portamento */
        unsigned char fromkey_portamento = INVALID_NOTE;

        if(fluid_channel_portamento(chan))
        {
            /* Portamento when Portamento pedal is On */
            /* 'fromkey portamento'is determined from the portamento mode
             and the most recent note played (prev_note)*/
            enum fluid_channel_portamento_mode portamentomode = chan->portamentomode;

            if(fluid_channel_is_valid_note(default_fromkey))
            {
                fromkey_portamento = default_fromkey; /* on each note */
            }
            else
            {
                fromkey_portamento = fluid_channel_prev_note(chan); /* on each note */
            }

            if(portamentomode == FLUID_CHANNEL_PORTAMENTO_MODE_LEGATO_ONLY)
            {
                /* Mode portamento:legato only */
                if(!(chan->mode  & FLUID_CHANNEL_LEGATO_PLAYING))
                {
                    fromkey_portamento = INVALID_NOTE;
                }
            }
            else if(portamentomode == FLUID_CHANNEL_PORTAMENTO_MODE_STACCATO_ONLY)
            {
                /* Mode portamento:staccato only */
                if(chan->mode  & FLUID_CHANNEL_LEGATO_PLAYING)
                {
                    fromkey_portamento = INVALID_NOTE;
                }
            }

            /* else Mode portamento: on each note (staccato/legato) */
        }

        /* Returns fromkey portamento */
        chan->synth->fromkey_portamento = fromkey_portamento;

        /* Determines and returns fromkey legato */
        if(!fluid_channel_is_valid_note(default_fromkey))
        {
            /* in staccato (poly/Mono) returns INVALID_NOTE */
            /* In mono mode legato playing returns the note prior most
               recent note played */
            if(fluid_channel_is_playing_mono(chan) && (chan->mode  & FLUID_CHANNEL_LEGATO_PLAYING))
            {
                default_fromkey = fluid_channel_prev_note(chan); /* note prior last note */
            }

            /* In poly mode legato playing, actually we don't want playing legato.
            So returns INVALID_NOTE */
        }
    }

    return default_fromkey; /* Returns legato fromkey */
}

/*****************************************************************************
 noteon - noteoff functions in Mono mode
******************************************************************************/
/*
 *  noteon - noteoff on a channel in "monophonic playing".
 *
 *  A channel needs to be played monophonic if this channel has been set in
 *  monophonic mode by basic channel API.(see fluid_synth_polymono.c).
 *  A channel needs also to be played monophonic if it has been set in
 *  polyphonic mode and legato pedal is On during the playing.
 *  When a channel is in "monophonic playing" state, only one note at a time can be
 *  played in a staccato or legato manner (with or without portamento).
 *  More information in FluidPolyMono-0004.pdf chapter 4 (Appendices).
 *                                           _______________
 *                 ________________         |    noteon     |
 *                | legato detector|    O-->| mono_staccato |--*-> preset_noteon
 *  noteon_mono ->| (add_monolist) |--O--   |_______________|  |   (with or without)
 *  LOCAL         |________________|    O         /|\          |   (portamento)
 *                  /|\ set_onenote     |          | fromkey   |
 *                   |                  |          | portamento|
 *  noteOn poly  >---*------------------*          |           |
 *                                      |          |           |
 *                                      |    _____ |________   |
 *                portamento modes >--- | ->|               |  |
 *                                      |   |  get_fromkey  |  |
 *  Porta.on/off >--------------------- | ->|_______________|  |
 *  (PTC)                               |          |           |
 *                                      |  fromkey | fromkey   |
 *                                      |  legato  | portamento|
 *                                      |    _____\|/_______   |
 *                                      *-->|    noteon     |--/
 *                                      |   |    monopoly   |
 *                                      |   |    legato     |----> voices
 *                legato modes >------- | ->|_______________|      triggering
 *                                      |                          (with or without)
 *                                      |                          (portamento)
 *                                      |
 *                                      |
 *  noteOff poly >---*----------------- | ---------+
 *                   |  clear           |          |
 *                 _\|/_____________    |          |
 *                | legato detector |   O          |
 *  noteoff_mono->|(search_monolist)|-O--    _____\|/_______
 *  LOCAL         |(remove_monolist)|   O-->|   noteoff     |
 *                |_________________|       |   monopoly    |----> noteoff
 *  Sust.on/off  >------------------------->|_______________|
 *  Sost.on/off
------------------------------------------------------------------------------*/

/**
 * Plays a noteon event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing".
 *                                          _______________
 *                ________________         |    noteon     |
 *               | legato detector|    O-->| mono_staccato |--->preset_noteon
 * noteon_mono ->| (add_monolist) |--O--   |_______________|
 * LOCAL         |________________|    O
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |
 *                                     |    _______________
 *                                     |   |   noteon      |
 *                                     +-->|  monopoly     |
 *                                         |   legato      |---> voices
 *                                         |_______________|     triggering
 *
 * The function uses the legato detector (see above) to determine if the note must
 * be played staccato or legato.
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
int fluid_synth_noteon_mono_LOCAL(fluid_synth_t *synth, int chan,
                                  int key,  int vel)
{
    fluid_channel_t *channel = synth->channel[chan];

    /* Adds the note into the monophonic list */
    fluid_channel_add_monolist(channel, key, vel, 0);

    /* in Breath Sync mode, the noteon triggering is postponed
       until the musician starts blowing in the breath controller */
    if(!(channel->mode &  FLUID_CHANNEL_BREATH_SYNC) ||
            fluid_channel_breath_msb(channel))
    {
        /* legato/staccato playing detection */
        if(channel->mode  & FLUID_CHANNEL_LEGATO_PLAYING)
        {
            /* legato playing */
            /* legato from prev_note to key */
            /* the voices from prev_note key number are to be used to play key number */
            /* fromkey must be valid */
            return 	fluid_synth_noteon_monopoly_legato(synth, chan,
                    fluid_channel_prev_note(channel), key, vel);
        }
        else
        {
            /* staccato playing */
            return fluid_synth_noteon_mono_staccato(synth, chan, key, vel);
        }
    }
    else
    {
        return FLUID_OK;
    }
}

/**
 * Plays a noteoff event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing"
 *
 *                                           _______________
 *                                          |   noteon      |
 *                                      +-->|  monopoly     |
 *                                      |   |   legato      |----> voices
 *                                      |   |_______________|      triggering
 *                                      |                          (with or without)
 *                                      |                          (portamento)
 *                                      |
 *                                      |
 *                                      |
 *                                      |
 *                                      |
 *                                      |
 *                 _________________    |
 *                | legato detector |   O
 *  noteoff_mono->|(search_monolist)|-O--    _______________
 *  LOCAL         |(remove_monolist)|   O-->|   noteoff     |
 *                |_________________|       |   monopoly    |----> noteoff
 *                                          |_______________|
 *
 * The function uses the legato detector (see above) to determine if the noteoff must
 * be played staccato or legato.
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
int fluid_synth_noteoff_mono_LOCAL(fluid_synth_t *synth, int chan, int key)
{
    int status;
    int i, i_prev;
    fluid_channel_t *channel = synth->channel[chan];
    /* searching the note in the monophonic list */
    i = fluid_channel_search_monolist(channel, key, &i_prev);

    if(i >= 0)
    {
        /* the note is in the monophonic list */
        /* Removes the note from the monophonic list */
        fluid_channel_remove_monolist(channel, i, &i_prev);

        /* in Breath Sync mode, the noteoff triggering is done
           if the musician is blowing in the breath controller */
        if(!(channel->mode &  FLUID_CHANNEL_BREATH_SYNC) ||
                fluid_channel_breath_msb(channel))
        {
            /* legato playing detection */
            if(channel->mode  & FLUID_CHANNEL_LEGATO_PLAYING)
            {
                /* the list contains others notes */
                if(i_prev >= 0)
                {
                    /* legato playing detection on noteoff */
                    /* legato from key to i_prev key */
                    /* the voices from key number are to be used to
                    play i_prev key number. */
                    status = fluid_synth_noteon_monopoly_legato(synth, chan,
                             key, channel->monolist[i_prev].note,
                             channel->monolist[i_prev].vel);
                }
                /* else the note doesn't need to be played off */
                else
                {
                    status = FLUID_OK;
                }
            }
            else
            {
                /* the monophonic list is empty */
                /* plays the monophonic note noteoff and eventually held
                by sustain/sostenuto */
                status = fluid_synth_noteoff_monopoly(synth, chan, key, 1);
            }
        }
        else
        {
            status = FLUID_OK;
        }
    }
    else
    {
        /* the note is not found in the list so the note was
        played On when the channel was in polyphonic playing */
        /* plays the noteoff as for polyphonic  */
        status = fluid_synth_noteoff_monopoly(synth, chan, key, 0);
    }

    return status;
}

/*----------------------------------------------------------------------------
 staccato playing
-----------------------------------------------------------------------------*/
/**
 * Plays noteon for a monophonic note in staccato manner.
 * Please see the description above about "monophonic playing".
 *                                         _______________
 *                                        |    noteon     |
 *  noteon_mono >------------------------>| mono_staccato |----> preset_noteon
 *                                        |_______________|      (with or without)
 *  LOCAL                                       /|\              (portamento)
 *                                               | fromkey
 *                                               | portamento
 *                                               |
 *                                               |
 *                                         ______|________
 *                portamento modes >----->|               |
 *                                        |  get_fromkey  |
 *  Porta.on/off >----------------------->|_______________|
 *  Portamento
 *  (PTC)
 *
 * We are in staccato situation (where no previous note have been depressed).
 * Before the note been passed to fluid_preset_noteon(), the function must determine
 * the from_key_portamento parameter used by fluid_preset_noteon().
 *
 * from_key_portamento is returned by fluid_synth_get_fromkey_portamento_legato() function.
 * fromkey_portamento is set to valid/invalid  key value depending of the portamento
 * modes (see portamento mode API) , CC portamento On/Off , and CC portamento control
 * (PTC).
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
int
fluid_synth_noteon_mono_staccato(fluid_synth_t *synth, int chan, int key, int vel)
{
    fluid_channel_t *channel = synth->channel[chan];

    /* Before playing a new note, if a previous monophonic note is currently
       sustained it needs to be released */
    fluid_synth_release_voice_on_same_note_LOCAL(synth, chan, channel->key_mono_sustained);
    /* Get possible 'fromkey portamento'   */
    fluid_synth_get_fromkey_portamento_legato(channel, INVALID_NOTE);
    /* The note needs to be played by voices allocation  */
    return fluid_preset_noteon(channel->preset, synth, chan, key, vel);
}

/**
 * Plays noteoff for a polyphonic or monophonic note
 * Please see the description above about "monophonic playing".
 *
 *
 *  noteOff poly >---------------------------------+
 *                                                 |
 *                                                 |
 *                                                 |
 *  noteoff_mono                             _____\|/_______
 *  LOCAL        >------------------------->|   noteoff     |
 *                                          |   monopoly    |----> noteoff
 *  Sust.on/off  >------------------------->|_______________|
 *  Sost.on/off
 *
 * The function has the same behaviour when the noteoff is poly of mono, except
 * that for mono noteoff, if any pedal (sustain or sostenuto ) is depressed, the
 * key is memorized. This is necessary when the next mono note will be played
 * staccato, as any current mono note currently sustained will need to be released
 * (see fluid_synth_noteon_mono_staccato()).
 * Note also that for a monophonic legato passage, the function is called only when
 * the last noteoff of the passage occurs. That means that if sustain or sostenuto
 * is depressed, only the last note of a legato passage will be sustained.
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param Mono, 1 noteoff on monophonic note.
 *              0 noteoff on polyphonic note.
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 *
 * Note: On return, on monophonic, possible sustained note is memorized in
 * key_mono_sustained. Memorization is done here on noteOff.
 */
int fluid_synth_noteoff_monopoly(fluid_synth_t *synth, int chan, int key,
                                 char Mono)
{
    int status = FLUID_FAILED;
    fluid_voice_t *voice;
    int i;
    fluid_channel_t *channel = synth->channel[chan];

    /* Key_sustained is prepared to return no note sustained (INVALID_NOTE) */
    if(Mono)
    {
        channel->key_mono_sustained = INVALID_NOTE; /* no mono note sustained */
    }

    /* noteoff for all voices with same chan and same key */
    for(i = 0; i < synth->polyphony; i++)
    {
        voice = synth->voice[i];

        if(fluid_voice_is_on(voice) &&
                fluid_voice_get_channel(voice) == chan &&
                fluid_voice_get_key(voice) == key)
        {
            if(synth->verbose)
            {
                int used_voices = 0;
                int k;

                for(k = 0; k < synth->polyphony; k++)
                {
                    if(!_AVAILABLE(synth->voice[k]))
                    {
                        used_voices++;
                    }
                }

                FLUID_LOG(FLUID_INFO, "noteoff\t%d\t%d\t%d\t%05d\t%.3f\t%d",
                          fluid_voice_get_channel(voice), fluid_voice_get_key(voice), 0,
                          fluid_voice_get_id(voice),
                          (fluid_curtime() - synth->start) / 1000.0f,
                          used_voices);
            } /* if verbose */

            fluid_voice_noteoff(voice);

            /* noteoff on monophonic note */
            /* Key memorization if the note is sustained  */
            if(Mono &&
                    (fluid_voice_is_sustained(voice) || fluid_voice_is_sostenuto(voice)))
            {
                channel->key_mono_sustained = key;
            }

            status = FLUID_OK;
        } /* if voice on */
    } /* for all voices */

    return status;
}

/*----------------------------------------------------------------------------
 legato playing
-----------------------------------------------------------------------------*/
/**
 * Plays noteon for a monophonic note played legato.
 * Please see the description above about "monophonic playing".
 *
 *
 *                                         _______________
 *                portamento modes >----->|               |
 *                                        | get_fromkey   |
 *  Porta.on/off >----------------------->|_______________|
 *  Portamento                                   |
 *  (PTC)                                        |           +-->preset_noteon
 *                                       fromkey | fromkey   |  (with or without)
 *                                       legato  | portamento|  (portamento)
 *                                         _____\|/_______   |
 *                                        |   noteon      |--+
 *  noteon_mono >------------------------>|  monopoly     |
 *  LOCAL                                 |   legato      |----->voices
 *                                        |_______________|      triggering
 *                                              /|\              (with or without)
 *                                               |               (portamento)
 *                legato modes >-----------------+
 *
 * We are in legato situation (where a previous note has been depressed).
 * The function must determine the from_key_portamento and from_key_legato parameters
 * used respectively by fluid_preset_noteon() function and voices triggering functions.
 *
 * from_key_portamento and from_key_legato are returned by
 * fluid_synth_get_fromkey_portamento_legato() function.
 * fromkey_portamento is set to valid/invalid  key value depending of the portamento
 * modes (see portamento mode API), CC portamento On/Off, and CC portamento control
 * (PTC).
 * Then, depending of the legato modes (see legato mode API), the function will call
 * the appropriate triggering functions.
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param fromkey MIDI note number (0-127).
 * @param tokey MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 *
 * Note: The voices with key 'fromkey' are to be used to play key 'tokey'.
 * The function is able to play legato through Preset Zone(s) (PZ) and
 * Instrument Zone(s) (IZ) as far as possible.
 * When key tokey is outside the current Instrument Zone, Preset Zone,
 * current 'fromkey' voices are released. If necessary new voices
 * are restarted when tokey enters inside new Instrument(s) Zones,Preset Zone(s).
 * More information in FluidPolyMono-0004.pdf chapter 4.7 (Appendices).
 */
int fluid_synth_noteon_monopoly_legato(fluid_synth_t *synth, int chan,
                                       int fromkey, int tokey, int vel)
{
    fluid_channel_t *channel = synth->channel[chan];
    enum fluid_channel_legato_mode legatomode = channel->legatomode;
    fluid_voice_t *voice;
    int i ;
    /* Gets possible 'fromkey portamento' and possible 'fromkey legato' note  */
    fromkey = fluid_synth_get_fromkey_portamento_legato(channel, fromkey);

    if(fluid_channel_is_valid_note(fromkey))
    {
        for(i = 0; i < synth->polyphony; i++)
        {
            /* searching fromkey voices: only those who don't have 'note off' */
            voice = synth->voice[i];

            if(fluid_voice_is_on(voice) &&
                    fluid_voice_get_channel(voice) == chan &&
                    fluid_voice_get_key(voice) == fromkey)
            {
                fluid_zone_range_t *zone_range = voice->zone_range;

                /* Ignores voice when there is no instrument zone (i.e no zone_range). Otherwise
                   checks if tokey is inside the range of the running voice */
                if(zone_range && fluid_zone_inside_range(zone_range, tokey, vel))
                {
                    switch(legatomode)
                    {
                    case FLUID_CHANNEL_LEGATO_MODE_RETRIGGER: /* mode 0 */
                        fluid_voice_release(voice); /* normal release */
                        break;

                    case FLUID_CHANNEL_LEGATO_MODE_MULTI_RETRIGGER: /* mode 1 */
                        /* Skip in attack section */
                        fluid_voice_update_multi_retrigger_attack(voice, tokey, vel);

                        /* Starts portamento if enabled */
                        if(fluid_channel_is_valid_note(synth->fromkey_portamento))
                        {
                            /* Sends portamento parameters to the voice dsp */
                            fluid_voice_update_portamento(voice,
                                                          synth->fromkey_portamento,
                                                          tokey);
                        }

                        /* The voice is now used to play tokey in legato manner */
                        /* Marks this Instrument Zone to be ignored during next
                        fluid_preset_noteon() */
                        zone_range->ignore = TRUE;
                        break;

                    default: /* Invalid mode: this should never happen */
                        FLUID_LOG(FLUID_WARN, "Failed to execute legato mode: %d",
                                  legatomode);
                        return FLUID_FAILED;
                    }
                }
                else
                {
                    /* tokey note is outside the voice range, so the voice is released */
                    fluid_voice_release(voice);
                }
            }
        }
    }

    /* May be,tokey will enter in new others Insrument Zone(s),Preset Zone(s), in
       this case it needs to be played by voices allocation  */
    return fluid_preset_noteon(channel->preset, synth, chan, tokey, vel);
}
