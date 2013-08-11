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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "text.h"
#include "staff.h"
#include "segment.h"

namespace Ms {

//---------------------------------------------------------
//   OttavaDefault
//---------------------------------------------------------

struct OttavaDefault {
      SymId id;
      SymId numbersOnlyId;
      QPointF offset;
      qreal  hookDirection;
      Element::Placement place;
      int shift;
      };

static const OttavaDefault ottavaDefault[] = {
      { octave8va,  octave8,  QPointF(0.0, .7),    1.0, Element::ABOVE,  12 },
      { octave8vb,  octave8,  QPointF(0.0, -1.0), -1.0, Element::BELOW, -12 },
      { octave15ma, octave15, QPointF(0.0, .7),    1.0, Element::ABOVE,  24 },
      { octave15mb, octave15, QPointF(0.0, -1.0), -1.0, Element::BELOW, -24 },
      { octave22ma, octave22, QPointF(0.0, .7),    1.0, Element::ABOVE,  36 },
      { octave22mb, octave22, QPointF(0.0, -1.0), -1.0, Element::BELOW, -36 }
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
      {
      TextLineSegment::layout1();
      if (parent()) {     // for palette
            qreal yo(score()->styleS(ST_ottavaY).val() * spatium());
            if (ottava()->placement() == BELOW)
                  yo = -yo + staff()->height();
            rypos() = yo;
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return ottava()->getProperty(id);
            default:
                  return TextLineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool OttavaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
            case P_OTTAVA_TYPE:
                  return ottava()->setProperty(id, v);
            default:
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant OttavaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
            case P_OTTAVA_TYPE:
            case P_PLACEMENT:
                  return ottava()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle OttavaSegment::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_OTTAVA_TYPE:
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return ottava()->propertyStyle(id);

            default:
                  return TextLineSegment::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void OttavaSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_OTTAVA_TYPE:
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return ottava()->resetProperty(id);

            default:
                  return TextLineSegment::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void OttavaSegment::styleChanged()
      {
      ottava()->styleChanged();
      }

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLine(s)
      {
      setOttavaType(OTTAVA_8VA);
      setLineWidth(score()->styleS(ST_ottavaLineWidth));
      lineWidthStyle = PropertyStyle::STYLED;
      setLineStyle(Qt::PenStyle(score()->styleI(ST_ottavaLineStyle)));
      lineStyleStyle = PropertyStyle::STYLED;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout()
      {
      setPos(0.0, 0.0);
      TextLine::layout();
      }

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
      {
      setEndHook(true);
      _ottavaType = val;

      Spatium hook(score()->styleS(ST_ottavaHook));

      SymId id;
      if (score()->styleB(ST_ottavaNumbersOnly))
            id = ottavaDefault[val].numbersOnlyId;
      else
            id = ottavaDefault[val].id;
      setBeginSymbol(id);
      setContinueSymbol(id);

      setBeginSymbolOffset(ottavaDefault[val].offset);
      setContinueSymbolOffset(ottavaDefault[val].offset);
      setEndHookHeight(hook * ottavaDefault[val].hookDirection);
      setPlacement(ottavaDefault[val].place);
      _pitchShift = ottavaDefault[val].shift;

      foreach(SpannerSegment* s, spannerSegments()) {
            OttavaSegment* os = static_cast<OttavaSegment*>(s);
            os->clearText();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      return new OttavaSegment(score());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Ottava::endEdit()
      {
      if (editTick != tick() || editTick2 != tick2()) {
            Staff* s = staff();
            s->pitchOffsets().remove(editTick);
            s->pitchOffsets().remove(editTick2);

            s->updateOttava(this);
            score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", ottavaType());
      TextLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setOttavaType(OttavaType(e.readInt()));
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "lineStyle") {
                  setLineStyle(Qt::PenStyle(e.readInt()));
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  }
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ottava::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_OTTAVA_TYPE:
                  return ottavaType();
            default:
                  break;
            }
      return TextLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_OTTAVA_TYPE:
                  setOttavaType(OttavaType(val.toInt()));
                  break;

            case P_LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
                  break;

            case P_LINE_STYLE:
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
                  break;

            default:
                  if (!TextLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Ottava::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_OTTAVA_TYPE:
                  return 0;

            case P_LINE_WIDTH:
                  return score()->styleS(ST_ottavaLineWidth).val();

            case P_LINE_STYLE:
                  return int(score()->styleI(ST_ottavaLineStyle));

            case P_PLACEMENT:
                  return ottavaDefault[_ottavaType].place;

            case P_END_HOOK_HEIGHT:
                  return score()->styleS(ST_ottavaHook).val() * ottavaDefault[_ottavaType].hookDirection;

            case P_BEGIN_SYMBOL:
            case P_CONTINUE_SYMBOL:
                  return ottavaDefault[_ottavaType].id;

            case P_BEGIN_SYMBOL_OFFSET:
            case P_CONTINUE_SYMBOL_OFFSET:
                  return ottavaDefault[_ottavaType].offset;

            default:
                  return TextLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   undoSetOttavaType
//---------------------------------------------------------

void Ottava::undoSetOttavaType(OttavaType val)
      {
      score()->undoChangeProperty(this, P_OTTAVA_TYPE, val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      qreal _spatium = spatium();
      qreal yo(score()->styleS(ST_ottavaY).val() * _spatium);
      if (placement() == BELOW)
            yo = -yo + staff()->height();
      rUserYoffset() += val * _spatium - yo;
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Ottava::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return PropertyStyle::NOSTYLE;

            case P_LINE_WIDTH:
                  return lineWidthStyle;

            case P_LINE_STYLE:
                  return lineStyleStyle;

            default:
                  return TextLine::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Ottava::resetProperty(P_ID id)
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return;

            case P_LINE_WIDTH:
                  setLineWidth(score()->styleS(ST_ottavaLineWidth));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            case P_LINE_STYLE:
                  setLineStyle(Qt::PenStyle(score()->styleI(ST_ottavaLineStyle)));
                  lineStyleStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return TextLine::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Ottava::styleChanged()
      {
      if (lineWidthStyle == PropertyStyle::STYLED)
            setLineWidth(score()->styleS(ST_ottavaLineWidth));
      if (lineStyleStyle == PropertyStyle::STYLED)
            setLineStyle(Qt::PenStyle(score()->styleI(ST_ottavaLineStyle)));
      setOttavaType(_ottavaType);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Ottava::reset()
      {
      if (lineWidthStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_LINE_WIDTH, propertyDefault(P_LINE_WIDTH), PropertyStyle::STYLED);
      if (lineStyleStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_LINE_STYLE, propertyDefault(P_LINE_STYLE), PropertyStyle::STYLED);
      TextLine::reset();
      }

}

