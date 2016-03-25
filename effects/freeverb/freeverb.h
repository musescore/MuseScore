/*
  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  modified for MuseScore Werner Schweer, 2009
*/

#ifndef _REV_H
#define _REV_H

#include "effects/effect.h"

static const int numcombs = 8;
static const int numallpasses = 4;

//---------------------------------------------------------
//   Allpass
//---------------------------------------------------------

class Allpass {
      float feedback;
      float* buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void setfeedback(float val) { feedback = val;  }
      float getfeedback() const   { return feedback; }

      float process(float _input) {
            float bufout   = buffer[bufidx];
            float output   = bufout - _input;
            buffer[bufidx] = _input + (bufout * feedback);
            ++bufidx      %= bufsize;
            return output;
            }
      };

//---------------------------------------------------------
//   Comb
//---------------------------------------------------------

class Comb {
      float feedback;
      float filterstore;
      float damp1;
      float damp2;
      float* buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void setdamp(float val);
      float getdamp() const       { return damp1;    }
      void setfeedback(float val) { feedback = val;  }
      float getfeedback() const   { return feedback; }

      float process(float input) {
            float tmp      = buffer[bufidx];
            filterstore    = (tmp * damp2) + (filterstore * damp1);
            buffer[bufidx] = input + (filterstore * feedback);
            ++bufidx      %= bufsize;
            return tmp;
            }
      };

//---------------------------------------------------------
//   Freeverb
//---------------------------------------------------------

class Freeverb : public Effect {
      //Q_OBJECT

      float roomsize, damp, width, sendLevel, wet;
      float newRoomsize, newDamp, newWidth, newSendLevel, newWet;
      float wet1, wet2;
      bool parameterChanged;

      Comb combL[numcombs];
      Comb combR[numcombs];

      Allpass allpassL[numallpasses];
      Allpass allpassR[numallpasses];

      void update();

   public:
      Freeverb();
      virtual void process(int n, float* in, float* out);

      virtual void setNValue(int idx, double value);
      virtual double nvalue(int idx) const;

      bool setPreset(int);
      virtual const char* name() const { return "Freeverb"; }
      virtual EffectGui* gui();
      virtual const std::vector<ParDescr>& parDescr() const;
      };

#endif
