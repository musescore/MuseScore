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

#include "fluid_chan.h"
#include "fluid_mod.h"
#include "fluid_synth.h"
#include "fluid_sfont.h"

/* Field shift amounts for sfont_bank_prog bit field integer */
#define PROG_SHIFTVAL   0
#define BANK_SHIFTVAL   8
#define SFONT_SHIFTVAL  22

/* Field mask values for sfont_bank_prog bit field integer */
#define PROG_MASKVAL    0x000000FF      /* Bit 7 is used to indicate unset state */
#define BANK_MASKVAL    0x003FFF00
#define BANKLSB_MASKVAL 0x00007F00
#define BANKMSB_MASKVAL 0x003F8000
#define SFONT_MASKVAL   0xFFC00000


static void fluid_channel_init(fluid_channel_t *chan);


fluid_channel_t *
new_fluid_channel(fluid_synth_t *synth, int num)
{
    fluid_channel_t *chan;

    chan = FLUID_NEW(fluid_channel_t);

    if(chan == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    chan->synth = synth;
    chan->channum = num;
    chan->preset = NULL;
    chan->tuning = NULL;

    fluid_channel_init(chan);
    fluid_channel_init_ctrl(chan, 0);

    return chan;
}

static void
fluid_channel_init(fluid_channel_t *chan)
{
    fluid_preset_t *newpreset;
    int i, prognum, banknum;

    chan->sostenuto_orderid = 0;
    /*--- Init poly/mono modes variables --------------------------------------*/
    chan->mode = 0;
    chan->mode_val = 0;

    /* monophonic list initialization */
    for(i = 0; i < FLUID_CHANNEL_SIZE_MONOLIST; i++)
    {
        chan->monolist[i].next = i + 1;
    }

    chan->monolist[FLUID_CHANNEL_SIZE_MONOLIST - 1].next = 0; /* ending element chained to the 1st */
    chan->i_last = chan->n_notes = 0; /* clears the list */
    chan->i_first = chan->monolist[chan->i_last].next; /* first note index in the list */
    fluid_channel_clear_prev_note(chan); /* Mark previous note invalid */
    /*---*/
    chan->key_mono_sustained = INVALID_NOTE; /* No previous mono note sustained */
    chan->legatomode = FLUID_CHANNEL_LEGATO_MODE_MULTI_RETRIGGER;		/* Default mode */
    chan->portamentomode = FLUID_CHANNEL_PORTAMENTO_MODE_LEGATO_ONLY;	/* Default mode */
    /*--- End of poly/mono initialization --------------------------------------*/

    chan->channel_type = (chan->channum == 9) ? CHANNEL_TYPE_DRUM : CHANNEL_TYPE_MELODIC;
    prognum = 0;
    banknum = (chan->channel_type == CHANNEL_TYPE_DRUM) ? DRUM_INST_BANK : 0;

    chan->sfont_bank_prog = 0 << SFONT_SHIFTVAL | banknum << BANK_SHIFTVAL
                            | prognum << PROG_SHIFTVAL;

    newpreset = fluid_synth_find_preset(chan->synth, banknum, prognum);
    fluid_channel_set_preset(chan, newpreset);

    chan->interp_method = FLUID_INTERP_DEFAULT;
    chan->tuning_bank = 0;
    chan->tuning_prog = 0;
    chan->nrpn_select = 0;
    chan->nrpn_active = 0;

    if(chan->tuning)
    {
        fluid_tuning_unref(chan->tuning, 1);
        chan->tuning = NULL;
    }
}

/*
  @param is_all_ctrl_off if nonzero, only resets some controllers, according to
  https://www.midi.org/techspecs/rp15.php
*/
void
fluid_channel_init_ctrl(fluid_channel_t *chan, int is_all_ctrl_off)
{
    int i;

    chan->channel_pressure = 0;
    chan->pitch_bend = 0x2000; /* Range is 0x4000, pitch bend wheel starts in centered position */

    for(i = 0; i < GEN_LAST; i++)
    {
        chan->gen[i] = 0.0f;
    }

    if(is_all_ctrl_off)
    {
        for(i = 0; i < ALL_SOUND_OFF; i++)
        {
            if(i >= EFFECTS_DEPTH1 && i <= EFFECTS_DEPTH5)
            {
                continue;
            }

            if(i >= SOUND_CTRL1 && i <= SOUND_CTRL10)
            {
                continue;
            }

            if(i == BANK_SELECT_MSB || i == BANK_SELECT_LSB || i == VOLUME_MSB ||
                    i == VOLUME_LSB || i == PAN_MSB || i == PAN_LSB ||
                    i == BALANCE_MSB || i == BALANCE_LSB
              )
            {
                continue;
            }

            fluid_channel_set_cc(chan, i, 0);
        }
    }
    else
    {
        for(i = 0; i < 128; i++)
        {
            fluid_channel_set_cc(chan, i, 0);
        }

        chan->previous_cc_breath = 0;/* Reset previous breath */
    }
    /* Unconditionally clear PTC receive (issue #1050) */
    fluid_channel_clear_portamento(chan);

    /* Reset polyphonic key pressure on all voices */
    for(i = 0; i < 128; i++)
    {
        fluid_channel_set_key_pressure(chan, i, 0);
    }

    /* Set RPN controllers to NULL state */
    fluid_channel_set_cc(chan, RPN_LSB, 127);
    fluid_channel_set_cc(chan, RPN_MSB, 127);

    /* Set NRPN controllers to NULL state */
    fluid_channel_set_cc(chan, NRPN_LSB, 127);
    fluid_channel_set_cc(chan, NRPN_MSB, 127);

    /* Expression (MSB & LSB) */
    fluid_channel_set_cc(chan, EXPRESSION_MSB, 127);
    fluid_channel_set_cc(chan, EXPRESSION_LSB, 127);

    if(!is_all_ctrl_off)
    {

        chan->pitch_wheel_sensitivity = 2; /* two semi-tones */

        /* Just like panning, a value of 64 indicates no change for sound ctrls */
        for(i = SOUND_CTRL1; i <= SOUND_CTRL10; i++)
        {
            fluid_channel_set_cc(chan, i, 64);
        }

        /* Volume / initial attenuation (MSB & LSB) */
        fluid_channel_set_cc(chan, VOLUME_MSB, 100);
        fluid_channel_set_cc(chan, VOLUME_LSB, 0);

        /* Pan (MSB & LSB) */
        fluid_channel_set_cc(chan, PAN_MSB, 64);
        fluid_channel_set_cc(chan, PAN_LSB, 0);

        /* Balance (MSB & LSB) */
        fluid_channel_set_cc(chan, BALANCE_MSB, 64);
        fluid_channel_set_cc(chan, BALANCE_LSB, 0);

        /* Reverb */
        /* fluid_channel_set_cc (chan, EFFECTS_DEPTH1, 40); */
        /* Note: although XG standard specifies the default amount of reverb to
           be 40, most people preferred having it at zero.
           See https://lists.gnu.org/archive/html/fluid-dev/2009-07/msg00016.html */
    }
}

/* Only called by delete_fluid_synth(), so no need to queue a preset free event */
void
delete_fluid_channel(fluid_channel_t *chan)
{
    fluid_return_if_fail(chan != NULL);

    FLUID_FREE(chan);
}

void
fluid_channel_reset(fluid_channel_t *chan)
{
    fluid_channel_init(chan);
    fluid_channel_init_ctrl(chan, 0);
}

/* Should only be called from synthesis context */
int
fluid_channel_set_preset(fluid_channel_t *chan, fluid_preset_t *preset)
{
    fluid_sfont_t *sfont;

    if(chan->preset == preset)
    {
        return FLUID_OK;
    }

    if(chan->preset)
    {
        sfont = chan->preset->sfont;
        sfont->refcount--;
    }

    fluid_preset_notify(chan->preset, FLUID_PRESET_UNSELECTED, chan->channum);

    chan->preset = preset;

    if(preset)
    {
        sfont = preset->sfont;
        sfont->refcount++;
    }

    fluid_preset_notify(preset, FLUID_PRESET_SELECTED, chan->channum);

    return FLUID_OK;
}

/* Set SoundFont ID, MIDI bank and/or program.  Use -1 to use current value. */
void
fluid_channel_set_sfont_bank_prog(fluid_channel_t *chan, int sfontnum,
                                  int banknum, int prognum)
{
    int oldval, newval, oldmask;

    newval = ((sfontnum != -1) ? sfontnum << SFONT_SHIFTVAL : 0)
             | ((banknum != -1) ? banknum << BANK_SHIFTVAL : 0)
             | ((prognum != -1) ? prognum << PROG_SHIFTVAL : 0);

    oldmask = ((sfontnum != -1) ? 0 : SFONT_MASKVAL)
              | ((banknum != -1) ? 0 : BANK_MASKVAL)
              | ((prognum != -1) ? 0 : PROG_MASKVAL);

    oldval = chan->sfont_bank_prog;
    newval = (newval & ~oldmask) | (oldval & oldmask);
    chan->sfont_bank_prog = newval;
}

/* Set bank LSB 7 bits */
void
fluid_channel_set_bank_lsb(fluid_channel_t *chan, int banklsb)
{
    int oldval, newval, style;

    style = chan->synth->bank_select;

    if(style == FLUID_BANK_STYLE_GM ||
            style == FLUID_BANK_STYLE_GS)
    {
        return;    /* ignored */
    }

    oldval = chan->sfont_bank_prog;

    if(style == FLUID_BANK_STYLE_XG)
    {
        newval = (oldval & ~BANK_MASKVAL) | (banklsb << BANK_SHIFTVAL);
    }
    else /* style == FLUID_BANK_STYLE_MMA */
    {
        newval = (oldval & ~BANKLSB_MASKVAL) | (banklsb << BANK_SHIFTVAL);
    }

    chan->sfont_bank_prog = newval;
}

/* Set bank MSB 7 bits */
void
fluid_channel_set_bank_msb(fluid_channel_t *chan, int bankmsb)
{
    int oldval, newval, style;

    style = chan->synth->bank_select;

    if(style == FLUID_BANK_STYLE_XG)
    {
        /* XG bank, do drum-channel auto-switch */
        /* The number "120" was based on several keyboards having drums at 120 - 127,
           reference: https://lists.nongnu.org/archive/html/fluid-dev/2011-02/msg00003.html */
        chan->channel_type = (120 <= bankmsb) ? CHANNEL_TYPE_DRUM : CHANNEL_TYPE_MELODIC;
        return;
    }

    if(style == FLUID_BANK_STYLE_GM ||
            chan->channel_type == CHANNEL_TYPE_DRUM)
    {
        return;    /* ignored */
    }

    oldval = chan->sfont_bank_prog;

    if(style == FLUID_BANK_STYLE_GS)
    {
        newval = (oldval & ~BANK_MASKVAL) | (bankmsb << BANK_SHIFTVAL);
    }
    else /* style == FLUID_BANK_STYLE_MMA */
    {
        newval = (oldval & ~BANKMSB_MASKVAL) | (bankmsb << (BANK_SHIFTVAL + 7));
    }

    chan->sfont_bank_prog = newval;

}

/* Get SoundFont ID, MIDI bank and/or program.  Use NULL to ignore a value. */
void
fluid_channel_get_sfont_bank_prog(fluid_channel_t *chan, int *sfont,
                                  int *bank, int *prog)
{
    int sfont_bank_prog;

    sfont_bank_prog = chan->sfont_bank_prog;

    if(sfont)
    {
        *sfont = (sfont_bank_prog & SFONT_MASKVAL) >> SFONT_SHIFTVAL;
    }

    if(bank)
    {
        *bank = (sfont_bank_prog & BANK_MASKVAL) >> BANK_SHIFTVAL;
    }

    if(prog)
    {
        *prog = (sfont_bank_prog & PROG_MASKVAL) >> PROG_SHIFTVAL;
    }
}

/**
 * Compute the pitch for a key after applying Fluidsynth's tuning functionality
 * and channel coarse/fine tunings.
 * @param chan fluid_channel_t
 * @param key MIDI note number (0-127)
 * @return the pitch of the key
 */
fluid_real_t fluid_channel_get_key_pitch(fluid_channel_t *chan, int key)
{
    if(chan->tuning)
    {
        return fluid_tuning_get_pitch(chan->tuning, key)
            + 100.0f * fluid_channel_get_gen(chan, GEN_COARSETUNE)
            + fluid_channel_get_gen(chan, GEN_FINETUNE);
    }
    else
    {
        return key * 100.0f;
    }
}

/**
 * Updates legato/ staccato playing state
 * The function is called:
 * - on noteon before adding a note into the monolist.
 * - on noteoff after removing a note out of the monolist.
 * @param chan  fluid_channel_t.
*/
static void
fluid_channel_update_legato_staccato_state(fluid_channel_t *chan)
{
    /* Updates legato/ staccato playing state */
    if(chan->n_notes)
    {
        chan->mode |= FLUID_CHANNEL_LEGATO_PLAYING; /* Legato state */
    }
    else
    {
        chan->mode &= ~ FLUID_CHANNEL_LEGATO_PLAYING; /* Staccato state */
    }
}

/**
 * Adds a note into the monophonic list. The function is part of the legato
 * detector. fluid_channel_add_monolist() is intended to be called by
 * fluid_synth_noteon_mono_LOCAL().
 *
 * When a note is added at noteOn each element is use in the forward direction
 * and indexed by i_last variable.
 *
 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127, 0=noteoff).
 * @param onenote. When 1 the function adds the note but the monophonic list
 *                 keeps only one note (used on noteOn poly).
 * Note: i_last index keeps a trace of the most recent note added.
 *       prev_note keeps a trace of the note prior i_last note.
 *       FLUID_CHANNEL_LEGATO_PLAYING bit keeps trace of legato/staccato playing state.
 *
 * More information in FluidPolyMono-0004.pdf chapter 4 (Appendices).
*/
void
fluid_channel_add_monolist(fluid_channel_t *chan, unsigned char key,
                           unsigned char vel, unsigned char onenote)
{
    unsigned char i_last = chan->i_last;
    /* Updates legato/ staccato playing state */
    fluid_channel_update_legato_staccato_state(chan);

    if(chan->n_notes)
    {
        /* keeps trace of the note prior last note */
        chan->prev_note = chan->monolist[i_last].note;
    }

    /* moves i_last forward before writing new note */
    i_last = chan->monolist[i_last].next;
    chan->i_last = i_last; 			/* now ilast indexes the last note */
    chan->monolist[i_last].note = key; /* we save note and velocity */
    chan->monolist[i_last].vel = vel;

    if(onenote)
    {
        /* clears monolist before one note addition */
        chan->i_first = i_last;
        chan->n_notes = 0;
    }

    if(chan->n_notes < FLUID_CHANNEL_SIZE_MONOLIST)
    {
        chan->n_notes++; /* updates n_notes */
    }
    else
    {
        /* The end of buffer is reach. So circular motion for i_first */
        /* i_first index is moved forward */
        chan->i_first = chan->monolist[i_last].next;
    }
}

/**
 * Searching a note in the monophonic list. The function is part of the legato
 * detector. fluid_channel_search_monolist() is intended to be called by
 * fluid_synth_noteoff_mono_LOCAL().
 *
 * The search starts from the first note in the list indexed by i_first

 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127) to search.
 * @param i_prev pointer on returned index of the note prior the note to search.
 * @return index of the note if find, FLUID_FAILED otherwise.
 *
 */
int
fluid_channel_search_monolist(fluid_channel_t *chan, unsigned char key, int *i_prev)
{
    short n = chan->n_notes; /* number of notes in monophonic list */
    short j, i = chan->i_first; /* searching starts from i_first included */

    for(j = 0 ; j < n ; j++)
    {
        if(chan->monolist[i].note == key)
        {
            if(i == chan->i_first)
            {
                /* tracking index of the previous note (i_prev) */
                for(j = chan->i_last ; n < FLUID_CHANNEL_SIZE_MONOLIST; n++)
                {
                    j = chan->monolist[j].next;
                }

                * i_prev = j; /* returns index of the previous note */
            }

            return i; /* returns index of the note to search */
        }

        * i_prev = i; /* tracking index of the previous note (i_prev) */
        i = chan->monolist[i].next; /* next element */
    }

    return FLUID_FAILED; /* not found */
}

/**
 * removes a note from the monophonic list. The function is part of
 * the legato detector.
 * fluid_channel_remove_monolist() is intended to be called by
 * fluid_synth_noteoff_mono_LOCAL().
 *
 * When a note is removed at noteOff the element concerned is fast unlinked
 * and relinked after the i_last element.
 *
 * @param chan  fluid_channel_t.
 * @param
 *   i, index of the note to remove. If i is invalid or the list is
 *      empty, the function do nothing and returns FLUID_FAILED.
 * @param
 *   On input, i_prev is a pointer on index of the note previous i.
 *   On output i_prev is a pointer on index of the note previous i if i is the last note
 *   in the list,FLUID_FAILED otherwise. When the returned index is valid it means
 *   a legato detection on noteoff.
 *
 * Note: the following variables in Channel keeps trace of the situation.
 *       - i_last index keeps a trace of the most recent note played even if
 *       the list is empty.
 *       - prev_note keeps a trace of the note removed if it is i_last.
 *       - FLUID_CHANNEL_LEGATO_PLAYING bit keeps a trace of legato/staccato playing state.
 *
 * More information in FluidPolyMono-0004.pdf chapter 4 (Appendices).
 */
void
fluid_channel_remove_monolist(fluid_channel_t *chan, int i, int *i_prev)
{
    unsigned char i_last = chan->i_last;

    /* checks if index is valid */
    if(i < 0 || i >= FLUID_CHANNEL_SIZE_MONOLIST || !chan->n_notes)
    {
        * i_prev =  FLUID_FAILED;
    }

    /* The element is about to be removed and inserted between i_last and next */
    /* Note: when i is egal to i_last or egal to i_first, removing/inserting
       isn't necessary */
    if(i == i_last)
    {
        /* Removing/Inserting isn't necessary */
        /* keeps trace of the note prior last note */
        chan->prev_note = chan->monolist[i_last].note;
        /* moves i_last backward to the previous  */
        chan->i_last = *i_prev; /* i_last index is moved backward */
    }
    else
    {
        /* i is before i_last */
        if(i == chan->i_first)
        {
            /* Removing/inserting isn't necessary */
            /* i_first index is moved forward to the next element*/
            chan->i_first = chan->monolist[i].next;
        }
        else
        {
            /* i is between i_first and i_last */
            /* Unlinks element i and inserts after i_last */
            chan->monolist[* i_prev].next = chan->monolist[i].next; /* unlinks i */
            /*inserts i after i_last */
            chan->monolist[i].next = chan->monolist[i_last].next;
            chan->monolist[i_last].next = i;
        }

        * i_prev =  FLUID_FAILED;
    }

    chan->n_notes--; /* updates the number of note in the list */
    /* Updates legato/ staccato playing state */
    fluid_channel_update_legato_staccato_state(chan);
}

/**
 * On noteOff on a polyphonic channel,the monophonic list is fully flushed.
 *
 * @param chan  fluid_channel_t.
 * Note: i_last index keeps a trace of the most recent note played even if
 *       the list is empty.
 *       prev_note keeps a trace of the note i_last .
 *       FLUID_CHANNEL_LEGATO_PLAYING bit keeps a trace of legato/staccato playing.
 */
void fluid_channel_clear_monolist(fluid_channel_t *chan)
{
    /* keeps trace off the most recent note played */
    chan->prev_note = chan->monolist[chan->i_last].note;

    /* flushes the monolist */
    chan->i_first = chan->monolist[chan->i_last].next;
    chan->n_notes = 0;
    /* Update legato/ sataccato playing state */
    chan->mode &= ~ FLUID_CHANNEL_LEGATO_PLAYING; /* Staccato state */
}

/**
 * On noteOn on a polyphonic channel,adds the note into the monophonic list
 * keeping only this note.
 * @param
 *   chan  fluid_channel_t.
 *   key, vel, note and velocity added in the monolist
 * Note: i_last index keeps a trace of the most recent note inserted.
 *       prev_note keeps a trace of the note prior i_last note.
 *       FLUID_CHANNEL_LEGATO_PLAYING bit keeps trace of legato/staccato playing.
 */
void fluid_channel_set_onenote_monolist(fluid_channel_t *chan, unsigned char key,
                                        unsigned char vel)
{
    fluid_channel_add_monolist(chan, key, vel, 1);
}

/**
 * The function changes the state (Valid/Invalid) of the previous note played in
 * a staccato manner (fluid_channel_prev_note()).
 * When potamento mode 'each note' or 'staccato only' is selected, on next
 * noteOn a portamento will be started from the most recent note played
 * staccato.
 * It will be possible that it isn't appropriate. To give the musician the
 * possibility to choose a portamento from this note , prev_note will be forced
 * to invalid state on noteOff if portamento pedal is Off.
 *
 * The function is intended to be called when the following event occurs:
 * - On noteOff (in poly or mono mode), to mark prev_note invalid.
 * - On Portamento Off(in poly or mono mode), to mark prev_note invalid.
 * @param chan  fluid_channel_t.
 */
void fluid_channel_invalid_prev_note_staccato(fluid_channel_t *chan)
{
    /* checks if the playing is staccato */
    if(!(chan->mode & FLUID_CHANNEL_LEGATO_PLAYING))
    {

        /* checks if portamento pedal is off */
        if(! fluid_channel_portamento(chan))
        {
            /* forces prev_note invalid */
            fluid_channel_clear_prev_note(chan);
        }
    }

    /* else prev_note still remains valid for next fromkey portamento */
}

/**
 * The function handles poly/mono commutation on legato pedal On/Off.
 * @param chan  fluid_channel_t.
 * @param value, value of the CC legato.
 */
void fluid_channel_cc_legato(fluid_channel_t *chan, int value)
{
    /* Special handling of the monophonic list  */
    if(!(chan->mode & FLUID_CHANNEL_POLY_OFF) && chan->n_notes)  /* The monophonic list have notes */
    {
        if(value < 64)   /* legato is released */
        {
            /* returns from monophonic to polyphonic with notes in the monophonic list */

            /* The monophonic list is flushed keeping last note only
               Note: i_last index keeps a trace of the most recent note played.
               prev_note keeps a trace of the note i_last.
               FLUID_CHANNEL_LEGATO_PLAYING bit keeps trace of legato/staccato playing.
            */
            chan->i_first = chan->i_last;
            chan->n_notes = 1;
        }
        else /* legato is depressed */
        {
            /* Inters in monophonic from polyphonic with note in monophonic list */
            /* Stops the running note to remain coherent with Breath Sync mode */
            if((chan->mode &  FLUID_CHANNEL_BREATH_SYNC) && !fluid_channel_breath_msb(chan))
            {
                fluid_synth_noteoff_monopoly(chan->synth, chan->channum,
                                             fluid_channel_last_note(chan), 1);
            }
        }
    }
}

/**
 * The function handles CC Breath On/Off detection. When a channel is in
 * Breath Sync mode and in monophonic playing, the breath controller allows
 * to trigger noteon/noteoff note when the musician starts to breath (noteon) and
 * stops to breath (noteoff).
 * @param chan  fluid_channel_t.
 * @param value, value of the CC Breath..
 */
void fluid_channel_cc_breath_note_on_off(fluid_channel_t *chan, int value)
{
    if((chan->mode &  FLUID_CHANNEL_BREATH_SYNC)  && fluid_channel_is_playing_mono(chan) &&
            (chan->n_notes))
    {
        /* The monophonic list isn't empty */
        if((value > 0) && (chan->previous_cc_breath == 0))
        {
            /* CC Breath On detection */
            fluid_synth_noteon_mono_staccato(chan->synth, chan->channum,
                                             fluid_channel_last_note(chan),
                                             fluid_channel_last_vel(chan));
        }
        else if((value == 0) && (chan->previous_cc_breath > 0))
        {
            /* CC Breath Off detection */
            fluid_synth_noteoff_monopoly(chan->synth, chan->channum,
                                         fluid_channel_last_note(chan), 1);
        }
    }

    chan->previous_cc_breath = value;
}
