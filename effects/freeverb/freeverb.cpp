/*
  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  modified for MuseScore Werner Schweer, 2009
*/

#include "stdio.h"
#include "freeverb.h"

#define DC_OFFSET 1e-8


static const float scaleroom  = 0.28f;
static const float offsetroom = 0.7f;
static const float scalewet   = 3.0f;
static const float scaledamp  = 0.4f;

//---------------------------------------------------------
//   ReverbParam
//---------------------------------------------------------

enum {
      WET = 0,
      ROOMSIZE,
      DAMP,
      WIDTH,
      SEND
      };

static const int SIZE = 5;

//---------------------------------------------------------
//   freeverbPd
//---------------------------------------------------------

static std::vector<ParDescr> freeverbPd = {
      { WET,      "wet",      false, 0.0, 1.0, 0.0 },
      { ROOMSIZE, "roomsize", false, 0.0, 1.0, 0.0 },
      { DAMP,     "damp",     false, 0.0, 1.0, 0.0 },
      { WIDTH,    "width",    false, 0.0, 1.0, 0.0 },
      { SEND,     "send",     false, 0.0, 1.0, 0.0 }
      };

//---------------------------------------------------------
//   ReverbPreset
//---------------------------------------------------------

struct ReverbPreset {
      const char* name;
      float values[SIZE];
      };

static ReverbPreset presets[] = {
      // name        wet  roomsize damp   width  send
      { "Default", { 0.2,     0.2,  1.0,  0.2,  0.1 }},
      { "Test 2",  { 0.4,     0.2,  0.5,  0.5,  0.8 }},
      { "Test 3",  { 0.6,     0.4,  0.5,  0.5,  0.7 }},
      { "Test 4",  { 0.8,     0.7,  0.5,  0.5,  0.6 }},
      { "Test 5",  { 0.8,     1.0,  0.5,  0.5,  0.5 }},
      { 0,         { 0.0,     0.0,  0.0,  0.0,  0.0 }}
      };

//---------------------------------------------------------
//   setbuffer
//---------------------------------------------------------

void Allpass::setbuffer(int size)
      {
      buffer  = new float[size];
      bufsize = size;
      bufidx  = 0;
      for (int i = 0; i < bufsize; i++)
            buffer[i] = DC_OFFSET;  // this is not 100 % correct.
      }

//---------------------------------------------------------
//   setbuffer
//---------------------------------------------------------

void Comb::setbuffer(int size)
      {
      filterstore = 0;
      bufidx      = 0;
      buffer      = new float[size];
      bufsize     = size;
      for (int i = 0; i < bufsize; i++)
            buffer[i] = DC_OFFSET;  // This is not 100 % correct.
      }

//---------------------------------------------------------
//   setdamp
//---------------------------------------------------------

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
//   Freeverb
//---------------------------------------------------------

Freeverb::Freeverb()
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
      setPreset(0);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Freeverb::process(int n, float* in, float* out)
      {
      if (parameterChanged) {
            roomsize  = newRoomsize;
            damp      = newDamp;
            width     = newWidth;
            sendLevel = newSendLevel;
            wet       = newWet;
            update();
            parameterChanged = false;
            }
      float* lp = in;
      float* rp = in+1;
      float* lo = out;
      float* ro = out+1;

      float dry = 1.0 - wet;

      for (int k = 0; k < n; k++) {
            float outL = 0.0;
            float outR = 0.0;

            float input = ((*lp + *rp) * sendLevel) + DC_OFFSET;

            for (int i = 0; i < numcombs; i++) {      // Accumulate comb filters in parallel
                  outL += combL[i].process(input);
                  outR += combR[i].process(input);
                  }
            for (int i = 0; i < numallpasses; i++) {  // Feed through allpasses in series
                  outL = allpassL[i].process(outL);
                  outR = allpassR[i].process(outR);
                  }

            // Remove the DC offset
            outL -= DC_OFFSET;
            outR -= DC_OFFSET;

            *lo = *lp * dry + (outL * wet1 + outR * wet2) * wet;
            *ro = *rp * dry + (outR * wet1 + outL * wet2) * wet;
            lp += 2;
            rp += 2;
            lo += 2;
            ro += 2;
            }
      }

//---------------------------------------------------------
//   update
//    Recalculate internal values after parameter change
//---------------------------------------------------------

void Freeverb::update()
      {
      wet1 = width * .5 + .5;
      wet2 = (1.0 - width) * .5;

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

bool Freeverb::setPreset(int nr)
      {
      if ((unsigned)nr >= sizeof(presets)/sizeof(*presets))
            return false;
      const ReverbPreset& preset = presets[nr];
      for (int i = 0; i < SIZE; ++i)
            setValue(i, preset.values[i]);
      parameterChanged = true;
      return true;
      }

//---------------------------------------------------------
//   setNValue
//---------------------------------------------------------

void Freeverb::setNValue(int idx, double value)
      {
      switch (idx) {
            case ROOMSIZE:
                  newRoomsize = value;
                  break;
            case DAMP:
                  newDamp = value;
                  break;
            case WIDTH:
                  newWidth = value;
                  break;
            case SEND:
                  newSendLevel = value;
                  break;
            case WET:
                  newWet = value;
                  break;
            }
      parameterChanged = true;
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

double Freeverb::nvalue(int idx) const
      {
      float val = 0.0;
      switch (idx) {
            case ROOMSIZE: val = newRoomsize;  break;
            case DAMP:     val = newDamp;      break;
            case WIDTH:    val = newWidth;     break;
            case SEND:     val = newSendLevel; break;
            case WET:      val = newWet;       break;
            }
      return val; // (val - parameters[idx].offset) / parameters[idx].scale;
      }

//---------------------------------------------------------
//   parDescr
//---------------------------------------------------------

const std::vector<ParDescr>& Freeverb::parDescr() const
      {
      return freeverbPd;
      }

