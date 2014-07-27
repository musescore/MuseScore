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

ClefTypeList ClefList::clef(int tick) const
      {
      if (empty())
            return _initial;
      auto i = upper_bound(tick);
      if (i == begin())
            return _initial;
      return (--i)->second;
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefTypeList ctl)
      {
      auto i = find(tick);
      if (i == end())
            insert(std::pair<int, ClefTypeList>(tick, ctl));
      else
            i->second = ctl;
      }

}

