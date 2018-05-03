//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include <math.h>

#include "compressor.h"

namespace Ms {

#define f_round(f) lrintf(f)

#define DB_TABLE_SIZE 1024
#define DB_MIN -60.0f
#define DB_MAX 24.0f
#define LIN_TABLE_SIZE 1024
#define LIN_MIN 0.0000000002f
#define LIN_MAX 9.0f

enum CompressorParameter {
      RMS_PEAK, ATTACK, RELEASE, THRESHOLD, RATIO, KNEE, GAIN
      };

//---------------------------------------------------------
//   ParDescr
//---------------------------------------------------------

static const std::vector<ParDescr> pd = {
      { RMS_PEAK,  "rms",       false, 0.0,   1.0,   0.0 },
      { ATTACK,    "attack",    false, 1.5,   400.0, 1.5 },
      { RELEASE,   "release",   false, 2.0,   800.0, 2.0 },
      { THRESHOLD, "threshold", false, -30.0, 0.0,   0.0 },
      { RATIO,     "ratio",     false, 1.0,   20.0,  1.0 },
      { KNEE,      "knee",      false, 1.0,   10.0,  1.0 },
      { GAIN,      "gain",      false, 0.0,   24.0,  0.0 }
      };
#if 0
//---------------------------------------------------------
//   cube_interp
//    Cubic interpolation function
//---------------------------------------------------------

static inline float cube_interp(const float fr, const float inm1, const float
   in, const float inp1, const float inp2)
      {
      return in + 0.5f * fr * (inp1 - inm1 +
       fr * (4.0f * inp1 + 2.0f * inm1 - 5.0f * in - inp2 +
       fr * (3.0f * (in - inp1) - inm1 + inp2)));
      }
#endif
//---------------------------------------------------------
//   f_max
//---------------------------------------------------------

static inline float f_max(float x, float a)
      {
      x -= a;
      x += fabs(x);
      x *= 0.5;
      x += a;

      return x;
      }

//---------------------------------------------------------
//   round_to_zero
//---------------------------------------------------------

static inline void round_to_zero(volatile float *f)
      {
      *f += 1e-18f;
      *f -= 1e-18f;
      }

// Linearly interpolate [ = a * (1 - f) + b * f]
#define LIN_INTERP(f,a,b) ((a) + (f) * ((b) - (a)))

#ifdef DB_DEFAULT_CUBE
#define db2lin(a) f_db2lin_cube(a)
#define lin2db(a) f_lin2db_cube(a)
#else
#define db2lin(a) f_db2lin_lerp(a)
#define lin2db(a) f_lin2db_lerp(a)
#endif

float db_data[DB_TABLE_SIZE];
float lin_data[LIN_TABLE_SIZE];
#if 0
//---------------------------------------------------------
//   f_db2lin_cube
//---------------------------------------------------------

static inline float f_db2lin_cube(float db)
      {
      float scale = (db - DB_MIN) * (float)LIN_TABLE_SIZE / (DB_MAX - DB_MIN);
      int base = f_round(scale - 0.5f);
      float ofs = scale - base;

      if (base < 1) {
            return 0.0f;
      } else if (base > LIN_TABLE_SIZE - 3) {
            return lin_data[LIN_TABLE_SIZE - 2];
      }
      return cube_interp(ofs, lin_data[base-1], lin_data[base], lin_data[base+1], lin_data[base+2]);
      }
#endif
//---------------------------------------------------------
//   f_db2lin_lerp
//---------------------------------------------------------

static inline float f_db2lin_lerp(float db)
      {
      float scale = (db - DB_MIN) * (float)LIN_TABLE_SIZE / (DB_MAX - DB_MIN);
      int base = f_round(scale - 0.5f);
      float ofs = scale - base;

      if (base < 1) {
            return 0.0f;
            }
      else if (base > LIN_TABLE_SIZE - 3) {
            return lin_data[LIN_TABLE_SIZE - 2];
            }
      return (1.0f - ofs) * lin_data[base] + ofs * lin_data[base+1];
      }
#if 0
//---------------------------------------------------------
//   f_lin2db_cube
//---------------------------------------------------------

static inline float f_lin2db_cube(float lin)
      {
      float scale = (lin - LIN_MIN) * (float)DB_TABLE_SIZE / (LIN_MAX - LIN_MIN);
      int base = f_round(scale - 0.5f);
      float ofs = scale - base;

      if (base < 2) {
            return db_data[2] * scale * 0.5f - 23 * (2.0f - scale);
      } else if (base > DB_TABLE_SIZE - 3) {
            return db_data[DB_TABLE_SIZE - 2];
      }
      return cube_interp(ofs, db_data[base-1], db_data[base], db_data[base+1], db_data[base+2]);
      }
#endif
//---------------------------------------------------------
//   f_lin2db_lerp
//---------------------------------------------------------

static inline float f_lin2db_lerp(float lin)
      {
      float scale = (lin - LIN_MIN) * (float)DB_TABLE_SIZE / (LIN_MAX - LIN_MIN);
      int base = f_round(scale - 0.5f);
      float ofs = scale - base;

      if (base < 2) {
            return db_data[2] * scale * 0.5f - 23.0f * (2.0f - scale);
      } else if (base > DB_TABLE_SIZE - 2) {
            return db_data[DB_TABLE_SIZE - 1];
      }
      return (1.0f - ofs) * db_data[base] + ofs * db_data[base+1];
      }

//---------------------------------------------------------
//   db_init
//---------------------------------------------------------

void db_init()
      {
      for (int i=0; i<LIN_TABLE_SIZE; i++) {
            lin_data[i] = powf(10.0f, ((DB_MAX - DB_MIN) *
                  (float)i/(float)LIN_TABLE_SIZE + DB_MIN) / 20.0f);
            }

      for (int i=0; i<DB_TABLE_SIZE; i++) {
            db_data[i] = 20.0f * log10f((LIN_MAX - LIN_MIN) *
                  (float)i/(float)DB_TABLE_SIZE + LIN_MIN);
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Compressor::init(float sr)
      {
      sampleRate = sr;
      sum        = 0.0f;
      amp        = 0.0f;
      gain       = 0.0f;
      gain_t     = 0.0f;
      env        = 0.0f;
      env_rms    = 0.0f;
      env_peak   = 0.0f;
      count      = 0;
      as[0]      = 1.0f;
      for (int i = 0; i < A_TBL; ++i)
            as[i] = expf(-1.0f / (sampleRate * (float)i / (float)A_TBL));
      db_init();
      }

//---------------------------------------------------------
//   Compressor::process
//---------------------------------------------------------

void Compressor::process(int frames, float* ip, float *op)
      {
      const float ga       = _attack < 2.0f ? 0.0f : as[f_round(_attack * 0.001f * (float)(A_TBL-1))];
      const float gr       = as[f_round(_release * 0.001f * (float)(A_TBL-1))];
      const float rs       = (_ratio - 1.0f) / _ratio;
      const float mug      = db2lin(_makeupGain);
      const float knee_min = db2lin(_threshold - _knee);
      const float knee_max = db2lin(_threshold + _knee);
      const float ef_a     = ga * 0.25f;
      const float ef_ai    = 1.0f - ef_a;

      for (int pos = 0; pos < frames; pos++) {
            const float la = fabs(ip[pos * 2]);
            const float ra = fabs(ip[pos * 2 + 1]);
            const float lev_in = f_max(la, ra);

            sum += lev_in * lev_in;
            if (amp > env_rms)
                  env_rms = env_rms * ga + amp * (1.0f - ga);
            else
                  env_rms = env_rms * gr + amp * (1.0f - gr);
            round_to_zero(&env_rms);
            if (lev_in > env_peak)
                  env_peak = env_peak * ga + lev_in * (1.0f - ga);
            else
                  env_peak = env_peak * gr + lev_in * (1.0f - gr);
            round_to_zero(&env_peak);
            if ((count++ & 3) == 3) {
                  amp = rms.process(sum * 0.25f);
                  sum = 0.0f;
                  if (qIsNaN(env_rms))     // This can happen sometimes, but I don't know why
                        env_rms = 0.0f;
                  env = LIN_INTERP(rms_peak, env_rms, env_peak);
                  if (env <= knee_min)
                        gain_t = 1.0f;
                  else if (env < knee_max) {
                        const float x = -(_threshold - _knee - lin2db(env)) / _knee;
                        gain_t = db2lin(-_knee * rs * x * x * 0.25f);
                        }
                  else
                        gain_t = db2lin((_threshold - lin2db(env)) * rs);
                  }
            gain          = gain * ef_a + gain_t * ef_ai;
            op[pos * 2]   = ip[pos * 2] * gain * mug;
            op[pos * 2+1] = ip[pos * 2 + 1] * gain * mug;
            }

//      printf("gain %f\n", gain);

//      amplitude = lin2db(env);
//      gain_red  = lin2db(gain);
      }

//---------------------------------------------------------
//   setNValue
//---------------------------------------------------------

void Compressor::setNValue(int idx, double value)
      {
      switch (idx) {
            case RMS_PEAK:  setRmsPeak(value);    break;
            case ATTACK:    setAttack(value);     break;
            case RELEASE:   setRelease(value);    break;
            case THRESHOLD: setThreshold(value);  break;
            case RATIO:     setRatio(value);      break;
            case KNEE:      setKnee(value);       break;
            case GAIN:      setMakeupGain(value); break;
            }
      }

//---------------------------------------------------------
//   value
//    return normalized value 0-1.0
//---------------------------------------------------------

double Compressor::nvalue(int idx) const
      {
      double v = 0.0;
      switch (idx) {
            case RMS_PEAK:  v = rmsPeak();    break;
            case ATTACK:    v = attack();     break;
            case RELEASE:   v = release();    break;
            case THRESHOLD: v = threshold();  break;
            case RATIO:     v = ratio();      break;
            case KNEE:      v = knee();       break;
            case GAIN:      v = makeupGain(); break;
            }
      return v;
      }

//---------------------------------------------------------
//   parDescr
//---------------------------------------------------------

const std::vector<ParDescr>& Compressor::parDescr() const
      {
      return pd;
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerGroup Compressor::state() const
      {
      SynthesizerGroup g;
      g.setName(name());

      for (const ParDescr& d : pd)
            g.push_back(IdValue(d.id, QString("%1").arg(value(d.id))));
      return g;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Compressor::setState(const SynthesizerGroup& g)
      {
      for (const IdValue& v : g)
            setValue(v.id, v.data.toDouble());
      }
}

