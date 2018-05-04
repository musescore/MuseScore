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
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VoltaSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());
      TextLineBaseSegment::layout();
      if (!parent())
            return;
      rypos() = score()->styleP(StyleIdx::voltaY) * mag();
      if (autoplace()) {
            qreal minDistance = spatium() * .7;
            Shape s1 = shape().translated(pos());
            qreal d  = system()->topDistance(staffIdx(), s1);
            if (d > -minDistance)
                  rUserYoffset() = -d - minDistance;
            }
      else
            adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant VoltaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::VOLTA_ENDING:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::BEGIN_TEXT_OFFSET:
                  return volta()->getProperty(id);
            default:
                  return TextLineBaseSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VoltaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::VOLTA_ENDING:
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::BEGIN_TEXT_OFFSET:
                  return volta()->setProperty(id, v);
            default:
                  return TextLineBaseSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VoltaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::ANCHOR:
//            case P_ID::BEGIN_HOOK:
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
            case P_ID::VOLTA_ENDING:
            case P_ID::BEGIN_TEXT_OFFSET:
            case P_ID::BEGIN_FONT_BOLD:
                  return volta()->propertyDefault(id);
            default:
                  return TextLineBaseSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx VoltaSegment::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::BEGIN_FONT_FACE:
            case P_ID::BEGIN_FONT_SIZE:
            case P_ID::BEGIN_FONT_BOLD:
            case P_ID::BEGIN_FONT_ITALIC:
            case P_ID::BEGIN_FONT_UNDERLINE:
            case P_ID::BEGIN_TEXT_ALIGN:
            case P_ID::BEGIN_TEXT_OFFSET:
                  return volta()->getPropertyStyle(id);

            default:
                  return TextLineBaseSegment::getPropertyStyle(id);
            }
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags& VoltaSegment::propertyFlags(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::LINE_COLOR:
                  return volta()->propertyFlags(id);

            default:
                  return TextLineBaseSegment::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void VoltaSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::VOLTA_ENDING:
            case P_ID::LINE_WIDTH:
                  return volta()->resetProperty(id);

            default:
                  return TextLineBaseSegment::resetProperty(id);
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
   : TextLineBase(s)
      {
      init();
      setBeginTextPlace(PlaceText::BELOW);
      setContinueTextPlace(PlaceText::BELOW);

      setBeginHookType(HookType::HOOK_90);
      setAnchor(Anchor::MEASURE);
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Volta::setText(const QString& s)
      {
      setBeginText(s);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Volta::text() const
      {
      return beginText();
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
            if (tag == "endings") {
                  QString s = e.readElementText();
                  QStringList sl = s.split(",", QString::SkipEmptyParts);
                  _endings.clear();
                  for (const QString& l : sl) {
                        int i = l.simplified().toInt();
                        _endings.append(i);
                        }
                  }
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(XmlWriter& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      TextLineBase::writeProperties(xml);
      QString s;
      for (int i : _endings) {
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
      for (int ending : endings()) {
            if (ending == repeat)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   lastEnding
//---------------------------------------------------------

int Volta::lastEnding() const
      {
      if (_endings.isEmpty())
            return 0;
      return _endings.last();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Volta::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::VOLTA_ENDING:
                  return QVariant::fromValue(endings());
            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_ID::VOLTA_ENDING:
                  setEndings(val.value<QList<int>>());
                  break;
            default:
                  if (!TextLineBase::setProperty(propertyId, val))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Volta::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_WIDTH:
                  return score()->styleV(StyleIdx::voltaLineWidth);

            case P_ID::LINE_STYLE:
                  return score()->styleV(StyleIdx::voltaLineStyle);

            case P_ID::BEGIN_FONT_BOLD:
                  return score()->styleV(StyleIdx::voltaFontBold);

            case P_ID::BEGIN_FONT_SIZE:
                  return score()->styleV(StyleIdx::voltaFontSize);

            case P_ID::VOLTA_ENDING:
                  return QVariant::fromValue(QList<int>());

            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
                  return int(PlaceText::BELOW);

            case P_ID::ANCHOR:
                  return int(Anchor::MEASURE);

            case P_ID::BEGIN_HOOK_TYPE:
                  return int(HookType::HOOK_90);

            case P_ID::BEGIN_TEXT_OFFSET:
                  return QPointF(0.5, 1.9);

            case P_ID::BEGIN_TEXT_ALIGN:
                  return QVariant::fromValue(Align::BASELINE);

            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleV(StyleIdx::voltaHook);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Volta::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(text());
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Volta::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return StyleIdx::voltaLineWidth;
            case P_ID::LINE_STYLE:
                  return StyleIdx::voltaLineStyle;
            case P_ID::BEGIN_FONT_FACE:
                  return StyleIdx::voltaFontFace;
            case P_ID::BEGIN_FONT_SIZE:
                  return StyleIdx::voltaFontSize;
            case P_ID::BEGIN_FONT_BOLD:
                  return StyleIdx::voltaFontBold;
            case P_ID::BEGIN_FONT_ITALIC:
                  return StyleIdx::voltaFontItalic;
            case P_ID::BEGIN_FONT_UNDERLINE:
                  return StyleIdx::voltaFontUnderline;
            case P_ID::BEGIN_TEXT_ALIGN:
                  return StyleIdx::voltaAlign;
            case P_ID::BEGIN_TEXT_OFFSET:
                  return StyleIdx::voltaOffset;
            default:
                  break;
            }
      return TextLineBase::getPropertyStyle(id);
      }

//---------------------------------------------------------
//   setVoltaType
//    deprecated
//---------------------------------------------------------

void Volta::setVoltaType(Type val)
      {
      setEndHookType(Type::CLOSED == val ? HookType::HOOK_90 : HookType::NONE);
      }

//---------------------------------------------------------
//   voltaType
//    deprecated
//---------------------------------------------------------

Volta::Type Volta::voltaType() const
      {
      return endHookType() != HookType::NONE ? Type::CLOSED : Type::OPEN;
      }


}

