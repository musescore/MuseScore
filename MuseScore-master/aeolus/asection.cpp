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


#include "asection.h"

extern float exp2ap (float);


#define N (MIXLEN * PERIOD)


void Diffuser::init (int size, float c)
{
    _size = size;
    _data = new float [size];
    memset (_data, 0, size * sizeof (float));
    _i = 0;
    _c = c;
}


void Diffuser::fini (void)
{
    delete[] _data;
}


float Asection::_refl [16] =
{
    0.250f, 0.440f, 0.615f, 0.940f,
    0.200f, 0.370f, 0.560f, 0.900f,
    0.230f, 0.400f, 0.500f, 0.920f,
    0.280f, 0.460f, 0.690f, 0.960f
};


Asection::Asection (float fsam) : _fsam (fsam)
      {
      _base = new float [NCHANN * N];
      memset (_base, 0, NCHANN * N * sizeof (float));

      _offs0 = 0;
      _sw = _sx = _sy = 0.0f;
      _dif0.init ((int)(fsam * 0.017f), 0.5f);
      _dif1.init ((int)(fsam * 0.029f), 0.5f);
      _dif2.init ((int)(fsam * 0.023f), 0.5f);
      _dif3.init ((int)(fsam * 0.013f), 0.5f);

      _apar [AZIMUTH].set("azimuth", 0.0f, -0.5f,  0.5f);
      _apar [STWIDTH].set("stwidth", 0.8f,  0.0f,  1.0f);
      _apar [DIRECT].set("direct",   0.56f, 0.00f, 1.00f);
      _apar [REFLECT].set("reflect", 0.25f, 0.00f, 1.00f);
      _apar [REVERB].set("reverb",   0.32f, 0.00f, 1.00f);
      }

Asection::~Asection()
{
    delete[] _base;
    _dif0.fini ();
    _dif1.fini ();
    _dif2.fini ();
    _dif3.fini ();
}


void Asection::set_size (float time)
{
    int   i, d;

    float r = (time * _fsam);
    if (r > N - PERIOD)
            r = N - PERIOD;
    for (i = 0; i < 16; i++)
    {
        d = (int)(r * _refl [i]);
        d = (_offs0 - d * PERIOD) & (N - 1);
        _offs [i] = d + (i >> 2) * N;
    }
}


//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Asection::process (float vol, float *W, float *X, float *Y, float *R)
      {
      float x[PERIOD];
      float y[PERIOD];

      float gw = vol * _apar [DIRECT].fval();
      float g = 0.45f * _apar [STWIDTH].fval();
      float s = 0.5f + g * (1 - g);
      float d = g - 0.5f;
      float gx1 = gw * (s - d);
      float gy1 = gw * (s + d);
      g = 0.25f * _apar [STWIDTH].fval();
      s = 0.5f + g * (1 - g);
      d = g - 0.5f;
      float gx2 = gw * (s - d);
      float gy2 = gw * (s + d);
      float* p = _base + _offs0;
      float gr = 0.5f * _apar [REVERB].fval();

      for (int i = 0; i < PERIOD; i++) {
            float t0 = p [0 * N];
            float t1 = p [1 * N];
            float t2 = p [2 * N];
            float t3 = p [3 * N];
            p++;
            s = t0 + t1 + t2 + t3;
            R [i] += gr * s;
            W [i] += gw * s;
            x [i] = gx1 * (t3 + t0) + gx2 * (t2 + t1);
            y [i] = gy1 * (t3 - t0) + gy2 * (t2 - t1);
            }

      gr = vol * _apar [REFLECT].fval();
      p = _base;

      for (int i = 0; i < PERIOD; i++) {
            float t0 = _dif0.process (p[_offs[1]] + p[_offs[5]] + p [_offs [11]] + p [_offs [15]] + 1e-20f);
            float t1 = _dif1.process (p[_offs[0]] + p[_offs[4]] + p [_offs [10]] + p [_offs [14]] + 1e-20f);
            float t2 = _dif2.process (p[_offs[2]] + p[_offs[6]] + p [_offs  [8]] + p [_offs [12]] + 2e-20f);
            float t3 = _dif3.process (p[_offs[3]] + p[_offs[7]] + p [_offs  [9]] + p [_offs [13]] + 2e-20f);
            p++;
            s = t0 + t1 + t2 + t3;
            _sw += 0.5f * (s - _sw);
            _sx += 0.5f * (0.4f * (t0 + t3) + 0.6f * (t2 + t1) - _sx);
            _sy += 0.5f * (0.9f * (t0 - t3) + 0.8f * (t2 - t1) - _sy);
            W [i] += gr * _sw;
            x [i] += gr * _sx;
            y [i] += gr * _sy;
            }

      g = 6.283184f * _apar [AZIMUTH].fval();
      gx1 = cosf (g);
      gy1 = sinf (g);
      for (int i = 0; i < PERIOD; i++) {
            X [i] += gx1 * x [i] + gy1 * y [i];
            Y [i] += gx1 * y [i] - gy1 * x [i];
            }
      _offs0 = (_offs0 + PERIOD) & (N - 1);
      for (int i = 0; i < 16; i++)
            _offs [i] = ((_offs [i] + PERIOD) & (N - 1)) + (i >> 2) * N;
      p = _base + _offs0;
      memset(p + 0 * N, 0, PERIOD * sizeof(float));
      memset(p + 1 * N, 0, PERIOD * sizeof(float));
      memset(p + 2 * N, 0, PERIOD * sizeof(float));
      memset(p + 3 * N, 0, PERIOD * sizeof(float));
      }

