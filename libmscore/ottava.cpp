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
#include "sym.h"

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
      const char* name;
      const char* numbersOnlyName;
      };

// order is important, should be the same as Ottava::Type
static const OttavaDefault ottavaDefault[] = {
      { SymId::ottavaAlta,        SymId::ottava,       QPointF(0.0, .7),    1.0, Element::Placement::ABOVE,  12, "8va", "8"   },
      { SymId::ottavaBassaBa,     SymId::ottava,       QPointF(0.0, -1.0), -1.0, Element::Placement::BELOW, -12, "8vb", "8"   },
      { SymId::quindicesimaAlta,  SymId::quindicesima, QPointF(0.0, .7),    1.0, Element::Placement::ABOVE,  24, "15ma", "15" },
      { SymId::quindicesimaBassa, SymId::quindicesima, QPointF(0.0, -1.0), -1.0, Element::Placement::BELOW, -24, "15mb", "15" },
      { SymId::ventiduesimaAlta,  SymId::ventiduesima, QPointF(0.0, .7),    1.0, Element::Placement::ABOVE,  36, "22ma", "22" },
      { SymId::ventiduesimaBassa, SymId::ventiduesima, QPointF(0.0, -1.0), -1.0, Element::Placement::BELOW, -36, "22mb", "22" }
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
      {
      TextLineSegment::layout1();
      if (parent()) {
            qreal yo(score()->styleS(StyleIdx::ottavaY).val() * spatium());
            if (ottava()->placement() == Element::Placement::BELOW)
                  yo = -yo + staff()->height();
            rypos() += yo;
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::OTTAVA_TYPE:
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
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::OTTAVA_TYPE:
            case P_ID::NUMBERS_ONLY:
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
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::OTTAVA_TYPE:
            case P_ID::PLACEMENT:
            case P_ID::NUMBERS_ONLY:
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
            case P_ID::OTTAVA_TYPE:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::NUMBERS_ONLY:
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
            case P_ID::OTTAVA_TYPE:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::NUMBERS_ONLY:
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
      _numbersOnly        = score()->styleB(StyleIdx::ottavaNumbersOnly);
      numbersOnlyStyle    = PropertyStyle::STYLED;
      beginTextStyle      = PropertyStyle::STYLED;
      continueTextStyle   = PropertyStyle::STYLED;
      setOttavaType(Type::OTTAVA_8VA);
      setLineWidth(score()->styleS(StyleIdx::ottavaLineWidth));
      lineWidthStyle = PropertyStyle::STYLED;
      setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
      lineStyleStyle = PropertyStyle::STYLED;
      setFlag(ElementFlag::ON_STAFF, true);
      }

Ottava::Ottava(const Ottava& o)
   : TextLine(o)
      {
      _numbersOnly = o._numbersOnly;
      _pitchShift  = o._pitchShift;
      lineStyleStyle = o.lineStyleStyle;
      setOttavaType(o._ottavaType);
      }

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(Type val)
      {
      setEndHook(true);
      _ottavaType = val;

      const OttavaDefault* def = &ottavaDefault[int(_ottavaType)];
      if (beginTextStyle == PropertyStyle::STYLED)
            setBeginText(propertyDefault(P_ID::BEGIN_TEXT).toString(), TextStyleType::OTTAVA);
      if (continueTextStyle == PropertyStyle::STYLED)
            setContinueText(propertyDefault(P_ID::CONTINUE_TEXT).toString(), TextStyleType::OTTAVA);

      setEndHookHeight(score()->styleS(StyleIdx::ottavaHook) * def->hookDirection);
      setPlacement(def->place);
      _pitchShift = def->shift;
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
            s->updateOttava();
            score()->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
            score()->setPlaylistDirty(true);
            }
      TextLine::endEdit();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      if (!xml.canWrite(this)) return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      writeProperty(xml, P_ID::NUMBERS_ONLY);
      xml.tag("subtype", ottavaDefault[int(ottavaType())].name);
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
      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype") {
                  QString s = e.readElementText();
                  bool ok;
                  int idx = s.toInt(&ok);
                  if (!ok) {
                        idx = int(Type::OTTAVA_8VA);
                        for (unsigned i = 0; i < sizeof(ottavaDefault)/sizeof(*ottavaDefault); ++i) {
                              if (s == ottavaDefault[i].name) {
                                    idx = i;
                                    break;
                                    }
                              }
                        }
                  else if (score()->mscVersion() <= 114) {
                        //subtype are now in a different order...
                        if (idx == 1)
                              idx = 2;
                        else if (idx == 2)
                              idx = 1;
                        }
                  setOttavaType(Type(idx));
                  }
            else if (tag == "numbersOnly") {
                  _numbersOnly = e.readInt();
                  numbersOnlyStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "lineStyle") {
                  setLineStyle(Qt::PenStyle(e.readInt()));
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "beginSymbol") {
                  beginTextStyle = PropertyStyle::UNSTYLED;
                  QString text(e.readElementText());
                  setBeginText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
                  }
            else if (tag == "continueSymbol") {
                  continueTextStyle = PropertyStyle::UNSTYLED;
                  QString text(e.readElementText());
                  setContinueText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
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
            case P_ID::OTTAVA_TYPE:
                  return int(ottavaType());
            case P_ID::NUMBERS_ONLY:
                  return _numbersOnly;
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
            case P_ID::OTTAVA_TYPE:
                  setOttavaType(Type(val.toInt()));
                  break;

            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
                  break;

            case P_ID::LINE_STYLE:
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
                  break;

            case P_ID::NUMBERS_ONLY:
                  setNumbersOnly(val.toBool());
                  setOttavaType(_ottavaType);
                  numbersOnlyStyle = PropertyStyle::UNSTYLED;
                  break;

            case P_ID::SPANNER_TICK2:
                  setTick2(val.toInt());
                  staff()->updateOttava();
                  break;

            case P_ID::SPANNER_TICK:
                  setTick(val.toInt());
                  staff()->updateOttava();
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
            case P_ID::OTTAVA_TYPE:
                  return 0;

            case P_ID::LINE_WIDTH:
                  return score()->styleS(StyleIdx::ottavaLineWidth).val();

            case P_ID::LINE_STYLE:
                  return int(score()->styleI(StyleIdx::ottavaLineStyle));

            case P_ID::PLACEMENT:
                  return int(ottavaDefault[int(_ottavaType)].place);

            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleS(StyleIdx::ottavaHook).val() * ottavaDefault[int(_ottavaType)].hookDirection;

            case P_ID::NUMBERS_ONLY:
                  return score()->styleB(StyleIdx::ottavaNumbersOnly);

            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
                  {
                  const OttavaDefault* def = &ottavaDefault[int(_ottavaType)];
                  SymId id = _numbersOnly ? def->numbersOnlyId : def->id;
                  QString s;
                  if (symIsValid(id))
                        s = QString("<sym>%1</sym>").arg(Sym::id2name(id));
                  else
                        s = _numbersOnly ? def->numbersOnlyName : def->name;
                  return s;
                  }

            case P_ID::END_TEXT:
                  return QString("");

            case P_ID::BEGIN_TEXT_STYLE:
            case P_ID::CONTINUE_TEXT_STYLE:
            case P_ID::END_TEXT_STYLE:
                  return QVariant::fromValue(score()->textStyle(TextStyleType::OTTAVA));

            default:
                  return TextLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   undoSetOttavaType
//---------------------------------------------------------

void Ottava::undoSetOttavaType(Type val)
      {
      score()->undoChangeProperty(this, P_ID::OTTAVA_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//    used in musicxml import
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      qreal _spatium = spatium();
      qreal yo(score()->styleS(StyleIdx::ottavaY).val() * _spatium);
      if (placement() == Element::Placement::BELOW)
            yo = -yo + staff()->height();
      rUserYoffset() += val * _spatium - yo;
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Ottava::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::OTTAVA_TYPE:
                  return PropertyStyle::NOSTYLE;

            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;

            case P_ID::LINE_STYLE:
                  return lineStyleStyle;

            case P_ID::NUMBERS_ONLY:
                  return numbersOnlyStyle;

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
            case P_ID::OTTAVA_TYPE:
                  return;

            case P_ID::LINE_WIDTH:
                  setLineWidth(score()->styleS(StyleIdx::ottavaLineWidth));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::LINE_STYLE:
                  setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
                  lineStyleStyle = PropertyStyle::STYLED;
                  break;

            case P_ID::NUMBERS_ONLY:
                  setNumbersOnly(score()->styleB(StyleIdx::ottavaNumbersOnly));
                  numbersOnlyStyle = PropertyStyle::STYLED;
                  setOttavaType(_ottavaType);
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
            setLineWidth(score()->styleS(StyleIdx::ottavaLineWidth));
      if (lineStyleStyle == PropertyStyle::STYLED)
            setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
      if (numbersOnlyStyle == PropertyStyle::STYLED)
            setNumbersOnly(score()->styleB(StyleIdx::ottavaNumbersOnly));
      setOttavaType(_ottavaType);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Ottava::reset()
      {
      if (lineWidthStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyStyle::STYLED);
      if (lineStyleStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::LINE_STYLE, propertyDefault(P_ID::LINE_STYLE), PropertyStyle::STYLED);
      if (numbersOnlyStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::NUMBERS_ONLY, propertyDefault(P_ID::NUMBERS_ONLY), PropertyStyle::STYLED);
      if (beginTextStyle == PropertyStyle::UNSTYLED) {
            ; // TODO score()->undoChangeProperty(this, P_ID::BEGIN_SYMBOL, propertyDefault(P_BEGIN_SYMBOL), PropertyStyle::STYLED);
            }
      if (continueTextStyle == PropertyStyle::UNSTYLED) {
            ; // TODO score()->undoChangeProperty(this, P_ID::CONTINUE_SYMBOL, propertyDefault(P_CONTINUE_SYMBOL), PropertyStyle::STYLED);
            }

      setOttavaType(_ottavaType);

      TextLine::reset();
      }
}

