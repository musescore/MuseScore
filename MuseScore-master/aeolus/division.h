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


#ifndef __DIVISION_H
#define __DIVISION_H

#include "asection.h"
#include "rankwave.h"

//---------------------------------------------------------
//   Division
//---------------------------------------------------------

class Division
      {
      Asection  *_asect;
      Rankwave  *_ranks [NRANKS];
      int        _nrank;
      int        _dmask;
      int        _trem;
      float      _fsam;
      float      _swel;
      float      _gain;
      float      _w;
      float      _c;
      float      _s;
      float      _m;
      float      _buff [NCHANN * PERIOD];

   public:
      Division (Asection *asect, float fsam);
      ~Division ();

      void set_rank (int ind, Rankwave *W, int pan, int del);
      void set_swell (float stat)   { _swel = 0.2 + 0.8 * stat * stat; }
      void set_tfreq (float freq)   { _w = 6.283184f * PERIOD * freq / _fsam; }
      void set_tmodd (float modd)   { _m = modd; }
      void set_div_mask (int bits);
      void clr_div_mask (int bits);
      void set_rank_mask (int ind, int bits);
      void clr_rank_mask (int ind, int bits);
      void trem_on()                { _trem = 1; }
      void trem_off()               { _trem = 2; }

      void process();
      void update(int note, int mask);
      void update(unsigned char *keys);
      };

#endif

