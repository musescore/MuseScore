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


#ifndef __RNGEN_H
#define __RNGEN_H


#include "prbsgen.h"


class Rngen
{
public:

    Rngen (void);

    void init (uint32_t seed);

    uint32_t irand (void)
    {
        uint32_t r;

        if (++_i == 55) _i = 0;
        if (_i < 24) r = _a [_i] += _a [_i + 31];
        else         r = _a [_i] += _a [_i - 24];
        return r;
    }

    double  urand (void) { return irand () / _p32; }
    double  grand (void);
    void    grand (double *x, double *y);
    float   urandf (void) { return irand () / _p32f; }
    float   grandf (void);
    void    grandf (float  *x, float *y);

    ~Rngen (void);
    Rngen (const Rngen&);           // disabled, not to be used
    Rngen& operator=(const Rngen&); // disabled, not to be used

private:
  
    uint32_t  _a [55];
    int       _i;
    bool      _md;
    bool      _mf;
    double    _vd;
    float     _vf;

    static const double  _p31;
    static const double  _p32;
    static const float   _p31f;
    static const float   _p32f;
};


#endif
