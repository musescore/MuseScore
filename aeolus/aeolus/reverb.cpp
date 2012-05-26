/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "reverb.h"


void Delelm::init (int size, float fb)
      {
      _size = size;
      _line = new float[size];
      memset(_line, 0, size * sizeof (float));
      _i   = 0;
      _fb  = fb;
      _slo = 0;
      _shi = 0;
      }

void Delelm::fini ()
      {
      delete[] _line;
      }

void Delelm::set_t60mf (float tmf)
      {
      _gmf = powf (0.001f, _size / tmf);
      }

void Delelm::set_t60lo (float tlo, float wlo)
      {
      _glo = powf (0.001f, _size / tlo) / _gmf - 1.0f;
      _wlo = wlo;
      }

void Delelm::set_t60hi (float thi, float chi)
      {
      float g = powf (0.001f, _size / thi) / _gmf;
      float t = (1 - g * g) / (2 * g * g * chi);
      _whi = (sqrt (1 + 4 * t) - 1) / (2 * t);
      }

float Delelm::process(float x)
      {
      float t = _line [_i] * _gmf;
      _slo += _wlo * (t - _slo);
      t += _glo * _slo;
      _shi += _whi * (t - _shi);
      t = x - _fb * _shi + 1e-10;
      _line [_i] = t;
      if (++_i == _size)
            _i = 0;
      return _shi + _fb * t;
      }

void Delelm::print()
      {
      printf ("%5d %6.3lf   %5.3lf %5.3lf   %6.4lf %6.4lf\n",
         _size, _fb, _glo, _gmf, _wlo, _whi);
      }

int Reverb::_sizes [16] =
      {
      839,  6732 -  839,
      1181,  7339 - 1181,
      1229,  8009 - 1229,
      2477,  8731 - 2477,
      2731,  9521 - 2731,
      1361, 10381 - 1361,
      3203, 11321 - 3203,
      1949, 12347 - 1949
      };

float Reverb::_feedb [16] =
      {
      -0.6f,  0.1f,
      0.6f,  0.1f,
      0.6f,  0.1f,
      -0.6f,  0.1f,
      0.6f,  0.1f,
      -0.6f,  0.1f,
      -0.6f,  0.1f,
      0.6f,  0.1f
      };

void Reverb::init(float rate)
      {
      _rate = rate;
      _size = (int)(0.15f * rate);
      _line = new float [_size];
      memset (_line, 0, _size * sizeof (float));
      _i = 0;
      int m = (rate < 64e3) ? 1 : 2;
      for (int i = 0; i < 16; i++)
            _delm [i].init (m * _sizes [i], _feedb [i]);
      _x0 = _x1 = _x2 = _x3 = _x4 = _x5 = _x6 = _x7 = _z = 0;
      set_delay (0.05);
      set_t60mf (4.0f);
      set_t60lo (5.0f, 250.0f);
      set_t60hi (2.0f, 4e3f);
      }

void Reverb::fini()
      {
      delete[] _line;
      for (int i = 0; i < 16; i++)
            _delm [i].fini();
      }

void Reverb::set_delay (float del)
      {
      if (del < 0.01f)
            del = 0.01f;
      _idel = (int)(_rate * del);
      if (_idel > _size)
            _idel = _size;
      }

void Reverb::set_t60mf (float tmf)
      {
      _tmf = tmf;
      float t = tmf * _rate;
      for (int i = 0; i < 16; i++)
            _delm [i].set_t60mf (t);
      _gain = 1.0f / sqrtf (tmf);
      }

void Reverb::set_t60lo (float tlo, float flo)
      {
      _tlo = tlo;
      _flo = flo;
      float t = tlo * _rate;
      float w = 2 * M_PI * flo / _rate;
      for (int i = 0; i < 16; i++)
            _delm [i].set_t60lo (t, w);
      }

void Reverb::set_t60hi(float thi, float fhi)
      {
      _thi = thi;
      _fhi = fhi;
      float t = thi * _rate;
      float c = 1 - cosf (2 * M_PI * fhi / _rate);
      for (int i = 0; i < 16; i++)
            _delm [i].set_t60hi (t, c);
      }

void Reverb::print()
      {
      for (int i = 0; i < 16; i++)
            _delm [i].print ();
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Reverb::process(int n, float gain, float* R, float *W, float *X, float *Y)
      {
      float g = sqrtf (0.125f);
      gain *= _gain;

      while (n--) {
            int j = _i - _idel;
            if (j < 0)
                  j += _size;
            float x = _line [j];
            _z += 0.6f * (*R++ - _z) + 1e-10f;
            _line[_i] = _z;
            if (++_i == _size)
                  _i = 0;

            _x0 = _delm  [0].process (g * _x0 + x);
            _x1 = _delm  [2].process (g * _x1 + x);
            _x2 = _delm  [4].process (g * _x2 + x);
            _x3 = _delm  [6].process (g * _x3 + x);
            _x4 = _delm  [8].process (g * _x4 + x);
            _x5 = _delm [10].process (g * _x5 + x);
            _x6 = _delm [12].process (g * _x6 + x);
            _x7 = _delm [14].process (g * _x7 + x);

            float t;
            t = _x0 - _x1; _x0 += _x1;  _x1 = t;
            t = _x2 - _x3; _x2 += _x3;  _x3 = t;
            t = _x4 - _x5; _x4 += _x5;  _x5 = t;
            t = _x6 - _x7; _x6 += _x7;  _x7 = t;

            t = _x0 - _x2; _x0 += _x2;  _x2 = t;
            t = _x1 - _x3; _x1 += _x3;  _x3 = t;
            t = _x4 - _x6; _x4 += _x6;  _x6 = t;
            t = _x5 - _x7; _x5 += _x7;  _x7 = t;

            t = _x0 - _x4; _x0 += _x4;  _x4 = t;
            t = _x1 - _x5; _x1 += _x5;  _x5 = t;
            t = _x2 - _x6; _x2 += _x6;  _x6 = t;
            t = _x3 - _x7; _x3 += _x7;  _x7 = t;

            *W++ += 1.25f * gain * _x0;
            *X++ += gain * (_x1 - 0.05f * _x2);
            *Y++ += gain * _x2;

            _x0 = _delm  [1].process (_x0);
            _x1 = _delm  [3].process (_x1);
            _x2 = _delm  [5].process (_x2);
            _x3 = _delm  [7].process (_x3);
            _x4 = _delm  [9].process (_x4);
            _x5 = _delm [11].process (_x5);
            _x6 = _delm [13].process (_x6);
            _x7 = _delm [15].process (_x7);
            }
      }

