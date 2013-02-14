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


#include "global.h"
#include "addsynth.h"

#define M (N_NOTE - 1)


#define swap4(a, b) { (a)[0] = (b)[3], (a)[1] = (b)[2], (a)[2] = (b)[1], (a)[3] = (b)[0]; }


//---------------------------------------------------------
//   N_func
//---------------------------------------------------------

N_func::N_func()
      {
      reset (0.0f);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void N_func::reset (float v)
      {
      _b = 16;
      for (int i = 0; i < N_NOTE; i++)
            _v [i] = v;
      }

//---------------------------------------------------------
//   setv
//---------------------------------------------------------

void N_func::setv (int i, float v)
      {
      int    j;
      float  d;

      if ((i < 0) || (i > M))
            return;
      _v [i] = v;
      _b |= 1 << i;

      for (j = i - 1; (j >= 0) && ! (_b & (1 << j)); j--)
            ;
      if (j < 0)
            while (++j != i)
                  _v [j] = v;
      else {
            d = (_v [j] - v) / (j - i);
            while (++j != i)
                  _v [j] = v + (j - i) * d;
            }

      for (j = i + 1; (j <= M) && ! (_b & (1 << j)); j++)
            ;
      if (j > M)
            while (--j != i) _v [j] = v;
      else {
            d = (_v [j] - v) / (j - i);
            while (--j != i)
                  _v [j] = v + (j - i) * d;
            }
      }

//---------------------------------------------------------
//   clrv
//---------------------------------------------------------

void N_func::clrv (int i)
      {
      int   j, k, m;
      float d;

      if ((i < 0) || (i > M))
            return;
      m = 1 << i;
      if (! (_b & m) || (_b == m))
            return;
      _b ^= m;

      for (j = i - 1; (j >= 0) && ! (_b & (1 << j)); j--)
            ;
      for (k = i + 1; (k <= M) && ! (_b & (1 << k)); k++)
            ;

      if ((j >= 0) && (k <= M)) {
            d = (_v [k] - _v [j]) / (k - j);
            for (i = j + 1; i < k; i++)
                  _v [i] = _v [j] + (i - j) * d;
            }
      else if (j >= 0) {
            d = _v [j];
            while (j < M)
                  _v [++j] = d;
            }
      else if (k <= M) {
            d = _v [k];
            while (k > 0)
                  _v [--k] = d;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void N_func::write (FILE *F)
      {
#ifdef __BYTE_ORDER
#if (__BYTE_ORDER == __LITTLE_ENDIAN)

      fwrite (&_b, 1, sizeof (int32_t), F);
      fwrite (_v, N_NOTE, sizeof (float), F);

#elif (__BYTE_ORDER == __BIG_ENDIAN)

      int  i;
      char d [N_NOTE * sizeof (float)];

      swap4 (d, (char *)(&_b));
      fwrite (d, 1, sizeof (int32_t), F);
      for (i = 0; i < N_NOTE; i++)
            swap4 (d + i * sizeof (float), (char *)(_v + i));
      fwrite (d, N_NOTE, sizeof (float), F);
#else
#error Byte order is not supported !
#endif
#else
#error Byte order is undefined !
#endif
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void N_func::read (QFile* f)
      {
#ifdef __BYTE_ORDER
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
      f->read ((char*)&_b, sizeof (int32_t));
      f->read ((char*)&_v, N_NOTE * sizeof (float));

#elif (__BYTE_ORDER == __BIG_ENDIAN)

      int  i;
      char d [sizeof (int) + N_NOTE * sizeof (float)];

      f->read (d, sizeof (int32_t));
      swap4 ((char *)(&_b), d);
      f->read (d, N_NOTE * sizeof (float));
      for (i = 0; i < N_NOTE; i++)
            swap4 ((char *)(_v + i), d + i * sizeof (float));

#else
#error Byte order is not supported !
#endif
#else
#error Byte order is undefined !
#endif
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void HN_func::reset (float v)
      {
      for (int j = 0; j < N_HARM; j++)
            (_h + j)->reset (v);
      }

//---------------------------------------------------------
//   setv
//---------------------------------------------------------

void HN_func::setv (int i, float v)
      {
      for (int j = 0; j < N_HARM; j++)
            (_h + j)->setv (i, v);
      }

//---------------------------------------------------------
//   clrv
//---------------------------------------------------------

void HN_func::clrv (int i)
      {
      for (int j = 0; j < N_HARM; j++)
            (_h + j)->clrv (i);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void HN_func::write (FILE *F, int k)
      {
      for (int j = 0; j < k; j++)
            (_h + j)->write (F);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void HN_func::read (QFile* f, int k)
      {
      for (int j = 0; j < k; j++)
            (_h + j)->read(f);
      }

//---------------------------------------------------------
//   Addsynth
//---------------------------------------------------------

Addsynth::Addsynth()
      {
      reset();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Addsynth::reset()
      {
      *_stopname = 0;
      *_mnemonic = 0;
      *_copyrite = 0;
      *_comments = 0;
      _fd = 1;
      _fn = 1;
      _n0 = NOTE_MIN;
      _n1 = NOTE_MAX;
      _n_vol.reset (-20.0f);
      _n_ins.reset (0.0f);
      _n_off.reset (0.0f);
      _n_att.reset (0.01f);
      _n_atd.reset (0.0f);
      _n_dct.reset (0.01f);
      _n_dcd.reset (0.0f);
      _n_ran.reset (0.0f);
      _h_lev.reset (-100.0f);
      _h_ran.reset (0.0f);
      _h_att.reset (0.050f);
      _h_atp.reset (0.0f);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

int Addsynth::save (const char* sdir)
      {
      FILE  *F;
      char   d [32];
      char   path [1024];

      strcpy (path, sdir);
      strcat (path, "/");
      strcat (path, _filename);

      if (! (F = fopen (path, "w"))) {
            fprintf (stderr, "Can't open '%s' for writing\n", path);
            return 1;
            }

      memset (d, 0, 32);
      strcpy (d, "AEOLUS");
      d [7]  =  2;
      d [26] = N_HARM;
      d [28] = _n0;
      d [29] = _n1;
      d [30] = _fn;
      d [31] = _fd;

      fwrite (d, 1, 32, F);
      fwrite (_stopname, 1, 32, F);
      fwrite (_copyrite, 1, 56, F);
      fwrite (_mnemonic, 1,  8, F);
      fwrite (_comments, 1, 56, F);
      fwrite (_reserved, 1,  8, F);

      _n_vol.write (F);
      _n_off.write (F);
      _n_ran.write (F);
      _n_ins.write (F);
      _n_att.write (F);
      _n_atd.write (F);
      _n_dct.write (F);
      _n_dcd.write (F);
      _h_lev.write (F, N_HARM);
      _h_ran.write (F, N_HARM);
      _h_att.write (F, N_HARM);
      _h_atp.write (F, N_HARM);

      fclose (F);
      return 0;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

int Addsynth::load (const char* sdir)
      {
      char   d [32];
      int    v, k;

      reset ();

      QFile f(QString("%1/%2").arg(sdir).arg(_filename));

      if (!f.open (QIODevice::ReadOnly)) {
            fprintf (stderr, "Can't open '%s' for reading\n", qPrintable(f.fileName()));
            return 1;
            }

      f.read (d, 32);
      if (strcmp (d, "AEOLUS")) {
            fprintf (stderr, "File '%s' is not an Aeolus file\n", _filename);
            f.close();
            return 1;
            }
      v = d [7];
      k = d [26];
      if (! k)
            k = 48;
      _n0 = d [28];
      _n1 = d [29];
      if (_n1 == 0x2E)
            _n1 = 96;   ////// FIX THIS
      _fn = d [30];
      _fd = d [31];

      f.read (_stopname, 32);
      f.read (_copyrite, 56);
      f.read (_mnemonic,  8);
      f.read (_comments, 56);
      f.read (_reserved,  8);

      _n_vol.read (&f);
      _n_off.read (&f);
      _n_ran.read (&f);
      if (v >= 2) {
            _n_ins.read (&f);
            _n_att.read (&f);
            _n_atd.read (&f);
            _n_dct.read (&f);
            _n_dcd.read (&f);
            }
      _h_lev.reset (-100.0f);
      _h_ran.reset (0.0f);
      _h_att.reset (0.050f);
      _h_atp.reset (0.0f);
      _h_lev.read (&f, k);
      _h_ran.read (&f, k);
      _h_att.read (&f, k);
      _h_atp.read (&f, k);

      f.close();
      return 0;
      }
