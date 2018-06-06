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
      if (ms < 0.0f)
            ms = 0.0f;
      steps = int(ms * sampleRate / 1000);
      count = steps;
      }

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

Voice::Voice(Zerberus* z)
      {
      _zerberus = z;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void Voice::stop(float time)
      {
      _state = VoiceState::STOP;
      envelopes[V1Envelopes::RELEASE].setTime(time, _zerberus->sampleRate());
      envelopes[currentEnvelope].step();
      envelopes[V1Envelopes::RELEASE].max = envelopes[currentEnvelope].val;
      currentEnvelope = V1Envelopes::RELEASE;
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

void Voice::start(Channel* c, int key, int v, const Zone* zone, double durSinceNoteOn)
      {
      z = zone;
      _state    = VoiceState::ATTACK;
      //_state    = VoiceState::PLAYING;
      _channel  = c;
      _key      = key;
      _velocity = v;
      Sample* s = z->sample;
      audioChan = s->channel();
      data      = s->data() + z->offset * audioChan;
      //avoid processing sample if offset is bigger than sample length
      eidx      = std::max((s->frames() - z->offset - 1) * audioChan, 0ll);
      _loopMode = z->loopMode;
      _loopStart = z->loopStart;
      _loopEnd   = z->loopEnd;
      _samplesSinceStart = 0;

      _offMode  = z->offMode;
      _offBy    = z->offBy;

      trigger = z->trigger;

      float offset = -z->ampVeltrack;
      if (offset <= 0)
            offset += 100;
      if (trigger == Trigger::CC)
            _velocity = 127;
      float curve = _velocity * _velocity / (127.0 * 127.0);

      double rt_decay_value = 1.0;
      if (trigger == Trigger::RELEASE)
            rt_decay_value = pow(10, (-z->rtDecay * durSinceNoteOn)/20);
      // the .005 in this calculation is made up like this:
      //    -> (offset + z->ampVeltrack*curve) being a percent value so
      //       this should be divided by 100 or multiplied by 0.01
      //    -> afterwards 0.5 (-6dB) is applied to compensate possible coherent
      //       signals in a stereo output see http://www.sengpielaudio.com/calculator-coherentsources.htm
      //    -> 0.005 = 0.01 * 0.5
      gain        = (z->volume * z->group_volume) * (offset + z->ampVeltrack * curve)
                    * .005 * c->gain() * rt_decay_value;

      phase.set(0);
      float sr = float(s->sampleRate()) / _zerberus->sampleRate();
      double targetcents = ((((key - z->keyBase) * z->pitchKeytrack) + z->keyBase) * 100.0) + z->tune;
      if (trigger == Trigger::CC)
            targetcents = z->keyBase * 100;
      phaseIncr.set(_zerberus->ct2hz(targetcents) * sr/_zerberus->ct2hz(z->keyBase * 100.0));

      fres = _zerberus->ct2hz(13500.0);
      if (z->isCutoffDefined) {
            //calculate current cutoff value
            float cutoffHz = z->cutoff;
            //Formula for converting the interval frequency ratio f2 / f1 to cents (c or ¢).
            //¢ or c = 1200 × log2 (f2 / f1)
            cutoffHz *= pow(2.0, _velocity / 127.0f * z->fil_veltrack / 1200.0);
            fres = cutoffHz;
            }

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

      currentEnvelope = V1Envelopes::DELAY;

      float velPercent = _velocity / 127.0;

      envelopes[V1Envelopes::DELAY].setTable(Envelope::egLin);
      envelopes[V1Envelopes::DELAY].setTime(z->ampegDelay + (z->ampegVel2Delay * velPercent) + z->delay, _zerberus->sampleRate());
      envelopes[V1Envelopes::DELAY].setConstant(0.0);

      envelopes[V1Envelopes::ATTACK].setTable(Envelope::egLin);
      envelopes[V1Envelopes::ATTACK].setVariable();
      envelopes[V1Envelopes::ATTACK].setTime(z->ampegAttack + (z->ampegVel2Attack * velPercent), _zerberus->sampleRate());
      envelopes[V1Envelopes::ATTACK].offset = z->ampegStart;

      envelopes[V1Envelopes::HOLD].setTable(Envelope::egLin);
      envelopes[V1Envelopes::HOLD].setTime(z->ampegHold + (z->ampegVel2Hold * velPercent), _zerberus->sampleRate());
      envelopes[V1Envelopes::HOLD].setConstant(1.0);

      envelopes[V1Envelopes::DECAY].setTable(Envelope::egPow);
      envelopes[V1Envelopes::DECAY].setVariable();
      envelopes[V1Envelopes::DECAY].setTime(z->ampegDecay + (z->ampegVel2Decay * velPercent), _zerberus->sampleRate());
      envelopes[V1Envelopes::DECAY].offset = z->ampegSustain;

      envelopes[V1Envelopes::SUSTAIN].setTable(Envelope::egLin);
      if (trigger == Trigger::RELEASE || trigger == Trigger::CC) {
            // Sample is played on noteoff. We need to stop the voice when it's done. Set the sustain duration accordingly.
            //in ZInstrument::readSample we create sample data array using frames*channels
            //so no need to devide by number of channels here, otherwise it reduces duration of samples by (Number of Channels)
            double sampleDur = ((double) z->sample->frames() / z->sample->sampleRate()) * 1000; // in ms
            double scaledSampleDur = sampleDur / (phaseIncr.data / 256.0);
            double sustainDur   = scaledSampleDur - (z->ampegDelay + z->ampegAttack + z->ampegHold + z->ampegDecay + z->ampegRelease + z->delay);
            envelopes[V1Envelopes::SUSTAIN].setTime(sustainDur, _zerberus->sampleRate());
            }
      else
            envelopes[V1Envelopes::SUSTAIN].setTime(std::numeric_limits<float>::infinity(), _zerberus->sampleRate());
      envelopes[V1Envelopes::SUSTAIN].setConstant(qBound(0.0f, z->ampegSustain + (z->ampegVel2Sustain * velPercent), 1.0f));

      envelopes[V1Envelopes::RELEASE].setTable(Envelope::egPow);
      envelopes[V1Envelopes::RELEASE].setVariable();
      envelopes[V1Envelopes::RELEASE].setTime(z->ampegRelease + (z->ampegVel2Release * velPercent), _zerberus->sampleRate());
      envelopes[V1Envelopes::RELEASE].max = envelopes[V1Envelopes::SUSTAIN].val;

      _looping = false;
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

      float a1_temp = 0.f;
      float a2_temp = 0.f;
      float b1_temp = 0.f;
      float b02_temp = 0.f;
      switch (z->fil_type) {
            case FilterType::lpf_2p: {
                  a1_temp = -2.0f * cos_coeff * a0_inv;
                  a2_temp = (1.0f - alpha_coeff) * a0_inv;
                  b1_temp = (1.0f - cos_coeff) * a0_inv * filter_gain;
                  // both b0 -and- b2
                  b02_temp = b1_temp * 0.5f;
                  break;
                  }
            case FilterType::hpf_2p: {
                  a1_temp = -2.0f * cos_coeff * a0_inv;
                  a2_temp = (1.0f - alpha_coeff) * a0_inv;
                  b1_temp = -(1.0f + cos_coeff) * a0_inv * filter_gain;
                  // both b0 -and- b2
                  b02_temp = -b1_temp * 0.5f;
                  break;
                  }
            default:
                  qWarning() << "fil_type is not implemented: " << (int)z->fil_type;
            }

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
//   updateEnvelopes
//---------------------------------------------------------

void Voice::updateEnvelopes() {
      if (_state == VoiceState::ATTACK && trigger != Trigger::RELEASE) {
            while (envelopes[currentEnvelope].step() && currentEnvelope != V1Envelopes::SUSTAIN)
                  currentEnvelope++;

            // triggered by noteon enter virtually infinite sustain (play state)
            if (currentEnvelope == V1Envelopes::SUSTAIN)
                  _state = VoiceState::PLAYING;
            }
      else if (_state == VoiceState::ATTACK && trigger == Trigger::RELEASE) {
            while (envelopes[currentEnvelope].step() && currentEnvelope != V1Envelopes::RELEASE)
                  currentEnvelope++;

            // triggered by noteoff stop sample when entering release
            if (currentEnvelope == V1Envelopes::RELEASE)
                  _state = VoiceState::STOP;
            }
      else if (_state == VoiceState::STOP) {
            if (envelopes[V1Envelopes::RELEASE].step()) {
                  off();
                  }
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Voice::process(int frames, float* p)
      {
      float adaptedFrequency = fres;
      int sr = _zerberus->sampleRate();
      if (adaptedFrequency > 0.45f * sr)
            adaptedFrequency = 0.45f * sr;
      else if (adaptedFrequency < 5.f)
            adaptedFrequency = 5.f;

      bool freqWasUpdated = fabs(adaptedFrequency - last_fres) > 0.01f;
      if (freqWasUpdated) {
            updateFilter(adaptedFrequency);
            last_fres = adaptedFrequency;
            }

      const float opcodePanLeftGain = 1.f - std::fmax(0.0f, z->pan / 100.0); //[0, 1]
      const float opcodePanRightGain = 1.f + std::fmin(0.0f, z->pan / 100.0); //[0, 1]
      if (audioChan == 1) {
            while (frames--) {

                  updateLoop();

                  long long idx = phase.index();

                  if (idx >= eidx) {
                        off();
                        break;
                        }
                  const float* coeffs = interpCoeff[phase.fract()];
                  float f;
                  f =  (coeffs[0] * getData(idx-1)
                      + coeffs[1] * getData(idx+0)
                      + coeffs[2] * getData(idx+1)
                      + coeffs[3] * getData(idx+2)) * gain
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

                  updateEnvelopes();
                  if (_state == VoiceState::OFF)
                        break;

                  v *= envelopes[currentEnvelope].val * z->ccGain;

                  *p++  += v * _channel->panLeftGain() * opcodePanLeftGain;
                  *p++  += v * _channel->panRightGain() * opcodePanRightGain;
                  if (V1Envelopes::DELAY != currentEnvelope)
                        phase += phaseIncr;

                  _samplesSinceStart++;
                  }
            }
      else {
            //
            // handle interleaved stereo samples
            //
            while (frames--) {

                  updateLoop();

                  long long idx = phase.index() * 2;
                  if (idx >= eidx) {
                        off();
                        // printf("end of sample\n");
                        break;
                        }

                  const float* coeffs = interpCoeff[phase.fract()];
                  float f1, f2;

                  f1 = (coeffs[0] * getData(idx-2)
                      + coeffs[1] * getData(idx)
                      + coeffs[2] * getData(idx+2)
                      + coeffs[3] * getData(idx+4))
                      * gain * _channel->panLeftGain() * z->ccGain;

                  f2 = (coeffs[0] * getData(idx-1)
                      + coeffs[1] * getData(idx+1)
                      + coeffs[2] * getData(idx+3)
                      + coeffs[3] * getData(idx+5))
                      * gain * _channel->panRightGain() * z->ccGain;

                  updateEnvelopes();
                  if (_state == VoiceState::OFF)
                        break;

                  f1 *= envelopes[currentEnvelope].val;
                  f2 *= envelopes[currentEnvelope].val;

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

                  *p++  += vl * opcodePanLeftGain;
                  *p++  += vr * opcodePanRightGain;

                  if (V1Envelopes::DELAY != currentEnvelope)
                        phase += phaseIncr;

                  _samplesSinceStart++;
                  }
            }
      }

//---------------------------------------------------------
//   updateLoop
//---------------------------------------------------------

void Voice::updateLoop()
      {
      long long idx = phase.index();
      int loopOffset = (audioChan * 3) - 1; // offset due to interpolation
      bool validLoop = _loopEnd > 0 && _loopStart >= 0 && (_loopEnd <= (eidx/audioChan));
      bool shallLoop = loopMode() == LoopMode::CONTINUOUS || (loopMode() == LoopMode::SUSTAIN && (_state < VoiceState::STOP));

      if (!(validLoop && shallLoop)) {
            _looping = false;
            return;
            }

      if (idx + loopOffset > _loopEnd)
            _looping = true;
      if (idx > _loopEnd)
            phase.setIndex(_loopStart+(idx-_loopEnd-1));
      }

short Voice::getData(long long pos) {
      if (pos < 0 && !_looping)
            return 0;

      if (!_looping)
            return data[pos];

      long long loopEnd = _loopEnd * audioChan;
      long long loopStart = _loopStart * audioChan;

      if (pos < loopStart)
            return data[loopEnd + (pos - loopStart) + audioChan];
      else if (pos > (loopEnd + audioChan - 1))
            return data[loopStart + (pos - loopEnd) - audioChan];
      else
            return data[pos];
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

const char* Voice::state() const
      {
      return voiceStateNames[int(_state)];
      }

