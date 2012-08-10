//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: layoutbreak.cpp 5632 2012-05-15 16:36:57Z wschweer $
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

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      _subtype             = LayoutBreakType(propertyDefault(P_LAYOUT_BREAK).toInt());
      _pause               = score->styleD(ST_SectionPause);
      _startWithLongNames  = true;
      _startWithMeasureOne = true;
      lw                   = spatium() * 0.3;
      setFlag(ELEMENT_HAS_TAG, true);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LayoutBreak::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);

      xml.tag(P_LAYOUT_BREAK, getProperty(P_LAYOUT_BREAK), propertyDefault(P_LAYOUT_BREAK));
      xml.tag(P_PAUSE,        getProperty(P_PAUSE),        propertyDefault(P_PAUSE));

      if (!_startWithLongNames)
            xml.tag("startWithLongNames", _startWithLongNames);
      if (!_startWithMeasureOne)
            xml.tag("startWithMeasureOne", _startWithMeasureOne);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LayoutBreak::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "subtype")
                  setProperty(P_LAYOUT_BREAK, ::getProperty(P_LAYOUT_BREAK, e));
            else if (tag == "pause")
                  _pause = val.toDouble();
            else if (tag == "startWithLongNames")
                  _startWithLongNames = val.toInt();
            else if (tag == "startWithMeasureOne")
                  _startWithMeasureOne = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
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
      painter->setPen(QPen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void LayoutBreak::layout0()
      {
      qreal _spatium = spatium();
      path      = QPainterPath();
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
      qreal w1 = w * .6;

      switch(subtype()) {
            case LAYOUT_BREAK_LINE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);

                  path.moveTo(w * .8, w * .7);
                  path.lineTo(w * .8, w);
                  path.lineTo(w * .2, w);

                  path.moveTo(w * .4, w * .8);
                  path.lineTo(w * .2, w);
                  path.lineTo(w * .4, w * 1.2);
                  break;

            case LAYOUT_BREAK_PAGE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h-w1);
                  path.lineTo(w1, h-w1);
                  path.lineTo(w1, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w, h-w1);
                  path.lineTo(w1, h);
                  break;

            case LAYOUT_BREAK_SECTION:
                  path.lineTo(w, 0.0);
                  path.lineTo(w,  h);
                  path.lineTo(0.0,  h);
                  path.moveTo(w-_spatium * .8,  0.0);
                  path.lineTo(w-_spatium * .8,  h);
                  break;

            default:
                  qDebug("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void LayoutBreak::setSubtype(LayoutBreakType val)
      {
      _subtype = val;
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
      return e->type() == LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->subtype() != subtype();
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
            case P_LAYOUT_BREAK:
                  return _subtype;
            case P_PAUSE:
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
            case P_LAYOUT_BREAK:
                  setSubtype(LayoutBreakType(v.toInt()));
                  break;
            case P_PAUSE:
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
            case P_LAYOUT_BREAK:
                  return QVariant(); // LAYOUT_BREAK_LINE;
            case P_PAUSE:
                  return 0.0; // score()->styleD(ST_SectionPause);
            default:
                  return Element::propertyDefault(id);
            }
      }

