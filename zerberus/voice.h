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

#ifndef __MVOICE_H__
#define __MVOICE_H__

#include <cstdint>
#include <math.h>
#include "filter.h"

class Channel;
struct Zone;
class Sample;
class Zerberus;

enum class LoopMode : char;
enum class OffMode : char;
enum class Trigger : char;

static const int EG_SIZE    = 256;

//---------------------------------------------------------
//   Envelope
//---------------------------------------------------------

struct Envelope {
      static float egPow[EG_SIZE];
      static float egLin[EG_SIZE];

      int steps, count;
      bool constant = false;
      float offset = 0.0;
      float max = 1.0;
      float val;
      float* table;

      void setTable(float* f) { table = f; }
      bool step() {
            if (count) {
                  --count;
                  if (!constant)
                        val = table[EG_SIZE * count/steps]*(max-offset)+offset;
                  return false;
                  }
            else
                  return true;
            }
      void setTime(float ms, int sampleRate);
      void setConstant(float v) { constant = true; val = v; }
      void setVariable()        { constant = false; }
      };

//-----------------------------------------------------------------------------
//   Phase
//    Playing pointer for voice playback
//
//    When a sample is played back at a different pitch, the playing pointer
//    in the source sample will not advance exactly one sample per output sample.
//
//    This playing pointer is implemented using Phase.
//-----------------------------------------------------------------------------

struct Phase {
      union {
            int64_t data;
            struct {
                  uint8_t _fract;
                  };
            };

      void operator+=(const Phase& p) { data += p.data;   }
      void set(int b)                 { data = b * 256;   }
      void set(double b)              { data = b * 256.0; }
      void setIndex(int b)            { data = b * 256 + _fract; }
      int index() const               { return data >> 8; }
      unsigned fract() const          { return _fract;    }

      Phase() {}
      Phase(int64_t v) : data(v) {}
      };

enum class VoiceState : char {
      OFF,
      ATTACK,
      PLAYING,
      SUSTAINED,
      STOP
      };

enum V1Envelopes : int {
      DELAY,
      ATTACK,
      HOLD,
      DECAY,
      SUSTAIN,
      RELEASE,
      COUNT
      };

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice {
      Voice* _next;
      Zerberus* _zerberus;

      VoiceState _state = VoiceState::OFF;
      Channel* _channel;
      int _key;
      int _velocity;
      int audioChan;

      short* data;
      long long eidx;
      LoopMode _loopMode;
      OffMode _offMode;
      int _offBy;
      long long _loopStart;
      long long _loopEnd;
      bool _looping;
      int _samplesSinceStart;

      float gain;

      Phase phase, phaseIncr;

      ZFilter filter;

      int currentEnvelope;
      Envelope envelopes[V1Envelopes::COUNT];

      Trigger trigger;

      const Zone* z;

   public:
      Voice(Zerberus*);
      Voice* next() const         { return _next; }
      void setNext(Voice* v)      { _next = v; }

      void start(Channel* channel, int key, int velo, const Zone*, double durSinceNoteOn);
      void updateEnvelopes();
      void process(int frames, float*);
      void updateLoop();
      short getData(long long pos);

      Channel* channel() const    { return _channel; }
      int key() const             { return _key;     }
      int velocity() const        { return _velocity; }

      bool isPlaying() const      { return _state == VoiceState::PLAYING || _state == VoiceState::ATTACK;   }
      bool isSustained() const    { return _state == VoiceState::SUSTAINED; }
      bool isOff() const          { return _state == VoiceState::OFF; }
      bool isStopped() const      { return _state == VoiceState::STOP; }
      void stop()                 { envelopes[currentEnvelope].step(); envelopes[V1Envelopes::RELEASE].max = envelopes[currentEnvelope].val; currentEnvelope = V1Envelopes::RELEASE; _state = VoiceState::STOP;      }
      void stop(float time);
      void sustained()            { _state = VoiceState::SUSTAINED; }
      void off()                  { _state = VoiceState::OFF;       }
      const char* state() const;
      LoopMode loopMode() const   { return _loopMode; }
      int getSamplesSinceStart()  { return _samplesSinceStart;    }
      float getGain()             { return gain; }

      OffMode offMode() const     { return _offMode;  }
      int offBy() const           { return _offBy;    }
      static void init();
      };

#endif

