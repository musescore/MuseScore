// -----------------------------------------------------------------------
//  Copyright (C) 2003-2010 Fons Adriaensen <fons@linuxaudio.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// -----------------------------------------------------------------------

#include <math.h>
#include "zita.h"

namespace Ms {

enum {
      R_DELAY, R_XOVER, R_RTLOW, R_RTMID, R_FDAMP,
      R_EQ1FR, R_EQ1GN,
      R_EQ2FR, R_EQ2GN,
      R_OPMIX
      };

static const std::vector<ParDescr> pd = {
      { R_DELAY, "delay", false,  0.02,        0.100,          0.04  },
      { R_XOVER, "xover", true,   logf(50.0),  logf(1000.0), 200.0 },
      { R_RTLOW, "rtlow", true,   logf(1.0),   logf(8.0),    3.0   },
      { R_RTMID, "rtmid", true,   logf(1.0),   logf(8.0),    2.0   },
      { R_FDAMP, "fdamp", true,   logf(1.5e3), logf(24.0e3), 6.0e3 },

      { R_EQ1FR, "eq1fr", true,   logf(40.0),  logf(2.5e3), 160.0 },
      { R_EQ1GN, "eq1gn", false, -15.0,        15.0,        0.0   },

      { R_EQ2FR, "eq2fr", true,   logf(160.0), logf(10e3),  2.5e3 },
      { R_EQ2GN, "eq2gn", false, -15.0,        15.0,        0.0   },

      { R_OPMIX, "opmix", false,  0.0,         1.0,         0.5   }
      };

//---------------------------------------------------------
//   Pareq
//---------------------------------------------------------

Pareq::Pareq()
   :
    _touch0 (0),
    _touch1 (0),
    _state (BYPASS),
    _g0 (1),
    _g1 (1),
    _f0 (1e3f),
    _f1 (1e3f)
      {
      setfsamp(0.0f);
      }

//---------------------------------------------------------
//   setfsamp
//---------------------------------------------------------

void Pareq::setfsamp(float fsamp)
      {
      _fsamp = fsamp;
      reset();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Pareq::reset()
      {
      memset (_z1, 0, sizeof (float) * MAXCH);
      memset (_z2, 0, sizeof (float) * MAXCH);
      }

//---------------------------------------------------------
//   prepare
//---------------------------------------------------------

void Pareq::prepare(int nsamp)
      {
      bool  upd = false;

      if (_touch1 != _touch0) {
            float g = _g0;
            float f = _f0;
            if (g != _g1) {
                  upd = true;
                  if (g > 2 * _g1)
                       _g1 *= 2;
                  else if (_g1 > 2 * g)
                        _g1 /= 2;
                  else
                        _g1 = g;
                  }
            if (f != _f1) {
                  upd = true;
                  if (f > 2 * _f1)
                        _f1 *= 2;
                  else if (_f1 > 2 * f)
                        _f1 /= 2;
                  else
                        _f1 = f;
                  }
            if (upd)  {
                  if ((_state == BYPASS) && (_g1 == 1)) {
                        calcpar1 (0, _g1, _f1);
                        }
                  else {
                        _state = SMOOTH;
                        calcpar1 (nsamp, _g1, _f1);
                        }
                  }
            else {
                  _touch1 = _touch0;
                  if (fabs (_g1 - 1) < 0.001f) {
                        _state = BYPASS;
                        reset ();
                        }
                  else {
                        _state = STATIC;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   calcpar1
//---------------------------------------------------------

void Pareq::calcpar1(int nsamp, float g, float f)
      {
      f *= float (M_PI) / _fsamp;

      float b = 2 * f / sqrtf (g);
      float gg = 0.5f * (g - 1);
      float c1 = -cosf (2 * f);
      float c2 = (1 - b) / (1 + b);
      if (nsamp) {
            _dc1 = (c1 - _c1) / nsamp + 1e-30f;
            _dc2 = (c2 - _c2) / nsamp + 1e-30f;
            _dgg = (gg - _gg) / nsamp + 1e-30f;
            }
      else {
            _c1 = c1;
            _c2 = c2;
            _gg = gg;
            }
      }

//---------------------------------------------------------
//   process1
//---------------------------------------------------------

void Pareq::process1(int nsamp, float* data)
      {
      int nchan = 2;
      float c1 = _c1;
      float c2 = _c2;
      float gg = _gg;

      if (_state == SMOOTH) {
            for (int i = 0; i < nchan; i++) {
                  float z1 = _z1 [i];
                  float z2 = _z2 [i];
                  c1       = _c1;
                  c2       = _c2;
                  gg = _gg;
                  for (int j = 0; j < nsamp; j++) {
                        c1 += _dc1;
                        c2 += _dc2;
                        gg += _dgg;
                        float* p = data + j * 2 + i;
                        float x = *p;
                        float y = x - c2 * z2;
                        *p = x - gg * (z2 + c2 * y - x);
                        y -= c1 * z1;
                        z2 = z1 + c1 * y;
                        z1 = y + 1e-20f;
                        }
                  _z1 [i] = z1;
                  _z2 [i] = z2;
                  }
            _c1 = c1;
            _c2 = c2;
            _gg = gg;
            }
      else {
            for (int i = 0; i < nchan; i++) {
                  float z1 = _z1 [i];
                  float z2 = _z2 [i];
                  for (int j = 0; j < nsamp; j++) {
                        float* p = data + j * 2 + i;
                        float x = *p;
                        float y = x - c2 * z2;
                        *p = x - gg * (z2 + c2 * y - x);
                        y -= c1 * z1;
                        z2 = z1 + c1 * y;
                        z1 = y + 1e-20f;
                        }
                  _z1 [i] = z1;
                  _z2 [i] = z2;
                  }
            }
      }

Diff1::~Diff1()
      {
      fini();
      }

void Diff1::init (int size, float c)
      {
      _size = size;
      _line = new float [size];
      memset (_line, 0, size * sizeof (float));
      _i = 0;
      _c = c;
      }

void Diff1::fini()
      {
      delete[] _line;
      _size = 0;
      _line = 0;
      }

Delay::Delay()
   : _size (0), _line (0)
      {
      }

Delay::~Delay()
      {
      fini();
      }

void Delay::init (int size)
      {
      _size = size;
      _line = new float [size];
      memset (_line, 0, size * sizeof (float));
      _i = 0;
      }

void Delay::fini ()
      {
      delete[] _line;
      _size = 0;
      _line = 0;
      }

Vdelay::Vdelay ()
   : _size (0), _line (0)
      {
      }

Vdelay::~Vdelay ()
      {
      fini();
      }

void Vdelay::init (int size)
      {
      _size = size;
      _line = new float [size];
      memset (_line, 0, size * sizeof (float));
      _ir = 0;
      _iw = 0;
      }

void Vdelay::fini ()
      {
      delete[] _line;
      _size = 0;
      _line = 0;
      }

void Vdelay::set_delay (int del)
      {
      _ir = _iw - del;
      if (_ir < 0)
            _ir += _size;
      }

void Filt1::set_params (float del, float tmf, float tlo, float wlo, float thi, float chi)
      {
      _gmf = powf (0.001f, del / tmf);
      _glo = powf (0.001f, del / tlo) / _gmf - 1.0f;
      _wlo = wlo;
      float g    = powf (0.001f, del / thi) / _gmf;
      float t    = (1 - g * g) / (2 * g * g * chi);
      _whi = (sqrtf (1 + 4 * t) - 1) / (2 * t);
      }

float ZitaReverb::_tdiff1 [8] = {
      20346e-6f,
      24421e-6f,
      31604e-6f,
      27333e-6f,
      22904e-6f,
      29291e-6f,
      13458e-6f,
      19123e-6f
      };

float ZitaReverb::_tdelay [8] = {
      153129e-6f,
      210389e-6f,
      127837e-6f,
      256891e-6f,
      174713e-6f,
      192303e-6f,
      125000e-6f,
      219991e-6f
      };

ZitaReverb::~ZitaReverb ()
      {
      fini();
      }

void ZitaReverb::init(float fsamp)
      {
      _fsamp = fsamp;
      _cntA1 = 1;
      _cntA2 = 0;
      _cntB1 = 1;
      _cntB2 = 0;
      _cntC1 = 1;
      _cntC2 = 0;

      _ipdel = 0.04f;
      _xover = 200.0f;
      _rtlow = 3.0f;
      _rtmid = 2.0f;
      _fdamp = 3e3f;
      _opmix = 0.5f;

      _g0 = _d0 = 0;
      _g1 = _d1 = 0;

      _vdelay0.init ((int)(0.1f * _fsamp));
      _vdelay1.init ((int)(0.1f * _fsamp));
      for (int i = 0; i < 8; i++) {
            int k1 = (int)(floorf (_tdiff1 [i] * _fsamp + 0.5f));
            int k2 = (int)(floorf (_tdelay [i] * _fsamp + 0.5f));
            _diff1 [i].init (k1, (i & 1) ? -0.6f : 0.6f);
            _delay [i].init (k2 - k1);
            }

      _pareq1.setfsamp(fsamp);
      _pareq2.setfsamp(fsamp);
      _pareq1.setparam(160.0, 0.0);
      _pareq2.setparam(2.5e3, 0.0);

      _fragm = 1024;
      _nsamp = 0;
      }


void ZitaReverb::fini ()
      {
      for (int i = 0; i < 8; i++)
            _delay [i].fini ();
      }

//---------------------------------------------------------
//   prepare
//---------------------------------------------------------

void ZitaReverb::prepare (int nfram)
      {
      int a = _cntA1;
      int b = _cntB1;
      int c = _cntC1;

      _d0 = _d1 = 0;

      if (a != _cntA2) {
            int k = (int)(floorf ((_ipdel - 0.020f) * _fsamp + 0.5f));
            _vdelay0.set_delay (k);
            _vdelay1.set_delay (k);
            _cntA2 = a;
            }

      if (b != _cntB2) {
            float wlo = 6.2832f * _xover / _fsamp;
            float chi;
            if (_fdamp > 0.49f * _fsamp)
                  chi = 2;
            else
                  chi = 1 - cosf (6.2832f * _fdamp / _fsamp);
            for (int i = 0; i < 8; i++) {
                  _filt1 [i].set_params (_tdelay [i], _rtmid, _rtlow, wlo, 0.5f * _rtmid, chi);
                  }
            _cntB2 = b;
            }

      if (c != _cntC2) {
            float t0 = (1 - _opmix) * (1 + _opmix);
            float t1 = 0.7f * _opmix * (2 - _opmix) / sqrtf (_rtmid);
            _d0 = (t0 - _g0) / nfram;
            _d1 = (t1 - _g1) / nfram;
            _cntC2 = c;
            }

      _pareq1.prepare (nfram);
      _pareq2.prepare (nfram);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void ZitaReverb::process (int nfram, float* inp, float* out)
      {
      float t, g, x0, x1, x2, x3, x4, x5, x6, x7;
      g = sqrtf (0.125f);


      while (nfram) {
            if (!_nsamp) {
                  prepare(_fragm);
                  _nsamp = _fragm;
                  }

            int k = _nsamp < nfram ? _nsamp : nfram;

            float* p0 = inp;
            float* p1 = inp + 1;
            float* q0 = out;
            float* q1 = out + 1;

            for (int i = 0; i < k * 2; i += 2) {
                  _vdelay0.write (p0 [i]);
                  _vdelay1.write (p1 [i]);

                  t = 0.3f * _vdelay0.read ();
                  x0 = _diff1 [0].process (_delay [0].read () + t);
                  x1 = _diff1 [1].process (_delay [1].read () + t);
                  x2 = _diff1 [2].process (_delay [2].read () - t);
                  x3 = _diff1 [3].process (_delay [3].read () - t);
                  t = 0.3f * _vdelay1.read ();
                  x4 = _diff1 [4].process (_delay [4].read () + t);
                  x5 = _diff1 [5].process (_delay [5].read () + t);
                  x6 = _diff1 [6].process (_delay [6].read () - t);
                  x7 = _diff1 [7].process (_delay [7].read () - t);

                  t = x0 - x1; x0 += x1;  x1 = t;
                  t = x2 - x3; x2 += x3;  x3 = t;
                  t = x4 - x5; x4 += x5;  x5 = t;
                  t = x6 - x7; x6 += x7;  x7 = t;
                  t = x0 - x2; x0 += x2;  x2 = t;
                  t = x1 - x3; x1 += x3;  x3 = t;
                  t = x4 - x6; x4 += x6;  x6 = t;
                  t = x5 - x7; x5 += x7;  x7 = t;
                  t = x0 - x4; x0 += x4;  x4 = t;
                  t = x1 - x5; x1 += x5;  x5 = t;
                  t = x2 - x6; x2 += x6;  x6 = t;
                  t = x3 - x7; x3 += x7;  x7 = t;

                  _g1 += _d1;

                  q0 [i] = _g1 * (x1 + x2);
                  q1 [i] = _g1 * (x1 - x2);

                  _delay [0].write (_filt1 [0].process (g * x0));
                  _delay [1].write (_filt1 [1].process (g * x1));
                  _delay [2].write (_filt1 [2].process (g * x2));
                  _delay [3].write (_filt1 [3].process (g * x3));
                  _delay [4].write (_filt1 [4].process (g * x4));
                  _delay [5].write (_filt1 [5].process (g * x5));
                  _delay [6].write (_filt1 [6].process (g * x6));
                  _delay [7].write (_filt1 [7].process (g * x7));
                  }
            _pareq1.process (k, out);
            _pareq2.process (k, out);

            for (int i = 0; i < k; i++) {
                  *out++ += _g0 * *inp++;
                  *out++ += _g0 * *inp++;
                  _g0 += _d0;
                  }
            nfram  -= k;
            _nsamp -= k;
            }
      }

void ZitaReverb::setNValue(int idx, double value)
      {
      switch (idx) {
            case R_DELAY: set_delay(value); break;
            case R_XOVER: set_xover(value); break;
            case R_RTLOW: set_rtlow(value); break;
            case R_RTMID: set_rtmid(value); break;
            case R_FDAMP: set_fdamp(value); break;
            case R_EQ1FR: set_eq1fr(value); break;
            case R_EQ1GN: set_eq1gn(value); break;
            case R_EQ2FR: set_eq2fr(value); break;
            case R_EQ2GN: set_eq2gn(value); break;
            case R_OPMIX: set_opmix(value); break;
            }
      }

//---------------------------------------------------------
//   value
//    return normalized value 0-1.0
//---------------------------------------------------------

double ZitaReverb::nvalue(int idx) const
      {
      double v = 0.0;
      switch (idx) {
            case R_DELAY: v = delay(); break;
            case R_XOVER: v = xover(); break;
            case R_RTLOW: v = rtlow(); break;
            case R_RTMID: v = rtmid(); break;
            case R_FDAMP: v = fdamp(); break;
            case R_EQ1FR: v = eq1fr(); break;
            case R_EQ1GN: v = eq1gn(); break;
            case R_EQ2FR: v = eq2fr(); break;
            case R_EQ2GN: v = eq2gn(); break;
            case R_OPMIX: v = opmix(); break;
            }
      return v;
      }

//---------------------------------------------------------
//   parDescr
//---------------------------------------------------------

const std::vector<ParDescr>& ZitaReverb::parDescr() const
      {
      return pd;
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SynthesizerGroup ZitaReverb::state() const
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

void ZitaReverb::setState(const SynthesizerGroup& g)
      {
      for (const IdValue& v : g)
            setValue(v.id, v.data.toDouble());
      }
}

