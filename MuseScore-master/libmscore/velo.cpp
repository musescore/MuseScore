//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of class VeloList.
*/

#include "velo.h"

namespace Ms {

//---------------------------------------------------------
//   velo
//    return velocity at tick position
//---------------------------------------------------------

int VeloList::velo(int tick) const
      {
      if (empty())
            return 80;
      VeloList::const_iterator i = upperBound(tick);
      if (i == constBegin())
            return 80;
      VeloList::const_iterator ii = i - 1;
      if (ii.value().type == VeloType::FIX)
            return ii.value().val;
      int tickDelta = i.key() - ii.key();
      int veloDelta = i.value().val - ii.value().val;
      return ii.value().val + ((tick-ii.key()) * veloDelta) / tickDelta;
      }

//---------------------------------------------------------
//   nextVelo
//    return next velocity event after tick position
//---------------------------------------------------------

int VeloList::nextVelo(int tick) const
      {
      if (empty())
            return 80;
      VeloList::const_iterator i = upperBound(tick);
      if (i != end())
            return i.value().val;
      else
            return 80;
      }

//---------------------------------------------------------
//   setVelo
//---------------------------------------------------------

void VeloList::setVelo(int tick, VeloEvent ve)
      {
      insert(tick, ve);
      }

void VeloList::setVelo(int tick, int velo)
      {
      insert(tick, VeloEvent(VeloType::FIX, velo));
      }

}

