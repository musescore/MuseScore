// -----------------------------------------------------------------------
//  Copyright (C) 2003-2011 Fons Adriaensen <fons@linuxaudio.org>
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

#ifndef __ZITA_H__
#define __ZITA_H__

#include "effects/effect.h"

namespace Ms {

class EffectGui;

//---------------------------------------------------------
//   Pareq
//---------------------------------------------------------

class Pareq
      {
      enum { BYPASS, STATIC, SMOOTH, MAXCH = 4 };

      void calcpar1 (int nsamp, float g, float f);
      void process1 (int nsamp, float*);

      volatile int16_t  _touch0;
      volatile int16_t  _touch1;
#if 0 // not yet (?) used
      bool              _bypass;
#endif
      int               _state;
      float             _fsamp;

      float             _g;
      float             _g0, _g1;
      float             _f;
      float             _f0, _f1;
      float             _c1, _dc1;
      float             _c2, _dc2;
      float             _gg, _dgg;

      float             _z1 [MAXCH];
      float             _z2 [MAXCH];

   public:
      Pareq();

      void setfsamp(float fsamp);
      void setparam(float f, float g) {
            _f  = f;
            _g  = g;
            _f0 = f;
            _g0 = powf (10.0f, 0.05f * g);
            _touch0++;
            }
      void set_gn(float g) { setparam(_f, g); }
      float gn() const     { return _g;       }
      void set_fr(float f) { setparam(f, _g); }
      float fr() const     { return _f;       }

      void reset();
      void prepare(int nsamp);
      void process(int nsamp, float* data) {
            if (_state != BYPASS)
                 process1(nsamp, data);
            }
      };

//---------------------------------------------------------
//   Diff1
//---------------------------------------------------------

class Diff1
      {
      friend class ZitaReverb;

      int     _i;
      float   _c;
      int     _size = 0;
      float* _line = 0;

      Diff1() {}
      ~Diff1();
      void  init(int size, float c);
      void  fini();

      float process(float x) {
            float z = _line [_i];
            x -= _c * z;
            _line [_i] = x;
            if (++_i == _size)
                  _i = 0;
            return z + _c * x;
            }
      };

//---------------------------------------------------------
//   Filt1
//---------------------------------------------------------

class Filt1
      {
      friend class ZitaReverb;

      Filt1 () : _slo (0), _shi (0) {}
      ~Filt1 () {}

      void  set_params (float del, float tmf, float tlo, float wlo, float thi, float chi);

      float process(float x) {
            _slo += _wlo * (x - _slo) + 1e-10f;
            x += _glo * _slo;
            _shi += _whi * (x - _shi);
            return _gmf * _shi;
            }
      float   _gmf;
      float   _glo;
      float   _wlo;
      float   _whi;
      float   _slo;
      float   _shi;
      };

//---------------------------------------------------------
//   Delay
//---------------------------------------------------------

class Delay
      {
      friend class ZitaReverb;

      Delay();
      ~Delay();

      void  init (int size);
      void  fini ();

      float read () { return _line [_i]; }

      void write (float x) {
            _line [_i++] = x;
            if (_i == _size)
                  _i = 0;
            }
      int     _i;
      int     _size;
      float  *_line;
      };

//---------------------------------------------------------
//   Vdelay
//---------------------------------------------------------

class Vdelay
      {
      friend class ZitaReverb;

      Vdelay();
      ~Vdelay();

      void  init (int size);
      void  fini ();
      void  set_delay (int del);

      float read () {
            float x = _line [_ir++];
            if (_ir == _size)
                  _ir = 0;
            return x;
            }

      void write (float x) {
            _line [_iw++] = x;
            if (_iw == _size)
                  _iw = 0;
            }
      int     _ir;
      int     _iw;
      int     _size;
      float* _line;
      };

//---------------------------------------------------------
//   ZitaReverb
//---------------------------------------------------------

class ZitaReverb : public Effect
      {
      Q_OBJECT

      float   _fsamp;

      Vdelay  _vdelay0;
      Vdelay  _vdelay1;
      Diff1   _diff1[8];
      Filt1   _filt1[8];
      Delay   _delay[8];

      volatile int _cntA1;
      volatile int _cntB1;
      volatile int _cntC1;
      int     _cntA2;
      int     _cntB2;
      int     _cntC2;

      float   _ipdel;
      float   _xover;
      float   _rtlow;
      float   _rtmid;
      float   _fdamp;
      float   _opmix;
      float   _rgxyz;

      float   _g0, _d0;
      float   _g1, _d1;

      Pareq   _pareq1;
      Pareq   _pareq2;

      static float _tdiff1 [8];
      static float _tdelay [8];

      int _fragm;
      int _nsamp;

      void prepare(int n);

   public:
      ZitaReverb() : Effect() {}
      ~ZitaReverb();

      virtual void init(float fsamp);
      void fini();

      virtual void process(int n, float* inp, float* out);

      void set_delay(float v) { _ipdel = v; _cntA1++; }
      float delay() const     { return _ipdel; }

      void set_xover(float v) { _xover = v; _cntB1++; }
      float xover() const     { return _xover; }

      void set_rtlow(float v) { _rtlow = v; _cntB1++; }
      float rtlow() const     { return _rtlow; }

      void set_rtmid(float v) { _rtmid = v; _cntB1++; _cntC1++; }
      float rtmid() const     { return _rtmid; }

      void set_fdamp(float v) { _fdamp = v; _cntB1++; }
      float fdamp() const     { return _fdamp; }

      void set_eq1fr(float f) { _pareq1.set_fr(f); }
      float eq1fr() const     { return _pareq1.fr(); }

      void set_eq1gn(float f) { _pareq1.set_gn(f); }
      float eq1gn() const     { return _pareq1.gn(); }

      void set_eq2fr(float f) { _pareq2.set_fr(f); }
      float eq2fr() const     { return _pareq2.fr(); }

      void set_eq2gn(float f) { _pareq2.set_gn(f);   }
      float eq2gn() const     { return _pareq2.gn(); }

      void set_opmix(float v) { _opmix = v; _cntC1++; }
      float opmix() const     { return _opmix; }

      virtual const char* name() const { return "Zita1"; }
      virtual EffectGui* gui();
      virtual const std::vector<ParDescr>& parDescr() const;

      virtual void setNValue(int parameter, double value);
      virtual double nvalue(int idx) const;

      virtual SynthesizerGroup state() const;
      virtual void setState(const SynthesizerGroup&);
      };
}

#endif

