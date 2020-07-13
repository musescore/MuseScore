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

#ifndef _FLUID_CHAN_H
#define _FLUID_CHAN_H

#include "fluidsynth_priv.h"
#include "fluid_midi.h"
#include "fluid_tuning.h"

/*
 * fluid_channel_t
 */
struct _fluid_channel_t
{
  int channum;
  unsigned int sfontnum;
  unsigned int banknum;
  unsigned int prognum;
  fluid_preset_t* preset;
  fluid_synth_t* synth;
  short key_pressure;
  short channel_pressure;
  short pitch_bend;
  short pitch_wheel_sensitivity;

  /* controller values */
  short cc[128];

  /* cached values of last MSB values of MSB/LSB controllers */
  unsigned char bank_msb;
  int interp_method;

  /* the micro-tuning */
  fluid_tuning_t* tuning;

  /* NRPN system */
  short nrpn_select;
  short nrpn_active;    /* 1 if data entry CCs are for NRPN, 0 if RPN */

  /* The values of the generators, set by NRPN messages, or by
   * fluid_synth_set_gen(), are cached in the channel so they can be
   * applied to future notes. They are copied to a voice's generators
   * in fluid_voice_init(), wihich calls fluid_gen_init().  */
  fluid_real_t gen[GEN_LAST];

  /* By default, the NRPN values are relative to the values of the
   * generators set in the SoundFont. For example, if the NRPN
   * specifies an attack of 100 msec then 100 msec will be added to the
   * combined attack time of the sound font and the modulators.
   *
   * However, it is useful to be able to specify the generator value
   * absolutely, completely ignoring the generators of the sound font
   * and the values of modulators. The gen_abs field, is a boolean
   * flag indicating whether the NRPN value is absolute or not.
   */
  char gen_abs[GEN_LAST];
};

fluid_channel_t* new_fluid_channel(fluid_synth_t* synth, int num);
int delete_fluid_channel(fluid_channel_t* chan);
void fluid_channel_init(fluid_channel_t* chan);
void fluid_channel_init_ctrl(fluid_channel_t* chan, int is_all_ctrl_off);
void fluid_channel_reset(fluid_channel_t* chan);
int fluid_channel_set_preset(fluid_channel_t* chan, fluid_preset_t* preset);
fluid_preset_t* fluid_channel_get_preset(fluid_channel_t* chan);
unsigned int fluid_channel_get_sfontnum(fluid_channel_t* chan);
int fluid_channel_set_sfontnum(fluid_channel_t* chan, unsigned int sfont);
unsigned int fluid_channel_get_banknum(fluid_channel_t* chan);
int fluid_channel_set_banknum(fluid_channel_t* chan, unsigned int bank);
int fluid_channel_set_prognum(fluid_channel_t* chan, int prognum);
int fluid_channel_get_prognum(fluid_channel_t* chan);
int fluid_channel_cc(fluid_channel_t* chan, int ctrl, int val);
int fluid_channel_pressure(fluid_channel_t* chan, int val);
int fluid_channel_pitch_bend(fluid_channel_t* chan, int val);
int fluid_channel_pitch_wheel_sens(fluid_channel_t* chan, int val);
int fluid_channel_get_cc(fluid_channel_t* chan, int num);
int fluid_channel_get_num(fluid_channel_t* chan);
void fluid_channel_set_interp_method(fluid_channel_t* chan, int new_method);
int fluid_channel_get_interp_method(fluid_channel_t* chan);

#define fluid_channel_set_tuning(_c, _t)        { (_c)->tuning = _t; }
#define fluid_channel_has_tuning(_c)            ((_c)->tuning != NULL)
#define fluid_channel_get_tuning(_c)            ((_c)->tuning)
#define fluid_channel_sustained(_c)             ((_c)->cc[SUSTAIN_SWITCH] >= 64)
#define fluid_channel_set_gen(_c, _n, _v, _a)   { (_c)->gen[_n] = _v; (_c)->gen_abs[_n] = _a; }
#define fluid_channel_get_gen(_c, _n)           ((_c)->gen[_n])
#define fluid_channel_get_gen_abs(_c, _n)       ((_c)->gen_abs[_n])

#define fluid_channel_get_min_note_length_ticks(chan) \
  ((chan)->synth->min_note_length_ticks)

#endif /* _FLUID_CHAN_H */
