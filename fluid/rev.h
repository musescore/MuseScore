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


#ifndef _FLUID_REV_H
#define _FLUID_REV_H

namespace FluidS {

static const int numcombs = 8;
static const int numallpasses = 4;

//---------------------------------------------------------
//   Allpass
//---------------------------------------------------------

class Allpass {
      float feedback;
      float *buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void init();
      void setfeedback(float val) { feedback = val;  }
      float getfeedback() const   { return feedback; }

      float process(float _input) {
            float bufout = buffer[bufidx];
            float output = bufout - _input;
            buffer[bufidx] = _input + (bufout * feedback);
            if (++bufidx >= bufsize)
                  bufidx = 0;
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
      float *buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void init();
      void setdamp(float val);
      float getdamp() const       { return damp1;    }
      void setfeedback(float val) { feedback = val;  }
      float getfeedback() const   { return feedback; }

      float process(float input) {
            float tmp = buffer[bufidx];
            filterstore      = (tmp * damp2) + (filterstore * damp1);
            buffer[bufidx]   = input + (filterstore * feedback);
            if (++bufidx >= bufsize)
                  bufidx = 0;
            return tmp;
            }
      };

static const float scaleroom  = 0.28f;
static const float offsetroom = 0.7f;
static const float scalewet   = 3.0f;
static const float scaledamp  = 0.4f;

//---------------------------------------------------------
//   Reverb
//---------------------------------------------------------

class Reverb {
      void init();
      void update();

      float roomsize;
      float damp;
      float width;
      float gain;
      float wet, wet1, wet2;

      float newRoomsize;
      float newDamp;
      float newWidth;
      float newGain;
      bool parameterChanged;

      /*
       The following are all declared inline
       to remove the need for dynamic allocation
       with its subsequent error-checking messiness
       */

      Comb combL[numcombs];
      Comb combR[numcombs];

      Allpass allpassL[numallpasses];
      Allpass allpassR[numallpasses];

   public:
      Reverb();
      void process(int n, float* in, float* left_out, float* right_out);

      void reset() { init(); }

      void setroomsize(float value) { roomsize = (value * scaleroom) + offsetroom; }
      void setdamp(float value)     { damp = value * scaledamp;  }
      void setlevel(float value)    { wet  = value * scalewet;   }
      void setLevel(float value)    { setlevel(value); update(); }
      void setwidth(float value)    { width = value;             }
      void setmode(float value);
      float getroomsize() const     { return (roomsize - offsetroom) / scaleroom; }
      float getdamp() const         { return damp / scaledamp; }
      float getlevel() const        { return wet / scalewet;   }
      float getwidth()              { return width;            }

      void setParameter(int parameter, double value);
      double parameter(int idx) const;

      bool setPreset(int);
      };

//---------------------------------------------------------
//   ReverbPreset
//---------------------------------------------------------

struct ReverbPreset {
      const char* name;
      float roomsize;
      float damp;
      float width;
      float level;
      };
}
#endif /* _FLUID_REV_H */
