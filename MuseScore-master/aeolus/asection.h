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


#ifndef __ASECTION_H
#define __ASECTION_H

#include "global.h"

using namespace Ms;

#define PERIOD 64
#define MIXLEN 64
#define NCHANN 4
#define NRANKS 32

//---------------------------------------------------------
//   Diffuser
//---------------------------------------------------------

class Diffuser {
      float     *_data;
      int        _size;
      int        _i;
      float      _c;

   public:
      void init(int size, float c);
      void fini();
      int  size() { return _size; }
      float process(float x) {
            float w = x - _c * _data [_i];
            x = _data [_i] + _c * w;
            _data [_i] = w;
            if (++_i == _size)
                  _i = 0;
            return x;
            }
      };

//---------------------------------------------------------
//   Asection
//---------------------------------------------------------

class Asection {
      enum { AZIMUTH, STWIDTH, DIRECT, REFLECT, REVERB };

      int      _offs0;
      int      _offs [16];
      float    _fsam;
      float   *_base;
      float    _sw;
      float    _sx;
      float    _sy;
      Diffuser _dif0;
      Diffuser _dif1;
      Diffuser _dif2;
      Diffuser _dif3;
      SyntiParameter _apar [5];

   public:
      Asection (float fsam);
      ~Asection ();

      float *get_wptr () { return _base + _offs0; }
      SyntiParameter *get_apar () { return _apar; }
      void set_size (float size);
      void process (float vol, float *W, float *X, float *Y, float *R);

      static float _refl [16];
      };

#endif

