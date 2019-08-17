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
      gain        = (z->volume * z->group_volume * z->global_volume) * (offset + z->ampVeltrack * curve)
                    * .005 * c->gain() * rt_decay_value *
                    pow(10.0, 4.5 / 20.0); //attenuated volume between Fluid and Zerberus on 4.5dB

      phase.set(0);
      float sr = float(s->sampleRate()) / _zerberus->sampleRate();
      double targetcents = ((((key - z->keyBase) * z->pitchKeytrack) + z->keyBase) * 100.0) + z->tune;
      if (trigger == Trigger::CC)
            targetcents = z->keyBase * 100;
      phaseIncr.set(_zerberus->ct2hz(targetcents) * sr/_zerberus->ct2hz(z->keyBase * 100.0));

      filter.initialize(_zerberus, z, _velocity);

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
      filter.update();

      const float opcodePanLeftGain = 1.f - std::fmax(0.0f, z->pan / 100.0); //[0, 1]
      const float opcodePanRightGain = 1.f + std::fmin(0.0f, z->pan / 100.0); //[0, 1]
      const float leftChannelVol = gain * z->ccGain * _channel->panLeftGain() * opcodePanLeftGain;
      const float rightChannelVol = gain * z->ccGain * _channel->panRightGain() * opcodePanRightGain;
      if (audioChan == 1) {
            while (frames--) {

                  updateLoop();

                  long long idx = phase.index();

                  if (idx >= eidx) {
                        off();
                        break;
                        }

                  float interpVal = filter.interpolate(phase.fract(),
                                                       getData(idx-1), getData(idx), getData(idx+1), getData(idx+2));
                  float v = filter.apply(interpVal, true);

                  updateEnvelopes();
                  if (_state == VoiceState::OFF)
                        break;

                  *p++  += v * envelopes[currentEnvelope].val * leftChannelVol;
                  *p++  += v * envelopes[currentEnvelope].val * rightChannelVol;

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
//printf("end of sample\n");
                        break;
                        }

                  float interpValL = filter.interpolate(phase.fract(),
                                                       getData(idx-2), getData(idx), getData(idx+2), getData(idx+4));
                  float interpValR = filter.interpolate(phase.fract(),
                                                       getData(idx-1), getData(idx+1), getData(idx+3), getData(idx+5));
                  float valueL = filter.apply(interpValL, true);
                  float valueR = filter.apply(interpValR, false);

                  //apply volume
                  updateEnvelopes();
                  if (_state == VoiceState::OFF)
                        break;

                  *p++  += valueL * envelopes[currentEnvelope].val * leftChannelVol;
                  *p++  += valueR * envelopes[currentEnvelope].val * rightChannelVol;

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

