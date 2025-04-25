/*
** Copyright (C) 2002-2015 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#define		DFT_DATA_LENGTH		(8192)

double	dft_cmp_float (int linenum, const float *orig, const float *test, int len, double tolerance, int allow_exit) ;

double	dft_cmp_double (int linenum, const double *orig, const double *test, int len, double tolerance, int allow_exit) ;

