/*
 * August 24, 1998
 * Copyright (C) 1998 Juergen Mueller And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Juergen Mueller And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */


/*

  CHANGES

  - Adapted for fluidsynth, Peter Hanappe, March 2002

  - Variable delay line implementation using bandlimited
    interpolation, code reorganization: Markus Nentwig May 2002

 */


/*
 * 	Chorus effect.
 *
 * Flow diagram scheme for n delays ( 1 <= n <= MAX_CHORUS ):
 *
 *        * gain-in                                           ___
 * ibuff -----+--------------------------------------------->|   |
 *            |      _________                               |   |
 *            |     |         |                   * level 1  |   |
 *            +---->| delay 1 |----------------------------->|   |
 *            |     |_________|                              |   |
 *            |        /|\                                   |   |
 *            :         |                                    |   |
 *            : +-----------------+   +--------------+       | + |
 *            : | Delay control 1 |<--| mod. speed 1 |       |   |
 *            : +-----------------+   +--------------+       |   |
 *            |      _________                               |   |
 *            |     |         |                   * level n  |   |
 *            +---->| delay n |----------------------------->|   |
 *                  |_________|                              |   |
 *                     /|\                                   |___|
 *                      |                                      |
 *              +-----------------+   +--------------+         | * gain-out
 *              | Delay control n |<--| mod. speed n |         |
 *              +-----------------+   +--------------+         +----->obuff
 *
 *
 * The delay i is controlled by a sine or triangle modulation i ( 1 <= i <= n).
 *
 * The delay of each block is modulated between 0..depth ms
 *
 */


/* Variable delay line implementation
 * ==================================
 *
 * The modulated delay needs the value of the delayed signal between
 * samples.  A lowpass filter is used to obtain intermediate values
 * between samples (bandlimited interpolation).  The sample pulse
 * train is convoluted with the impulse response of the low pass
 * filter (sinc function).  To make it work with a small number of
 * samples, the sinc function is windowed (Hamming window).
 *
 */

#include "chorus.h"
#include "fluid.h"

namespace FluidS {

#define MAX_DEPTH	      10
#define MIN_SPEED_HZ	0.29
#define MAX_SPEED_HZ    5

/* Length of one delay line in samples:
 * Set through MAX_SAMPLES_LN2.
 * For example:
 * MAX_SAMPLES_LN2=12
 * => MAX_SAMPLES=pow(2,12)=4096
 * => MAX_SAMPLES_ANDMASK=4095
 */
#define MAX_SAMPLES_LN2 12

#define MAX_SAMPLES (1 << (MAX_SAMPLES_LN2-1))
#define MAX_SAMPLES_ANDMASK (MAX_SAMPLES-1)

#define fluid_log(a, ...)

//---------------------------------------------------------
//   Chorus
//---------------------------------------------------------

Chorus::Chorus(float sr)
      {
      counter     = 0;
      sample_rate = sr;

      /* Lookup table for the SI function (impulse response of an ideal low pass) */

      /* i: Offset in terms of whole samples */
      for (int i = 0; i < INTERPOLATION_SAMPLES; i++) {
            /* ii: Offset in terms of fractional samples ('subsamples') */
            for (int ii = 0; ii < INTERPOLATION_SUBSAMPLES; ii++){
                  /* Move the origin into the center of the table */
                  double i_shifted = ((double) i- ((double) INTERPOLATION_SAMPLES) / 2.
			  + (double) ii / (double) INTERPOLATION_SUBSAMPLES);
                  if (fabs(i_shifted) < 0.000001) {
                        /* sinc(0) cannot be calculated straightforward (limit needed
                           for 0/0) */
                        sinc_table[i][ii] = (float)1.;
                        }
                  else {
                        sinc_table[i][ii] = (float)sin(i_shifted * M_PI) / (M_PI * i_shifted);
                        /* Hamming window */
                        sinc_table[i][ii] *= (float)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (float)INTERPOLATION_SAMPLES));
                        }
                  }
            }
      lookup_tab = new int[(int) (sample_rate / MIN_SPEED_HZ)];
      chorusbuf  = new float[MAX_SAMPLES];
      reset();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chorus::reset()
      {
      memset(chorusbuf, 0, MAX_SAMPLES * sizeof(*chorusbuf));
      number_blocks = 0;
      number_blocks = FLUID_CHORUS_DEFAULT_N;
      level         = FLUID_CHORUS_DEFAULT_LEVEL;
      speed_Hz      = FLUID_CHORUS_DEFAULT_SPEED;
      depth_ms      = FLUID_CHORUS_DEFAULT_DEPTH;
      type          = FLUID_CHORUS_MOD_SINE;

      update();
      }

//---------------------------------------------------------
//   ~Chorus
//---------------------------------------------------------

Chorus::~Chorus()
      {
      delete[] chorusbuf;
      delete[] lookup_tab;
      }

//---------------------------------------------------------
//   update
//    Calculates the internal chorus parameters using the settings from
//    fluid_chorus_set_xxx.
//---------------------------------------------------------

void Chorus::update()
      {
      /* The modulating LFO goes through a full period every x samples: */
      modulation_period_samples = lrint(sample_rate / speed_Hz);

      /* The variation in delay time is x: */
      int modulation_depth_samples = (int)
         (depth_ms / 1000.0  /* convert modulation depth in ms to s*/
         * sample_rate);

      if (modulation_depth_samples > MAX_SAMPLES) {
            fluid_log(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
            modulation_depth_samples = MAX_SAMPLES;
            }

      /* initialize LFO table */
      if (type == FLUID_CHORUS_MOD_SINE)
            sine(lookup_tab, modulation_period_samples, modulation_depth_samples);
      else if (type == FLUID_CHORUS_MOD_TRIANGLE)
            triangle(lookup_tab, modulation_period_samples, modulation_depth_samples);
      else {
            type = FLUID_CHORUS_MOD_SINE;
            sine(lookup_tab, modulation_period_samples, modulation_depth_samples);
            }

      for (int i = 0; i < number_blocks; i++) {
            /* Set the phase of the chorus blocks equally spaced */
            phase[i] = (int) ((double) modulation_period_samples
               * (double) i / (double) number_blocks);
            }

      /* Start of the circular buffer */
      counter = 0;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Chorus::process(int n, float* in, float* left_out, float* right_out)
      {
      for (int sample_index = 0; sample_index < n; sample_index++) {
            float d_in = in[sample_index];
            float d_out = 0.0f;

            /* Write the current sample into the circular buffer */
            chorusbuf[counter] = d_in;

            for (int i = 0; i < number_blocks; i++) {
                  /* Calculate the delay in subsamples for the delay line of chorus block nr. */

                  /* The value in the lookup table is so, that this expression
                   * will always be positive.  It will always include a number of
                   * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
                   * remain positive at all times.
                   */
                  int pos_subsamples = (INTERPOLATION_SUBSAMPLES * counter
                     - lookup_tab[phase[i]]);

                  int pos_samples = pos_subsamples/INTERPOLATION_SUBSAMPLES;

                  /* modulo divide by INTERPOLATION_SUBSAMPLES */
                  pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

                  for (int ii = 0; ii < INTERPOLATION_SAMPLES; ii++) {
	                  /* Add the delayed signal to the chorus sum d_out Note: The
	                   * delay in the delay line moves backwards for increasing
	                   * delay!*/

	                  /* The & in chorusbuf[...] is equivalent to a division modulo
	                     MAX_SAMPLES, only faster.
                         */
                        d_out += (chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
                           * sinc_table[ii][pos_subsamples]);
                        pos_samples--;
                        }
                  /* Cycle the phase of the modulating LFO */
                  phase[i]++;
                  phase[i] %= (modulation_period_samples);
                  } /* foreach chorus block */

            d_out *= level;

            /* Add the chorus sum d_out to output */
            left_out[sample_index]  += d_out;
            right_out[sample_index] += d_out;

            /* Move forward in circular buffer */
            counter++;
            counter %= MAX_SAMPLES;
            }
      }

/* Purpose:
 *
 * Calculates a modulation waveform (sine) Its value ( modulo
 * MAXSAMPLES) varies between 0 and depth*INTERPOLATION_SUBSAMPLES.
 * Its period length is len.  The waveform data will be used modulo
 * MAXSAMPLES only.  Since MAXSAMPLES is substracted from the waveform
 * a couple of times here, the resulting (current position in
 * buffer)-(waveform sample) will always be positive.
 */
void Chorus::sine(int *buf, int len, int depth)
      {
      for (int i = 0; i < len; i++) {
            double val = sin((double) i / (double)len * 2.0 * M_PI);
            buf[i] = (int) ((1.0 + val) * (double) depth / 2.0 * (double) INTERPOLATION_SUBSAMPLES);
            buf[i] -= 3* MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
            }
      }

/* Purpose:
 * Calculates a modulation waveform (triangle)
 * See fluid_chorus_sine for comments.
 */
void Chorus::triangle(int *buf, int len, int depth)
      {
      int i=0;
      int ii=len-1;
      double val;
      double val2;

      while (i <= ii){
            val = i * 2.0 / len * (double)depth * (double) INTERPOLATION_SUBSAMPLES;
            val2= (int) (val + 0.5) - 3 * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
            buf[i++] = (int) val2;
            buf[ii--] = (int) val2;
            }
      }

//---------------------------------------------------------
//   pNames
//    chorus parameter names, sync with fluid.h
//---------------------------------------------------------

static const char* pNames[] = {
      "CHORUS_TYPE",
      "CHORUS_SPEED",
      "CHORUS_DEPTH",
      "CHORUS_BLOCKS",
      "CHORUS_GAIN"
      };

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void Chorus::setParameter(int idx, float value)
      {
// printf("Chorus: setParameter %s(%d) %f\n", pNames[idx], idx, value);
      switch (idx) {
            case CHORUS_TYPE:
                  type = lrint(value);
                  break;
            case CHORUS_SPEED:
                  speed_Hz = value * MAX_SPEED_HZ + MIN_SPEED_HZ;
                  break;
            case CHORUS_DEPTH:
                  depth_ms = value * MAX_DEPTH;
                  break;
            case CHORUS_BLOCKS:
                  number_blocks = lrint(value * 100.0);
                  break;
            case CHORUS_GAIN:
                  level = value;
                  return;     // do not call update
            default:
                  printf("Chorus:setParameter: %x invalid\n", idx);
                  break;
            }
      update();
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

double Chorus::parameter(int idx) const
      {
      float value = 0.0;
      switch (idx) {
            case CHORUS_TYPE:   value = type; break;
            case CHORUS_SPEED:  value = (speed_Hz-MIN_SPEED_HZ) / MAX_SPEED_HZ; break;
            case CHORUS_DEPTH:  value = depth_ms / MAX_DEPTH; break;
            case CHORUS_BLOCKS: value = number_blocks / 100.0; break;
            case CHORUS_GAIN:   value = level; break;
            default:
                  printf("Chorus::parameter: 0x%x invalid\n", idx);
                  break;
            }
// printf("Chorus: parameter %s(%d) %f\n", pNames[idx], idx, value);
      return value;
      }
}
