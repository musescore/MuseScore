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

#include "volta.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      rypos() = 0.0;
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(StyleIdx::voltaY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant VoltaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::VOLTA_TYPE:
                  return volta()->getProperty(id);
            default:
                  return TextLineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VoltaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::VOLTA_TYPE:
                  return volta()->setProperty(id, v);
            default:
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VoltaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::VOLTA_TYPE:
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::ANCHOR:
            case P_ID::BEGIN_HOOK:
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return volta()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle VoltaSegment::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::VOLTA_TYPE:
                  return PropertyStyle::NOSTYLE;

            case P_ID::LINE_WIDTH:
                  return volta()->propertyStyle(id);

            default:
                  return TextLineSegment::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void VoltaSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::VOLTA_TYPE:
                  return;

            case P_ID::LINE_WIDTH:
                  return volta()->resetProperty(id);

            default:
                  return TextLineSegment::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void VoltaSegment::styleChanged()
      {
      volta()->styleChanged();
      }

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLine(s)
      {
      setBeginText("1.", TextStyleType::VOLTA);

      setBeginTextPlace(PlaceText::BELOW);
      setContinueTextPlace(PlaceText::BELOW);

      setBeginHook(true);
      Spatium hook(s->styleS(StyleIdx::voltaHook));
      setBeginHookHeight(hook);
      setEndHookHeight(hook);
      setAnchor(Anchor::MEASURE);

      setLineWidth(score()->styleS(StyleIdx::voltaLineWidth));
      lineWidthStyle = PropertyStyle::STYLED;
      }

//---------------------------------------------------------
//   setVoltaType
//---------------------------------------------------------

void Volta::setVoltaType(Type val)
      {
      setEndHook(Type::CLOSED == val);
      }

//---------------------------------------------------------
//   voltaType
//---------------------------------------------------------

Volta::Type Volta::voltaType() const
      {
      return endHook() ? Type::CLOSED : Type::OPEN;
      }

//---------------------------------------------------------
//   undoSetVoltaType
//---------------------------------------------------------

void Volta::undoSetVoltaType(Type val)
      {
      undoChangeProperty(P_ID::VOLTA_TYPE, int(val));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s, TextStyleType::VOLTA);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      if (_beginText)
            return _beginText->text();
      else
            return QString();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();

      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "text")            // obsolete
                  setText(e.readElementText());
            else if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  foreach(const QString& l, sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "subtype") {  // obsolete
                  int st = e.readInt();
                  if (st == 1)
                        setEndHook(true);
                  }
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      TextLine::writeProperties(xml);
      QString s;
      foreach(int i, _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Volta::createLineSegment()
      {
      return new VoltaSegment(score());
      }

//---------------------------------------------------------
//   hasEnding
//---------------------------------------------------------

bool Volta::hasEnding(int repeat) const
      {
      foreach (int ending, endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Volta::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::VOLTA_TYPE:
                  return int(voltaType());
            default:
                  break;
            }
      return TextLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(P_ID propertyId, const QVariant& val)
      {
      score()->addRefresh(pageBoundingRect());
      switch (propertyId) {
            case P_ID::VOLTA_TYPE:
                  setVoltaType(Type(val.toInt()));
                  break;
            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  // fall through
            default:
                  if (!TextLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      // layout();
      // score()->addRefresh(pageBoundingRect());
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Volta::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::VOLTA_TYPE:
                  return 0;

            case P_ID::LINE_WIDTH:
                  return score()->styleS(StyleIdx::voltaLineWidth).val();

            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
                  return int(PlaceText::BELOW);

            case P_ID::ANCHOR:
                  return int(Anchor::MEASURE);

            case P_ID::BEGIN_HOOK:
                  return true;

            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleS(StyleIdx::voltaHook).val();

            default:
                  return TextLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Volta::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(StyleIdx::voltaY).val()) * spatium();
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Volta::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::VOLTA_TYPE:
                  return PropertyStyle::NOSTYLE;

            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;

            default:
                  return TextLine::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Volta::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::VOLTA_TYPE:
                  return;

            case P_ID::LINE_WIDTH:
                  setLineWidth(score()->styleS(StyleIdx::voltaLineWidth));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return TextLine::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Volta::styleChanged()
      {
      if (lineWidthStyle == PropertyStyle::STYLED)
            setLineWidth(score()->styleS(StyleIdx::voltaLineWidth));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Volta::reset()
      {
      if (lineWidthStyle == PropertyStyle::UNSTYLED)
            score()->undoChangeProperty(this, P_ID::LINE_WIDTH, propertyDefault(P_ID::LINE_WIDTH), PropertyStyle::STYLED);
      TextLine::reset();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Volta::accessibleInfo()
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(text());
      }

}

