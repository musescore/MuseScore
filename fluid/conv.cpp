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

#include "conv.h"

namespace FluidS {

/* conversion tables */
static float fluid_cb2amp_tab[FLUID_CB_AMP_SIZE];
static float fluid_atten2amp_tab[FLUID_ATTEN_AMP_SIZE];
static float fluid_concave_tab[128];
static float fluid_convex_tab[128];
static float fluid_pan_tab[FLUID_PAN_SIZE];

/*
 * void fluid_synth_init
 *
 * Does all the initialization for this module.
 */
void fluid_conversion_config()
      {
      /* centibels to amplitude conversion
       * Note: SF2.01 section 8.1.3: Initial attenuation range is
       * between 0 and 144 dB. Therefore a negative attenuation is
       * not allowed.
       */
      for (int i = 0; i < FLUID_CB_AMP_SIZE; i++)
            fluid_cb2amp_tab[i] = (float) pow(10.0, (double) i / -200.0);

      /* NOTE: EMU8k and EMU10k devices don't conform to the SoundFont
       * specification in regards to volume attenuation.  The below calculation
       * is an approx. equation for generating a table equivalent to the
       * cb_to_amp_table[] in tables.c of the TiMidity++ source, which I'm told
       * was generated from device testing.  By the spec this should be centibels.
       */
      for (int i = 0; i < FLUID_ATTEN_AMP_SIZE; i++)
            fluid_atten2amp_tab[i] = (float) pow(10.0, (double) i / FLUID_ATTEN_POWER_FACTOR);

      /* initialize the conversion tables (see fluid_mod.c
         fluid_mod_get_value cases 4 and 8) */

      /* concave unipolar positive transform curve */
      fluid_concave_tab[0] = 0.0;
      fluid_concave_tab[127] = 1.0;

      /* convex unipolar positive transform curve */
      fluid_convex_tab[0] = 0;
      fluid_convex_tab[127] = 1.0;

      /* There seems to be an error in the specs. The equations are
         implemented according to the pictures on SF2.01 page 73. */

      for (int i = 1; i < 127; i++) {
            double x = -20.0 / 96.0 * log((i * i) / (127.0 * 127.0)) / log(10.0);
            fluid_convex_tab[i] = (float) (1.0 - x);
            fluid_concave_tab[127 - i] = (float) x;
            }

      /* initialize the pan conversion table */
      double x = M_PI / 2.0 / (FLUID_PAN_SIZE - 1.0);
      for (int i = 0; i < FLUID_PAN_SIZE; i++)
            fluid_pan_tab[i] = (float) sin(i * x);
      }

/*
 * fluid_cb2amp
 *
 * in: a value between 0 and 960, 0 is no attenuation
 * out: a value between 1 and 0
 */
float fluid_cb2amp(float cb)
      {
      /*
       * cb: an attenuation in 'centibels' (1/10 dB)
       * SF2.01 page 49 # 48 limits it to 144 dB.
       * 96 dB is reasonable for 16 bit systems, 144 would make sense for 24 bit.
       */

      /* minimum attenuation: 0 dB */
      if (cb < 0)
            return 1.0;
      if (cb >= FLUID_CB_AMP_SIZE)
            return 0.0;
      return fluid_cb2amp_tab[(int) cb];
      }

/*
 * fluid_atten2amp
 *
 * in: a value between 0 and 1440, 0 is no attenuation
 * out: a value between 1 and 0
 *
 * Note: Volume attenuation is supposed to be centibels but EMU8k/10k don't
 * follow this.  That's the reason for separate fluid_cb2amp and fluid_atten2amp.
 */
float fluid_atten2amp(float atten)
      {
      if (atten < 0)
            return 1.0;
      else if (atten >= FLUID_ATTEN_AMP_SIZE)
            return 0.0;
      else
            return fluid_atten2amp_tab[(int) atten];
      }

/*
 * fluid_tc2sec_delay
 */
float fluid_tc2sec_delay(float tc)
      {
      /* SF2.01 section 8.1.2 items 21, 23, 25, 33
       * SF2.01 section 8.1.3 items 21, 23, 25, 33
       *
       * The most negative number indicates a delay of 0. Range is limited
       * from -12000 to 5000
       */
      if (tc <= -32768.0f)
            return (float) 0.0f;
      if (tc < -12000.)
            tc = (float) -12000.0f;
      if (tc > 5000.0f)
            tc = (float) 5000.0f;
      return (float) pow(2.0, (double) tc / 1200.0);
      }

/*
 * fluid_tc2sec_attack
 */
float fluid_tc2sec_attack(float tc)
      {
      /* SF2.01 section 8.1.2 items 26, 34
      * SF2.01 section 8.1.3 items 26, 34
      * The most negative number indicates a delay of 0
      * Range is limited from -12000 to 8000 */
      if (tc<=-32768.)
            return (float) 0.0;
      if (tc<-12000.)
            tc=(float) -12000.0;
      if (tc>8000.)
            tc=(float) 8000.0;
      return (float) pow(2.0, (double) tc / 1200.0);
      }

/*
 * fluid_tc2sec
 */
float fluid_tc2sec(float tc)
      {
      /* No range checking here! */
      return (float) pow(2.0, (double) tc / 1200.0);
      }

/*
 * fluid_tc2sec_release
 */
float fluid_tc2sec_release(float tc)
      {
      /* SF2.01 section 8.1.2 items 30, 38
       * SF2.01 section 8.1.3 items 30, 38
       * No 'most negative number' rule here!
       * Range is limited from -12000 to 8000
       */
      if (tc<=-32768.)
            return (float) 0.0;
      if (tc<-12000.)
            tc=(float) -12000.0;
      if (tc>8000.)
            tc=(float) 8000.0;
      return (float) pow(2.0, (double) tc / 1200.0);
      }

/*
 * fluid_act2hz
 *
 * Convert from absolute cents to Hertz
 */
float fluid_act2hz(float c)
      {
      return (float) (8.176 * pow(2.0, (double) c / 1200.0));
      }

/*
 * fluid_pan
 */
float fluid_pan(float c, int left)
      {
      if (left)
            c = -c;
      if (c < -500)
            return (float) 0.0;
      else if (c > 500)
            return (float) 1.0;
      else
            return fluid_pan_tab[(int) (c + 500)];
      }

/*
 * fluid_concave
 */
float fluid_concave(float val)
      {
      if (val < 0)
            return 0;
      else if (val > 127)
            return 1;
      return fluid_concave_tab[(int) val];
      }

/*
 * fluid_convex
 */
float fluid_convex(float val)
      {
      if (val < 0)
            return 0;
      else if (val > 127)
            return 1;
      return fluid_convex_tab[(int) val];
      }
}

