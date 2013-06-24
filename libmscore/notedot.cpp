//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "notedot.h"
#include "score.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

NoteDot::NoteDot(Score* s)
   : Element(s)
      {
      setFlag(ELEMENT_MOVABLE, false);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void NoteDot::layout()
      {
      setbbox(symbols[score()->symIdx()][dotSym].bbox(magS()));
      }

//---------------------------------------------------------
//   NoteDot::draw
//---------------------------------------------------------

void NoteDot::draw(QPainter* p) const
      {
      if (!staff()->isTabStaff() || ((StaffTypeTablature*)staff()->staffType())->stemThrough()) {
            p->setPen(curColor());
            symbols[score()->symIdx()][dotSym].draw(p, magS());
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteDot::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteDot::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "name")    // obsolete
                  e.readElementText();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal NoteDot::mag() const
      {
      return parent()->mag() * score()->styleD(ST_dotMag);
      }
}

