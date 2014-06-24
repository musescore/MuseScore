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

#include "layoutbreak.h"
#include "score.h"
#include "mscore.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      _layoutBreakType     = LayoutBreakType(propertyDefault(P_ID::LAYOUT_BREAK).toInt());
      _pause               = score->styleD(StyleIdx::SectionPause);
      _startWithLongNames  = true;
      _startWithMeasureOne = true;
      lw                   = spatium() * 0.3;
      setFlag(ElementFlag::HAS_TAG, true);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LayoutBreak::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);

      writeProperty(xml, P_ID::LAYOUT_BREAK);
      writeProperty(xml, P_ID::PAUSE);

      if (!_startWithLongNames)
            xml.tag("startWithLongNames", _startWithLongNames);
      if (!_startWithMeasureOne)
            xml.tag("startWithMeasureOne", _startWithMeasureOne);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LayoutBreak::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setProperty(P_ID::LAYOUT_BREAK, Ms::getProperty(P_ID::LAYOUT_BREAK, e));
            else if (tag == "pause")
                  _pause = e.readDouble();
            else if (tag == "startWithLongNames")
                  _startWithLongNames = e.readInt();
            else if (tag == "startWithMeasureOne")
                  _startWithMeasureOne = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      layout0();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LayoutBreak::draw(QPainter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;

      QPainterPathStroker stroker;
      stroker.setWidth(lw/2);
      stroker.setJoinStyle(Qt::MiterJoin);
      stroker.setCapStyle(Qt::SquareCap);

      QVector<qreal> dashes ;
      dashes.append(1);
      dashes.append(3);
      stroker.setDashPattern(dashes);
      QPainterPath stroke = stroker.createStroke(path);

      painter->fillPath(stroke, selected() ? MScore::selectColor[0] : MScore::layoutBreakColor);


      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path2);

      }

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void LayoutBreak::layout0()
      {
      qreal _spatium = spatium();
      path      = QPainterPath();
      path2      = QPainterPath();
      qreal h  = _spatium * 2.5;
      qreal w  = _spatium * 2.5;

      QRectF rect(0.0, 0.0, w, h);
      path.addRect(rect);

      switch(layoutBreakType()) {
            case LayoutBreakType::LINE:
                  path2.moveTo(w * .8, h * .3);
                  path2.lineTo(w * .8, h * .6);
                  path2.lineTo(w * .3, h * .6);

                  path2.moveTo(w * .4, h * .5);
                  path2.lineTo(w * .25, h * .6);
                  path2.lineTo(w * .4, h * .7);
                  path2.lineTo(w * .4, h * .5);
                  break;

            case LayoutBreakType::PAGE:
                  path2.moveTo(w*.25, h*.2);
                  path2.lineTo(w*.60, h*.2);
                  path2.lineTo(w*.75, h*.35);
                  path2.lineTo(w*.75, h*.8);
                  path2.lineTo(w*.25, h*.8);
                  path2.lineTo(w*.25, h*.2);

                  path2.moveTo(w*.55, h*.21); // 0.01 to avoid overlap
                  path2.lineTo(w*.55, h*.40);
                  path2.lineTo(w*.74, h*.40);
                  break;

            case LayoutBreakType::SECTION:
                  path2.moveTo(w*.25, h*.2);
                  path2.lineTo(w*.75, h*.2);
                  path2.lineTo(w*.75, h*.8);
                  path2.lineTo(w*.25, h*.8);

                  path2.moveTo(w*.55, h*.21); // 0.01 to avoid overlap
                  path2.lineTo(w*.55, h*.79);
                  break;

            default:
                  qDebug("unknown layout break symbol");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   setLayoutBreakType
//---------------------------------------------------------

void LayoutBreak::setLayoutBreakType(LayoutBreakType val)
      {
      _layoutBreakType = val;
      layout0();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LayoutBreak::spatiumChanged(qreal, qreal)
      {
      lw = spatium() * 0.3;
      layout0();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool LayoutBreak::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == Element::Type::LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->layoutBreakType() != layoutBreakType();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* LayoutBreak::drop(const DropData& data)
      {
      Element* e = data.element;
      score()->undoChangeElement(this, e);
      return e;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant LayoutBreak::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::LAYOUT_BREAK:
                  return int(_layoutBreakType);
            case P_ID::PAUSE:
                  return _pause;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LayoutBreak::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::LAYOUT_BREAK:
                  setLayoutBreakType(LayoutBreakType(v.toInt()));
                  break;
            case P_ID::PAUSE:
                  setPause(v.toDouble());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant LayoutBreak::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::LAYOUT_BREAK:
                  return QVariant(); // LAYOUT_BREAK_LINE;
            case P_ID::PAUSE:
                  return 0.0; // score()->styleD(StyleIdx::SectionPause);
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   undoLayoutBreakType
//---------------------------------------------------------

void LayoutBreak::undoSetLayoutBreakType(LayoutBreakType t)
      {
      undoChangeProperty(P_ID::LAYOUT_BREAK, int(t));
      }

}

