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


float exp2ap (float x)
{
    int i;

    i = (int)(floor (x));
    x -= i;
//    return ldexp (1 + x * (0.66 + 0.34 * x), i);
    return ldexp (1 + x * (0.6930 + x * (0.2416 + x * (0.0517 + x * 0.0137))), i);
}


