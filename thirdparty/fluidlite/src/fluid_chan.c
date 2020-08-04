/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluid_chan.h"
#include "fluid_mod.h"
#include "fluid_synth.h"
#include "fluid_sfont.h"

#define SETCC(_c,_n,_v)  _c->cc[_n] = _v

/*
 * new_fluid_channel
 */
fluid_channel_t*
new_fluid_channel(fluid_synth_t* synth, int num)
{
  fluid_channel_t* chan;

  chan = FLUID_NEW(fluid_channel_t);
  if (chan == NULL) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return NULL;
  }

  chan->synth = synth;
  chan->channum = num;
  chan->preset = NULL;

  fluid_channel_init(chan);
  fluid_channel_init_ctrl(chan,0);

  return chan;
}

void
fluid_channel_init(fluid_channel_t* chan)
{
  chan->prognum = 0;
  chan->banknum = 0;
  chan->sfontnum = 0;

  if (chan->preset) delete_fluid_preset (chan->preset);
  chan->preset = fluid_synth_find_preset(chan->synth, chan->banknum, chan->prognum);

  chan->interp_method = FLUID_INTERP_DEFAULT;
  chan->tuning = NULL;
  chan->nrpn_select = 0;
  chan->nrpn_active = 0;
}

/*
  @param is_all_ctrl_off if nonzero, only resets some controllers, according to
  http://www.midi.org/techspecs/rp15.php
*/
void
fluid_channel_init_ctrl(fluid_channel_t* chan, int is_all_ctrl_off)
{
  int i;

  chan->key_pressure = 0;
  chan->channel_pressure = 0;
  chan->pitch_bend = 0x2000; /* Range is 0x4000, pitch bend wheel starts in centered position */

  for (i = 0; i < GEN_LAST; i++) {
    chan->gen[i] = 0.0f;
    chan->gen_abs[i] = 0;
  }

  if (is_all_ctrl_off) {
    for (i = 0; i < ALL_SOUND_OFF; i++) {
      if (i >= EFFECTS_DEPTH1 && i <= EFFECTS_DEPTH5) {
        continue;
      }
      if (i >= SOUND_CTRL1 && i <= SOUND_CTRL10) {
        continue;
      }
      if (i == BANK_SELECT_MSB || i == BANK_SELECT_LSB || i == VOLUME_MSB ||
          i == VOLUME_LSB || i == PAN_MSB || i == PAN_LSB) {
        continue;
      }

      SETCC(chan, i, 0);
    }
  }
  else {
    for (i = 0; i < 128; i++) {
      SETCC(chan, i, 0);
    }
  }

  /* Set RPN controllers to NULL state */
  SETCC(chan, RPN_LSB, 127);
  SETCC(chan, RPN_MSB, 127);

  /* Set NRPN controllers to NULL state */
  SETCC(chan, NRPN_LSB, 127);
  SETCC(chan, NRPN_MSB, 127);

  /* Expression (MSB & LSB) */
  SETCC(chan, EXPRESSION_MSB, 127);
  SETCC(chan, EXPRESSION_LSB, 127);

  if (!is_all_ctrl_off) {

    chan->pitch_wheel_sensitivity = 2; /* two semi-tones */

    /* Just like panning, a value of 64 indicates no change for sound ctrls */
    for (i = SOUND_CTRL1; i <= SOUND_CTRL10; i++) {
      SETCC(chan, i, 64);
    }

    /* Volume / initial attenuation (MSB & LSB) */
    SETCC(chan, VOLUME_MSB, 100);
    SETCC(chan, VOLUME_LSB, 0);

    /* Pan (MSB & LSB) */
    SETCC(chan, PAN_MSB, 64);
    SETCC(chan, PAN_LSB, 0);

    /* Reverb */
    /* SETCC(chan, EFFECTS_DEPTH1, 40); */
    /* Note: although XG standard specifies the default amount of reverb to
       be 40, most people preferred having it at zero.
       See http://lists.gnu.org/archive/html/fluid-dev/2009-07/msg00016.html */
  }
}

void
fluid_channel_reset(fluid_channel_t* chan)
{
  fluid_channel_init(chan);
  fluid_channel_init_ctrl(chan,0);
}

/*
 * delete_fluid_channel
 */
int
delete_fluid_channel(fluid_channel_t* chan)
{
  if (chan->preset) delete_fluid_preset (chan->preset);
  FLUID_FREE(chan);
  return FLUID_OK;
}

/*
 * fluid_channel_set_preset
 */
int
fluid_channel_set_preset(fluid_channel_t* chan, fluid_preset_t* preset)
{
  fluid_preset_notify(chan->preset, FLUID_PRESET_UNSELECTED, chan->channum);
  fluid_preset_notify(preset, FLUID_PRESET_SELECTED, chan->channum);

  if (chan->preset) delete_fluid_preset (chan->preset);
  chan->preset = preset;
  return FLUID_OK;
}

/*
 * fluid_channel_get_preset
 */
fluid_preset_t*
fluid_channel_get_preset(fluid_channel_t* chan)
{
  return chan->preset;
}

/*
 * fluid_channel_get_banknum
 */
unsigned int
fluid_channel_get_banknum(fluid_channel_t* chan)
{
  return chan->banknum;
}

/*
 * fluid_channel_set_prognum
 */
int
fluid_channel_set_prognum(fluid_channel_t* chan, int prognum)
{
  chan->prognum = prognum;
  return FLUID_OK;
}

/*
 * fluid_channel_get_prognum
 */
int
fluid_channel_get_prognum(fluid_channel_t* chan)
{
  return chan->prognum;
}

/*
 * fluid_channel_set_banknum
 */
int
fluid_channel_set_banknum(fluid_channel_t* chan, unsigned int banknum)
{
  chan->banknum = banknum;
  return FLUID_OK;
}

/*
 * fluid_channel_cc
 */
int
fluid_channel_cc(fluid_channel_t* chan, int num, int value)
{
  chan->cc[num] = value;

  switch (num) {

  case SUSTAIN_SWITCH:
    {
      if (value < 64) {
/*  	printf("** sustain off\n"); */
	fluid_synth_damp_voices(chan->synth, chan->channum);
      } else {
/*  	printf("** sustain on\n"); */
      }
    }
    break;

  case BANK_SELECT_MSB:
    {
      chan->bank_msb = (unsigned char) (value & 0x7f);
/*      printf("** bank select msb recieved: %d\n", value); */

      /* I fixed the handling of a MIDI bank select controller 0,
	 e.g., bank select MSB (or "coarse" bank select according to
	 my spec).  Prior to this fix a channel's bank number was only
	 changed upon reception of MIDI bank select controller 32,
	 e.g, bank select LSB (or "fine" bank-select according to my
	 spec). [KLE]

	 FIXME: is this correct? [PH] */
      fluid_channel_set_banknum(chan, (unsigned int)(value & 0x7f));  /* KLE */
    }
    break;

  case BANK_SELECT_LSB:
    {
      /* FIXME: according to the Downloadable Sounds II specification,
         bit 31 should be set when we receive the message on channel
         10 (drum channel) */
	fluid_channel_set_banknum(chan, (((unsigned int) value & 0x7f)
					+ ((unsigned int) chan->bank_msb << 7)));
    }
    break;

  case ALL_NOTES_OFF:
    fluid_synth_all_notes_off(chan->synth, chan->channum);
    break;

  case ALL_SOUND_OFF:
    fluid_synth_all_sounds_off(chan->synth, chan->channum);
    break;

  case ALL_CTRL_OFF:
    fluid_channel_init_ctrl(chan,1);
    fluid_synth_modulate_voices_all(chan->synth, chan->channum);
    break;

  case DATA_ENTRY_MSB:
    {
      int data = (value << 7) + chan->cc[DATA_ENTRY_LSB];

      if (chan->nrpn_active)  /* NRPN is active? */
      {
	/* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
	if ((chan->cc[NRPN_MSB] == 120) && (chan->cc[NRPN_LSB] < 100))
	{
	  if (chan->nrpn_select < GEN_LAST)
	  {
	    float val = fluid_gen_scale_nrpn(chan->nrpn_select, data);
	    fluid_synth_set_gen(chan->synth, chan->channum, chan->nrpn_select, val);
	  }

	  chan->nrpn_select = 0;  /* Reset to 0 */
	}
      }
      else if (chan->cc[RPN_MSB] == 0)    /* RPN is active: MSB = 0? */
      {
	switch (chan->cc[RPN_LSB])
	{
	  case RPN_PITCH_BEND_RANGE:
	    fluid_channel_pitch_wheel_sens (chan, value);   /* Set bend range in semitones */
	    /* FIXME - Handle LSB? (Fine bend range in cents) */
	    break;
          case RPN_CHANNEL_FINE_TUNE:   /* Fine tune is 14 bit over +/-1 semitone (+/- 100 cents, 8192 = center) */
	    fluid_synth_set_gen(chan->synth, chan->channum, GEN_FINETUNE,
                                (data - 8192) / 8192.0 * 100.0);
	    break;
	  case RPN_CHANNEL_COARSE_TUNE: /* Coarse tune is 7 bit and in semitones (64 is center) */
	    fluid_synth_set_gen(chan->synth, chan->channum, GEN_COARSETUNE,
				value - 64);
	    break;
	  case RPN_TUNING_PROGRAM_CHANGE:
	    break;
	  case RPN_TUNING_BANK_SELECT:
	    break;
	  case RPN_MODULATION_DEPTH_RANGE:
	    break;
	}
      }

      break;
    }

  case NRPN_MSB:
    chan->cc[NRPN_LSB] = 0;
    chan->nrpn_select = 0;
    chan->nrpn_active = 1;
    break;

  case NRPN_LSB:
    /* SontFont 2.01 NRPN Message (Sect. 9.6, p. 74)  */
    if (chan->cc[NRPN_MSB] == 120) {
      if (value == 100) {
	chan->nrpn_select += 100;
      } else if (value == 101) {
	chan->nrpn_select += 1000;
      } else if (value == 102) {
	chan->nrpn_select += 10000;
      } else if (value < 100) {
	chan->nrpn_select += value;
      }
    }

    chan->nrpn_active = 1;
    break;

  case RPN_MSB:
  case RPN_LSB:
    chan->nrpn_active = 0;
    break;

  default:
    fluid_synth_modulate_voices(chan->synth, chan->channum, 1, num);
  }

  return FLUID_OK;
}

/*
 * fluid_channel_get_cc
 */
int
fluid_channel_get_cc(fluid_channel_t* chan, int num)
{
  return ((num >= 0) && (num < 128))? chan->cc[num] : 0;
}

/*
 * fluid_channel_pressure
 */
int
fluid_channel_pressure(fluid_channel_t* chan, int val)
{
  chan->channel_pressure = val;
  fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_CHANNELPRESSURE);
  return FLUID_OK;
}

/*
 * fluid_channel_pitch_bend
 */
int
fluid_channel_pitch_bend(fluid_channel_t* chan, int val)
{
  chan->pitch_bend = val;
  fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_PITCHWHEEL);
  return FLUID_OK;
}

/*
 * fluid_channel_pitch_wheel_sens
 */
int
fluid_channel_pitch_wheel_sens(fluid_channel_t* chan, int val)
{
  chan->pitch_wheel_sensitivity = val;
  fluid_synth_modulate_voices(chan->synth, chan->channum, 0, FLUID_MOD_PITCHWHEELSENS);
  return FLUID_OK;
}

/*
 * fluid_channel_get_num
 */
int
fluid_channel_get_num(fluid_channel_t* chan)
{
  return chan->channum;
}

/* Purpose:
 * Sets the index of the interpolation method used on this channel,
 * as in fluid_interp in fluidlite.h
 */
void fluid_channel_set_interp_method(fluid_channel_t* chan, int new_method)
{
  chan->interp_method = new_method;
}

/* Purpose:
 * Returns the index of the interpolation method used on this channel,
 * as in fluid_interp in fluidlite.h
 */
int fluid_channel_get_interp_method(fluid_channel_t* chan)
{
  return chan->interp_method;
}

unsigned int fluid_channel_get_sfontnum(fluid_channel_t* chan)
{
  return chan->sfontnum;
}

int fluid_channel_set_sfontnum(fluid_channel_t* chan, unsigned int sfontnum)
{
  chan->sfontnum = sfontnum;
  return FLUID_OK;
}
