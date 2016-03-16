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


#include <math.h>
#include <string.h>
#include "cs_chorus.h"



Ladspa_CS_chorus1::Ladspa_CS_chorus1 (unsigned long fsam) : LadspaPlugin (fsam)
{
    _size = (unsigned long)(ceil (30 * fsam / 1000.0)) + 64;
    _size = (_size >> 6) << 6; 
    _line = new float [_size + 1];
}


Ladspa_CS_chorus1::~Ladspa_CS_chorus1 (void)
{
    delete[] _line;
}


void Ladspa_CS_chorus1::setport (unsigned long port, LADSPA_Data *data)
{
  _port [port] = data;
}


void Ladspa_CS_chorus1::active (bool act)
{
    unsigned int i;

    if (act)
    {
	_wi = _gi = 0;
	_x1 = _x2 = 1;
	_y1 = _y2 = 0;
        memset (_line, 0, (_size + 1) * sizeof (float));
	for (i = 0; i < 3; i++) _ri [i] = _dr [i] = 0;
    }
}


void Ladspa_CS_chorus1::runproc (unsigned long len, bool add)
{
    unsigned long i, k, wi;
    int   j;
    float *p0, *p1;
    float t, x, y;
    p0 = _port [INPUT];    
    p1 = _port [OUTPUT];    
    
    wi = _wi;
    do
    {
        if (_gi == 0)
	{
	    _gi = 64;

            t = 402.12f * _port [FREQ1][0] / _fsam;
            x = _x1 - t * _y1; 
            y = _y1 + t * _x1;
            t = sqrtf (x * x + y * y);
            _x1 = x / t;
            _y1 = y / t;

            t = 402.12f * _port [FREQ2][0] / _fsam;
            x = _x2 - t * _y2; 
            y = _y2 + t * _x2;
            t = sqrtf (x * x + y * y);
            _x2 = x / t;
            _y2 = y / t;

            x = _port [TMOD1][0] * _x1 + _port [TMOD2][0] * _x2;
            y = _port [TMOD1][0] * _y1 + _port [TMOD2][0] * _y2;
            
            _dr [0] = x;
            _dr [1] = -0.500f * x + 0.866f * y;
            _dr [2] = -0.500f * x - 0.866f * y;

            for (j = 0; j < 3; j++)
	    {
                t = _port [DELAY][0] + _dr [j];
                if (t <  0) t =  0;
                if (t > 30) t = 30;
                t *= _fsam / 1000.0f;
                _dr [j] = (t - _ri [j]) / 64;
	    }
	}

        k = (_gi < len) ? _gi : len;
        _gi -= k;
        len -= k;
   
        while (k--)
	{
            _line [++wi] = *p0++;
	    y = 0;
	    for (j = 0; j < 3; j++)
	    {
		x = wi - _ri [j];
		_ri [j] += _dr [j];
		if (x < 0) x += _size;
		i = (int)(floorf (x));
		x -= i;
		y += (1 - x) * _line [i] + x * _line [i + 1];
	    }
	    y *= 0.333f;
	    if (add) *p1++ += y * _gain;
	    else     *p1++  = y;

	}
	if (wi == _size) _line [wi = 0] = _line [_size];
    }
    while (len);

    _wi = wi;
}




Ladspa_CS_chorus2::Ladspa_CS_chorus2 (unsigned long fsam) : LadspaPlugin (fsam)
{
    _size = (unsigned long)(ceil (30 * fsam / 500.0)) + 192;
    _size = (_size >> 6) << 6; 
    _line = new float [_size + 1];
}


Ladspa_CS_chorus2::~Ladspa_CS_chorus2 (void)
{
    delete[] _line;
}


void Ladspa_CS_chorus2::setport (unsigned long port, LADSPA_Data *data)
{
  _port [port] = data;
}


void Ladspa_CS_chorus2::active (bool act)
{
    unsigned int i;

    if (act)
    {
	_wi = _gi = 0;
	_x1 = _x2 = 1;
	_y1 = _y2 = 0;
	_a = _b = 0;
        memset (_line, 0, (_size + 1) * sizeof (float));
	for (i = 0; i < 3; i++) _ri [i] = _dr [i] = 0;
    }
}


void Ladspa_CS_chorus2::runproc (unsigned long len, bool add)
{
    unsigned long i, k, wi;
    int   j;
    float *p0, *p1;
    float a, b, t, x, y;

    p0 = _port [INPUT];    
    p1 = _port [OUTPUT];    
    
    wi = _wi;
    a = _a;
    b = _b;
    do
    {
        if (_gi == 0)
	{
	    _gi = 64;

            t = 402.12f * _port [FREQ1][0] / _fsam;
            x = _x1 - t * _y1; 
            y = _y1 + t * _x1;
            t = sqrtf (x * x + y * y);
            _x1 = x / t;
            _y1 = y / t;

            t = 402.12f * _port [FREQ2][0] / _fsam;
            x = _x2 - t * _y2; 
            y = _y2 + t * _x2;
            t = sqrtf (x * x + y * y);
            _x2 = x / t;
            _y2 = y / t;

            x = _port [TMOD1][0] * _x1 + _port [TMOD2][0] * _x2;
            y = _port [TMOD1][0] * _y1 + _port [TMOD2][0] * _y2;
            
            _dr [0] = x;
            _dr [1] = -0.500f * x + 0.866f * y;
            _dr [2] = -0.500f * x - 0.866f * y;

            for (j = 0; j < 3; j++)
	    {
                t = _port [DELAY][0] + _dr [j];
                if (t <  0) t =  0;
                if (t > 30) t = 30;
                t *= _fsam / 500.0f;
                _dr [j] = (t - _ri [j]) / 64;
	    }
	}

        k = (_gi < len) ? _gi : len;
        _gi -= k;
        len -= k;
       
        while (k--)
	{
            x = *p0++ + 0.52f * a - 0.25f * b;
            _line [++wi] = 0.5f * (x + b) + a;
            b = a;
            a = x;
            x = 0.52f * a - 0.25f * b;
            _line [++wi] = 0.5f * (x + b) + a;
            b = a;
            a = x;

            y = 0;
	    for (j = 0; j < 3; j++)
	    {
		x = wi - _ri [j];
		_ri [j] += _dr [j];
		if (x < 0) x += _size;
		i = (int)(floorf (x));
		x -= i;
		y += (1 - x) * _line [i] + x * _line [i + 1];
            }
	    y *= 0.333f;
	    if (add) *p1++ += y * _gain;
	    else     *p1++  = y;

	}
	if (wi == _size) _line [wi = 0] = _line [_size];
    }
    while (len);

    _wi = wi;
    _a = a;
    _b = b;
}




Ladspa_CS_chorus3::Ladspa_CS_chorus3 (unsigned long fsam) : LadspaPlugin (fsam)
{
    _size = (unsigned long)(ceil (30 * fsam / 500.0)) + 192;
    _size = (_size >> 6) << 6; 
    _line = new float [_size + 1];
}


Ladspa_CS_chorus3::~Ladspa_CS_chorus3 (void)
{
    delete[] _line;
}


void Ladspa_CS_chorus3::setport (unsigned long port, LADSPA_Data *data)
{
  _port [port] = data;
}


void Ladspa_CS_chorus3::active (bool act)
{
    unsigned int i;

    if (act)
    {
	_wi = _gi = 0;
	_x1 = _x2 = 1;
	_y1 = _y2 = 0;
	_a = _b = 0;
        memset (_line, 0, (_size + 1) * sizeof (float));
	for (i = 0; i < 3; i++) _ri [i] = _dr [i] = 0;
    }
}


void Ladspa_CS_chorus3::runproc (unsigned long len, bool add)
{
    unsigned long i, k, wi;
    int   j;
    float *p0, *p1, *p2, *p3;
    float a, b, t, x, y;

    p0 = _port [INPUT];    
    p1 = _port [OUTPUT1];    
    p2 = _port [OUTPUT2];    
    p3 = _port [OUTPUT3];    
    
    wi = _wi;
    a = _a;
    b = _b;
    do
    {
        if (_gi == 0)
	{
	    _gi = 64;

            t = 402.12f * _port [FREQ1][0] / _fsam;
            x = _x1 - t * _y1; 
            y = _y1 + t * _x1;
            t = sqrtf (x * x + y * y);
            _x1 = x / t;
            _y1 = y / t;

            t = 402.12f * _port [FREQ2][0] / _fsam;
            x = _x2 - t * _y2; 
            y = _y2 + t * _x2;
            t = sqrtf (x * x + y * y);
            _x2 = x / t;
            _y2 = y / t;

            x = _port [TMOD1][0] * _x1 + _port [TMOD2][0] * _x2;
            y = _port [TMOD1][0] * _y1 + _port [TMOD2][0] * _y2;
            
            _dr [0] = x;
            _dr [1] = -0.500f * x + 0.866f * y;
            _dr [2] = -0.500f * x - 0.866f * y;

            for (j = 0; j < 3; j++)
	    {
                t = _port [DELAY][0] + _dr [j];
                if (t <  0) t =  0;
                if (t > 30) t = 30;
                t *= _fsam / 500.0f;
                _dr [j] = (t - _ri [j]) / 64;
	    }
	}

        k = (_gi < len) ? _gi : len;
        _gi -= k;
        len -= k;
       
        while (k--)
	{
            x = *p0++ + 0.52f * a - 0.25f * b;
            _line [++wi] = 0.5f * (x + b) + a;
            b = a;
            a = x;
            x = 0.52f * a - 0.25f * b;
            _line [++wi] = 0.5f * (x + b) + a;
            b = a;
            a = x;

	    x = wi - _ri [0];
	    _ri [0] += _dr [0];
	    if (x < 0) x += _size;
	    i = (int)(floorf (x));
	    x -= i;
	    y = (1 - x) * _line [i] + x * _line [i + 1];
	    if (add) *p1++ += y * _gain;
	    else     *p1++  = y;

	    x = wi - _ri [1];
	    _ri [1] += _dr [1];
	    if (x < 0) x += _size;
	    i = (int)(floorf (x));
	    x -= i;
	    y = (1 - x) * _line [i] + x * _line [i + 1];
	    if (add) *p2++ += y * _gain;
	    else     *p2++  = y;

	    x = wi - _ri [2];
	    _ri [2] += _dr [2];
	    if (x < 0) x += _size;
	    i = (int)(floorf (x));
	    x -= i;
	    y = (1 - x) * _line [i] + x * _line [i + 1];
	    if (add) *p3++ += y * _gain;
	    else     *p3++  = y;
	}
	if (wi == _size) _line [wi = 0] = _line [_size];
    }
    while (len);

    _wi = wi;
    _a = a;
    _b = b;
}

