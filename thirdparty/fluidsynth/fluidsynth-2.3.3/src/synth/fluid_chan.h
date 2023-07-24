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

#ifndef _FLUID_CHAN_H
#define _FLUID_CHAN_H

#include "fluidsynth_priv.h"
#include "fluid_midi.h"
#include "fluid_tuning.h"

/* The mononophonic list is part of the legato detector for monophonic mode */
/* see fluid_synth_monopoly.c about a description of the legato detector device */
/* Size of the monophonic list
   - 1 is the minimum. it allows playing legato passage of any number
     of notes on noteon only.
   - Size above 1 allows playing legato on noteon but also on noteOff.
     This allows the  musician to play fast trills.
     This feature is particularly usful when the MIDI input device is a keyboard.
     Choosing a size of 10 is sufficient (because most musicians have only 10
     fingers when playing a monophonic instrument).
*/
#define FLUID_CHANNEL_SIZE_MONOLIST  10

/*

            The monophonic list
   +------------------------------------------------+
   |    +----+   +----+          +----+   +----+    |
   |    |note|   |note|          |note|   |note|    |
   +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
        +----+   +----+          +----+   +----+
         /|\                      /|\
          |                        |
       i_first                   i_last

 The monophonic list is a circular buffer of FLUID_CHANNEL_SIZE_MONOLIST elements.
 Each element is linked forward at initialisation time.
 - when a note is added at noteOn  (see fluid_channel_add_monolist()) each
   element is use in the forward direction and indexed by i_last variable.
 - when a note is removed at noteOff (see fluid_channel_remove_monolist()),
   the element concerned is fast unlinked and relinked after the i_last element.

 The most recent note added is indexed by i_last.
 The most ancient note added is the first note indexed by i_first. i_first is
 moving in the forward direction in a circular manner.

*/
struct mononote
{
    unsigned char next; /* next note */
    unsigned char note; /* note */
    unsigned char vel;  /* velocity */
};

/*
 * fluid_channel_t
 *
 * Mutual exclusion notes (as of 1.1.2):
 * None - everything should have been synchronized by the synth.
 */
struct _fluid_channel_t
{
    fluid_synth_t *synth;                 /**< Parent synthesizer instance */
    int channum;                          /**< MIDI channel number */

    /* Poly Mono variables see macro access description */
    int mode;								/**< Poly Mono mode */
    int mode_val;							/**< number of channel in basic channel group */

    /* monophonic list - legato detector */
    unsigned char i_first;          /**< First note index */
    unsigned char i_last;           /**< most recent note index since the most recent add */
    unsigned char prev_note;        /**< previous note of the most recent add/remove */
    unsigned char n_notes;          /**< actual number of notes in the list */
    struct mononote monolist[FLUID_CHANNEL_SIZE_MONOLIST];   /**< monophonic list */

    unsigned char key_mono_sustained;         /**< previous sustained monophonic note */
    unsigned char previous_cc_breath;		  /**< Previous Breath */
    enum fluid_channel_legato_mode legatomode;       /**< legato mode */
    enum fluid_channel_portamento_mode portamentomode;   /**< portamento mode */
    /*- End of Poly/mono variables description */

    unsigned char cc[128];                         /**< MIDI controller values from [0;127] */
    unsigned char key_pressure[128];               /**< MIDI polyphonic key pressure from [0;127] */

    /* Drum channel flag, CHANNEL_TYPE_MELODIC, or CHANNEL_TYPE_DRUM. */
    enum fluid_midi_channel_type channel_type;
    enum fluid_interp interp_method;                    /**< Interpolation method (enum fluid_interp) */

    unsigned char channel_pressure;                 /**< MIDI channel pressure from [0;127] */
    unsigned char pitch_wheel_sensitivity;          /**< Current pitch wheel sensitivity */
    short pitch_bend;                      /**< Current pitch bend value */
    /* Sostenuto order id gives the order of SostenutoOn event.
     * This value is useful to known when the sostenuto pedal is depressed
     * (before or after a key note). We need to compare SostenutoOrderId with voice id.
     */
    unsigned int  sostenuto_orderid;

    int tuning_bank;                      /**< Current tuning bank number */
    int tuning_prog;                      /**< Current tuning program number */
    fluid_tuning_t *tuning;               /**< Micro tuning */

    fluid_preset_t *preset;               /**< Selected preset */
    int sfont_bank_prog;                  /**< SoundFont ID (bit 21-31), bank (bit 7-20), program (bit 0-6) */

    /* NRPN system */
    enum fluid_gen_type nrpn_select;      /* Generator ID of SoundFont NRPN message */
    char nrpn_active;      /* 1 if data entry CCs are for NRPN, 0 if RPN */

    /* The values of the generators, set by NRPN messages, or by
     * fluid_synth_set_gen(), are cached in the channel so they can be
     * applied to future notes. They are copied to a voice's generators
     * in fluid_voice_init(), which calls fluid_gen_init().  */
    fluid_real_t gen[GEN_LAST];
};

fluid_channel_t *new_fluid_channel(fluid_synth_t *synth, int num);
void fluid_channel_init_ctrl(fluid_channel_t *chan, int is_all_ctrl_off);
void delete_fluid_channel(fluid_channel_t *chan);
void fluid_channel_reset(fluid_channel_t *chan);
int fluid_channel_set_preset(fluid_channel_t *chan, fluid_preset_t *preset);
void fluid_channel_set_sfont_bank_prog(fluid_channel_t *chan, int sfont,
                                       int bank, int prog);
void fluid_channel_set_bank_lsb(fluid_channel_t *chan, int banklsb);
void fluid_channel_set_bank_msb(fluid_channel_t *chan, int bankmsb);
void fluid_channel_get_sfont_bank_prog(fluid_channel_t *chan, int *sfont,
                                       int *bank, int *prog);
fluid_real_t fluid_channel_get_key_pitch(fluid_channel_t *chan, int key);

#define fluid_channel_get_preset(chan)          ((chan)->preset)
#define fluid_channel_set_cc(chan, num, val) \
  ((chan)->cc[num] = (val))
#define fluid_channel_get_cc(chan, num) \
  ((chan)->cc[num])
#define fluid_channel_get_key_pressure(chan, key) \
  ((chan)->key_pressure[key])
#define fluid_channel_set_key_pressure(chan, key, val) \
  ((chan)->key_pressure[key] = (val))
#define fluid_channel_get_channel_pressure(chan) \
  ((chan)->channel_pressure)
#define fluid_channel_set_channel_pressure(chan, val) \
  ((chan)->channel_pressure = (val))
#define fluid_channel_get_pitch_bend(chan) \
  ((chan)->pitch_bend)
#define fluid_channel_set_pitch_bend(chan, val) \
  ((chan)->pitch_bend = (val))
#define fluid_channel_get_pitch_wheel_sensitivity(chan) \
  ((chan)->pitch_wheel_sensitivity)
#define fluid_channel_set_pitch_wheel_sensitivity(chan, val) \
  ((chan)->pitch_wheel_sensitivity = (val))
#define fluid_channel_get_num(chan)             ((chan)->channum)
#define fluid_channel_set_interp_method(chan, new_method) \
  ((chan)->interp_method = (new_method))
#define fluid_channel_get_interp_method(chan) \
  ((chan)->interp_method);
#define fluid_channel_set_tuning(_c, _t)        { (_c)->tuning = _t; }
#define fluid_channel_has_tuning(_c)            ((_c)->tuning != NULL)
#define fluid_channel_get_tuning(_c)            ((_c)->tuning)
#define fluid_channel_get_tuning_bank(chan)     \
  ((chan)->tuning_bank)
#define fluid_channel_set_tuning_bank(chan, bank) \
  ((chan)->tuning_bank = (bank))
#define fluid_channel_get_tuning_prog(chan)     \
  ((chan)->tuning_prog)
#define fluid_channel_set_tuning_prog(chan, prog) \
  ((chan)->tuning_prog = (prog))
#define fluid_channel_portamentotime(_c) \
    ((_c)->cc[PORTAMENTO_TIME_MSB] * 128 + (_c)->cc[PORTAMENTO_TIME_LSB])
#define fluid_channel_portamento(_c)			((_c)->cc[PORTAMENTO_SWITCH] >= 64)
#define fluid_channel_breath_msb(_c)			((_c)->cc[BREATH_MSB] > 0)
#define fluid_channel_clear_portamento(_c)		((_c)->cc[PORTAMENTO_CTRL] = INVALID_NOTE)
#define fluid_channel_legato(_c)			    ((_c)->cc[LEGATO_SWITCH] >= 64)
#define fluid_channel_sustained(_c)             ((_c)->cc[SUSTAIN_SWITCH] >= 64)
#define fluid_channel_sostenuto(_c)             ((_c)->cc[SOSTENUTO_SWITCH] >= 64)
#define fluid_channel_set_gen(_c, _n, _v)   { (_c)->gen[_n] = _v; }
#define fluid_channel_get_gen(_c, _n)           ((_c)->gen[_n])
#define fluid_channel_get_min_note_length_ticks(chan) \
  ((chan)->synth->min_note_length_ticks)

/* Macros interface to poly/mono mode variables */
#define MASK_BASICCHANINFOS  (FLUID_CHANNEL_MODE_MASK|FLUID_CHANNEL_BASIC|FLUID_CHANNEL_ENABLED)
/* Set the basic channel infos for a MIDI basic channel */
#define fluid_channel_set_basic_channel_info(chan,Infos) \
    (chan->mode = (chan->mode & ~MASK_BASICCHANINFOS) | (Infos & MASK_BASICCHANINFOS))
/* Reset the basic channel infos for a MIDI basic channel */
#define fluid_channel_reset_basic_channel_info(chan) (chan->mode &=  ~MASK_BASICCHANINFOS)

/* Macros interface to breath variables */
#define FLUID_CHANNEL_BREATH_MASK  (FLUID_CHANNEL_BREATH_POLY|FLUID_CHANNEL_BREATH_MONO|FLUID_CHANNEL_BREATH_SYNC)
/* Set the breath infos for a MIDI  channel */
#define fluid_channel_set_breath_info(chan,BreathInfos) \
(chan->mode = (chan->mode & ~FLUID_CHANNEL_BREATH_MASK) | (BreathInfos & FLUID_CHANNEL_BREATH_MASK))
/* Get the breath infos for a MIDI  channel */
#define fluid_channel_get_breath_info(chan) (chan->mode & FLUID_CHANNEL_BREATH_MASK)

/* Returns true when channel is mono or legato is on */
#define fluid_channel_is_playing_mono(chan) ((chan->mode & FLUID_CHANNEL_POLY_OFF) ||\
                                             fluid_channel_legato(chan))

/* Macros interface to monophonic list variables */
#define INVALID_NOTE (255)
/* Returns true when a note is a valid note */
#define fluid_channel_is_valid_note(n)    (n != INVALID_NOTE)
/* Marks prev_note as invalid. */
#define fluid_channel_clear_prev_note(chan)	(chan->prev_note = INVALID_NOTE)

/* Returns the most recent note from i_last entry of the monophonic list */
#define fluid_channel_last_note(chan)	(chan->monolist[chan->i_last].note)

/* Returns the most recent velocity from i_last entry of the monophonic list */
#define fluid_channel_last_vel(chan)	(chan->monolist[chan->i_last].vel)

/*
  prev_note is used to determine fromkey_portamento as well as
  fromkey_legato (see fluid_synth_get_fromkey_portamento_legato()).

  prev_note is updated on noteOn/noteOff mono by the legato detector as this:
  - On noteOn mono, before adding a new note into the monolist,the most
    recent  note in the list (i.e at i_last position) is kept in prev_note.
  - Similarly, on  noteOff mono , before removing a note out of the monolist,
    the most recent note (i.e those at i_last position) is kept in prev_note.
*/
#define fluid_channel_prev_note(chan)	(chan->prev_note)

/* Interface to poly/mono mode variables */
enum fluid_channel_mode_flags_internal
{
    FLUID_CHANNEL_BASIC = 0x04,    /**< if flag set the corresponding midi channel is a basic channel */
    FLUID_CHANNEL_ENABLED = 0x08,  /**< if flag set the corresponding midi channel is enabled, else disabled, i.e. channel ignores any MIDI messages */

    /*
      FLUID_CHANNEL_LEGATO_PLAYING bit of channel mode keeps trace of the legato /staccato
      state playing.
      FLUID_CHANNEL_LEGATO_PLAYING bit is updated on noteOn/noteOff mono by the legato detector:
      - On noteOn, before inserting a new note into the monolist.
      - On noteOff, after removing a note out of the monolist.

      - On noteOn, this state is used by fluid_synth_noteon_mono_LOCAL()
      to play the current  note legato or staccato.
      - On noteOff, this state is used by fluid_synth_noteoff_mono_LOCAL()
      to play the current noteOff legato with the most recent note.
    */
    /* bit7, 1: means legato playing , 0: means staccato playing */
    FLUID_CHANNEL_LEGATO_PLAYING = 0x80
};

/* End of interface to monophonic list variables */

void fluid_channel_add_monolist(fluid_channel_t *chan, unsigned char key, unsigned char vel, unsigned char onenote);
int fluid_channel_search_monolist(fluid_channel_t *chan, unsigned char key, int *i_prev);
void fluid_channel_remove_monolist(fluid_channel_t *chan, int i, int *i_prev);
void fluid_channel_clear_monolist(fluid_channel_t *chan);
void fluid_channel_set_onenote_monolist(fluid_channel_t *chan, unsigned char key, unsigned char vel);
void fluid_channel_invalid_prev_note_staccato(fluid_channel_t *chan);
void fluid_channel_cc_legato(fluid_channel_t *chan, int value);
void fluid_channel_cc_breath_note_on_off(fluid_channel_t *chan, int value);


#endif /* _FLUID_CHAN_H */
