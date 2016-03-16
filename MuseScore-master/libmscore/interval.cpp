//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "interval.h"
#include "utils.h"

namespace Ms {

//---------------------------------------------------------
//   Interval
//---------------------------------------------------------

Interval::Interval()
   : diatonic(0), chromatic(0)
      {
      }

Interval::Interval(int a, int b)
   : diatonic(a), chromatic(b)
      {
      }

Interval::Interval(int c)
      {
      chromatic = c;
      diatonic = chromatic2diatonic(c);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void Interval::flip()
      {
      diatonic = -diatonic;
      chromatic = -chromatic;
      }

//---------------------------------------------------------
//   isZero
//---------------------------------------------------------

bool Interval::isZero() const
      {
      return diatonic == 0 && chromatic == 0;
      }

}

