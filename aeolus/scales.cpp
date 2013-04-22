/*
  Copyright (C) 2003-2010 Fons Adriaensen <fons@kokkinizita.net>

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


#include "scales.h"


// Pythagorean

static float scale_pythagorean [12] = {
      1.00000000,
      1.06787109,
      1.12500000,
      1.18518519,
      1.26562500,
      1.33333333,
      1.42382812,
      1.50000000,
      1.60180664,
      1.68750000,
      1.77777778,
      1.89843750,
      };


// 1/4 comma meantone (Pietro Aaron, 1523)

static float scale_meanquart [12] =  {
      1.0000000,
      1.0449067,
      1.1180340,
      1.1962790,
      1.2500000,
      1.3374806,
      1.3975425,
      1.4953488,
      1.5625000,
      1.6718508,
      1.7888544,
      1.8691860
      };


// Andreas Werckmeister III, 1681

static float scale_werckm3 [12] = {
      1.00000000,
      1.05349794,
      1.11740331,
      1.18518519,
      1.25282725,
      1.33333333,
      1.40466392,
      1.49492696,
      1.58024691,
      1.67043633,
      1.77777778,
      1.87924088
      };


// Kirnberger III

static float scale_kirnberg3 [12] =  {
      1.00000000,
      1.05349794,
      1.11848107,
      1.18518519,
      1.25000021,
      1.33333333,
      1.40625000,
      1.49542183,
      1.58024691,
      1.67176840,
      1.77777778,
      1.87500000
      };


// Well-tempered (Jacob Breetvelt)

static float scale_welltemp [12] =
{
    1.00000000,
    1.05468828,
    1.12246205,
    1.18652432,
    1.25282725,
    1.33483985,
    1.40606829,
    1.49830708,
    1.58203242,
    1.67705161,
    1.77978647,
    1.87711994
};


// Equally Tempered

static float scale_equaltemp [12] =
{
    1.00000000,
    1.05946309,
    1.12246205,
    1.18920712,
    1.25992105,
    1.33483985,
    1.41421356,
    1.49830708,
    1.58740105,
    1.68179283,
    1.78179744,
    1.88774863,
};


// The following five were contributed by Hanno Hoffstadt.
// The Lehman temperament was also provided by Adam Sampson.

// Vogel/Ahrend

static float scale_ahrend [12] =
{
    1.00000000,
    1.05064661,
    1.11891853,
    1.18518519,
    1.25197868,
    1.33695184,
    1.40086215,
    1.49594019,
    1.57596992,
    1.67383521,
    1.78260246,
    1.87288523,
};


// Vallotti

static float scale_vallotti [12] =
{
    1.00000000,
    1.05647631,
    1.12035146,
    1.18808855,
    1.25518740,
    1.33609659,
    1.40890022,
    1.49689777,
    1.58441623,
    1.67705160,
    1.78179744,
    1.87888722,
};


// Kellner

static float scale_kellner [12] =
{
    1.00000000,
    1.05349794,
    1.11891853,
    1.18518519,
    1.25197868,
    1.33333333,
    1.40466392,
    1.49594019,
    1.58024691,
    1.67383521,
    1.77777778,
    1.87796802,
};


// Lehman

static float scale_lehman [12] =
{
    1.00000000,
    1.05826737,
    1.11992982,
    1.18786496,
    1.25424281,
    1.33634808,
    1.41102316,
    1.49661606,
    1.58560949,
    1.67610496,
    1.77978647,
    1.88136421,
};

// Pythagorean

static float scale_pure_cfg [12] =
{
    1.00000000,
    1.04166667,
    1.12500000,
    1.1892,
    1.25000000,
    1.33333333,
    1.40625000,
    1.50000000,
    1.5874,
    1.66666667,
    1.77777778,
    1.87500000,
};


struct temper scales [NSCALES] =
{
    { "Pythagorean", "pyt", scale_pythagorean },
    { "Meantone 1/4", "mtq", scale_meanquart },
    { "Werckmeister III", "we3", scale_werckm3 },
    { "Kirnberger III", "ki3", scale_kirnberg3 },
    { "Well Tempered", "wt",  scale_welltemp },
    { "Equally Tempered", "et", scale_equaltemp },
    { "Vogel/Ahrend", "ahr", scale_ahrend },
    { "Vallotti", "val", scale_vallotti },
    { "Kellner", "kel", scale_kellner },
    { "Lehman", "leh", scale_lehman },
    { "Pure C/F/G", "cfg", scale_pure_cfg },
};

