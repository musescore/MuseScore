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


#ifndef __ADDSYNTH_H
#define __ADDSYNTH_H

#define N_NOTE 11
#define N_HARM 64
#define NOTE_MIN 36
#define NOTE_MAX 96


//---------------------------------------------------------
//   N_func
//---------------------------------------------------------

class N_func
      {
      int   _b;
      float _v [N_NOTE];

   public:

      N_func();
      void reset (float v);
      void setv (int i, float v);
      void clrv (int i);
      float vs (int i) const { return _v [i]; }
      int   st (int i) const { return (_b & (1 << i)) ? 1 : 0; }
      float vi (int n) const {
            int   i = n / 6;
            int   k = n - 6 * i;
            float v = _v [i];
            if (k)
                  v += k * (_v [i + 1] - v) / 6;
            return v;
            }
      void write(FILE*);
      void read(QFile*);
      };

//---------------------------------------------------------
//   HN_func
//---------------------------------------------------------

class HN_func
      {
      N_func _h [N_HARM];

   public:

      HN_func() {}
      void reset (float v);
      void setv (int i, float v);
      void clrv (int i);
      void setv (int h, int i, float v) { _h [h].setv (i, v); }
      void clrv (int h, int i) { _h [h].clrv (i); }
      float vs (int h, int i) const { return _h [h].vs (i); }
      int   st (int h, int i) const { return _h [h].st (i); }
      float vi (int h, int n) const { return _h [h].vi (n); }
      void write (FILE *F, int k);
      void read (QFile *F, int k);
      };

//---------------------------------------------------------
//   Addsynth
//---------------------------------------------------------

class Addsynth
      {
   public:
      Addsynth();

      void reset();
      int save (const char *sdir);
      int load (const char *sdir);

      char       _filename [64];
      char       _stopname [32];
      char       _copyrite [56];
      char       _mnemonic [8];
      char       _comments [56];
      char       _reserved [8];
      int32_t    _n0;
      int32_t    _n1;
      int32_t    _fn;
      int32_t    _fd;
      N_func     _n_vol;
      N_func     _n_off;
      N_func     _n_ran;
      N_func     _n_ins;
      N_func     _n_att;
      N_func     _n_atd;
      N_func     _n_dct;
      N_func     _n_dcd;
      HN_func    _h_lev;
      HN_func    _h_ran;
      HN_func    _h_att;
      HN_func    _h_atp;

      char       _pan;
      int32_t    _del;
      };


#endif

