//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __TIMEPOSITION_H__
#define __TIMEPOSITION_H__

#include "fraction.h"

namespace Ms {
//---------------------------------------------------------
//   TimePosition
//    helper class used as key in std::map for
//    ClefList, KeyList, StafftypeList
//---------------------------------------------------------

class TimePosition {
      Fraction f;
      qreal t;                // cached float value of fraction

   public:
      constexpr TimePosition(const Fraction& t) : f(t), t(qreal(t.numerator())/qreal(t.denominator())) {}
      const Fraction& tick() const                 { return f; }
      bool operator<(const TimePosition& tp) const { return t < tp.t; }
      };
} // namespace

#endif


