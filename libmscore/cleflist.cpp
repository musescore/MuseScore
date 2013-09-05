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
      auto i = upper_bound(tick);
      if (i != begin())
            --i;
      return i->second;
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

