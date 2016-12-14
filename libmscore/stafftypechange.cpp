//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "stafftypechange.h"
#include "score.h"
#include "mscore.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   StaffTypeChange
//---------------------------------------------------------

StaffTypeChange::StaffTypeChange(Score* score)
   : Element(score)
      {
      setFlag(ElementFlag::HAS_TAG, true);
      }

StaffTypeChange::StaffTypeChange(const StaffTypeChange& lb)
   : Element(lb)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTypeChange::write(XmlWriter& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTypeChange::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
//            const QStringRef& tag(e.name());
            if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffTypeChange::draw(QPainter* /*painter*/) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
#if 0
      QPainterPathStroker stroker;
      stroker.setWidth(lw/2);
      stroker.setJoinStyle(Qt::MiterJoin);
      stroker.setCapStyle(Qt::SquareCap);

      QVector<qreal> dashes;
      dashes.append(1);
      dashes.append(3);
      stroker.setDashPattern(dashes);
      QPainterPath stroke = stroker.createStroke(path);

      painter->fillPath(stroke, selected() ? MScore::selectColor[0] : MScore::layoutBreakColor);

      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path2);
#endif
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant StaffTypeChange::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StaffTypeChange::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant StaffTypeChange::propertyDefault(P_ID id) const
      {
      switch (id) {
            default:
                  return Element::propertyDefault(id);
            }
      }

}

