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
  : Text(SubStyle::FINGERING, s)
      {
      setFlag(ElementFlag::HAS_TAG, true);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(XmlWriter& xml) const
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
      if (staff() && staff()->isTabStaff(tick()))     // in TAB staves
            setbbox(QRectF());                  // fingerings have no area
      else
            Text::layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Fingering::draw(QPainter* painter) const
      {
      if (staff() && staff()->isTabStaff(tick()))     // hide fingering in TAB staves
            return;
      Text::draw(painter);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Fingering::accessibleInfo() const
      {
      QString rez = Element::accessibleInfo();
      if (subStyle() == SubStyle::STRING_NUMBER) {
            rez += " " + tr("String number");
            }
      return QString("%1: %2").arg(rez).arg(plainText());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Fingering::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fingering::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            default:
                  return Text::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Fingering::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SUB_STYLE:
                  return int(SubStyle::FINGERING);
            default:
                  return Text::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags Fingering::propertyFlags(P_ID id) const
      {
      switch (id) {
            default:
                  return Text::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fingering::resetProperty(P_ID id)
      {
      switch (id) {
            default:
                  return Text::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Fingering::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            default:
                  return Text::getPropertyStyle(id);
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Fingering::styleChanged()
      {
      Text::styleChanged();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Fingering::reset()
      {
      QPointF o(userOff());
      score()->layoutFingering(this);
      QPointF no = userOff();
      setUserOff(o);
      score()->undoChangeProperty(this, P_ID::USER_OFF, no);
      Text::reset();
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Fingering::subtypeName() const
      {
      return subStyleName(subStyle());
      }

}

