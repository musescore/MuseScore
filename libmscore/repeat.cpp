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

#include "repeat.h"
#include "sym.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "mscore.h"
#include "undo.h"
#include <QPointF>

namespace Ms {

//---------------------------------------------------------
//   RepeatMeasure
///   default size is a single-measure repeat
///   default slashes is 1.
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score, int repeatMeasureSize, int slashes)
   : Rest(score)
      {
      _repeatMeasureSize = repeatMeasureSize;
      _repeatMeasureSlashes = slashes;
      }

RepeatMeasure::RepeatMeasure(const RepeatMeasure& rm, bool link)
   : Rest(rm, link)
      {
      if (link)
            score()->undo(new Link(const_cast<RepeatMeasure*>(&rm), this)); //don't know need to do this linking here, but just following rest's constructor
      _repeatMeasureSize    = rm._repeatMeasureSize;
      _repeatMeasureSlashes = rm._repeatMeasureSlashes;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant RepeatMeasure::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  return _repeatMeasureSize;
            case P_ID::REPEAT_MEASURE_SLASHES:
                  return _repeatMeasureSlashes;
            default:
                  return Rest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant RepeatMeasure::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  return 1;
            case P_ID::REPEAT_MEASURE_SLASHES:
                  return 1;
            default:
                  return Rest::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool RepeatMeasure::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::REPEAT_MEASURE_SIZE:
                  _repeatMeasureSize = v.toInt();
                  break;

            case P_ID::REPEAT_MEASURE_SLASHES:
                  _repeatMeasureSlashes = v.toInt();
                  break;
            default:
                  return Rest::setProperty(propertyId, v);
            }
      return true;
      }

//--------------------------------------------------
//   Rest::write
//---------------------------------------------------------

void RepeatMeasure::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperty(xml, P_ID::REPEAT_MEASURE_SIZE);
      writeProperty(xml, P_ID::REPEAT_MEASURE_SLASHES);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void RepeatMeasure::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "repeatMeasureSize")
                  _repeatMeasureSize = e.readInt();
            else if (tag == "repeatMeasureSlashes")
                  _repeatMeasureSlashes = e.readInt();
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter* painter) const
      {
      qreal _spatium = spatium();
      QPointF yoffset(0.0, 2.0 * _spatium);
      if (repeatMeasureSlashes() == 1)
            drawSymbol(SymId::repeat1Bar, painter, yoffset);
      else if (repeatMeasureSlashes() == 2)
            drawSymbol(SymId::repeat2Bars, painter, yoffset); // maybe add xoffset too?
      else if (repeatMeasureSlashes() == 4)
            drawSymbol(SymId::repeat4Bars, painter, yoffset); // maybe add xoffset too?
      else {
            // fallback to generalized
            painter->setBrush(QBrush(curColor()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(path);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout()
      {
      for (Element* e : _el)
            e->layout();

      qreal sp  = spatium();

      qreal y   = sp;
      qreal w   = sp * 2.4;
      qreal h   = sp * 2.0;
      qreal lw  = sp * .50;  // line width
      qreal r   = sp * .20;  // dot radius

      path      = QPainterPath();

      path.moveTo(w - lw, y);
      path.lineTo(w,  y);
      path.lineTo(lw,  h+y);
      path.lineTo(0.0, h+y);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, y+h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, y+h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
//      _space.setRw(width());
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

Fraction RepeatMeasure::duration() const
      {
      if (measure())
            return measure()->stretchedLen(staff()) * _repeatMeasureSize;
      return Fraction(0, 1);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString RepeatMeasure::accessibleInfo() const
      {
      return Element::accessibleInfo();
      }

}

