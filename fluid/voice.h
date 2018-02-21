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

#ifndef _FLUID_VOICE_H
#define _FLUID_VOICE_H

#include "fluid.h"
#include "gen.h"

namespace FluidS {

#define NO_CHANNEL             0xff

enum fluid_voice_status {
	FLUID_VOICE_OFF,
	FLUID_VOICE_ON,
	FLUID_VOICE_SUSTAINED
      };

/*
 * envelope data
 */
struct fluid_env_data_t {
	unsigned int count;     // sample count
	float coeff;
	float incr;
	float min;
	float max;
      };

/* Indices for envelope tables */
enum fluid_voice_envelope_index_t {
	FLUID_VOICE_ENVDELAY,
	FLUID_VOICE_ENVATTACK,
	FLUID_VOICE_ENVHOLD,
	FLUID_VOICE_ENVDECAY,
	FLUID_VOICE_ENVSUSTAIN,
	FLUID_VOICE_ENVRELEASE,
	FLUID_VOICE_ENVFINISHED,
	FLUID_VOICE_ENVLAST
      };

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice
      {
      static float interp_coeff_linear[FLUID_INTERP_MAX][2];
      static float interp_coeff[FLUID_INTERP_MAX][4];
      static float sinc_table7[FLUID_INTERP_MAX][7];

      Fluid* _fluid;
      double _noteTuning;             // +/- in midicent

      void effects(int count, float* out, float* effect1, float* effect2);

   public:
	unsigned int id;                // the id is incremented for every new noteon.
					        // it's used for noteoff's
	unsigned char status;
	unsigned char chan;             // the channel number, quick access for channel messages
	unsigned char key;              // the key, quick access for noteoff
	unsigned char vel;              // the velocity

	Channel* channel;
	Generator gen[GEN_LAST];
	Mod mod[FLUID_NUM_MOD];

	int mod_count;
	bool has_looped;                /* Flag that is set as soon as the first loop is completed. */
	Sample* sample;
	int check_sample_sanity_flag;   /* Flag that initiates, that sample-related parameters
					           have to be checked. */
	unsigned int ticks;

	float amp;                       /* the linear amplitude */
	Phase phase;                     // the phase of the sample wave

	// Temporary variables used in write()
	float phase_incr;	      /* the phase increment for the next 64 samples */
    qreal amp_incr;		/* amplitude increment value */
	float* dsp_buf;	      /* buffer to store interpolated sample data to */

	/* basic parameters */
	float pitch;              /* the pitch in midicents */
	float attenuation;        /* the attenuation in centibels */
	float min_attenuation_cB; /* Estimate on the smallest possible attenuation
					          * during the lifetime of the voice */
	float root_pitch, root_pitch_hz;

	/* sample and loop start and end points (offset in sample memory).  */
	int start;
	int end;
	int loopstart;
	int loopend;

	/* vol env */
	fluid_env_data_t volenv_data[FLUID_VOICE_ENVLAST];
	unsigned int volenv_count;
	int volenv_section;
   std::map<int, qreal> Sample2AmpInc;
	float volenv_val;
	float amplitude_that_reaches_noise_floor_nonloop;
	float amplitude_that_reaches_noise_floor_loop;
   int positionToTurnOff; // this is the sample accurate position where the sample reaches the noise floor

	/* mod env */
	fluid_env_data_t modenv_data[FLUID_VOICE_ENVLAST];
	unsigned int modenv_count;
	int modenv_section;
	float modenv_val;         /* the value of the modulation envelope */
	float modenv_to_fc;
	float modenv_to_pitch;

	/* mod lfo */
	float modlfo_val;          /* the value of the modulation LFO */
	unsigned int modlfo_delay;       /* the delay of the lfo in samples */
   unsigned int modlfo_pos;
   unsigned int modlfo_dur; // duration in samples
   float modlfo_to_fc;
	float modlfo_to_pitch;
	float modlfo_to_vol;

	/* vib lfo */
	float viblfo_val;        /* the value of the vibrato LFO */
	unsigned int viblfo_delay;      /* the delay of the lfo in samples */
	float viblfo_incr;       /* the lfo frequency is converted to a per-buffer increment */
	float viblfo_to_pitch;

	/* resonant filter */
	float fres;              /* the resonance frequency, in cents (not absolute cents) */
	float last_fres;         /* Current resonance frequency of the IIR filter */
	/* Serves as a flag: A deviation between fres and last_fres */
	/* indicates, that the filter has to be recalculated. */
	float q_lin;             /* the q-factor on a linear scale */
	float filter_gain;       /* Gain correction factor, depends on q */
	float hist1, hist2;      /* Sample history for the IIR filter */
	int filter_startup;             /* Flag: If set, the filter will be set directly.
					   Else it changes smoothly. */

	/* filter coefficients */
	/* The coefficients are normalized to a0. */
	/* b0 and b2 are identical => b02 */
	float b02;              /* b0 / a0 */
	float b1;              /* b1 / a0 */
	float a1;              /* a0 / a0 */
	float a2;              /* a1 / a0 */

	float b02_incr;
	float b1_incr;
	float a1_incr;
	float a2_incr;
	int filter_coeff_incr_count;

	/* pan */
	float pan;
	float amp_left;
	float amp_right;

	/* reverb */
	float reverb_send;
	float amp_reverb;

	/* chorus */
	float chorus_send;
	float amp_chorus;

	/* interpolation method, as in fluid_interp in fluidsynth.h */
	int interp_method;

	/* for debugging */
	int debug;
	double ref;

   public:
      Voice(Fluid*);
      Channel* get_channel() const    { return channel; }
      void voice_start();
      void off();
      void init(Sample*, Channel*, int key, int vel, unsigned id, double tuning);
      void gen_incr(int i, float val);
      void gen_set(int i, float val);
      float gen_get(int gen);
      unsigned int get_id() const { return id; }
      bool isPlaying()            { return ((status == FLUID_VOICE_ON) || (status == FLUID_VOICE_SUSTAINED)); }
      void set_param(int gen, float nrpn_value, int abs);

      // Update all the synthesis parameters, which depend on generator
      // 'gen'. This is only necessary after changing a generator of an
      // already operating voice.  Most applications will not need this
      // function.

      void update_param(int gen);

      double GEN(int n) { return gen[n].val + gen[n].mod + gen[n].nrpn; }

      void modulate_all();
      void modulate(bool _cc, int _ctrl);
      float get_lower_boundary_for_attenuation();
      void check_sample_sanity();
      void noteoff();
      void kill_excl();
      int calculate_hold_decay_frames(int gen_base, int gen_key2base, int is_decay);
      void calculate_gen_pitch();

      /* A voice is 'ON', if it has not yet received a noteoff
       * event. Sending a noteoff event will advance the envelopes to
       * section 5 (release).
       */
      bool RELEASED() const    { return chan == NO_CHANNEL; }
      bool SUSTAINED() const   { return status == FLUID_VOICE_SUSTAINED; }
      bool PLAYING() const     { return (status == FLUID_VOICE_ON) || (status == FLUID_VOICE_SUSTAINED); }
      bool ON() const          { return (status == FLUID_VOICE_ON) && (volenv_section < FLUID_VOICE_ENVRELEASE); }
      int SAMPLEMODE() const   { return ((int)gen[GEN_SAMPLEMODE].val); }

      void calcVolEnv(int n, fluid_env_data_t *env_data);
      void write(unsigned n, float* out, float* reverb, float* chorus);
      void add_mod(const Mod* mod, int mode);

      static void dsp_float_config();
      bool updateAmpInc(unsigned int &nextNewAmpInc, std::map<int, qreal>::iterator &curSample2AmpInc, qreal &dsp_amp_incr, unsigned int &dsp_i);
      int dsp_float_interpolate_none(unsigned);
      int dsp_float_interpolate_linear(unsigned);
      int dsp_float_interpolate_4th_order(unsigned);
      int dsp_float_interpolate_7th_order(unsigned);
      };
}


#endif /* _FLUID_VOICE_H */
