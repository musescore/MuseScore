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
#include "fluid.h"
#include "sfont.h"
#include "gen.h"
#include "voice.h"

namespace FluidS {

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)

#define fluid_clip(_val, _min, _max) \
   { (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

/* used for filter turn off optimization - if filter cutoff is above the
   specified value and filter q is below the other value, turn filter off */
#define FLUID_MAX_AUDIBLE_FILTER_FC 19000.0f
#define FLUID_MIN_AUDIBLE_FILTER_Q 1.2f

/* Smallest amplitude that can be perceived (full scale is +/- 0.5)
 * 16 bits => 96+4=100 dB dynamic range => 0.00001
 * 0.00001 * 2 is approximately 0.00003 :)
 */
#define FLUID_NOISE_FLOOR 0.00003f

/* these should be the absolute minimum that FluidSynth can deal with */
#define FLUID_MIN_LOOP_SIZE 2
#define FLUID_MIN_LOOP_PAD  0

/* min vol envelope release (to stop clicks) in SoundFont timecents */
#define FLUID_MIN_VOLENVRELEASE -7200.0f /* ~16ms */


//---------------------------------------------------------
//   triangle - calc value of triangle function for lfos
//---------------------------------------------------------

float triangle(int dur,int pos) {
      pos += dur/4;
      pos %= dur;
      if (pos>dur/2)
            return 2*(0.5-((pos/(0.5*dur))-1));
      else
            return 2*((pos/(0.5*dur))-0.5);
      }

//---------------------------------------------------------
//  samplesToNextTurningPoint
//  calculate how many samples it is to the next change
//  from rising to falling in a triangle function
//---------------------------------------------------------

int samplesToNextTurningPoint(int dur, int pos) {
      pos += dur/4;
      return ((dur/2)-(pos%(dur/2))) % (dur/2);
      }


//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

Voice::Voice(Fluid* f)
      {
      _fluid  = f;
      status  = FLUID_VOICE_OFF;
      chan    = NO_CHANNEL;
      key     = 0;
      vel     = 0;
      channel = 0;
      sample  = 0;

      /* The 'sustain' and 'finished' segments of the volume / modulation
       * envelope are constant. They are never affected by any modulator
       * or generator. Therefore it is enough to initialize them once
       * during the lifetime of the synth.
       */
      volenv_data[FLUID_VOICE_ENVSUSTAIN].count = 0xffffffff;
      volenv_data[FLUID_VOICE_ENVSUSTAIN].coeff = 1.0f;
      volenv_data[FLUID_VOICE_ENVSUSTAIN].incr  = 0.0f;
      volenv_data[FLUID_VOICE_ENVSUSTAIN].min   = -1.0f;
      volenv_data[FLUID_VOICE_ENVSUSTAIN].max   = 2.0f;

      volenv_data[FLUID_VOICE_ENVFINISHED].count = 0xffffffff;
      volenv_data[FLUID_VOICE_ENVFINISHED].coeff = 0.0f;
      volenv_data[FLUID_VOICE_ENVFINISHED].incr  = 0.0f;
      volenv_data[FLUID_VOICE_ENVFINISHED].min   = -1.0f;
      volenv_data[FLUID_VOICE_ENVFINISHED].max   = 1.0f;

      modenv_data[FLUID_VOICE_ENVSUSTAIN].count = 0xffffffff;
      modenv_data[FLUID_VOICE_ENVSUSTAIN].coeff = 1.0f;
      modenv_data[FLUID_VOICE_ENVSUSTAIN].incr  = 0.0f;
      modenv_data[FLUID_VOICE_ENVSUSTAIN].min   = -1.0f;
      modenv_data[FLUID_VOICE_ENVSUSTAIN].max   = 2.0f;

      modenv_data[FLUID_VOICE_ENVFINISHED].count = 0xffffffff;
      modenv_data[FLUID_VOICE_ENVFINISHED].coeff = 0.0f;
      modenv_data[FLUID_VOICE_ENVFINISHED].incr  = 0.0f;
      modenv_data[FLUID_VOICE_ENVFINISHED].min   = -1.0f;
      modenv_data[FLUID_VOICE_ENVFINISHED].max   = 1.0f;
      }

//---------------------------------------------------------
//   init
//    Initialize the synthesis process
//---------------------------------------------------------

void Voice::init(Sample* _sample, Channel* _channel, int _key, int _vel,
   unsigned int _id, double tuning)
      {
      // Note: The voice parameters will be initialized later, when the
      // generators have been retrieved from the sound font. Here, only
      // the 'working memory' of the voice (position in envelopes, history
      // of IIR filters, position in sample etc) is initialized.

      id             = _id;
      _noteTuning    = tuning;
      chan           = _channel->getNum();
      key            = _key;
      vel            = _vel;
      channel        = _channel;
      mod_count      = 0;
      sample         = _sample;
      ticks          = 0;
      debug          = 0;
      has_looped     = false; // Will be set during voice_write when the 2nd loop point is reached
      last_fres      = -1;    // The filter coefficients have to be calculated later in the DSP loop.
      filter_startup = 1;     // Set the filter immediately, don't fade between old and new settings
      interp_method  = _channel->getInterpMethod();

      // vol env initialization
      volenv_count   = 0;
      volenv_section = 0;
      volenv_val     = 0.0f;
      amp            = 0.0f;  // The last value of the volume envelope, used to
                              // calculate the volume increment during
                              // processing

      // mod env initialization
      modenv_count   = 0;
      modenv_section = 0;
      modenv_val     = 0.0f;

      /* mod lfo */
      modlfo_val = 0.0;       // Fixme: Retrieve from any other existing
                              // voice on this channel to keep LFOs in
                              // unison?
      modlfo_pos = 0;

      /* vib lfo */
      viblfo_val = 0.0f;      // Fixme: See mod lfo

      /* Clear sample history in filter */
      hist1 = 0;
      hist2 = 0;

      /* Set all the generators to their default value, according to SF
       * 2.01 section 8.1.3 (page 48). The value of NRPN messages are
       * copied from the channel to the voice's generators. The sound font
       * loader overwrites them. The generator values are later converted
       * into voice parameters in calculate_runtime_synthesis_parameters.
       */
      fluid_gen_init(&gen[0], channel);

     /* For a looped sample, this value will be overwritten as soon as the
      * loop parameters are initialized (they may depend on modulators).
      * This value can be kept, it is a worst-case estimate.
      */

      amplitude_that_reaches_noise_floor_nonloop = FLUID_NOISE_FLOOR;
      amplitude_that_reaches_noise_floor_loop    = FLUID_NOISE_FLOOR;
      }

//---------------------------------------------------------
//   gen_set
//---------------------------------------------------------

void Voice::gen_set(int i, float val)
      {
      gen[i].val   = val;
      gen[i].flags = GEN_SET;
      }

//---------------------------------------------------------
//   gen_incr
//---------------------------------------------------------

void Voice::gen_incr(int i, float val)
      {
      gen[i].val += val;
      gen[i].flags = GEN_SET;
      }

//---------------------------------------------------------
//   gen_get
//---------------------------------------------------------

float Voice::gen_get(int g)
      {
      return gen[g].val;
      }

inline void Voice::calcVolEnv(int n, fluid_env_data_t *env_data)
      {
      float x;
      /* calculate the envelope value and check for valid range */
      x = env_data->coeff * volenv_val + env_data->incr * n;
      if (x < env_data->min)
            x = env_data->min;
      else if (x > env_data->max)
            x = env_data->max;
      volenv_val = x;
      }
      
std::tuple<unsigned, bool> Voice::interpolateGeneratedDSPData(unsigned n)
      {
            dsp_buf.resize(n, 0.0f);
            std::fill(dsp_buf.begin(), dsp_buf.end(), 0.0f);
            unsigned generatedFrames = 0;
            switch (interp_method) {
                  case FLUID_INTERP_NONE:
                        generatedFrames = dsp_float_interpolate_none(n);
                        break;
                  case FLUID_INTERP_LINEAR:
                        generatedFrames = dsp_float_interpolate_linear(n);
                        break;
                  case FLUID_INTERP_4THORDER:
                  default:
                        generatedFrames = dsp_float_interpolate_4th_order(n);
                        break;
                  case FLUID_INTERP_7THORDER:
                        generatedFrames = dsp_float_interpolate_7th_order(n);
                        break;
                  }
            
            /* turn off voice if short count (sample ended and not looping) or voice reached noise floor*/
            if (generatedFrames < n || positionToTurnOff > 0)
                  off();
            
            bool needToRunBuffFilling = generatedFrames > 0;
            return std::make_tuple(generatedFrames, needToRunBuffFilling);
      }
      

bool Voice::generateDataForDSPChain(unsigned framesBufCount)
      {
            /* Range checking for sample- and loop-related parameters
             * Initial phase is calculated here*/
            check_sample_sanity();
            
            /******************* vol env **********************/
            
            fluid_env_data_t* env_data = &volenv_data[volenv_section];
            Sample2AmpInc.clear();
            std::map<int, int> sample2VolEnvSection;
            std::set<int> volumeChanges;
            
            if (volenv_section >= FLUID_VOICE_ENVFINISHED) {
                  off();
                  return false;
            }
            
            // determine points where volume envelope changes
            unsigned curVolEnvCount = volenv_count;
            unsigned restN = framesBufCount;
            while (curVolEnvCount + restN >= env_data->count) {
                  restN -= env_data->count - curVolEnvCount;
                  
                  sample2VolEnvSection.insert(std::pair<int, int>(framesBufCount - restN, volenv_section));
                  volumeChanges.insert(framesBufCount-restN);
                  
                  curVolEnvCount = 0;
                  volenv_section++;
                  
                  env_data = &volenv_data[volenv_section];
                  }
            
            sample2VolEnvSection.insert(std::pair<int, int>(framesBufCount, volenv_section));
            volumeChanges.insert(framesBufCount);
            
            fluid_check_fpe ("voice_write vol env");
            
            /******************* mod env **********************/
            
            /* Skip to decay phase if delay and attack envelope sections each are
             * less than 100 samples long. This avoids popping noises due to the
             * mod envelope being out-of-sync with the sample-based volume envelope. */
            if (modenv_section < 2 && modenv_data[FLUID_VOICE_ENVDELAY].count < 100 && modenv_data[FLUID_VOICE_ENVATTACK].count < 100) {
                  modenv_section = 2;
                  modenv_val     = 1.0f;
                  }
            
            env_data = &modenv_data[modenv_section];
            
            /* skip to the next section of the envelope if necessary */
            while (modenv_count >= env_data->count) {
                  env_data = &modenv_data[++modenv_section];
                  modenv_count = 0;
                  }
            
            /* calculate the envelope value and check for valid range */
            float x = env_data->coeff * modenv_val + env_data->incr * framesBufCount;
            
            if (x < env_data->min) {
                  x = env_data->min;
                  modenv_section++;
                  modenv_count = 0;
                  }
            else if (x > env_data->max) {
                  x = env_data->max;
                  modenv_section++;
                  modenv_count = 0;
                  }
            
            modenv_val = x;
            modenv_count += framesBufCount;
            fluid_check_fpe ("voice_write mod env");
            
            /******************* mod lfo **********************/
            // calculate all points where we need to consider
            // the mod lfo (where it changes its slope)
            
            int modLfoStart = -1;
            
            if (fabs(modlfo_to_vol) > 0) {
                  if (ticks >= modlfo_delay)
                        modLfoStart = 0;
                  else if (framesBufCount >= modlfo_delay)
                        modLfoStart = modlfo_delay;
                  
                  if (modLfoStart >= 0) {
                        if (modLfoStart > 0)
                              volumeChanges.insert(modLfoStart);
                        
                        unsigned int modLfoNextTurn = samplesToNextTurningPoint(modlfo_dur, modlfo_pos);
                        
                        while (modLfoNextTurn+modLfoStart < framesBufCount) {
                              volumeChanges.insert(modLfoNextTurn+modLfoStart);
                              modLfoNextTurn++;
                              modLfoNextTurn += samplesToNextTurningPoint(modlfo_dur, modLfoNextTurn);
                              }
                        }
                  }
            
            fluid_check_fpe ("voice_write mod LFO");
            
            /******************* vib lfo **********************/
            
            if (ticks >= viblfo_delay) {
                  viblfo_val += viblfo_incr * framesBufCount;
                  
                  if (viblfo_val > (float) 1.0) {
                        viblfo_incr = -viblfo_incr;
                        viblfo_val = (float) 2.0 - viblfo_val;
                        }
                  else if (viblfo_val < -1.0) {
                        viblfo_incr = -viblfo_incr;
                        viblfo_val = (float) -2.0 - viblfo_val;
                        }
                  }
            
            fluid_check_fpe ("voice_write Vib LFO");
            
            /******************* amplitude **********************/
            
            if (volenv_section == FLUID_VOICE_ENVDELAY) {
                  return false;     /* The volume amplitude is in hold phase. No sound is produced. */
                  }
            
            qreal oldTargetAmp = amp;
            int lastPos = 0;
            auto oldVolEnvSection = sample2VolEnvSection.begin();
            auto curVolEnvSection = oldVolEnvSection;
            
            for (auto curPos : volumeChanges)
            {
                  if (modLfoStart >= 0 && curPos >= modLfoStart)
                        modlfo_val = triangle(modlfo_dur, modlfo_pos+curPos-modLfoStart);
                  else
                        modlfo_val = 0;
                  
                  // never calculate anything for the very first sample
                  // everything should have been calculated in the last
                  // cycle - it would also cause a divion by zero later
                  if (curPos == 0) {
                        curPos = 1;
                        
                        // if we should calculate for position 1 already make sure we don't do it twice
                        // could lead to curPos==lastPos which causes devision by zero
                        if (volumeChanges.find(1) != volumeChanges.end())
                              volumeChanges.erase(volumeChanges.find(1));
                        }
                  
                  // just go to the next volume section if we're below last volume point
                  if (curPos >= curVolEnvSection->first && (unsigned int) curVolEnvSection->first < framesBufCount)
                        curVolEnvSection++;
                  
                  volenv_count += curPos-lastPos;
                  calcVolEnv(curPos-lastPos, &volenv_data[oldVolEnvSection->second]);
                  
                  volenv_section = oldVolEnvSection->second;
                  
                  qreal target_amp {0.0};    /* target amplitude */
                  if (volenv_section <= FLUID_VOICE_ENVATTACK) {
                        /* the envelope is in the attack section: ramp linearly to max value.
                         * A positive modlfo_to_vol should increase volume (negative attenuation).
                         */
                        target_amp = fluid_atten2amp (attenuation)
                        * fluid_cb2amp (modlfo_val * -modlfo_to_vol)
                        * volenv_val;
                        }
                  else {
                        //float amplitude_that_reaches_noise_floor;
                        //float amp_max;
                        
                        target_amp = fluid_atten2amp (attenuation)
                        * fluid_cb2amp (960.0f * (1.0f - volenv_val)
                                        + modlfo_val * -modlfo_to_vol);
                        
                        /* A voice can be turned off, when an estimate for the volume
                         * (upper bound) falls below that volume, that will drop the
                         * sample below the noise floor.
                         */
                        
                        /* If the loop amplitude is known, we can use it if the voice loop is within
                         * the sample loop
                         */
                        
                        float amplitude_that_reaches_noise_floor;
                        /* Is the playing pointer already in the loop? */
                        if (has_looped)
                              amplitude_that_reaches_noise_floor = amplitude_that_reaches_noise_floor_loop;
                        else
                              amplitude_that_reaches_noise_floor = amplitude_that_reaches_noise_floor_nonloop;
                        
                        /* voice->attenuation_min is a lower boundary for the attenuation
                         * now and in the future (possibly 0 in the worst case).  Now the
                         * amplitude of sample and volenv cannot exceed amp_max (since
                         * volenv_val can only drop):
                         */
                        
                        float amp_max = fluid_atten2amp (min_attenuation_cB) * volenv_val;
                        
                        /* And if amp_max is already smaller than the known amplitude,
                         * which will attenuate the sample below the noise floor, then we
                         * can safely turn off the voice. Duh. */
                        if (amp_max < amplitude_that_reaches_noise_floor) {
                              positionToTurnOff = curPos;
                              }
                        
                        }
                  
                  if (curVolEnvSection->second != oldVolEnvSection->second) {
                        if (oldVolEnvSection->second == FLUID_VOICE_ENVDECAY) {
                              env_data = &volenv_data[oldVolEnvSection->second];
                              volenv_val = env_data->min * env_data->coeff;
                              }
                        volenv_count = 0;
                        oldVolEnvSection = curVolEnvSection;
                        }
                  /* Volume increment to go from voice->amp to target_amp in FLUID_BUFSIZE steps */
                  amp_incr = (target_amp - oldTargetAmp) / (curPos - lastPos);
                  lastPos = curPos;
                  Sample2AmpInc.insert(std::pair<int, qreal>(curPos, amp_incr));
                  
                  // if voice is turned off after this no need to calculate any more values
                  if (positionToTurnOff > 0)
                        break;
                  
                  oldTargetAmp = target_amp;
            }
            
            if (modLfoStart >= 0) {
                  modlfo_pos += framesBufCount - modLfoStart;
                  modlfo_val = triangle(modlfo_dur, modlfo_pos-modLfoStart);
            }
            
            fluid_check_fpe ("voice_write amplitude calculation");
            
            /* Calculate the number of samples, that the DSP loop advances
             * through the original waveform with each step in the output
             * buffer. It is the ratio between the frequencies of original
             * waveform and output waveform.*/
            
            {
                  float cent = pitch + modlfo_val * modlfo_to_pitch
                  + viblfo_val * viblfo_to_pitch
                  + modenv_val * modenv_to_pitch;
                  phase_incr = _fluid->ct2hz_real(cent) / root_pitch_hz;
            }
            
            /* if phase_incr is not advancing, set it to the minimum fraction value (prevent stuckage) */
            if (phase_incr == 0)
                  phase_incr = 1;
            
            /*************** resonant filter ******************/
            
            /* calculate the frequency of the resonant filter in Hz */
            float _fres = _fluid->ct2hz_real(fres
                                       + modlfo_val * modlfo_to_fc
                                       + modenv_val * modenv_to_fc);
            
            /* FIXME - Still potential for a click during turn on, can we interpolate
             between 20khz cutoff and 0 Q? */
            
            /* I removed the optimization of turning the filter off when the
             * resonance frequence is above the maximum frequency. Instead, the
             * filter frequency is set to a maximum of 0.45 times the sampling
             * rate. For a 44100 kHz sampling rate, this amounts to 19845
             * Hz. The reason is that there were problems with anti-aliasing when the
             * synthesizer was run at lower sampling rates. Thanks to Stephan
             * Tassart for pointing me to this bug. By turning the filter on and
             * clipping the maximum filter frequency at 0.45*srate, the filter
             * is used as an anti-aliasing filter. */
            
            if (_fres > 0.45f * _fluid->sample_rate)
                  _fres = 0.45f * _fluid->sample_rate;
            else if (_fres < 5)
                  _fres = 5;
            
            /* if filter enabled and there is a significant frequency change.. */
            if ((qAbs(_fres - last_fres) > 0.01)) {
                  /* The filter coefficients have to be recalculated (filter
                   * parameters have changed). Recalculation for various reasons is
                   * forced by setting last_fres to -1.  The flag filter_startup
                   * indicates, that the DSP loop runs for the first time, in this
                   * case, the filter is set directly, instead of smoothly fading
                   * between old and new settings.
                   *
                   * Those equations from Robert Bristow-Johnson's `Cookbook
                   * formulae for audio EQ biquad filter coefficients', obtained
                   * from Harmony-central.com / Computer / Programming. They are
                   * the result of the bilinear transform on an analogue filter
                   * prototype. To quote, `BLT frequency warping has been taken
                   * into account for both significant frequency relocation and for
                   * bandwidth readjustment'. */
                  
                  float omega = (float) (2.0 * M_PI * (_fres / ((float) _fluid->sample_rate)));
                  float sin_coeff = (float) sin(omega);
                  float cos_coeff = (float) cos(omega);
                  float alpha_coeff = sin_coeff / (2.0f * q_lin);
                  float a0_inv = 1.0f / (1.0f + alpha_coeff);
                  
                  /* Calculate the filter coefficients. All coefficients are
                   * normalized by a0. Think of `a1' as `a1/a0'.
                   *
                   * Here a couple of multiplications are saved by reusing common expressions.
                   * The original equations should be:
                   *  b0=(1.-cos_coeff)*a0_inv*0.5*voice->filter_gain;
                   *  b1=(1.-cos_coeff)*a0_inv*voice->filter_gain;
                   *  b2=(1.-cos_coeff)*a0_inv*0.5*voice->filter_gain; */
                  
                  float a1_temp = -2.0f * cos_coeff * a0_inv;
                  float a2_temp = (1.0f - alpha_coeff) * a0_inv;
                  float b1_temp = (1.0f - cos_coeff) * a0_inv * filter_gain;
                  /* both b0 -and- b2 */
                  float b02_temp = b1_temp * 0.5f;
                  
                  if (filter_startup) {
                        /* The filter is calculated, because the voice was started up.
                         * In this case set the filter coefficients without delay.
                         */
                        a1 = a1_temp;
                        a2 = a2_temp;
                        b02 = b02_temp;
                        b1 = b1_temp;
                        filter_coeff_incr_count = 0;
                        filter_startup = 0;
                        //       printf("Setting initial filter coefficients.\n");
                        }
                  else {
                        /* The filter frequency is changed.  Calculate an increment
                         * factor, so that the new setting is reached after one buffer
                         * length. x_incr is added to the current value FLUID_BUFSIZE
                         * times. The length is arbitrarily chosen. Longer than one
                         * buffer will sacrifice some performance, though.  Note: If
                         * the filter is still too 'grainy', then increase this number
                         * at will.
                         */
                        
#define FILTER_TRANSITION_SAMPLES 64     // (FLUID_BUFSIZE)
                        
                        a1_incr = (a1_temp - a1) / FILTER_TRANSITION_SAMPLES;
                        a2_incr = (a2_temp - a2) / FILTER_TRANSITION_SAMPLES;
                        b02_incr = (b02_temp - b02) / FILTER_TRANSITION_SAMPLES;
                        b1_incr = (b1_temp - b1) / FILTER_TRANSITION_SAMPLES;
                        /* Have to add the increments filter_coeff_incr_count times. */
                        filter_coeff_incr_count = FILTER_TRANSITION_SAMPLES;
                        }
                  last_fres = _fres;
                  fluid_check_fpe ("voice_write filter calculation");
                  }
            
            
            fluid_check_fpe ("voice_write DSP coefficients");
            return true;
      }
      
static float* shiftBufferPosition(unsigned shift, float* buff)
      {
      float* resBuffPosition = buff;
      for (unsigned i = 0; i < shift; ++i) {
            ++resBuffPosition; //left channel
            ++resBuffPosition; //right channel
            }
      return resBuffPosition;
      }
      
//-----------------------------------------------------------------------------
// write
//
// This is where it all happens. This function is called by the
// synthesizer to generate the sound samples. The synthesizer passes
// four audio buffers: left, right, reverb out, and chorus out.
//
// The biggest part of this function sets the correct values for all
// the dsp parameters (all the control data boil down to only a few
// dsp parameters). The dsp routine is #included in several places (fluid_dsp_core.c).
//-----------------------------------------------------------------------------
      
void Voice::write(unsigned n, float* out, float* reverb, float* chorus)
      {
      /* make sure we're playing and that we have sample data */
      if (!PLAYING())
            return;
      if (!sample) {
            printf("!sample\n");
            off();
            return;
            }
            
      /*
       /  ------- CACHING ALGORITHM -------
       /  If cached frames exist and number of required frames is less than cache size
       /          apply effects to the required number of frames from cache, put cache to buffer
       /  Else
       /          put all cached frames to buffer
       /          (buffer* is a buffer after putting left cache data)
       /          generate DSP data and interpolation for buffer* size data or required buffer size if buffer* is too small
       /          fill cache data with raw buffer* data
       /          apply effects to cache and put cache data to raw buffer*
       */
      if (n <= _cachedFrames) {
            effects(_initialCacheFrames - _cachedFrames, n, out, reverb, chorus);
            _cachedFrames -= n;
            }
      else {
            const unsigned leftBufferFramesToFill = n - _cachedFrames; //must be called before setting null to _cachedFrames
            unsigned buffShiftAfterApplyingCache = 0;
            if (_cachedFrames > 0) {
                  buffShiftAfterApplyingCache = _cachedFrames;
                  effects(_initialCacheFrames - _cachedFrames, _cachedFrames, out, reverb, chorus);
                  _cachedFrames = 0;
                  }
            
            const unsigned requiredNumberOfFramesToGenerateEnvelope = FLUID_VOICE_ENVLAST * volenv_data[FLUID_VOICE_ENVDELAY].count;
            const unsigned framesToGenerateData = std::max(leftBufferFramesToFill, requiredNumberOfFramesToGenerateEnvelope);
            if (generateDataForDSPChain(framesToGenerateData)) {
                  auto interpolationRes = interpolateGeneratedDSPData(framesToGenerateData);
                  if (std::get<1>(interpolationRes)) {
                        _initialCacheFrames = std::get<0>(interpolationRes);
                        float* shiftedOut = shiftBufferPosition(buffShiftAfterApplyingCache, out);
                        float* shiftedReverb = shiftBufferPosition(buffShiftAfterApplyingCache, reverb);
                        float* shiftedChorus = shiftBufferPosition(buffShiftAfterApplyingCache, chorus);
                        effects(0, leftBufferFramesToFill, shiftedOut, shiftedReverb, shiftedChorus);
                        _cachedFrames = _initialCacheFrames;
                        _cachedFrames -= std::min(leftBufferFramesToFill, _cachedFrames); //to keep positive
                        }
                  }
            }

      ticks += n;
      }


//---------------------------------------------------------
//   voice_start
//---------------------------------------------------------

void Voice::voice_start()
      {
      /* The maximum volume of the loop is calculated and cached once for each
       * sample with its nominal loop settings. This happens, when the sample is used
       * for the first time.*/

      /*
       * in this function we calculate the values of all the parameters. the
       * parameters are converted to their most useful unit for the DSP
       * algorithm, for example, number of samples instead of
       * timecents. Some parameters keep their "perceptual" unit and
       * conversion will be done in the DSP function. This is the case, for
       * example, for the pitch since it is modulated by the controllers in
       * cents. */

      static const int list_of_generators_to_initialize[35] = {
            GEN_STARTADDROFS,                    // SF2.01 page 48 #0
            GEN_ENDADDROFS,                      //                #1
            GEN_STARTLOOPADDROFS,                //                #2
            GEN_ENDLOOPADDROFS,                  //                #3
            // GEN_STARTADDRCOARSEOFS see comment below [1]        #4
            GEN_MODLFOTOPITCH,                   //                #5
            GEN_VIBLFOTOPITCH,                   //                #6
            GEN_MODENVTOPITCH,                   //                #7
            GEN_FILTERFC,                        //                #8
            GEN_FILTERQ,                         //                #9
            GEN_MODLFOTOFILTERFC,                //                #10
            GEN_MODENVTOFILTERFC,                //                #11
            // GEN_ENDADDRCOARSEOFS [1]                            #12
            GEN_MODLFOTOVOL,                     //                #13
            // not defined                                         #14
            GEN_CHORUSSEND,                      //                #15
            GEN_REVERBSEND,                      //                #16
            GEN_PAN,                             //                #17
            // not defined                                         #18
            // not defined                                         #19
            // not defined                                         #20
            GEN_MODLFODELAY,                     //                #21
            GEN_MODLFOFREQ,                      //                #22
            GEN_VIBLFODELAY,                     //                #23
            GEN_VIBLFOFREQ,                      //                #24
            GEN_MODENVDELAY,                     //                #25
            GEN_MODENVATTACK,                    //                #26
            GEN_MODENVHOLD,                      //                #27
            GEN_MODENVDECAY,                     //                #28
            // GEN_MODENVSUSTAIN [1]                               #29
            GEN_MODENVRELEASE,                   //                #30
            // GEN_KEYTOMODENVHOLD [1]                             #31
            // GEN_KEYTOMODENVDECAY [1]                            #32
            GEN_VOLENVDELAY,                     //                #33
            GEN_VOLENVATTACK,                    //                #34
            GEN_VOLENVHOLD,                      //                #35
            GEN_VOLENVDECAY,                     //                #36
            // GEN_VOLENVSUSTAIN [1]                               #37
            GEN_VOLENVRELEASE,                   //                #38
            // GEN_KEYTOVOLENVHOLD [1]                             #39
            // GEN_KEYTOVOLENVDECAY [1]                            #40
            // GEN_STARTLOOPADDRCOARSEOFS [1]                      #45
            GEN_KEYNUM,                          //                #46
            GEN_VELOCITY,                        //                #47
            GEN_ATTENUATION,                     //                #48
            // GEN_ENDLOOPADDRCOARSEOFS [1]                        #50
            // GEN_COARSETUNE           [1]                        #51
            // GEN_FINETUNE             [1]                        #52
            GEN_OVERRIDEROOTKEY,                 //                #58
            GEN_PITCH,                           //                ---
            -1                                   // end-of-list marker
            };

     /* When the voice is made ready for the synthesis process, a lot of
      * voice-internal parameters have to be calculated.
      *
      * At this point, the sound font has already set the -nominal- value
      * for all generators (excluding GEN_PITCH). Most generators can be
      * modulated - they include a nominal value and an offset (which
      * changes with velocity, note number, channel parameters like
      * aftertouch, mod wheel...) Now this offset will be calculated as
      * follows:
      *
      *  - Process each modulator once.
      *  - Calculate its output value.
      *  - Find the target generator.
      *  - Add the output value to the modulation value of the generator.
      *
      * Note: The generators have been initialized with
      * fluid_gen_set_default_values.
      */

      for (int i = 0; i < mod_count; i++) {
            Mod* m              = &mod[i];
            float modval        = m->get_value(channel, this);
            int dest_gen_index  = m->dest;
            Generator* dest_gen = &gen[dest_gen_index];
            dest_gen->mod      += modval;
            }

     /* Now the generators are initialized, nominal and modulation value.
      * The voice parameters (which depend on generators) are calculated
      * with update_param. Processing the list of generator
      * changes will calculate each voice parameter once.
      *
      * Note [1]: Some voice parameters depend on several generators. For
      * example, the pitch depends on GEN_COARSETUNE, GEN_FINETUNE and
      * GEN_PITCH.  voice->pitch.  Unnecessary recalculation is avoided
      * by removing all but one generator from the list of voice
      * parameters.  Same with GEN_XXX and GEN_XXXCOARSE: the
      * initialisation list contains only GEN_XXX.
      */

      /* Calculate the voice parameter(s) dependent on each generator. */
      for (int i = 0; list_of_generators_to_initialize[i] != -1; i++)
            update_param(list_of_generators_to_initialize[i]);

      /* Make an estimate on how loud this voice can get at any time (attenuation). */
      min_attenuation_cB = get_lower_boundary_for_attenuation();

//      qDebug("DELAY (%d) %d", FLUID_VOICE_ENVDELAY,volenv_data[FLUID_VOICE_ENVDELAY].count);
//      qDebug("ATTACK (%d) %d", FLUID_VOICE_ENVATTACK,volenv_data[FLUID_VOICE_ENVATTACK].count);
//      qDebug("HOLD (%d) %d", FLUID_VOICE_ENVHOLD, volenv_data[FLUID_VOICE_ENVHOLD].count);
//      qDebug("DECAY (%d) %d", FLUID_VOICE_ENVDECAY, volenv_data[FLUID_VOICE_ENVDECAY].count);
//      qDebug("SUSTAIN (%d) %d", FLUID_VOICE_ENVSUSTAIN, volenv_data[FLUID_VOICE_ENVSUSTAIN].count);
//      qDebug("RELEASE (%d) %d", FLUID_VOICE_ENVRELEASE, volenv_data[FLUID_VOICE_ENVRELEASE].count);

      /* Force setting of the phase at the first DSP loop run
       * This cannot be done earlier, because it depends on modulators.
       */
      check_sample_sanity_flag = FLUID_SAMPLESANITY_STARTUP;
      positionToTurnOff = -1;

      status = FLUID_VOICE_ON;
      }

//---------------------------------------------------------
//   fluid_voice_calculate_gen_pitch
//---------------------------------------------------------

void Voice::calculate_gen_pitch()
      {
      float x;
     /* The GEN_PITCH is a hack to fit the pitch bend controller into the
      * modulator paradigm.  Now the nominal pitch of the key is set.
      * Note about SCALETUNE: SF2.01 8.1.3 says, that this generator is a
      * non-realtime parameter. So we don't allow modulation (as opposed
      * to _GEN(voice, GEN_SCALETUNE) When the scale tuning is varied,
      * one key remains fixed. Here C3 (MIDI number 60) is used.
      */
      //if (channel->tuning != 0) {
            /* pitch(scalekey) + scale * (pitch(key) - pitch(scalekey)) */
      x = _fluid->getPitch((int)(root_pitch / 100.0f));
      gen[GEN_PITCH].val = _noteTuning + (x + (gen[GEN_SCALETUNE].val / 100.0f * (_fluid->getPitch(key) - x)));
      //      }
      //else {
      //      gen[GEN_PITCH].val = _noteTuning + (gen[GEN_SCALETUNE].val * (key - root_pitch / 100.0f) + root_pitch);
      //      }
      }

/*
 * calculate_hold_decay_frames
 */
int Voice::calculate_hold_decay_frames(int gen_base, int gen_key2base, int is_decay)
      {
      /* Purpose:
       *
       * Returns the number of DSP loops, that correspond to the hold
       * (is_decay=0) or decay (is_decay=1) time.
       * gen_base=GEN_VOLENVHOLD, GEN_VOLENVDECAY, GEN_MODENVHOLD,
       * GEN_MODENVDECAY gen_key2base=GEN_KEYTOVOLENVHOLD,
       * GEN_KEYTOVOLENVDECAY, GEN_KEYTOMODENVHOLD, GEN_KEYTOMODENVDECAY
       *
       * SF2.01 section 8.4.3 # 31, 32, 39, 40
       * GEN_KEYTOxxxENVxxx uses key 60 as 'origin'.
       * The unit of the generator is timecents per key number.
       * If KEYTOxxxENVxxx is 100, a key one octave over key 60 (72)
       * will cause (60-72)*100=-1200 timecents of time variation.
       * The time is cut in half.
       */
      float timecents = (GEN(gen_base) + GEN(gen_key2base) * (60.0 - key));

      /* Range checking */
      if (is_decay){
            /* SF 2.01 section 8.1.3 # 28, 36 */
            if (timecents > 8000.0)
                  timecents = 8000.0;
            }
      else {
            /* SF 2.01 section 8.1.3 # 27, 35 */
            if (timecents > 5000)
                  timecents = 5000.0;
            /* SF 2.01 section 8.1.2 # 27, 35:
             * The most negative number indicates no hold time
             */
            if (timecents <= -32768.)
                  return 0;
            }
      /* SF 2.01 section 8.1.3 # 27, 28, 35, 36 */
      if (timecents < -12000.0)
            timecents = -12000.0;

      float seconds = fluid_tc2sec(timecents);
      return (int)((float)_fluid->sample_rate * seconds);
      }

/*
 * update_param
 *
 * Purpose:
 *
 * The value of a generator (gen) has changed.  (The different
 * generators are listed in fluid.h, or in SF2.01 page 48-49)
 * Now the dependent 'voice' parameters are calculated.
 *
 * fluid_voice_update_param can be called during the setup of the
 * voice (to calculate the initial value for a voice parameter), or
 * during its operation (a generator has been changed due to
 * real-time parameter modifications like pitch-bend).
 *
 * Note: The generator holds three values: The base value .val, an
 * offset caused by modulators .mod, and an offset caused by the
 * NRPN system. _GEN(voice, generator_enumerator) returns the sum
 * of all three.
 */
void Voice::update_param(int _gen)
      {
      double q_dB;
      float x;
      float y;
      unsigned int count;
      // Alternate attenuation scale used by EMU10K1 cards when setting the attenuation at the preset or instrument level within the SoundFont bank.
      static const float ALT_ATTENUATION_SCALE = 0.4f;

      double gain = 1.0 / 32768.0f;
      switch (_gen) {
            case GEN_PAN:
                  /* range checking is done in the fluid_pan function */
                  pan       = GEN(GEN_PAN);
                  amp_left  = fluid_pan(pan, 1) * gain;
                  amp_right = fluid_pan(pan, 0) * gain;
                  break;

            case GEN_ATTENUATION:
                  attenuation = gen[GEN_ATTENUATION].val * ALT_ATTENUATION_SCALE + gen[GEN_ATTENUATION].mod + gen[GEN_ATTENUATION].nrpn;

                  /* Range: SF2.01 section 8.1.3 # 48
                   * Motivation for range checking:
                   * OHPiano.SF2 sets initial attenuation to a whooping -96 dB
                   */
                  attenuation = qBound(0.0f, attenuation, 1440.0f);
                  break;

      /* The pitch is calculated from three different generators.
       * Read comment in fluid.h about GEN_PITCH.
       */
            case GEN_PITCH:
            case GEN_COARSETUNE:
            case GEN_FINETUNE:
                  /* The testing for allowed range is done in 'fluid_ct2hz' */
                  pitch = GEN(GEN_PITCH) + 100.0f * GEN(GEN_COARSETUNE) + GEN(GEN_FINETUNE);
                  break;

            case GEN_REVERBSEND:
                  /* The generator unit is 'tenths of a percent'. */
                  // reverb_send = GEN(GEN_REVERBSEND) / 1000.0f;
                  reverb_send = float(channel->cc[EFFECTS_DEPTH1]) / 128.0;
                  // fluid_clip(reverb_send, 0.0, 1.0);
                  amp_reverb = reverb_send * gain;
                  break;

            case GEN_CHORUSSEND:
                  /* The generator unit is 'tenths of a percent'. */
                  chorus_send = GEN(GEN_CHORUSSEND) / 1000.0f;
                  fluid_clip(chorus_send, 0.0, 1.0);
                  amp_chorus = chorus_send * gain;
                  break;

            case GEN_OVERRIDEROOTKEY:
                  /* This is a non-realtime parameter. Therefore the .mod part of the generator
                   * can be neglected.
                   * NOTE: origpitch sets MIDI root note while pitchadj is a fine tuning amount
                   * which offsets the original rate.  This means that the fine tuning is
                   * inverted with respect to the root note (so subtract it, not add).
                   */
                  if (sample != 0) {
                        if (gen[GEN_OVERRIDEROOTKEY].val > -1) {   //FIXME: use flag instead of -1
                              root_pitch = gen[GEN_OVERRIDEROOTKEY].val * 100.0f - sample->pitchadj;
                              }
                        else {
                              root_pitch = sample->origpitch * 100.0f - sample->pitchadj;
                              }
                        root_pitch_hz = _fluid->ct2hz(root_pitch);
                        root_pitch_hz *= (float) _fluid->sample_rate / sample->samplerate;
                        /* voice pitch depends on voice root_pitch, so calculate voice pitch now */
                        calculate_gen_pitch();
                        }
                  break;

            case GEN_FILTERFC:
                  /* The resonance frequency is converted from absolute cents to
                   * midicents .val and .mod are both used, this permits real-time
                   * modulation.  The allowed range is tested in the 'fluid_ct2hz'
                   * function [PH,20021214]
                   */
                  fres = GEN(GEN_FILTERFC);

                  /* The synthesis loop will have to recalculate the filter
                   * coefficients. */
                  last_fres = -1.0f;
                  break;

            case GEN_FILTERQ:
                  /* The generator contains 'centibels' (1/10 dB) => divide by 10 to
                   * obtain dB
                   */
                  q_dB = GEN(GEN_FILTERQ) / 10.0f;

                  /* Range: SF2.01 section 8.1.3 # 8 (convert from cB to dB => /10) */
                  fluid_clip(q_dB, 0.0f, 96.0f);

                  /* Short version: Modify the Q definition in a way, that a Q of 0
                   * dB leads to no resonance hump in the freq. response.
                   *
                   * Long version: From SF2.01, page 39, item 9 (initialFilterQ):
                   * "The gain at the cutoff frequency may be less than zero when
                   * zero is specified".  Assume q_dB=0 / q_lin=1: If we would leave
                   * q as it is, then this results in a 3 dB hump slightly below
                   * fc. At fc, the gain is exactly the DC gain (0 dB).  What is
                   * (probably) meant here is that the filter does not show a
                   * resonance hump for q_dB=0. In this case, the corresponding
                   * q_lin is 1/sqrt(2)=0.707.  The filter should have 3 dB of
                   * attenuation at fc now.  In this case Q_dB is the height of the
                   * resonance peak not over the DC gain, but over the frequency
                   * response of a non-resonant filter.  This idea is implemented as
                   * follows:
                   */
                  q_dB -= 3.01f;

                  /* The 'sound font' Q is defined in dB. The filter needs a linear
                     q. Convert.
                   */
                  q_lin = (float) (pow(10.0f, q_dB / 20.0f));

                  /* SF 2.01 page 59:
                   *
                   *  The SoundFont specs ask for a gain reduction equal to half the
                   *  height of the resonance peak (Q).  For example, for a 10 dB
                   *  resonance peak, the gain is reduced by 5 dB.  This is done by
                   *  multiplying the total gain with sqrt(1/Q).  `Sqrt' divides dB
                   *  by 2 (100 lin = 40 dB, 10 lin = 20 dB, 3.16 lin = 10 dB etc)
                   *  The gain is later factored into the 'b' coefficients
                   *  (numerator of the filter equation).  This gain factor depends
                   *  only on Q, so this is the right place to calculate it.
                   */
                  filter_gain = (float) (1.0 / sqrt(q_lin));

                  /* The synthesis loop will have to recalculate the filter coefficients. */
                  last_fres = -1.;
                  break;

            case GEN_MODLFOTOPITCH:
                  modlfo_to_pitch = GEN(GEN_MODLFOTOPITCH);
                  fluid_clip(modlfo_to_pitch, -12000.0, 12000.0);
                  break;

            case GEN_MODLFOTOVOL:
                  modlfo_to_vol = GEN(GEN_MODLFOTOVOL);
                  fluid_clip(modlfo_to_vol, -960.0, 960.0);
                  break;

            case GEN_MODLFOTOFILTERFC:
                  modlfo_to_fc = GEN(GEN_MODLFOTOFILTERFC);
                  fluid_clip(modlfo_to_fc, -12000, 12000);
                  break;

            case GEN_MODLFODELAY:
                  x = GEN(GEN_MODLFODELAY);
                  fluid_clip(x, -12000.0f, 5000.0f);
                  modlfo_delay = (unsigned int) (_fluid->sample_rate * fluid_tc2sec_delay(x));
                  break;

            case GEN_MODLFOFREQ:
                  {
                  /* - the frequency is converted into a delta value, per frame
                   * - the delay into a sample delay
                   */
                  unsigned int old_modlfo_dur = modlfo_dur;
                  x = GEN(GEN_MODLFOFREQ);
                  fluid_clip(x, -16000.0f, 4500.0f);
                  modlfo_dur = _fluid->sample_rate / fluid_act2hz(x);
                  if (old_modlfo_dur > 0)
                        modlfo_pos = (modlfo_pos/old_modlfo_dur) * modlfo_dur;
                  break;
                  }

            case GEN_VIBLFOFREQ:
                  /* vib lfo
                   *
                   * - the frequency is converted into a delta value per frame
                   * - the delay into a sample delay
                   */
                  x = GEN(GEN_VIBLFOFREQ);
                  fluid_clip(x, -16000.0f, 4500.0f);
                  viblfo_incr = (4.0f * fluid_act2hz(x) / _fluid->sample_rate);
                  break;

            case GEN_VIBLFODELAY:
                  x = GEN(GEN_VIBLFODELAY);
                  fluid_clip(x, -12000.0f, 5000.0f);
                  viblfo_delay = (unsigned int) (_fluid->sample_rate * fluid_tc2sec_delay(x));
                  break;

            case GEN_VIBLFOTOPITCH:
                  viblfo_to_pitch = GEN(GEN_VIBLFOTOPITCH);
                  fluid_clip(viblfo_to_pitch, -12000.0, 12000.0);
                  break;

            case GEN_KEYNUM:
                 /* GEN_KEYNUM: SF2.01 page 46, item 46
                  *
                  * If this generator is active, it forces the key number to its
                  * value.  Non-realtime controller.
                  *
                  * There is a flag, which should indicate, whether a generator is
                  * enabled or not.  But here we rely on the default value of -1.
                  */
                  x = GEN(GEN_KEYNUM);
                  if (x >= 0)
                        key = x;
                  break;

            case GEN_VELOCITY:
                 /* GEN_VELOCITY: SF2.01 page 46, item 47
                  *
                  * If this generator is active, it forces the velocity to its
                  * value. Non-realtime controller.
                  *
                  * There is a flag, which should indicate, whether a generator is
                  * enabled or not. But here we rely on the default value of -1.
                  */
                  x = GEN(GEN_VELOCITY);
                  if (x > 0)
                        vel = x;
                  break;

            case GEN_MODENVTOPITCH:
                  modenv_to_pitch = GEN(GEN_MODENVTOPITCH);
                  fluid_clip(modenv_to_pitch, -12000.0, 12000.0);
                  break;

            case GEN_MODENVTOFILTERFC:
                  modenv_to_fc = GEN(GEN_MODENVTOFILTERFC);

                  /* Range: SF2.01 section 8.1.3 # 1
                   * Motivation for range checking:
                   * Filter is reported to make funny noises now and then
                   */
                  fluid_clip(modenv_to_fc, -12000.0, 12000.0);
                  break;


    /* sample start and ends points
     *
     * Range checking is initiated via the
     * check_sample_sanity flag,
     * because it is impossible to check here:
     * During the voice setup, all modulators are processed, while
     * the voice is inactive. Therefore, illegal settings may
     * occur during the setup (for example: First move the loop
     * end point ahead of the loop start point => invalid, then
     * move the loop start point forward => valid again.
     */

            case GEN_STARTADDROFS:              /* SF2.01 section 8.1.3 # 0 */
            case GEN_STARTADDRCOARSEOFS:        /* SF2.01 section 8.1.3 # 4 */
                  if (sample != 0) {
                        start = (sample->start
			         + (int) GEN(GEN_STARTADDROFS)
			         + 32768 * (int) GEN(GEN_STARTADDRCOARSEOFS));
                        check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
                        }
                  break;

            case GEN_ENDADDROFS:                 /* SF2.01 section 8.1.3 # 1 */
            case GEN_ENDADDRCOARSEOFS:           /* SF2.01 section 8.1.3 # 12 */
                  if (sample != 0) {
                        end = (sample->end
			         + (int) GEN(GEN_ENDADDROFS)
			         + 32768 * (int) GEN(GEN_ENDADDRCOARSEOFS));
                        check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
                        }
                  break;

            case GEN_STARTLOOPADDROFS:           /* SF2.01 section 8.1.3 # 2 */
            case GEN_STARTLOOPADDRCOARSEOFS:     /* SF2.01 section 8.1.3 # 45 */
                  if (sample != 0) {
                        loopstart = (sample->loopstart
				  + (int) GEN(GEN_STARTLOOPADDROFS)
				  + 32768 * (int) GEN(GEN_STARTLOOPADDRCOARSEOFS));
                        check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
                        }
                  break;

            case GEN_ENDLOOPADDROFS:             /* SF2.01 section 8.1.3 # 3 */
            case GEN_ENDLOOPADDRCOARSEOFS:       /* SF2.01 section 8.1.3 # 50 */
                  if (sample != 0) {
                        loopend = (sample->loopend
				   + (int) GEN(GEN_ENDLOOPADDROFS)
				   + 32768 * (int) GEN(GEN_ENDLOOPADDRCOARSEOFS));
                        check_sample_sanity_flag = FLUID_SAMPLESANITY_CHECK;
                        }
                  break;

    /* Conversion functions differ in range limit */
#define NUM_FRAMES_DELAY(_v)   (unsigned int) (_fluid->sample_rate * fluid_tc2sec_delay(_v))
#define NUM_FRAMES_ATTACK(_v)  (unsigned int) (_fluid->sample_rate * fluid_tc2sec_attack(_v))
#define NUM_FRAMES_RELEASE(_v) (unsigned int) (_fluid->sample_rate * fluid_tc2sec_release(_v))

           /* volume envelope
            *
            * - delay and hold times are converted to absolute number of samples
            * - sustain is converted to its absolute value
            * - attack, decay and release are converted to their increment per sample
            */

            case GEN_VOLENVDELAY:                /* SF2.01 section 8.1.3 # 33 */
                  x = GEN(GEN_VOLENVDELAY);
                  fluid_clip(x, -12000.0f, 5000.0f);
                  count = NUM_FRAMES_DELAY(x);
                  volenv_data[FLUID_VOICE_ENVDELAY].count = count;
                  volenv_data[FLUID_VOICE_ENVDELAY].coeff = 0.0f;
                  volenv_data[FLUID_VOICE_ENVDELAY].incr = 0.0f;
                  volenv_data[FLUID_VOICE_ENVDELAY].min = -1.0f;
                  volenv_data[FLUID_VOICE_ENVDELAY].max = 1.0f;
                  break;

            case GEN_VOLENVATTACK:               /* SF2.01 section 8.1.3 # 34 */
                  x = GEN(GEN_VOLENVATTACK);
                  fluid_clip(x, -12000.0f, 8000.0f);
                  count = 1 + NUM_FRAMES_ATTACK(x);
                  volenv_data[FLUID_VOICE_ENVATTACK].count = count;
                  volenv_data[FLUID_VOICE_ENVATTACK].coeff = 1.0f;
                  volenv_data[FLUID_VOICE_ENVATTACK].incr = count ? 1.0f / count : 0.0f;
                  volenv_data[FLUID_VOICE_ENVATTACK].min = -1.0f;
                  volenv_data[FLUID_VOICE_ENVATTACK].max = 1.0f;
                  break;

            case GEN_VOLENVHOLD:                 /* SF2.01 section 8.1.3 # 35 */
            case GEN_KEYTOVOLENVHOLD:            /* SF2.01 section 8.1.3 # 39 */
                  count = calculate_hold_decay_frames(GEN_VOLENVHOLD, GEN_KEYTOVOLENVHOLD, 0); /* 0 means: hold */
                  volenv_data[FLUID_VOICE_ENVHOLD].count = count;
                  volenv_data[FLUID_VOICE_ENVHOLD].coeff = 1.0f;
                  volenv_data[FLUID_VOICE_ENVHOLD].incr = 0.0f;
                  volenv_data[FLUID_VOICE_ENVHOLD].min = -1.0f;
                  volenv_data[FLUID_VOICE_ENVHOLD].max = 2.0f;
                  break;

            case GEN_VOLENVDECAY:               /* SF2.01 section 8.1.3 # 36 */
            case GEN_VOLENVSUSTAIN:             /* SF2.01 section 8.1.3 # 37 */
            case GEN_KEYTOVOLENVDECAY:          /* SF2.01 section 8.1.3 # 40 */
                  y = 1.0f - 0.001f * GEN(GEN_VOLENVSUSTAIN);
                  fluid_clip(y, 0.0f, 1.0f);
                  count = calculate_hold_decay_frames(GEN_VOLENVDECAY, GEN_KEYTOVOLENVDECAY, 1); /* 1 for decay */
                  volenv_data[FLUID_VOICE_ENVDECAY].count = count;
                  volenv_data[FLUID_VOICE_ENVDECAY].coeff = 1.0f;
                  volenv_data[FLUID_VOICE_ENVDECAY].incr = count ? -1.0f / count : 0.0f;
                  volenv_data[FLUID_VOICE_ENVDECAY].min = y;
                  volenv_data[FLUID_VOICE_ENVDECAY].max = 2.0f;
                  break;

            case GEN_VOLENVRELEASE:             /* SF2.01 section 8.1.3 # 38 */
                  x = GEN(GEN_VOLENVRELEASE);
                  fluid_clip(x, FLUID_MIN_VOLENVRELEASE, 8000.0f);
                  count = 1 + NUM_FRAMES_RELEASE(x);
                  volenv_data[FLUID_VOICE_ENVRELEASE].count = count;
                  volenv_data[FLUID_VOICE_ENVRELEASE].coeff = 1.0f;
                  volenv_data[FLUID_VOICE_ENVRELEASE].incr = count ? -1.0f / count : 0.0f;
                  volenv_data[FLUID_VOICE_ENVRELEASE].min = 0.0f;
                  volenv_data[FLUID_VOICE_ENVRELEASE].max = 1.0f;
                  break;

            /* Modulation envelope */
            case GEN_MODENVDELAY:               /* SF2.01 section 8.1.3 # 25 */
                  x = GEN(GEN_MODENVDELAY);
                  fluid_clip(x, -12000.0f, 5000.0f);
                  modenv_data[FLUID_VOICE_ENVDELAY].count = NUM_FRAMES_DELAY(x);
                  modenv_data[FLUID_VOICE_ENVDELAY].coeff = 0.0f;
                  modenv_data[FLUID_VOICE_ENVDELAY].incr = 0.0f;
                  modenv_data[FLUID_VOICE_ENVDELAY].min = -1.0f;
                  modenv_data[FLUID_VOICE_ENVDELAY].max = 1.0f;
                  break;

            case GEN_MODENVATTACK:               /* SF2.01 section 8.1.3 # 26 */
                  x = GEN(GEN_MODENVATTACK);
                  fluid_clip(x, -12000.0f, 8000.0f);
                  count = 1 + NUM_FRAMES_ATTACK(x);
                  modenv_data[FLUID_VOICE_ENVATTACK].count = count;
                  modenv_data[FLUID_VOICE_ENVATTACK].coeff = 1.0f;
                  modenv_data[FLUID_VOICE_ENVATTACK].incr = count ? 1.0f / count : 0.0f;
                  modenv_data[FLUID_VOICE_ENVATTACK].min = -1.0f;
                  modenv_data[FLUID_VOICE_ENVATTACK].max = 1.0f;
                  break;

            case GEN_MODENVHOLD:               /* SF2.01 section 8.1.3 # 27 */
            case GEN_KEYTOMODENVHOLD:          /* SF2.01 section 8.1.3 # 31 */
                  count = calculate_hold_decay_frames(GEN_MODENVHOLD, GEN_KEYTOMODENVHOLD, 0); /* 1 means: hold */
                  modenv_data[FLUID_VOICE_ENVHOLD].count = count;
                  modenv_data[FLUID_VOICE_ENVHOLD].coeff = 1.0f;
                  modenv_data[FLUID_VOICE_ENVHOLD].incr = 0.0f;
                  modenv_data[FLUID_VOICE_ENVHOLD].min = -1.0f;
                  modenv_data[FLUID_VOICE_ENVHOLD].max = 2.0f;
                  break;

            case GEN_MODENVDECAY:                                   /* SF 2.01 section 8.1.3 # 28 */
            case GEN_MODENVSUSTAIN:                                 /* SF 2.01 section 8.1.3 # 29 */
            case GEN_KEYTOMODENVDECAY:                              /* SF 2.01 section 8.1.3 # 32 */
                  count = calculate_hold_decay_frames(GEN_MODENVDECAY, GEN_KEYTOMODENVDECAY, 1); /* 1 for decay */
                  y = 1.0f - 0.001f * GEN(GEN_MODENVSUSTAIN);
                  fluid_clip(y, 0.0f, 1.0f);
                  modenv_data[FLUID_VOICE_ENVDECAY].count = count;
                  modenv_data[FLUID_VOICE_ENVDECAY].coeff = 1.0f;
                  modenv_data[FLUID_VOICE_ENVDECAY].incr = count ? -1.0f / count : 0.0f;
                  modenv_data[FLUID_VOICE_ENVDECAY].min = y;
                  modenv_data[FLUID_VOICE_ENVDECAY].max = 2.0f;
                  break;

            case GEN_MODENVRELEASE:                                  /* SF 2.01 section 8.1.3 # 30 */
                  x = GEN(GEN_MODENVRELEASE);
                  fluid_clip(x, -12000.0f, 8000.0f);
                  count = 1 + NUM_FRAMES_RELEASE(x);
                  modenv_data[FLUID_VOICE_ENVRELEASE].count = count;
                  modenv_data[FLUID_VOICE_ENVRELEASE].coeff = 1.0f;
                  modenv_data[FLUID_VOICE_ENVRELEASE].incr = count ? -1.0f / count : 0.0;
                  modenv_data[FLUID_VOICE_ENVRELEASE].min = 0.0f;
                  modenv_data[FLUID_VOICE_ENVRELEASE].max = 2.0f;
                  break;

            } /* switch gen */
      }

/**
 * fluid_voice_modulate
 *
 * In this implementation, I want to make sure that all controllers
 * are event based: the parameter values of the DSP algorithm should
 * only be updates when a controller event arrived and not at every
 * iteration of the audio cycle (which would probably be feasible if
 * the synth was made in silicon).
 *
 * The update is done in three steps:
 *
 * - first, we look for all the modulators that have the changed
 * controller as a source. This will yield a list of generators that
 * will be changed because of the controller event.
 *
 * - For every changed generator, calculate its new value. This is the
 * sum of its original value plus the values of al the attached
 * modulators.
 *
 * - For every changed generator, convert its value to the correct
 * unit of the corresponding DSP parameter
 * */

void Voice::modulate(bool _cc, int _ctrl)
      {
      for (int i = 0; i < mod_count; i++) {
            Mod* m = &mod[i];

            /* step 1: find all the modulators that have the changed controller
             * as input source.
             */
            if (m->has_source(_cc, _ctrl)) {
                  int g = m->get_dest();
                  float modval = 0.0;

                  /* step 2: for every changed modulator, calculate the modulation
                   * value of its associated generator
                   */
                  for (int k = 0; k < mod_count; k++) {
                        if (fluid_mod_has_dest(&mod[k], g)) {
                              modval += mod[k].get_value(channel, this);
                              }
                        }
                  gen[g].set_mod(modval);

                  /* step 3: now that we have the new value of the generator,
                   * recalculate the parameter values that are derived from the
                   * generator
                   */
                  update_param(g);
                  }
            }
      }

/**
 * fluid_voice_modulate_all
 *
 * Update all the modulators. This function is called after a
 * ALL_CTRL_OFF MIDI message has been received (CC 121).
 *
 */
void Voice::modulate_all()
      {
      /* Loop through all the modulators.
        FIXME: we should loop through the set of generators instead of
        the set of modulators. We risk to call 'fluid_voice_update_param'
        several times for the same generator if several modulators have
        that generator as destination. It's not an error, just a wast of
        energy (think pollution, global warming, unhappy musicians, ...)
       */

      for (int i = 0; i < mod_count; i++) {
            Mod* m       = &mod[i];
            int g        = m->get_dest();
            float modval = 0.0;

            /* Accumulate the modulation values of all the modulators with
             * destination generator 'gen'
             */
            for (int k = 0; k < mod_count; k++) {
                  if (fluid_mod_has_dest(&mod[k], g))
                        modval += mod[k].get_value(channel, this);
                  }

            gen[g].set_mod(modval);

            /* Update the parameter values that are depend on the generator
             * 'gen'
             */
            update_param(g);
            }
      }

/*
 * fluid_voice_noteoff
 */
void Voice::noteoff()
      {
      if (channel && channel->sustained())
            status = FLUID_VOICE_SUSTAINED;
      else {
            if (volenv_section == FLUID_VOICE_ENVATTACK) {
                  /* A voice is turned off during the attack section of the volume
                  * envelope.  The attack section ramps up linearly with
                  * amplitude. The other sections use logarithmic scaling. Calculate new
                  * volenv_val to achieve equievalent amplitude during the release phase
                  * for seamless volume transition.
                  */
                  if (volenv_val > 0) {
                        float lfo = modlfo_val * -modlfo_to_vol;
                        float ampl = volenv_val * pow (10.0, lfo / -200);
                        float env_value = - ((-200 * log (ampl) / log (10.0) - lfo) / 960.0 - 1);
                        fluid_clip (env_value, 0.0, 1.0);
                        volenv_val = env_value;
                        }
                  }
            volenv_section = FLUID_VOICE_ENVRELEASE;
            volenv_count = 0;
            modenv_section = FLUID_VOICE_ENVRELEASE;
            modenv_count = 0;
            }
      }

/*
 * fluid_voice_kill_excl
 *
 * Percussion sounds can be mutually exclusive: for example, a 'closed
 * hihat' sound will terminate an 'open hihat' sound ringing at the
 * same time. This behaviour is modeled using 'exclusive classes',
 * turning on a voice with an exclusive class other than 0 will kill
 * all other voices having that exclusive class within the same preset
 * or channel.  fluid_voice_kill_excl gets called, when 'voice' is to
 * be killed for that reason.
 */

void Voice::kill_excl()
      {
      if (!isPlaying())
            return;

      /* Turn off the exclusive class information for this voice,
         so that it doesn't get killed twice
         */
      gen_set(GEN_EXCLUSIVECLASS, 0);

      /* If the voice is not yet in release state, put it into release state */
      if (volenv_section != FLUID_VOICE_ENVRELEASE) {
            volenv_section = FLUID_VOICE_ENVRELEASE;
            volenv_count = 0;
            modenv_section = FLUID_VOICE_ENVRELEASE;
            modenv_count = 0;
            }

      /* Speed up the volume envelope */
      /* The value was found through listening tests with hi-hat samples. */
      gen_set(GEN_VOLENVRELEASE, -200);
      update_param(GEN_VOLENVRELEASE);

      /* Speed up the modulation envelope */
      gen_set(GEN_MODENVRELEASE, -200);
      update_param(GEN_MODENVRELEASE);
      }

//---------------------------------------------------------
//   off
//    Turns off a voice, meaning that it is not processed
//    anymore by the DSP loop.
//---------------------------------------------------------

void Voice::off()
      {
      chan           = NO_CHANNEL;
      volenv_section = FLUID_VOICE_ENVFINISHED;
      volenv_count   = 0;
      modenv_section = FLUID_VOICE_ENVFINISHED;
      modenv_count   = 0;
      status         = FLUID_VOICE_OFF;
      _fluid->freeVoice(this);
      _cachedFrames = 0;
      _initialCacheFrames = 0;
      }

/*
 * fluid_voice_add_mod
 *
 * Adds a modulator to the voice.  "mode" indicates, what to do, if
 * an identical modulator exists already.
 *
 * mode == FLUID_VOICE_ADD: Identical modulators on preset level are added
 * mode == FLUID_VOICE_OVERWRITE: Identical modulators on instrument level are overwritten
 * mode == FLUID_VOICE_DEFAULT: This is a default modulator, there can be no identical modulator.
 *                             Don't check.
 */
void Voice::add_mod(const Mod* _mod, int mode)
      {
      /*
       * Some soundfonts come with a huge number of non-standard
       * controllers, because they have been designed for one particular
       * sound card.  Discard them, maybe print a warning.
       */

      if (((_mod->flags1 & FLUID_MOD_CC) == 0)
         && ((_mod->src1 != 0)      /* SF2.01 section 8.2.1: Constant value */
	   && (_mod->src1 != 2)       /* Note-on velocity */
	   && (_mod->src1 != 3)       /* Note-on key number */
	   && (_mod->src1 != 10)      /* Poly pressure */
	   && (_mod->src1 != 13)      /* Channel pressure */
	   && (_mod->src1 != 14)      /* Pitch wheel */
	   && (_mod->src1 != 16))) {  /* Pitch wheel sensitivity */
            qDebug("Ignoring invalid controller, using non-CC source %i.", _mod->src1);
            return;
            }

      if (mode == FLUID_VOICE_ADD) {
            /* if identical modulator exists, add them */
            for (int i = 0; i < mod_count; i++) {
                  if (test_identity(&mod[i], _mod)) {
                        //		printf("Adding modulator...\n");
                        mod[i].amount += _mod->amount;
                        return;
                        }
                  }
            }
      else if (mode == FLUID_VOICE_OVERWRITE) {
            /* if identical modulator exists, replace it (only the amount has to be changed) */
            for (int i = 0; i < mod_count; i++) {
                  if (test_identity(&mod[i], _mod)) {
                        //  printf("Replacing modulator...amount is %f\n",mod->amount);
                        mod[i].amount = _mod->amount;
                        return;
                        }
                  }
            }
      /* Add a new modulator (No existing modulator to add / overwrite).
         Also, default modulators (FLUID_VOICE_DEFAULT) are added without
         checking, if the same modulator already exists.
         */
      if (mod_count < FLUID_NUM_MOD)
            _mod->clone(&mod[mod_count++]);
      }

/*
 * fluid_voice_get_lower_boundary_for_attenuation
 *
 * Purpose:
 *
 * A lower boundary for the attenuation (as in 'the minimum
 * attenuation of this voice, with volume pedals, modulators
 * etc. resulting in minimum attenuation, cannot fall below x cB) is
 * calculated.  This has to be called during fluid_voice_init, after
 * all modulators have been run on the voice once.  Also,
 * voice->attenuation has to be initialized.
 */

float Voice::get_lower_boundary_for_attenuation()
      {
      float possible_att_reduction_cB = 0;

      for (int i = 0; i < mod_count; i++) {
            Mod* m = &mod[i];

            /* Modulator has attenuation as target and can change over time? */
            if ((m->dest == GEN_ATTENUATION) && ((m->flags1 & FLUID_MOD_CC) || (m->flags2 & FLUID_MOD_CC))) {
                  float current_val = m->get_value(channel, this);
                  float v = fabs(m->amount);

                  if ((m->src1 == FLUID_MOD_PITCHWHEEL)
                     || (m->flags1 & FLUID_MOD_BIPOLAR)
	               || (m->flags2 & FLUID_MOD_BIPOLAR)
	               || (m->amount < 0)) {
                        /* Can this modulator produce a negative contribution? */
	                  v *= -1.0;
                        }
                  else {
	                  /* No negative value possible. But still, the minimum contribution is 0. */
	                  v = 0;
                        }

                  /* For example:
                   * - current_val=100
                   * - min_val=-4000
                   * - possible_att_reduction_cB += 4100
                   */
                  if (current_val > v)
	                  possible_att_reduction_cB += (current_val - v);
                  }
            }
      float lower_bound = attenuation - possible_att_reduction_cB;

      /* SF2.01 specs do not allow negative attenuation */
      if (lower_bound < 0)
            lower_bound = 0;
      return lower_bound;
      }


/* Purpose:
 *
 * Make sure, that sample start / end point and loop points are in
 * proper order. When starting up, calculate the initial phase.
 */
void Voice::check_sample_sanity()
      {
      int min_index_nonloop=(int) sample->start;
      int max_index_nonloop=(int) sample->end;

      /* make sure we have enough samples surrounding the loop */
      int min_index_loop=(int) sample->start + FLUID_MIN_LOOP_PAD;
      int max_index_loop=(int) sample->end - FLUID_MIN_LOOP_PAD;
      fluid_check_fpe("voice_check_sample_sanity start");

      if (!check_sample_sanity_flag)
	      return;

#if 0
      printf("Sample from %i to %i\n", sample->start, sample->end);
      printf("Sample loop from %i %i\n", sample->loopstart, sample->loopend);
      printf("Playback from %i to %i\n", start, end);
      printf("Playback loop from %i to %i\n", loopstart, loopend);
#endif

      /* Keep the start point within the sample data */
      if (start < min_index_nonloop)
            start = min_index_nonloop;
      else if (start > max_index_nonloop)
            start = max_index_nonloop;

      /* Keep the end point within the sample data */
      if (end < min_index_nonloop)
            end = min_index_nonloop;
      else if (end > max_index_nonloop)
            end = max_index_nonloop;

      /* Keep start and end point in the right order */
      if (start > end) {
            int temp = start;
            start    = end;
            end      = temp;
            }

      /* Zero length? */
      if (start == end) {
            off();
	      return;
            }

      if ((SAMPLEMODE() == FLUID_LOOP_UNTIL_RELEASE) || (SAMPLEMODE() == FLUID_LOOP_DURING_RELEASE)) {
            /* Keep the loop start point within the sample data */
	      if (loopstart < min_index_loop)
	            loopstart = min_index_loop;
            else if (loopstart > max_index_loop)
	            loopstart = max_index_loop;

            /* Keep the loop end point within the sample data */
            if (loopend < min_index_loop)
	            loopend = min_index_loop;
            else if (loopend > max_index_loop)
	            loopend = max_index_loop;

            /* Keep loop start and end point in the right order */
            if (loopstart > loopend){
	            int temp  = loopstart;
	            loopstart = loopend;
	            loopend   = temp;
                  }

            /* Loop too short? Then don't loop. */
            if (loopend < loopstart + FLUID_MIN_LOOP_SIZE)
	            gen[GEN_SAMPLEMODE].val = FLUID_UNLOOPED;

            /* The loop points may have changed. Obtain a new estimate for the loop volume. */
            /* Is the voice loop within the sample loop?
             */
            if ((int)loopstart >= (int)sample->loopstart && (int)loopend <= (int)sample->loopend){
	            /* Is there a valid peak amplitude available for the loop? */
	            if (sample->amplitude_that_reaches_noise_floor_is_valid) {
	                  amplitude_that_reaches_noise_floor_loop = sample->amplitude_that_reaches_noise_floor;
                        }
                  else
	                  /* Worst case */
	                  amplitude_that_reaches_noise_floor_loop = amplitude_that_reaches_noise_floor_nonloop;
                  }
            } /* if sample mode is looped */

      /* Run startup specific code (only once, when the voice is started) */
      if (check_sample_sanity_flag & FLUID_SAMPLESANITY_STARTUP) {
            if (max_index_loop - min_index_loop < FLUID_MIN_LOOP_SIZE){
                  if ((SAMPLEMODE() == FLUID_LOOP_UNTIL_RELEASE) || (SAMPLEMODE() == FLUID_LOOP_DURING_RELEASE))
	                  gen[GEN_SAMPLEMODE].val = FLUID_UNLOOPED;
                  }

            /* Set the initial phase of the voice (using the result from the
	         start offset modulators).
             */
            phase.setInt(start);
            } /* if startup */

      /* Is this voice run in loop mode, or does it run straight to the
       end of the waveform data?
       */
      if (((SAMPLEMODE() == FLUID_LOOP_UNTIL_RELEASE) && (volenv_section < FLUID_VOICE_ENVRELEASE)) || (SAMPLEMODE() == FLUID_LOOP_DURING_RELEASE)) {
           /* Yes, it will loop as soon as it reaches the loop point.  In
            * this case we must prevent, that the playback pointer (phase)
            * happens to end up beyond the 2nd loop point, because the
            * point has moved.  The DSP algorithm is unable to cope with
            * that situation.  So if the phase is beyond the 2nd loop
            * point, set it to the start of the loop. No way to avoid some
            * noise here.  Note: If the sample pointer ends up -before the
            * first loop point- instead, then the DSP loop will just play
            * the sample, enter the loop and proceed as expected => no
            * actions required.
            */
            int index_in_sample = phase.index();
            if (index_in_sample >= loopend) {
     	            /* FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Phase after 2nd loop point!"); */
     	            phase.setInt(loopstart);
                  }
            }
/*    FLUID_LOG(FLUID_DBG, "Loop / sample sanity check: Sample from %i to %i, loop from %i to %i", voice->start, voice->end, voice->loopstart, voice->loopend); */

    /* Sample sanity has been assured. Don't check again, until some
       sample parameter is changed by modulation.
     */
      check_sample_sanity_flag = 0;
      fluid_check_fpe("voice_check_sample_sanity");
      }

//---------------------------------------------------------
//   set_param
//---------------------------------------------------------

void Voice::set_param(int g, float nrpn_value, int abs)
      {
      gen[g].nrpn = nrpn_value;
      gen[g].flags = (abs)? GEN_ABS_NRPN : GEN_SET;
      update_param(g);
      }

  /** If the peak volume during the loop is known, then the voice can
   * be released earlier during the release phase. Otherwise, the
   * voice will operate (inaudibly), until the envelope is at the
   * nominal turnoff point. In many cases the loop volume is many dB
   * below the maximum volume.  For example, the loop volume for a
   * typical acoustic piano is 20 dB below max.  Taking that into
   * account in the turn-off algorithm we can save 20 dB / 100 dB =>
   * 1/5 of the total release time.
   * So it's a good idea to call fluid_voice_optimize_sample
   * on each sample once.
   */
/* - Scan the loop
 * - determine the peak level
 * - Calculate, what factor will make the loop inaudible
 * - Store in sample
 */
void Sample::optimize()
      {
      Sample* s = this;
      signed short peak_max = 0;
      signed short peak_min = 0;
      signed short peak;
      float normalized_amplitude_during_loop;
      double result;
      int i;

      /* ignore ROM and other(?) invalid samples */
      if (!s->valid())
            return;

      if (!s->amplitude_that_reaches_noise_floor_is_valid) { /* Only once */
            /* Scan the loop */
            for (i = (int)s->loopstart; i < (int) s->loopend; i ++) {
                  signed short val = s->data[i];
                  if (val > peak_max)
                        peak_max = val;
                  else if (val < peak_min)
                        peak_min = val;
                  }

            /* Determine the peak level */
            if (peak_max > -peak_min)
                  peak = peak_max;
            else
                  peak = -peak_min;
            if (peak == 0)    /* Avoid division by zero */
                  peak = 1;

            /* Calculate what factor will make the loop inaudible
             * For example: Take a peak of 3277 (10 % of 32768).  The
             * normalized amplitude is 0.1 (10 % of 32768).  An amplitude
             * factor of 0.0001 (as opposed to the default 0.00001) will
             * drop this sample to the noise floor.
             */

            /* 16 bits => 96+4=100 dB dynamic range => 0.00001 */
            normalized_amplitude_during_loop = ((float)peak)/32768.;
            result = FLUID_NOISE_FLOOR / normalized_amplitude_during_loop;

            /* Store in sample */
            s->amplitude_that_reaches_noise_floor = (double)result;
            s->amplitude_that_reaches_noise_floor_is_valid = 1;
            }
      }
/* Purpose:
 *
 * - filters (applies a lowpass filter with variable cutoff frequency and quality factor)
 * - mixes the processed sample to left and right output using the pan setting
 * - sends the processed sample to chorus and reverb
 *
 * A couple of variables are used internally, their results are discarded:
 * - dsp_phase_fractional: The fractional part of dsp_phase
 * - dsp_coeff: A table of four coefficients, depending on the fractional phase.
 *              Used to interpolate between samples.
 * - dsp_process_buffer: Holds the processed signal between stages
 * - dsp_centernode: delay line for the IIR filter
 * - dsp_hist1: same
 * - dsp_hist2: same
 *
 */
void Voice::effects(int startBufIdx, int count, float* out, float* reverb, float* chorus)
      {
      /* filter (implement the voice filter according to SoundFont standard) */

      /* Check for denormal number (too close to zero). */
      if (fabs (hist1) < 1e-20)
            hist1 = 0.0f;             /* FIXME JMG - Is this even needed? */

      /* Two versions of the filter loop. One, while the filter is
       * changing towards its new setting. The other, if the filter
       * doesn't change.
       */

      if (filter_coeff_incr_count > 0) {
            /* Increment is added to each filter coefficient filter_coeff_incr_count times. */
            for (int i = startBufIdx; i < startBufIdx + count; i++) {
                  /* The filter is implemented in Direct-II form. */
                  auto& dspValRef = dsp_buf[i];
                  float dsp_centernode = dspValRef - a1 * hist1 - a2 * hist2;
                  dspValRef = b02 * (dsp_centernode + hist2) + b1 * hist1;
                  hist2 = hist1;
                  hist1 = dsp_centernode;

                  if (filter_coeff_incr_count-- > 0) {
                        a1  += a1_incr;
                        a2  += a2_incr;
                        b02 += b02_incr;
                        b1  += b1_incr;
                        }

                  //code duplication is needed to optimize using std::vector::operator[]
                  float vv = dspValRef * amp_left;
                  *out++ += vv;
                  *reverb++ += vv * amp_reverb;
                  *chorus++ += vv * amp_chorus;

                  vv = dspValRef * amp_right;
                  *out++ += vv;
                  *reverb++ += vv * amp_reverb;
                  *chorus++ += vv * amp_chorus;
                  }
            }
      else { /* The filter parameters are constant.  This is duplicated to save time. */
            for (int i = startBufIdx; i < startBufIdx + count; i++) {   // The filter is implemented in Direct-II form.
                  auto& dspValRef = dsp_buf[i];
                  float dsp_centernode = dspValRef - a1 * hist1 - a2 * hist2;
                  dspValRef      = b02 * (dsp_centernode + hist2) + b1 * hist1;
                  hist2          = hist1;
                  hist1          = dsp_centernode;

                  //code duplication is needed to optimize using std::vector::operator[]
                  float vv = dspValRef * amp_left;
                  *out++ += vv;
                  *reverb++ += vv * amp_reverb;
                  *chorus++ += vv * amp_chorus;

                  vv = dspValRef * amp_right;
                  *out++ += vv;
                  *reverb++ += vv * amp_reverb;
                  *chorus++ += vv * amp_chorus;
                  }
            }
      }
}

