/*
  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  Translated to C by Peter Hanappe, Mai 2001
  Further modified by Werner Schweer, 2009
*/

#include "rev.h"
#include "fluid.h"

namespace FluidS {

#define DC_OFFSET 1e-8

static ReverbPreset revmodel_preset[] = {
      // name           roomsize       damp      width        level
      { "Default",         0.5f,      0.5f,       1.0f,       0.2f },
      { "Test 2",          0.4f,      0.2f,       0.5f,       0.8f },
      { "Test 3",          0.6f,      0.4f,       0.5f,       0.7f },
      { "Test 4",          0.8f,      0.7f,       0.5f,       0.6f },
      { "Test 5",          0.8f,      1.0f,       0.5f,       0.5f },
      { 0, 0.0f, 0.0f, 0.0f, 0.0f }
      };

//---------------------------------------------------------
//   setbuffer
//---------------------------------------------------------

void Allpass::setbuffer(int size)
      {
      buffer  = new float[size];
      bufsize = size;
      bufidx  = 0;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Allpass::init()
      {
      for (int i = 0; i < bufsize; i++)
            buffer[i] = DC_OFFSET;  // this is not 100 % correct.
      }

void Comb::setbuffer(int size)
      {
      filterstore = 0;
      bufidx      = 0;
      buffer      = new float[size];
      bufsize     = size;
      }

void Comb::init()
      {
      for (int i = 0; i < bufsize; i++)
            buffer[i] = DC_OFFSET;  // This is not 100 % correct.
      }

void Comb::setdamp(float val)
      {
      damp1 = val;
      damp2 = 1 - val;
      }

static const int stereospread = 23;

/*
 These values assume 44.1KHz sample rate
 they will probably be OK for 48KHz sample rate
 but would need scaling for 96KHz (or other) sample rates.
 The values were obtained by listening tests.
*/

static const int combtuning[] = {
      1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617
      };

static const int allpasstuning[] = {
      556, 441, 341, 225
      };

//---------------------------------------------------------
//   Reverb
//---------------------------------------------------------

Reverb::Reverb()
      {
      for (int i = 0; i < numcombs; ++i) {
            combL[i].setbuffer(combtuning[i]);
            combR[i].setbuffer(combtuning[i] + stereospread);
            }

      for (int i = 0; i < numallpasses; ++i) {
            allpassL[i].setbuffer(allpasstuning[i]);
            allpassR[i].setbuffer(allpasstuning[i] + stereospread);
            allpassL[i].setfeedback(0.5f);
            allpassR[i].setfeedback(0.5f);
            }
      gain = 0.30;      // input gain
      setPreset(0);
      init();           // Clear all buffers
      parameterChanged = false;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Reverb::init()
      {
      for (int i = 0; i < numcombs; i++) {
            combL[i].init();
            combR[i].init();
            }
      for (int i = 0; i < numallpasses; i++) {
            allpassL[i].init();
            allpassR[i].init();
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Reverb::process(int n, float* in, float* l, float* r)
      {
      if (parameterChanged) {
            roomsize = newRoomsize;
            damp     = newDamp;
            width    = newWidth;
            gain     = newGain;
            update();
            parameterChanged = false;
            }
      for (int k = 0; k < n; k++) {
            float outL = 0.0;
            float outR = 0.0;

            float input = (in[k] * 2.0 + DC_OFFSET) * gain;

            for (int i = 0; i < numcombs; i++) {      // Accumulate comb filters in parallel
                  outL += combL[i].process(input);
                  outR += combR[i].process(input);
                  }
            for (int i = 0; i < numallpasses; i++) {  // Feed through allpasses in series
                  outL = allpassL[i].process(outL);
                  outR = allpassR[i].process(outR);
                  }

            /* Remove the DC offset */
            outL -= DC_OFFSET;
            outR -= DC_OFFSET;

            /* Calculate output MIXING with anything already there */
            l[k] += outL * wet1 + outR * wet2;
            r[k] += outR * wet1 + outL * wet2;
            }
      }

//---------------------------------------------------------
//   update
//    Recalculate internal values after parameter change
//---------------------------------------------------------

void Reverb::update()
      {
      wet1 = wet * (width * .5 + 0.5f);
      wet2 = wet * ((1.0 - width) * .5);

      for (int i = 0; i < numcombs; i++) {
            combL[i].setfeedback(roomsize);
            combR[i].setfeedback(roomsize);
            }
      for (int i = 0; i < numcombs; i++) {
            combL[i].setdamp(damp);
            combR[i].setdamp(damp);
            }
      }

//---------------------------------------------------------
//   setPreset
//---------------------------------------------------------

bool Reverb::setPreset(int nr)
      {
      if ((unsigned)nr >= sizeof(revmodel_preset)/sizeof(*revmodel_preset))
            return false;
      setroomsize(revmodel_preset[nr].roomsize);
      setdamp(revmodel_preset[nr].damp);
      setwidth(revmodel_preset[nr].width);
      setlevel(revmodel_preset[nr].level);
      newRoomsize = roomsize;
      newDamp     = damp;
      newWidth    = width;

      update();
      return true;
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void Reverb::setParameter(int idx, double value)
      {
// printf("Reverb:setParameter %d: %f\n", idx, value);
      switch (idx) {
            case REVERB_ROOMSIZE: newRoomsize = value; break;
            case REVERB_DAMP:     newDamp     = value; break;
            case REVERB_WIDTH:    newWidth    = value; break;
            case REVERB_GAIN:     newGain     = value; break;
            default:
                  printf("Reverb:setParameter: %x invalid\n", idx);
                  break;
            }
      parameterChanged = true;
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

double Reverb::parameter(int idx) const
      {
      double val = 0.0;
      switch (idx) {
            case REVERB_ROOMSIZE: val = newRoomsize; break;
            case REVERB_DAMP:     val = newDamp; break;
            case REVERB_WIDTH:    val = newWidth; break;
            case REVERB_GAIN:     val = newGain; break;
            default:
                  break;
            }
// printf("Reverb:parameter %d: %f\n", idx, val);
      return val;
      }


}
