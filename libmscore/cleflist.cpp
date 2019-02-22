//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "cleflist.h"
#include "clef.h"
#include "score.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   ClefTypeList::operator==
//---------------------------------------------------------

bool ClefTypeList::operator==(const ClefTypeList& t) const
      {
      return t._concertClef == _concertClef && t._transposingClef == _transposingClef;
      }

//---------------------------------------------------------
//   ClefTypeList::operator!=
//---------------------------------------------------------

bool ClefTypeList::operator!=(const ClefTypeList& t) const
      {
      return t._concertClef != _concertClef || t._transposingClef != _transposingClef;
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

ClefTypeList ClefList::clef(const Fraction& tick) const
      {
      if (empty())
            return ClefTypeList(ClefType::INVALID, ClefType::INVALID);
      auto i = upper_bound(TimePosition(tick));
      if (i == begin())
            return ClefTypeList(ClefType::INVALID, ClefType::INVALID);
      return (--i)->second;
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(const Fraction& tick, ClefTypeList ctl)
      {
      auto i = find(tick);
      if (i == end())
            insert(tick, ctl);
      else
            i->second = ctl;
      }

//---------------------------------------------------------
//   nextClefTick
//
//    return the tick at which the clef after tick is located
//    return -1, if no such clef
//---------------------------------------------------------

Fraction ClefList::nextClefTick(const Fraction& tick) const
      {
      if (empty())
            return Fraction(-1, 1);
//      auto i = upper_bound(tick + Fraction::fromTicks(1));
      auto i = upper_bound(TimePosition(tick));
      if (i == end())
            return -1_Fr;

      return i->first.tick();
      }
}

