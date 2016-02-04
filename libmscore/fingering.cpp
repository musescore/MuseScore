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

#include "fingering.h"
#include "score.h"
#include "staff.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
  : Text(s)
      {
      setTextStyleType(TextStyleType::FINGERING);
      setFlag(ElementFlag::HAS_TAG, true);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(name());
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fingering::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!Text::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      if (staff() && staff()->isTabStaff())     // in TAB staves
            setbbox(QRectF());                  // fingerings have no area
      else
            Text::layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Fingering::draw(QPainter* painter) const
      {
      if (staff() && staff()->isTabStaff())     // hide fingering in TAB staves
            return;
      Text::draw(painter);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Fingering::reset()
      {
      QPointF o(userOff());
      score()->layoutFingering(this);
      QPointF no;
      TextStyleType tst = textStyleType();
      if (tst == TextStyleType::FINGERING || tst == TextStyleType::RH_GUITAR_FINGERING || tst == TextStyleType::STRING_NUMBER)
            no = userOff();
      setUserOff(o);
      score()->undoChangeProperty(this, P_ID::USER_OFF, no);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fingering::accessibleInfo() const
      {
      QString rez = Element::accessibleInfo();
      if (textStyleType() == TextStyleType::STRING_NUMBER) {
            rez += " " + tr("String number");
            }
      return QString("%1: %2").arg(rez).arg(plainText());
      }

}

