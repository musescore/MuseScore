//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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
            return ClefTypeList(CLEF_G, CLEF_G);
      auto i = upperBound(tick);
      if (i == begin())
            return ClefTypeList(CLEF_G, CLEF_G);
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefTypeList idx)
      {
qDebug("setClef...\n");
      replace(tick, idx);
      }

//---------------------------------------------------------
//   ClefList::read
//---------------------------------------------------------

void ClefList::read(XmlReader& e, Score* cs)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "clef") {
                  int tick    = e.intAttribute("tick", 0);
                  ClefType ct = Clef::clefType(e.attribute("idx", "0"));
                  insert(cs->fileDivision(tick), ClefTypeList(ct, ct));
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

