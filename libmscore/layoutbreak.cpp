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
#include "measurebase.h"

namespace Ms {

//---------------------------------------------------------
//   sectionBreakStyle
//---------------------------------------------------------

static const ElementStyle sectionBreakStyle {
      { Sid::SectionPause, Pid::PAUSE }
      };

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score, ElementFlag::SYSTEM | ElementFlag::HAS_TAG)
      {
      _pause = 0.;
      _startWithLongNames = false;
      _startWithMeasureOne = false;
      _firstSystemIndentation = false;
      _layoutBreakType = Type(propertyDefault(Pid::LAYOUT_BREAK).toInt());

      initElementStyle(&sectionBreakStyle);

      resetProperty(Pid::PAUSE);
      resetProperty(Pid::START_WITH_LONG_NAMES);
      resetProperty(Pid::START_WITH_MEASURE_ONE);
      resetProperty(Pid::FIRST_SYSTEM_INDENTATION);
      lw = spatium() * 0.3;
      }

LayoutBreak::LayoutBreak(const LayoutBreak& lb)
   : Element(lb)
      {
      _layoutBreakType        = lb._layoutBreakType;
      lw                      = lb.lw;
      _pause                  = lb._pause;
      _startWithLongNames     = lb._startWithLongNames;
      _startWithMeasureOne    = lb._startWithMeasureOne;
      _firstSystemIndentation = lb._firstSystemIndentation;
      layout0();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LayoutBreak::write(XmlWriter& xml) const
      {
      xml.stag(this);
      Element::writeProperties(xml);

      for (auto id : { Pid::LAYOUT_BREAK, Pid::PAUSE, Pid::START_WITH_LONG_NAMES, Pid::START_WITH_MEASURE_ONE, Pid::FIRST_SYSTEM_INDENTATION })
            writeProperty(xml, id);

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
                  readProperty(e, Pid::LAYOUT_BREAK);
            else if (tag == "pause")
                  readProperty(e, Pid::PAUSE);
            else if (tag == "startWithLongNames")
                  readProperty(e, Pid::START_WITH_LONG_NAMES);
            else if (tag == "startWithMeasureOne")
                  readProperty(e, Pid::START_WITH_MEASURE_ONE);
            else if (tag == "firstSystemIndentation")
                  readProperty(e, Pid::FIRST_SYSTEM_INDENTATION);
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

      switch (layoutBreakType()) {
            case Type::LINE:
                  path2.moveTo(w * .8, h * .3);
                  path2.lineTo(w * .8, h * .6);
                  path2.lineTo(w * .3, h * .6);

                  path2.moveTo(w * .4, h * .5);
                  path2.lineTo(w * .25, h * .6);
                  path2.lineTo(w * .4, h * .7);
                  path2.lineTo(w * .4, h * .5);
                  break;

            case Type::PAGE:
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

            case Type::SECTION:
                  path2.moveTo(w*.25, h*.2);
                  path2.lineTo(w*.75, h*.2);
                  path2.lineTo(w*.75, h*.8);
                  path2.lineTo(w*.25, h*.8);

                  path2.moveTo(w*.55, h*.21); // 0.01 to avoid overlap
                  path2.lineTo(w*.55, h*.79);
                  break;

            case Type::NOBREAK:
                  path2.moveTo(w * .1,  h * .5);
                  path2.lineTo(w * .9,  h * .5);

                  path2.moveTo(w * .7, h * .3);
                  path2.lineTo(w * .5, h * .5);
                  path2.lineTo(w * .7, h * .7);
                  path2.lineTo(w * .7, h * .3);

                  path2.moveTo(w * .3,  h * .3);
                  path2.lineTo(w * .5,  h * .5);
                  path2.lineTo(w * .3,  h * .7);
                  path2.lineTo(w * .3,  h * .3);
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

void LayoutBreak::setLayoutBreakType(Type val)
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

bool LayoutBreak::acceptDrop(EditData& data) const
      {
      return data.dropElement->type() == ElementType::LAYOUT_BREAK
         && toLayoutBreak(data.dropElement)->layoutBreakType() != layoutBreakType();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* LayoutBreak::drop(EditData& data)
      {
      Element* e = data.dropElement;
      score()->undoChangeElement(this, e);
      return e;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant LayoutBreak::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LAYOUT_BREAK:
                  return int(_layoutBreakType);
            case Pid::PAUSE:
                  return _pause;
            case Pid::START_WITH_LONG_NAMES:
                  return _startWithLongNames;
            case Pid::START_WITH_MEASURE_ONE:
                  return _startWithMeasureOne;
            case Pid::FIRST_SYSTEM_INDENTATION:
                  return _firstSystemIndentation;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LayoutBreak::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::LAYOUT_BREAK:
                  setLayoutBreakType(Type(v.toInt()));
                  break;
            case Pid::PAUSE:
                  setPause(v.toDouble());
                  break;
            case Pid::START_WITH_LONG_NAMES:
                  setStartWithLongNames(v.toBool());
                  break;
            case Pid::START_WITH_MEASURE_ONE:
                  setStartWithMeasureOne(v.toBool());
                  break;
            case Pid::FIRST_SYSTEM_INDENTATION:
                  setFirstSystemIndentation(v.toBool());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      if (parent() && measure()->next())
            measure()->next()->triggerLayout();
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant LayoutBreak::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::LAYOUT_BREAK:
                  return QVariant(); // LAYOUT_BREAK_LINE;
            case Pid::PAUSE:
                  return score()->styleD(Sid::SectionPause);
            case Pid::START_WITH_LONG_NAMES:
                  return true;
            case Pid::START_WITH_MEASURE_ONE:
                  return true;
            case Pid::FIRST_SYSTEM_INDENTATION:
                  return true;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid LayoutBreak::propertyId(const QStringRef& name) const
      {
      if (name == propertyName(Pid::LAYOUT_BREAK))
            return Pid::LAYOUT_BREAK;
      return Element::propertyId(name);
      }
}

