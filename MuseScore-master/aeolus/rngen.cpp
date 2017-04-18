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


#include <time.h>
#include "rngen.h"


const double Rngen::_p31 = 2147483648.0;
const double Rngen::_p32 = 4294967296.0;
const float Rngen::_p31f = 2147483648.0f;
const float Rngen::_p32f = 4294967296.0f;


Rngen::Rngen (void)
{
    init (0);
}


Rngen::~Rngen (void)
{
}


void Rngen::init (uint32_t s)
{
    int i, j;
    Prbsgen G;

    if (s == 0) s = time (0);

    G.set_poly (Prbsgen::G32);
    G.set_stat (s);

    for (i = 0; i < 55; i++)
    {
	G.step ();
	j = G.stat () & 4095;
	while (j--) G.step ();
	_a [i] = G.stat ();
    }

    _i = 0;
    _md = false;
    _mf = false;
}


double Rngen::grand (void)
{
    double a, b, r;

    if (_md)
    {
	_md = false;
	return _vd;
    }

    do
    {
	a = irand () / _p31 - 1.0;
	b = irand () / _p31 - 1.0;
	r = a * a + b * b;
    }
    while ((r > 1.0) || (r < 1e-20));

    r = sqrt (-2.0 * log (r) / r);
    _md = true;
    _vd = r * b;

    return r * a;
}


void Rngen::grand (double *x, double *y)
{
    double a, b, r;

    do
    {
	a = irand () / _p31 - 1.0;
	b = irand () / _p31 - 1.0;
	r = a * a + b * b;
    }
    while ((r > 1.0) || (r < 1e-20));

    r = sqrt (-log (r) / r);
    *x = r * a;
    *y = r * b;
}


float Rngen::grandf (void)
{
    float a, b, r;

    if (_mf)
    {
	_mf = false;
	return _vf;
    }

    do
    {
	a = irand () / _p31f - 1.0f;
	b = irand () / _p31f - 1.0f;
	r = a * a + b * b;
    }
    while ((r > 1.0f) || (r < 1e-20f));

    r = sqrtf (-2.0f * logf (r) / r);
    _mf = true;
    _vf = r * b;

    return r * a;
}


void Rngen::grandf (float *x, float *y)
{
    float a, b, r;

    do
    {
	a = irand () / _p31f - 1.0f;
	b = irand () / _p31f - 1.0f;
	r = a * a + b * b;
    }
    while ((r > 1.0f) || (r < 1e-20f));

    r = sqrtf (-logf (r) / r);
    *x = r * a;
    *y = r * b;
}
