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

#include "rankwave.h"

#define DEBUG

extern float exp2ap (float);


Rngen   Pipewave::_rgen;
float  *Pipewave::_arg = 0;
float  *Pipewave::_att = 0;

void Pipewave::initstatic(float fsamp)
      {
      if (_arg)
            return;
      int k = (int)(fsamp);
      _arg  = new float [k];
      k     = (int)(0.5f * fsamp);
      _att  = new float [k];
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Pipewave::play()
      {
      int     i, d, k1, k2;
      float   g, dg, y, dy, t;
      float   *q;

      float* p = _p_p;
      float* r = _p_r;

      if (_sdel & 1) {
            if (!p) {
                  p    = _p0;
                  _y_p = 0.0f;
                  _z_p = 0.0f;
                  }
            }
      else
    {
        if (! r)
	{
  	    r = p;
            p = 0;
            _g_r = 1.0f;
            _y_r = _y_p;
            _i_r = _k_r;
	}
    }

    if (r)
    {
        k1 = PERIOD;
        q = _out;
	g = _g_r;
        i = _i_r - 1;
        dg = g / PERIOD;
        if (i) dg *= _m_r ;

        if (r < _p1)
        {
            while (k1--)
            {
	        *q++ += g * *r++;
                g -= dg;
            }
        }
        else
	{
            y = _y_r;
            dy = _d_r;
            while (k1)
	    {
                t = y + k1 * dy;
                d = 0;
                k2 = k1;
                if (t > 1.0f)
	        {
		    d = 1;
                    k2 = (int)((1.0f - y) / dy);
                }
                else if (t < 0.0f)
	        {
		    d = -1;
                    k2 = (int)(-y / dy);
	        }
                k1 -= k2;
                if (k2<0)
                  k2 = 0;
                while (k2--)
       	        {
                    *q++ += g * (r [0] + y * (r [1] - r [0]));
                    g -= dg;
                    y += dy;
                    r += _k_s;
	        }
                y -= d;
                r += d;
	    }
            _y_r = y;
	}

        if (i)
	{
	    _g_r = g;
            _i_r = i;
            if (r >= _p2) r -= _l1;
	}
        else r = 0;
    }

    if (p)
    {
        k1 = PERIOD;
        q = _out;
        if (p < _p1)
        {
            while (k1--)
            {
		*q++ += *p++;
	    }
        }
        else
	{
            y = _y_p;
            _z_p += _d_p * 0.0005f * (0.05f * _d_p * (_rgen.urandf () - 0.5f) - _z_p);
            dy = _z_p * _k_s;
            while (k1)
	    {
                t = y + k1 * dy;
                d = 0;
                k2 = k1;
                if (t > 1.0f)
	        {
		    d = 1;
                    k2 = (int)((1.0f - y) / dy);
                }
                else if (t < 0.0f)
	        {
		    d = -1;
                    k2 = (int)(-y / dy);
	        }
                k1 -= k2;
                if (k2<0)
                  k2 = 0;
                while (k2--)
       	        {
                    *q++ += p [0] + y * (p [1] - p [0]);
                    y += dy;
                    p += _k_s;
	        }
                y -= d;
                p += d;
	    }
            if (p >= _p2) p -= _l1;
            _y_p = y;
	}
    }

    _p_p = p;
    _p_r = r;
}


void Pipewave::genwave (Addsynth *D, int n, float fsamp, float fpipe)
{
    int    h, i, k, nc;
    float  f0, f1, f, m, t, v, v0;

    m = D->_n_att.vi (n);
    for (h = 0; h < N_HARM; h++)
    {
	t = D->_h_att.vi (h, n);
        if (t > m) m = t;
    }
    _l0 = (int)(fsamp * m + 0.5);
    _l0 = (_l0 + PERIOD - 1) & ~(PERIOD - 1);

    f1 = (fpipe + D->_n_off.vi (n) + D->_n_ran.vi (n) * (2 * _rgen.urand () - 1)) / fsamp;
    f0 = f1 * exp2ap (D->_n_atd.vi (n) / 1200.0f);

    for (h = N_HARM - 1; h >= 0; h--)
    {
        f = (h + 1) * f1;
	if ((f < 0.45f) && (D->_h_lev.vi (h, n) >= -40.0f)) break;
    }
    if      (f > 0.250f) _k_s = 3;
    else if (f > 0.125f) _k_s = 2;
    else                 _k_s = 1;

    looplen (f1 * fsamp, _k_s * fsamp, (int)(fsamp / 6.0f), &_l1, &nc);
    if (_l1 < _k_s * PERIOD)
    {
        k = (_k_s * PERIOD - 1) / _l1 + 1;
        _l1 *= k;
        nc *= k;
    }

    k = _l0 + _l1 + _k_s * (PERIOD + 4);

    delete[] _p0;
    _p0 = new float [k];
    _p1 = _p0 + _l0;
    _p2 = _p1 + _l1;
    memset (_p0, 0, k * sizeof (float));

    _k_r = (int)(ceilf (D->_n_dct.vi (n) * fsamp / PERIOD) + 1);
    _m_r = 1.0f - powf (0.1, 1.0 / _k_r);
    _d_r = _k_s * (exp2ap (D->_n_dcd.vi (n) / 1200.0f) - 1.0f);
    _d_p = D->_n_ins.vi (n);

    t = 0.0f;
    k = (int)(fsamp * D->_n_att.vi (n) + 0.5);
    for (i = 0; i <= _l0; i++)
    {
        _arg [i] = t - floorf (t + 0.5);
	t += (i < k) ? (((k - i) * f0 + i * f1) / k) : f1;
    }

    for (i = 1; i < _l1; i++)
    {
	t = _arg [_l0]+ (float) i * nc / _l1;
        _arg [i + _l0] = t - floorf (t + 0.5);
    }

    v0 = exp2ap (0.1661 * D->_n_vol.vi (n));
    for (h = 0; h < N_HARM; h++)
    {
        if ((h + 1) * f1 > 0.45) break;
        v = D->_h_lev.vi (h, n);
        if (v < -80.0) continue;

        v = v0 * exp2ap (0.1661 * (v + D->_h_ran.vi (h, n) * (2 * _rgen.urand () - 1)));
        k = (int)(fsamp * D->_h_att.vi (h, n) + 0.5);
        attgain (k, D->_h_atp.vi (h, n));

        for (i = 0; i < _l0 + _l1; i++)
        {
	    t = _arg [i] * (h + 1);
            t -= floorf (t);
            m = v * sinf (2 * M_PI * t);
            if (i < k) m *= _att [i];
            _p0 [i] += m;
        }
    }
    for (i = 0; i < _k_s * (PERIOD + 4); i++) _p0 [i + _l0 + _l1] = _p0 [i + _l0];
}


void Pipewave::looplen (float f, float fsamp, int lmax, int *aa, int *bb)
{
    int     i, j, a, b, t;
    int     z [8];
    double  g, d;

    g = fsamp / f;
    for (i = 0; i < 8; i++)
    {
	a = z [i] = (int)(floor (g + 0.5));
        g -= a;
        b = 1;
        j = i;
        while (j > 0)
	{
            t = a;
  	    a = z [--j] * a + b;
	    b = t;
	}
        if (a < 0)
	{
	    a = -a;
            b = -b;
	}
        if (a <= lmax)
	{
	    d = fsamp * b / a - f;
	    if ((fabs (d) < 0.1) && (fabs (d) < 3e-4 * f)) break;
	    g = (fabs (g) < 1e-6) ? 1e6 : 1.0 / g;
	}
        else
	{
	    b = (int)(lmax * f / fsamp);
            a = (int)(b * fsamp / f + 0.5);
            d = fsamp * b / a - f;
            break;
	}
    }
    *aa = a;
    *bb = b;
}


void Pipewave::attgain (int n, float p)
{
    int    i, j, k;
    float  d, m, w, x, y, z;

    w = 0.05;
    x = 0.0;
    y = 0.6;
    if (p > 0) y += 0.11 * p;
    z = 0.0;
    j = 0;
    for (i = 1; i <= 24; i++)
    {
        k = n * i / 24;
        x =  1.0 - z - 1.5 * y;
        y += w * x;
        d = w * y * p / (k - j);
        while (j < k)
	{
            m = (double) j / n;
            _att [j++] = (1.0 - m) * z + m;
            z += d;
	}
    }
}


void Pipewave::save (FILE *F)
{
    int  k;
    union
    {
        int16_t i16 [16];
        int32_t i32 [8];
	float   flt [8];
    } d;

    d.i32 [0] = _l0;
    d.i32 [1] = _l1;
    d.i16 [4] = _k_s;
    d.i16 [5] = _k_r;
    d.flt [3] = _m_r;
    d.i32 [4] = 0;
    d.i32 [5] = 0;
    d.i32 [6] = 0;
    d.i32 [7] = 0;
    fwrite (&d, 1, 32, F);
    k = _l0 +_l1 + _k_s * (PERIOD + 4);
    fwrite (_p0, k, sizeof (float), F);
}


void Pipewave::load (FILE *F)
{
    int  k;
    union
    {
        int16_t i16 [16];
        int32_t i32 [8];
	float   flt [8];
    } d;

    fread (&d, 1, 32, F);
    _l0  = d.i32 [0];
    _l1  = d.i32 [1];
    _k_s = d.i16 [4];
    _k_r = d.i16 [5];
    _m_r = d.flt [3];
    k = _l0 +_l1 + _k_s * (PERIOD + 4);
    delete[] _p0;
    _p0 = new float [k];
    _p1 = _p0 + _l0;
    _p2 = _p1 + _l1;
    fread (_p0, k, sizeof (float), F);
}




Rankwave::Rankwave (int n0, int n1) : _n0 (n0), _n1 (n1), _list (0), _modif (false)
{
    _pipes = new Pipewave [n1 - n0 + 1];
}


Rankwave::~Rankwave (void)
{
    delete[] _pipes;
}


void Rankwave::gen_waves (Addsynth *D, float fsamp, float fbase, float *scale)
{
    Pipewave::initstatic (fsamp);

    fbase *=  D->_fn / (D->_fd * scale [9]);
    for (int i = _n0; i <= _n1; i++)
    {
	_pipes [i - _n0].genwave (D, i - _n0, fsamp, ldexpf (fbase * scale [i % 12], i / 12 - 5));
    }
    _modif = true;
}


void Rankwave::set_param (float *out, int del, int pan)
{
    int         n, a, b;
    Pipewave   *P;

    _sbit = 1 << del;
    switch (pan)
    {
    case 'L': a = 2, b = 0; break;
    case 'C': a = 2, b = 1; break;
    case 'R': a = 2, b = 2; break;
    default:  a = 4, b = 0;
    }
    for (n = _n0, P = _pipes; n <= _n1; n++, P++) P->_out = out + ((n % a) + b) * PERIOD;
}


void Rankwave::play (int shift)
{
    Pipewave *P, *Q;

    for (P = 0, Q = _list; Q; Q = Q->_link)
    {
	Q->play ();
        if (shift) Q->_sdel = (Q->_sdel >> 1) | Q->_sbit;
        if (Q->_sdel || Q->_p_p || Q->_p_r) P = Q;
        else
	{
  	    if (P) P->_link = Q->_link;
            else      _list = Q->_link;
	}
    }
}


int Rankwave::save (const char *path, Addsynth *D, float fsamp, float fbase, float *scale)
{
    FILE      *F;
    Pipewave  *P;
    int        i;
    char       name [1024];
    char       data [64];
    char      *p;

    sprintf (name, "%s/%s", path, D->_filename);
    if ((p = strrchr (name, '.'))) strcpy (p, ".ae1");
    else strcat (name, ".ae1");

    F = fopen (name, "wb");
    if (F == NULL)
    {
	fprintf (stderr, "Can't open waveform file '%s' for writing\n", name);
        return 1;
    }

    memset (data, 0, 16);
    strcpy (data, "ae1");
    data [4] = 1;
    fwrite (data, 1, 16, F);

    memset (data, 0, 64);
    data [0] = 0;
    data [1] = 0;
    data [2] = 0;
    data [3] = 0;
    data [4] = _n0;
    data [5] = _n1;
    data [6] = 0;
    data [7] = 0;
    *((float *)(data +  8)) = fsamp;
    *((float *)(data + 12)) = fbase;
    memcpy (data + 16, scale, 12 * sizeof (float));
    fwrite (data, 1, 64, F);

    for (i = _n0, P = _pipes; i <= _n1; i++, P++) P->save (F);

    fclose (F);

    _modif = false;
    return 0;
}


int Rankwave::load (const char *path, Addsynth *D, float fsamp, float fbase, float *scale)
{
    FILE      *F;
    Pipewave  *P;
    int        i;
    char       name [1024];
    char       data [64];
    char      *p;
    float      f;

    sprintf (name, "%s/%s", path, D->_filename);
    if ((p = strrchr (name, '.'))) strcpy (p, ".ae1");
    else strcat (name, ".ae1");

    F = fopen (name, "rb");
    if (F == NULL)
    {
#ifdef DEBUG
	fprintf (stderr, "Can't open waveform file '%s' for reading\n", name);
#endif
        return 1;
    }

    fread (data, 1, 16, F);
    if (strcmp (data, "ae1"))
    {
#ifdef DEBUG
	fprintf (stderr, "File '%s' is not an Aeolus waveform file\n", name);
#endif
        fclose (F);
        return 1;
    }

    if (data [4] != 1)
    {
#ifdef DEBUG
	fprintf (stderr, "File '%s' has an incompatible version tag (%d)\n", name, data [4]);
#endif
        fclose (F);
        return 1;
    }

    fread (data, 1, 64, F);
    if (_n0 != data [4] || _n1 != data [5])
    {
#ifdef DEBUG
	fprintf (stderr, "File '%s' has an incompatible note range (%d %d), (%d %d)\n", name, _n0, _n1, data [4], data [5]);
#endif
        fclose (F);
        return 1;
    }

    f = *((float *)(data + 8));
    if (fabsf (f - fsamp) > 0.1f)
    {
#ifdef DEBUG
	fprintf (stderr, "File '%s' has a different sample frequency (%3.1lf)\n", name, f);
#endif
        fclose (F);
        return 1;
    }

    f = *((float *)(data + 12));
    if (fabsf (f - fbase) > 0.1f)
    {
#ifdef DEBUG
	fprintf (stderr, "File '%s' has a different tuning (%3.1lf)\n", name, f);
#endif
        fclose (F);
        return 1;
    }

    for (i = 0; i < 12; i++)
    {
        f = *((float *)(data + 16 + 4 * i));
        if (fabsf (f /  scale [i] - 1.0f) > 6e-5f)
        {
#ifdef DEBUG
	    fprintf (stderr, "File '%s' has a different temperament\n", name);
#endif
            fclose (F);
            return 1;
        }
    }

    for (i = _n0, P = _pipes; i <= _n1; i++, P++) P->load (F);

    fclose (F);

    _modif = false;
    return 0;
}
