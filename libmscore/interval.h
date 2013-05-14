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

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

namespace Ms {

//---------------------------------------------------------
//   Interval
//---------------------------------------------------------

struct Interval {
      char diatonic;
      char chromatic;

      Interval();
      Interval(int a, int b);
      Interval(int _chromatic);
      void flip();
      bool isZero() const;
      };


}     // namespace Ms
#endif

