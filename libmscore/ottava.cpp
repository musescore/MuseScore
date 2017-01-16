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
      if (autoplace())
            setUserOff(QPointF());

      TextLineBaseSegment::layout();
      if (parent()) {
            qreal yo = score()->styleP(ottava()->placeBelow() ? StyleIdx::ottavaPosBelow : StyleIdx::ottavaPosAbove) * mag();
            rypos() += yo;
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());
                  if (ottava()->placeAbove()) {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = -d - minDistance;
                        }
                  else {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = d + minDistance;
                        }
                  }
            else
                  adjustReadPos();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::OTTAVA_TYPE:
            case P_ID::PLACEMENT:
            case P_ID::NUMBERS_ONLY:
                  return ottava()->getProperty(id);
            default:
                  return TextLineBaseSegment::getProperty(id);
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
            case P_ID::PLACEMENT:
            case P_ID::NUMBERS_ONLY:
                  return ottava()->setProperty(id, v);
            default:
                  return TextLineBaseSegment::setProperty(id, v);
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
                  return TextLineBaseSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags OttavaSegment::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::OTTAVA_TYPE:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
            case P_ID::NUMBERS_ONLY:
                  return ottava()->propertyFlags(id);

            default:
                  return TextLineBaseSegment::propertyFlags(id);
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
                  return TextLineBaseSegment::resetProperty(id);
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
   : TextLineBase(s)
      {
      _numbersOnly = score()->styleB(StyleIdx::ottavaNumbersOnly);
      setOttavaType(Type::OTTAVA_8VA);
      setLineWidth(score()->styleS(StyleIdx::ottavaLineWidth));
      setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
      setFlag(ElementFlag::ON_STAFF, true);
      }

Ottava::Ottava(const Ottava& o)
   : TextLineBase(o)
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
      if (beginTextStyle == PropertyFlags::STYLED) {
            setBeginText(propertyDefault(P_ID::BEGIN_TEXT).toString(), SubStyle::OTTAVA);
            beginTextElement()->setGenerated(true);
            }
      if (continueTextStyle == PropertyFlags::STYLED) {
            setContinueText(propertyDefault(P_ID::CONTINUE_TEXT).toString(), SubStyle::OTTAVA);
            continueTextElement()->setGenerated(true);
            }

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
            score()->setPlaylistDirty();
            }
      TextLineBase::endEdit();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      writeProperty(xml, P_ID::NUMBERS_ONLY);
      xml.tag("subtype", ottavaDefault[int(ottavaType())].name);
      TextLineBase::writeProperties(xml);
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
                  numbersOnlyStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "lineStyle") {
                  setLineStyle(Qt::PenStyle(e.readInt()));
                  lineStyleStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "beginSymbol") {                      // obsolete
                  beginTextStyle = PropertyFlags::UNSTYLED;
                  QString text(e.readElementText());
                  setBeginText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
                  }
            else if (tag == "continueSymbol") {                   // obsolete
                  continueTextStyle = PropertyFlags::UNSTYLED;
                  QString text(e.readElementText());
                  setContinueText(QString("<sym>%1</sym>").arg(text[0].isNumber() ? Sym::id2name(SymId(text.toInt())) : text));
                  }
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      if (beginText() != propertyDefault(P_ID::BEGIN_TEXT))
            beginTextStyle = PropertyFlags::UNSTYLED;
      if (continueText() != propertyDefault(P_ID::CONTINUE_TEXT))
            continueTextStyle = PropertyFlags::UNSTYLED;
      }

//---------------------------------------------------------
//   undoSetOttavaType
//---------------------------------------------------------

void Ottava::undoSetOttavaType(Type val)
      {
      undoChangeProperty(P_ID::OTTAVA_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//    used in musicxml import
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? StyleIdx::ottavaPosAbove : StyleIdx::ottavaPosBelow);
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
      return TextLineBase::getProperty(propertyId);
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

            case P_ID::PLACEMENT:
                  if (val != getProperty(propertyId)) {
                        // reverse hooks
                        setBeginHookHeight(-beginHookHeight());
                        setEndHookHeight(-endHookHeight());
                        }
                  setPlacement(Placement(val.toInt()));
                  break;

            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyFlags::UNSTYLED;
                  TextLineBase::setProperty(propertyId, val);
                  break;

            case P_ID::LINE_STYLE:
                  lineStyleStyle = PropertyFlags::UNSTYLED;
                  TextLineBase::setProperty(propertyId, val);
                  break;

            case P_ID::NUMBERS_ONLY:
                  setNumbersOnly(val.toBool());
                  setOttavaType(_ottavaType);
                  numbersOnlyStyle = PropertyFlags::UNSTYLED;
                  break;

            case P_ID::SPANNER_TICKS:
                  setTicks(val.toInt());
                  staff()->updateOttava();
                  break;

            case P_ID::SPANNER_TICK:
                  setTick(val.toInt());
                  staff()->updateOttava();
                  break;

            default:
                  if (!TextLineBase::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll();
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
                  return score()->styleV(StyleIdx::ottavaLineWidth);

            case P_ID::LINE_STYLE:
                  return int(score()->styleI(StyleIdx::ottavaLineStyle));

            case P_ID::PLACEMENT:
                  return int(ottavaDefault[int(_ottavaType)].place);

            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleS(StyleIdx::ottavaHook) * ottavaDefault[int(_ottavaType)].hookDirection;

            case P_ID::NUMBERS_ONLY:
                  return score()->styleB(StyleIdx::ottavaNumbersOnly);

            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
                  {
                  const OttavaDefault* def = &ottavaDefault[int(_ottavaType)];
                  SymId id = _numbersOnly ? def->numbersOnlyId : def->id;
                  return QString("<sym>%1</sym>").arg(Sym::id2name(id));
                  }

            case P_ID::END_TEXT:
                  return QString("");

            case P_ID::END_HOOK:
                  return true;

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags Ottava::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::OTTAVA_TYPE:
            case P_ID::PLACEMENT:
                  return PropertyFlags::NOSTYLE;

            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;

            case P_ID::LINE_STYLE:
                  return lineStyleStyle;

            case P_ID::NUMBERS_ONLY:
                  return numbersOnlyStyle;

            default:
                  return TextLineBase::propertyFlags(id);
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
                  lineWidthStyle = PropertyFlags::STYLED;
                  break;

            case P_ID::LINE_STYLE:
                  setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
                  lineStyleStyle = PropertyFlags::STYLED;
                  break;

            case P_ID::NUMBERS_ONLY:
                  setNumbersOnly(score()->styleB(StyleIdx::ottavaNumbersOnly));
                  numbersOnlyStyle = PropertyFlags::STYLED;
                  setOttavaType(_ottavaType);
                  break;

            case P_ID::PLACEMENT:
                  setProperty(id, propertyDefault(id));
                  break;

            default:
                  return TextLineBase::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Ottava::styleChanged()
      {
      if (lineWidthStyle == PropertyFlags::STYLED)
            setLineWidth(score()->styleS(StyleIdx::ottavaLineWidth));
      if (lineStyleStyle == PropertyFlags::STYLED)
            setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::ottavaLineStyle)));
      if (numbersOnlyStyle == PropertyFlags::STYLED)
            setNumbersOnly(score()->styleB(StyleIdx::ottavaNumbersOnly));
//      setOttavaType(_ottavaType);       // this resets placement
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Ottava::reset()
      {
      if (lineWidthStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyFlags::STYLED);
      if (lineStyleStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(P_ID::LINE_STYLE, propertyDefault(P_ID::LINE_STYLE), PropertyFlags::STYLED);
      if (numbersOnlyStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(P_ID::NUMBERS_ONLY, propertyDefault(P_ID::NUMBERS_ONLY), PropertyFlags::STYLED);
      if (beginTextStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(P_ID::BEGIN_TEXT, propertyDefault(P_ID::BEGIN_TEXT), PropertyFlags::STYLED);
      if (continueTextStyle == PropertyFlags::UNSTYLED)
            undoChangeProperty(P_ID::CONTINUE_TEXT, propertyDefault(P_ID::CONTINUE_TEXT), PropertyFlags::STYLED);
      setOttavaType(_ottavaType);
      TextLineBase::reset();
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Ottava::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::NUMBERS_ONLY:
                  return StyleIdx::ottavaNumbersOnly;
            case P_ID::LINE_WIDTH:
                  return StyleIdx::ottavaLineWidth;
            case P_ID::LINE_STYLE:
                  return StyleIdx::ottavaLineStyle;
            case P_ID::PLACEMENT:
                  return StyleIdx::ottavaPlacement;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Ottava::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(ottavaDefault[static_cast<int>(ottavaType())].name);
      }

}

