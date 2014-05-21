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
            return ClefTypeList();
      auto i = upper_bound(tick);
      if (i == begin())
            return ClefTypeList();
      return (--i)->second;
      }

//---------------------------------------------------------
//   isClefChangeAt
//    returns true if there is a clef change at tick
//---------------------------------------------------------

bool ClefList::isClefChangeAt(int tick) const
      {
      if (empty())
            return (tick == 0);           // there is always a clef set at 0 (possibly implicitly)
      return (count(tick) > 0);
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefTypeList ctl)
      {
      if (clef(tick) == ctl)
            return;
      if (clef(tick-1) == ctl)
            erase(tick);
      else  {
            auto i = find(tick);
            if (i == end())
                  insert(std::pair<int, ClefTypeList>(tick, ctl));
            else
                  i->second = ctl;
            }
      }

//---------------------------------------------------------
//   ClefList::read
//    only used for 1.3 scores
//---------------------------------------------------------

void ClefList::read(XmlReader& e, Score* cs)
      {
      clear();
      while (e.readNextStartElement()) {
            if (e.name() == "clef") {
                  int tick    = e.intAttribute("tick", 0);
                  ClefType ct = Clef::clefType(e.attribute("idx", "0"));
                  insert(std::pair<int, ClefTypeList>(cs->fileDivision(tick), ClefTypeList(ct, ct)));
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }
}

