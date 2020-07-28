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

#include "fluid_chorus.h"
#include "fluid_sys.h"

#define MAX_CHORUS	99
#define MAX_DELAY	100
#define MAX_DEPTH	10
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


/* Interpolate how many steps between samples? Must be power of two
   For example: 8 => use a resolution of 256 steps between any two
   samples
*/
#define INTERPOLATION_SUBSAMPLES_LN2 8
#define INTERPOLATION_SUBSAMPLES (1 << (INTERPOLATION_SUBSAMPLES_LN2-1))
#define INTERPOLATION_SUBSAMPLES_ANDMASK (INTERPOLATION_SUBSAMPLES-1)

/* Use how many samples for interpolation? Must be odd.  '7' sounds
   relatively clean, when listening to the modulated delay signal
   alone.  For a demo on aliasing try '1' With '3', the aliasing is
   still quite pronounced for some input frequencies
*/
#define INTERPOLATION_SAMPLES 5

/* Private data for SKEL file */
struct _fluid_chorus_t {
  /* Store the values between fluid_chorus_set_xxx and fluid_chorus_update
   * Logic behind this:
   * - both 'parameter' and 'new_parameter' hold the same value.
   * - To change the chorus settings, 'new_parameter' is modified and
   *   fluid_chorus_update is called.
   * - If the new value is valid, it is copied to 'parameter'.
   * - If it is invalid, 'new_parameter' is restored to 'parameter'.
   */
  int type;                  /* current value */
  int new_type;              /* next value, if parameter check is OK */
  fluid_real_t depth_ms;      /* current value */
  fluid_real_t new_depth_ms;  /* next value, if parameter check is OK */
  fluid_real_t level;         /* current value */
  fluid_real_t new_level;     /* next value, if parameter check is OK */
  fluid_real_t speed_Hz;      /* current value */
  fluid_real_t new_speed_Hz;  /* next value, if parameter check is OK */
  int number_blocks;         /* current value */
  int new_number_blocks;     /* next value, if parameter check is OK */

  fluid_real_t *chorusbuf;
  int counter;
  long phase[MAX_CHORUS];
  long modulation_period_samples;
  int *lookup_tab;
  fluid_real_t sample_rate;

  /* sinc lookup table */
  fluid_real_t sinc_table[INTERPOLATION_SAMPLES][INTERPOLATION_SUBSAMPLES];
};

void fluid_chorus_triangle(int *buf, int len, int depth);
void fluid_chorus_sine(int *buf, int len, int depth);

fluid_chorus_t*
new_fluid_chorus(fluid_real_t sample_rate)
{
  int i; int ii;
  fluid_chorus_t* chorus;

  chorus = FLUID_NEW(fluid_chorus_t);
  if (chorus == NULL) {
    fluid_log(FLUID_PANIC, "chorus: Out of memory");
    return NULL;
  }

  FLUID_MEMSET(chorus, 0, sizeof(fluid_chorus_t));

  chorus->sample_rate = sample_rate;

  /* Lookup table for the SI function (impulse response of an ideal low pass) */

  /* i: Offset in terms of whole samples */
  for (i = 0; i < INTERPOLATION_SAMPLES; i++){

    /* ii: Offset in terms of fractional samples ('subsamples') */
    for (ii = 0; ii < INTERPOLATION_SUBSAMPLES; ii++){
      /* Move the origin into the center of the table */
      double i_shifted = ((double) i- ((double) INTERPOLATION_SAMPLES) / 2.
			  + (double) ii / (double) INTERPOLATION_SUBSAMPLES);
      if (fabs(i_shifted) < 0.000001) {
	/* sinc(0) cannot be calculated straightforward (limit needed
	   for 0/0) */
	chorus->sinc_table[i][ii] = (fluid_real_t)1.;

      } else {
	chorus->sinc_table[i][ii] = (fluid_real_t)sin(i_shifted * M_PI) / (M_PI * i_shifted);
	/* Hamming window */
	chorus->sinc_table[i][ii] *= (fluid_real_t)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (fluid_real_t)INTERPOLATION_SAMPLES));
      };
    };
  };

  /* allocate lookup tables */
  chorus->lookup_tab = FLUID_ARRAY(int, (int) (chorus->sample_rate / MIN_SPEED_HZ));
  if (chorus->lookup_tab == NULL) {
    fluid_log(FLUID_PANIC, "chorus: Out of memory");
    goto error_recovery;
  }

  /* allocate sample buffer */

  chorus->chorusbuf = FLUID_ARRAY(fluid_real_t, MAX_SAMPLES);
  if (chorus->chorusbuf == NULL) {
    fluid_log(FLUID_PANIC, "chorus: Out of memory");
    goto error_recovery;
  }

  if (fluid_chorus_init(chorus) != FLUID_OK){
    goto error_recovery;
  };

  return chorus;

 error_recovery:
  delete_fluid_chorus(chorus);
  return NULL;
}


int
fluid_chorus_init(fluid_chorus_t* chorus)
{
  int i;

  for (i = 0; i < MAX_SAMPLES; i++) {
    chorus->chorusbuf[i] = 0.0;
  }

  /* initialize the chorus with the default settings */
  fluid_chorus_set_nr(chorus, FLUID_CHORUS_DEFAULT_N);
  fluid_chorus_set_level(chorus, FLUID_CHORUS_DEFAULT_LEVEL);
  fluid_chorus_set_speed_Hz(chorus, FLUID_CHORUS_DEFAULT_SPEED);
  fluid_chorus_set_depth_ms(chorus, FLUID_CHORUS_DEFAULT_DEPTH);
  fluid_chorus_set_type(chorus, FLUID_CHORUS_MOD_SINE);

  return fluid_chorus_update(chorus);
}

/* Purpose:
 * Sets the number of stages.
 * Requires call to fluid_chorus_update afterwards.
 * Range checking is performed there.*/
void fluid_chorus_set_nr(fluid_chorus_t* chorus, int nr)
{
  chorus->new_number_blocks = nr;
}

/* Purpose:
 * API function, read the current state of the chorus
 */
int fluid_chorus_get_nr(fluid_chorus_t* chorus)
{
    return chorus->number_blocks;
};

/* Purpose:
 * Sets the mixing level of the signal from each delay line (linear).
 * Requires calling fluid_chorus_update afterwards.*/
void fluid_chorus_set_level(fluid_chorus_t* chorus, fluid_real_t level)
{
  chorus->new_level = level;
}

/* Purpose:
 * API function, read the current state of the chorus
 */
fluid_real_t fluid_chorus_get_level(fluid_chorus_t* chorus)
{
    return chorus->level;
};

/* Purpose:
 * Sets the modulation frequency.
 * Requires call to fluid_chorus_update afterwards.
 * Range checking is performed there.*/
void fluid_chorus_set_speed_Hz(fluid_chorus_t* chorus, fluid_real_t speed_Hz)
{
  chorus->new_speed_Hz = speed_Hz;
}

/* Purpose:
 * API function, read the current state of the chorus
 */
fluid_real_t fluid_chorus_get_speed_Hz(fluid_chorus_t* chorus)
{
    return chorus->speed_Hz;
};

/* Purpose:
 * Sets the modulation depth in ms.
 * Requires call to fluid_chorus_update afterwards.
 * Range checking is performed there.*/
void fluid_chorus_set_depth_ms(fluid_chorus_t* chorus, fluid_real_t depth_ms)
{
  chorus->new_depth_ms=depth_ms;
}

/* Purpose:
 * API function, read the current state of the chorus
 */
fluid_real_t fluid_chorus_get_depth_ms(fluid_chorus_t* chorus)
{
    return chorus->depth_ms;
};

/* Purpose:
 * Sets the type of the modulation waveform.
 * Requires call to fluid_chorus_update afterwards.
 * Check for meaningful values is performed there.*/
void fluid_chorus_set_type(fluid_chorus_t* chorus, int type)
{
  chorus->new_type=type;
}

/* Purpose:
 * API function, read the current state of the chorus
 */
int fluid_chorus_get_type(fluid_chorus_t* chorus)
{
    return chorus->type;
};

void
delete_fluid_chorus(fluid_chorus_t* chorus)
{
  if (chorus == NULL) {
    return;
  }

  if (chorus->chorusbuf != NULL) {
    FLUID_FREE(chorus->chorusbuf);
  }

  if (chorus->lookup_tab != NULL) {
    FLUID_FREE(chorus->lookup_tab);
  }

  FLUID_FREE(chorus);
}


/* Purpose:
 * Calculates the internal chorus parameters using the settings from
 * fluid_chorus_set_xxx. */
int
fluid_chorus_update(fluid_chorus_t* chorus)
{
  int i;
  int modulation_depth_samples;

  if (chorus->new_number_blocks < 0) {
    fluid_log(FLUID_WARN, "chorus: number blocks must be >=0! Setting value to 0.");
    chorus->new_number_blocks = 0;
  } else if (chorus->new_number_blocks > MAX_CHORUS) {
    fluid_log(FLUID_WARN, "chorus: number blocks larger than max. allowed! Setting value to %d.",
	     MAX_CHORUS);
    chorus->new_number_blocks = MAX_CHORUS;
  };

  if (chorus->new_speed_Hz < MIN_SPEED_HZ) {
    fluid_log(FLUID_WARN, "chorus: speed is too low (min %f)! Setting value to min.",
	     (double) MIN_SPEED_HZ);
    chorus->new_speed_Hz = MIN_SPEED_HZ;
  } else if (chorus->new_speed_Hz > MAX_SPEED_HZ) {
    fluid_log(FLUID_WARN, "chorus: speed must be below %f Hz! Setting value to max.",
	     (double) MAX_SPEED_HZ);
    chorus->new_speed_Hz = MAX_SPEED_HZ;
  }
  if (chorus->new_depth_ms < 0.0) {
    fluid_log(FLUID_WARN, "chorus: depth must be positive! Setting value to 0.");
    chorus->new_depth_ms = 0.0;
  }
  /* Depth: Check for too high value through modulation_depth_samples. */

  if (chorus->new_level < 0.0) {
    fluid_log(FLUID_WARN, "chorus: level must be positive! Setting value to 0.");
    chorus->new_level = 0.0;
  } else if (chorus->new_level > 10) {
    fluid_log(FLUID_WARN, "chorus: level must be < 10. A reasonable level is << 1! "
	     "Setting it to 0.1.");
    chorus->new_level = 0.1;
  }

  /* The modulating LFO goes through a full period every x samples: */
  chorus->modulation_period_samples = chorus->sample_rate / chorus->new_speed_Hz;

  /* The variation in delay time is x: */
  modulation_depth_samples = (int)
    (chorus->new_depth_ms / 1000.0  /* convert modulation depth in ms to s*/
     * chorus->sample_rate);

  if (modulation_depth_samples > MAX_SAMPLES) {
    fluid_log(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
    modulation_depth_samples = MAX_SAMPLES;
  }

  /* initialize LFO table */
  if (chorus->type == FLUID_CHORUS_MOD_SINE) {
    fluid_chorus_sine(chorus->lookup_tab, chorus->modulation_period_samples,
		     modulation_depth_samples);
  } else if (chorus->type == FLUID_CHORUS_MOD_TRIANGLE) {
    fluid_chorus_triangle(chorus->lookup_tab, chorus->modulation_period_samples,
			 modulation_depth_samples);
  } else {
    fluid_log(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
    chorus->type = FLUID_CHORUS_MOD_SINE;
    fluid_chorus_sine(chorus->lookup_tab, chorus->modulation_period_samples,
		     modulation_depth_samples);
  };

  for (i = 0; i < chorus->number_blocks; i++) {
    /* Set the phase of the chorus blocks equally spaced */
    chorus->phase[i] = (int) ((double) chorus->modulation_period_samples
			      * (double) i / (double) chorus->number_blocks);
  }

  /* Start of the circular buffer */
  chorus->counter = 0;

  chorus->type = chorus->new_type;
  chorus->depth_ms = chorus->new_depth_ms;
  chorus->level = chorus->new_level;
  chorus->speed_Hz = chorus->new_speed_Hz;
  chorus->number_blocks = chorus->new_number_blocks;
  return FLUID_OK;

/*  failure: */
  /* Note: This lives on the assumption, that the last chorus values were correct.
   * If not, this will loop forever and a day. */
/*   fluid_log(FLUID_WARN, "chorus: Restoring last good settings"); */
/*   chorus->new_type = chorus->type; */
/*   chorus->new_depth_ms = chorus->depth_ms; */
/*   chorus->new_level = chorus->level; */
/*   chorus->new_speed_Hz = chorus->speed_Hz; */
/*   chorus->new_number_blocks = chorus->number_blocks; */
/*   return FLUID_FAILED; */
}


void fluid_chorus_processmix(fluid_chorus_t* chorus, fluid_real_t *in,
			    fluid_real_t *left_out, fluid_real_t *right_out)
{
  int sample_index;
  int i;
  fluid_real_t d_in, d_out;

  for (sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++) {

    d_in = in[sample_index];
    d_out = 0.0f;

# if 0
    /* Debug: Listen to the chorus signal only */
    left_out[sample_index]=0;
    right_out[sample_index]=0;
#endif

    /* Write the current sample into the circular buffer */
    chorus->chorusbuf[chorus->counter] = d_in;

    for (i = 0; i < chorus->number_blocks; i++) {
      int ii;
      /* Calculate the delay in subsamples for the delay line of chorus block nr. */

      /* The value in the lookup table is so, that this expression
       * will always be positive.  It will always include a number of
       * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
       * remain positive at all times. */
      int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
			    - chorus->lookup_tab[chorus->phase[i]]);

      int pos_samples = pos_subsamples/INTERPOLATION_SUBSAMPLES;

      /* modulo divide by INTERPOLATION_SUBSAMPLES */
      pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

      for (ii = 0; ii < INTERPOLATION_SAMPLES; ii++){
	/* Add the delayed signal to the chorus sum d_out Note: The
	 * delay in the delay line moves backwards for increasing
	 * delay!*/

	/* The & in chorusbuf[...] is equivalent to a division modulo
	   MAX_SAMPLES, only faster. */
	d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
	  * chorus->sinc_table[ii][pos_subsamples];

	pos_samples--;
      };
      /* Cycle the phase of the modulating LFO */
      chorus->phase[i]++;
      chorus->phase[i] %= (chorus->modulation_period_samples);
    } /* foreach chorus block */

    d_out *= chorus->level;

    /* Add the chorus sum d_out to output */
    left_out[sample_index] += d_out;
    right_out[sample_index] += d_out;

    /* Move forward in circular buffer */
    chorus->counter++;
    chorus->counter %= MAX_SAMPLES;

  } /* foreach sample */
}

/* Duplication of code ... (replaces sample data instead of mixing) */
void fluid_chorus_processreplace(fluid_chorus_t* chorus, fluid_real_t *in,
				fluid_real_t *left_out, fluid_real_t *right_out)
{
  int sample_index;
  int i;
  fluid_real_t d_in, d_out;

  for (sample_index = 0; sample_index < FLUID_BUFSIZE; sample_index++) {

    d_in = in[sample_index];
    d_out = 0.0f;

# if 0
    /* Debug: Listen to the chorus signal only */
    left_out[sample_index]=0;
    right_out[sample_index]=0;
#endif

    /* Write the current sample into the circular buffer */
    chorus->chorusbuf[chorus->counter] = d_in;

    for (i = 0; i < chorus->number_blocks; i++) {
      int ii;
      /* Calculate the delay in subsamples for the delay line of chorus block nr. */

      /* The value in the lookup table is so, that this expression
       * will always be positive.  It will always include a number of
       * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
       * remain positive at all times. */
      int pos_subsamples = (INTERPOLATION_SUBSAMPLES * chorus->counter
			    - chorus->lookup_tab[chorus->phase[i]]);

      int pos_samples = pos_subsamples / INTERPOLATION_SUBSAMPLES;

      /* modulo divide by INTERPOLATION_SUBSAMPLES */
      pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

      for (ii = 0; ii < INTERPOLATION_SAMPLES; ii++){
	/* Add the delayed signal to the chorus sum d_out Note: The
	 * delay in the delay line moves backwards for increasing
	 * delay!*/

	/* The & in chorusbuf[...] is equivalent to a division modulo
	   MAX_SAMPLES, only faster. */
	d_out += chorus->chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
	  * chorus->sinc_table[ii][pos_subsamples];

	pos_samples--;
      };
      /* Cycle the phase of the modulating LFO */
      chorus->phase[i]++;
      chorus->phase[i] %= (chorus->modulation_period_samples);
    } /* foreach chorus block */

    d_out *= chorus->level;

    /* Store the chorus sum d_out to output */
    left_out[sample_index] = d_out;
    right_out[sample_index] = d_out;

    /* Move forward in circular buffer */
    chorus->counter++;
    chorus->counter %= MAX_SAMPLES;

  } /* foreach sample */
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
void fluid_chorus_sine(int *buf, int len, int depth)
{
  int i;
  double val;

  for (i = 0; i < len; i++) {
    val = sin((double) i / (double)len * 2.0 * M_PI);
    buf[i] = (int) ((1.0 + val) * (double) depth / 2.0 * (double) INTERPOLATION_SUBSAMPLES);
    buf[i] -= 3* MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
    //    printf("%i %i\n",i,buf[i]);
  }
}

/* Purpose:
 * Calculates a modulation waveform (triangle)
 * See fluid_chorus_sine for comments.
 */
void fluid_chorus_triangle(int *buf, int len, int depth)
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

void
fluid_chorus_reset(fluid_chorus_t* chorus)
{
  fluid_chorus_init(chorus);
}
