//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>

#include "voice.h"
#include "instrument.h"
#include "channel.h"
#include "zerberus.h"
#include "zone.h"
#include "sample.h"
#include "synthesizer/msynthesizer.h"

float Voice::interpCoeff[INTERP_MAX][4];
float Envelope::egPow[EG_SIZE];
float Envelope::egLin[EG_SIZE];

static const char* voiceStateNames[] = {
      "OFF", "ATTACK", "PLAYING", "SUSTAINED", "STOP"
      };

//---------------------------------------------------------
//   setTime
//---------------------------------------------------------

void Envelope::setTime(float ms, int sampleRate)
      {
      val   = 1.0;
      steps = int(ms * sampleRate / 1000);
      count = steps;
      }

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

Voice::Voice(Zerberus* z)
   : _zerberus(z), attackEnv(Envelope::egLin), stopEnv(Envelope::egPow)
      {
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void Voice::stop(float time)
      {
      _state = VoiceState::STOP;
      stopEnv.setTime(time, _zerberus->sampleRate());
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Voice::init()
      {
      // Initialize the coefficients for the interpolation. The math comes
      // from a mail, posted by Olli Niemitalo to the music-dsp mailing
      // list (I found it in the music-dsp archives
      // http://www.smartelectronix.com/musicdsp/).

      double ff = 1.0 / 32768.0;
      for (int i = 0; i < INTERP_MAX; i++) {
            double x = (double) i / (double) INTERP_MAX;
            interpCoeff[i][0] = (x * (-0.5 + x * (1 - 0.5 * x)))  * ff;
            interpCoeff[i][1] = (1.0 + x * x * (1.5 * x - 2.5))   * ff;
            interpCoeff[i][2] = (x * (0.5 + x * (2.0 - 1.5 * x))) * ff;
            interpCoeff[i][3] = (0.5 * x * x * (x - 1.0))         * ff;
            }
      static const float MIN_GAIN = -80.0;
      static const float dbStep = MIN_GAIN / float(EG_SIZE);

      for (int i = 0; i < EG_SIZE; ++i) {
            Envelope::egPow[EG_SIZE-i-1] = pow(10.0, (dbStep * i)/20.0);
            Envelope::egLin[i]           = 1.0 - (double(i) / double(EG_SIZE));
            }
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void Voice::start(Channel* c, int key, int v, const Zone* z)
      {
      _state    = VoiceState::ATTACK;
      //_state    = VoiceState::PLAYING;
      _channel  = c;
      _key      = key;
      _velocity = v;
      Sample* s = z->sample;
      audioChan = s->channel();
      data      = s->data() + z->offset * audioChan;
      eidx      = s->frames() * audioChan;
      _loopMode = z->loopMode;

      _offMode  = z->offMode;
      _offBy    = z->offBy;

      float offset = -z->ampVeltrack;
      if (offset <= 0)
            offset += 100;
      float curve = _velocity * _velocity / (127.0 * 127.0);
      gain        = z->volume * (offset + z->ampVeltrack * curve)
                    * .005 * c->gain();

      phase.set(0);
      float sr = float(s->sampleRate()) / _zerberus->sampleRate();
      phaseIncr.set(_zerberus->ct2hz(key * 100.0 + z->tune) * sr/_zerberus->ct2hz(z->keyBase * 100.0));

      fres        = 13500.0;
      last_fres   = -1.0;
      qreal GEN_FILTERQ = 100.0;  // 0 - 960
      qreal q_db  = GEN_FILTERQ / 10.0f - 3.01f;
      q_lin       = pow(10.0f, q_db / 20.0f);
      filter_gain = 1.0 / sqrt(q_lin);

      hist1r = 0;
      hist2r = 0;
      hist1l = 0;
      hist2l = 0;

      filter_startup = true;

      modenv_val = 0.0;
      modlfo_val = 0.0;

      attackEnv.setTime(1, _zerberus->sampleRate());        // 1 ms attack
      stopEnv.setTime(z->ampegRelease, _zerberus->sampleRate());
      }

//---------------------------------------------------------
//   updateFilter
//---------------------------------------------------------

void Voice::updateFilter(float _fres)
      {
      // The filter coefficients have to be recalculated (filter
      // parameters have changed). Recalculation for various reasons is
      // forced by setting last_fres to -1.  The flag filter_startup
      // indicates, that the DSP loop runs for the first time, in this
      // case, the filter is set directly, instead of smoothly fading
      // between old and new settings.
      //
      // Those equations from Robert Bristow-Johnson's `Cookbook
      // formulae for audio EQ biquad filter coefficients', obtained
      // from Harmony-central.com / Computer / Programming. They are
      // the result of the bilinear transform on an analogue filter
      // prototype. To quote, `BLT frequency warping has been taken
      // into account for both significant frequency relocation and for
      // bandwidth readjustment'.

      float sr          = _zerberus->sampleRate();
      float omega       = (float) 2.0f * M_PI * (_fres / sr);
      float sin_coeff   = sin(omega);
      float cos_coeff   = cos(omega);
      float alpha_coeff = sin_coeff / (2.0f * q_lin);
      float a0_inv      = 1.0f / (1.0f + alpha_coeff);

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
      // both b0 -and- b2
      float b02_temp = b1_temp * 0.5f;

      if (filter_startup) {
            /* The filter is calculated, because the voice was started up.
             * In this case set the filter coefficients without delay.
             */
            a1  = a1_temp;
            a2  = a2_temp;
            b02 = b02_temp;
            b1  = b1_temp;
            filter_coeff_incr_count = 0;
            filter_startup = false;
            }
      else {
            /* The filter frequency is changed.  Calculate an increment
            * factor, so that the new setting is reached after some time.
            */

            static const int FILTER_TRANSITION_SAMPLES = 64;

            a1_incr  = (a1_temp - a1) / FILTER_TRANSITION_SAMPLES;
            a2_incr  = (a2_temp - a2) / FILTER_TRANSITION_SAMPLES;
            b02_incr = (b02_temp - b02) / FILTER_TRANSITION_SAMPLES;
            b1_incr  = (b1_temp - b1) / FILTER_TRANSITION_SAMPLES;
            /* Have to add the increments filter_coeff_incr_count times. */
            filter_coeff_incr_count = FILTER_TRANSITION_SAMPLES;
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Voice::process(int frames, float* p)
      {
      float modlfo_to_fc = 0.0;
      float modenv_to_fc = 0.0;

      float _fres = _zerberus->ct2hz(fres
              + modlfo_val * modlfo_to_fc
              + modenv_val * modenv_to_fc);

      int sr = _zerberus->sampleRate();
      if (_fres > 0.45f * sr)
            _fres = 0.45f * sr;
      else if (_fres < 5.f)
            _fres = 5.f;

      if ((fabs(_fres - last_fres) > 0.01f)) {
            updateFilter(_fres);
            last_fres = _fres;
            }

      if (audioChan == 1) {
            while (frames--) {
                  int idx = phase.index();
                  if (idx >= eidx) {
                        off();
                        break;
                        }
                  const float* coeffs = interpCoeff[phase.fract()];
                  float f;
                  f =  (coeffs[0] * data[idx-1]
                      + coeffs[1] * data[idx+0]
                      + coeffs[2] * data[idx+1]
                      + coeffs[3] * data[idx+2]) * gain
                      - a1 * hist1l
                      - a2 * hist2l;
                  float v = b02 * (f + hist2l) + b1 * hist1l;
                  hist2l  = hist1l;
                  hist1l  = f;

                  if (filter_coeff_incr_count) {
                        --filter_coeff_incr_count;
                        a1  += a1_incr;
                        a2  += a2_incr;
                        b02 += b02_incr;
                        b1  += b1_incr;
                        }

                  if (_state == VoiceState::STOP) {
                        if (stopEnv.step()) {
                              off();
                              break;
                              }
                        v *= stopEnv.val;
                        }
                  *p++  += v * _channel->panLeftGain();
                  *p++  += v * _channel->panRightGain();
                  phase += phaseIncr;
                  }
            }
      else {
            //
            // handle interleaved stereo samples
            //
            while (frames--) {
                  int idx = phase.index() * 2;
                  if (idx >= eidx) {
                        off();
                        // printf("end of sample\n");
                        break;
                        }

                  const float* coeffs = interpCoeff[phase.fract()];
                  float f1, f2;

                  f1 = (coeffs[0] * data[idx-2]
                      + coeffs[1] * data[idx]
                      + coeffs[2] * data[idx+2]
                      + coeffs[3] * data[idx+4])
                      * gain * _channel->panLeftGain();

                  f2 = (coeffs[0] * data[idx-1]
                      + coeffs[1] * data[idx+1]
                      + coeffs[2] * data[idx+3]
                      + coeffs[3] * data[idx+5])
                      * gain * _channel->panRightGain();

                  if (_state == VoiceState::ATTACK) {
                        if (attackEnv.step())
                              _state = VoiceState::PLAYING;
                        else {
                              f1 *= attackEnv.val;
                              f2 *= attackEnv.val;
                              }
                        }
                  else if (_state == VoiceState::STOP) {
                        if (stopEnv.step()) {
                              off();
                              break;
                              }
                        f1 *= stopEnv.val;
                        f2 *= stopEnv.val;
                        }

                  f1      += -a1 * hist1l - a2 * hist2l;
                  float vl = b02 * (f1 + hist2l) + b1 * hist1l;
                  hist2l   = hist1l;
                  hist1l   = f1;

                  f2      +=  -a1 * hist1r - a2 * hist2r;
                  float vr = b02 * (f2 + hist2r) + b1 * hist1r;
                  hist2r   = hist1r;
                  hist1r   = f2;

                  if (filter_coeff_incr_count) {
                        --filter_coeff_incr_count;
                        a1  += a1_incr;
                        a2  += a2_incr;
                        b02 += b02_incr;
                        b1  += b1_incr;
                        }

                  *p++  += vl;
                  *p++  += vr;
                  phase += phaseIncr;
                  }
            }
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

const char* Voice::state() const
      {
      return voiceStateNames[int(_state)];
      }

