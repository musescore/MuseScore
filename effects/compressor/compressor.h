//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include "effects/effect.h"

namespace Ms {

#define RMS_BUF_SIZE 64
static const int A_TBL = 256;

//---------------------------------------------------------
//   RmsEnv
//---------------------------------------------------------

class RmsEnv {
      float        buffer[RMS_BUF_SIZE];
      unsigned int pos;
      float        sum;

   public:
      RmsEnv() {
            for (int i=0; i<RMS_BUF_SIZE; i++)
                  buffer[i] = 0.0f;
            pos = 0;
            sum = 0.0f;
            }
      float process(const float x) {
            sum -= buffer[pos];
            sum += x;
            if (sum < 1.0e-6)
                  sum = 0.0f;
            buffer[pos] = x;
            pos = (pos + 1) & (RMS_BUF_SIZE - 1);
            return sqrt(sum / (float)RMS_BUF_SIZE);
            }
      };

//---------------------------------------------------------
//   Compressor
//---------------------------------------------------------

class Compressor : public Effect
      {
      Q_OBJECT

      float sampleRate;
      RmsEnv rms;
      float sum;
      float amp;
      float gain;
      float gain_t;
      float env;
      float env_rms;
      float env_peak;
      unsigned int count;
      float as[A_TBL];

      // The blanace between the RMS and peak envelope followers.
      // RMS is generally better for subtle, musical compression and peak is better for heavier,
      // fast compression and percussion.
      // <range min="0" max="1"/>
      float rms_peak { .5 };

      // Attack time (ms)
      //The attack time in milliseconds.
      // <range min="1.5" max="400"/>
      float _attack   {1.5 };   // 1.5 - 400 (ms)

      // <port label="release" dir="input" type="control" hint="default_middle">
      // Release time (ms)
      // The release time in milliseconds.
      // <range min="2" max="800"/>
      float _release = 400;

      // <port label="threshold" dir="input" type="control" hint="default_maximum">
      // Threshold level (dB)
      // The point at which the compressor will start to kick in.
      // <range min="-30" max="0"/>
      float _threshold = -10;

      // Ratio (1:n)
      // The gain reduction ratio used when the signal level exceeds the threshold.
      // <range min="1" max="20"/>
      float _ratio = 5;

      // <port label="knee" dir="input" type="control" hint="default_low">
      // Knee radius (dB)
      // The distance from the threshold where the knee curve starts.
      // <range min="1" max="10"/>
      float _knee = 1.0;

      // Amplitude (dB)
      // The level of the input signal, in decibels.
      // <range min="-40" max="+12"/>
//      float amplitude = 0.0;

      // Gain reduction (dB)
      // The degree of gain reduction applied to the input signal, in decibels.
      // <range min="-24" max="0"/>
//      float gain_red = -20;

      float _makeupGain = 1.0;

      void setRmsPeak(float v)    { rms_peak = v;       }
      void setAttack(float v)     { _attack = v;        }
      void setRelease(float v)    { _release = v;       }
      void setThreshold(float v)  { _threshold = v;     }
      void setRatio(float v)      { _ratio = v;         }
      void setKnee(float v)       { _knee = v;          }
      void setMakeupGain(float v) { _makeupGain = v;    }

      float rmsPeak() const       { return rms_peak;    }
      float attack() const        { return _attack;     }
      float release() const       { return _release;    }
      float threshold() const     { return _threshold;  }
      float ratio() const         { return _ratio;      }
      float knee() const          { return _knee;       }
      float makeupGain() const    { return _makeupGain; }

   public:
      virtual void init(float fsamp);
      virtual void process(int n, float* inp, float* out);
      virtual const char* name() const { return "SC4"; }
      virtual EffectGui* gui();
      virtual const std::vector<ParDescr>& parDescr() const;

      virtual void setNValue(int parameter, double value);
      virtual double nvalue(int idx) const;

      virtual SynthesizerGroup state() const;
      virtual void setState(const SynthesizerGroup&);
      };

}

#endif

