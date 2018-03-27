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
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::VOLTA_ENDING:
                  return spanner()->getProperty(id);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->getProperty(id);
            }
      return TextLineBaseSegment::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VoltaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::VOLTA_ENDING:
                  return spanner()->setProperty(id, v);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->setProperty(id, v);
            }
      return TextLineBaseSegment::setProperty(id, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VoltaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
            case P_ID::VOLTA_ENDING:
                  return volta()->propertyDefault(id);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->propertyDefault(id);
            }
      return TextLineBaseSegment::propertyDefault(id);
      }

//---------------------------------------------------------
//   Volta
//---------------------------------------------------------

Volta::Volta(Score* s)
   : TextLineBase(s)
      {
      initSubStyle(SubStyleId::VOLTA);

      setBeginTextPlace(PlaceText::BELOW);
      setContinueTextPlace(PlaceText::BELOW);
      setLineVisible(true);
      resetProperty(P_ID::BEGIN_TEXT);
      resetProperty(P_ID::CONTINUE_TEXT);
      resetProperty(P_ID::END_TEXT);
      resetProperty(P_ID::BEGIN_TEXT_PLACE);
      resetProperty(P_ID::CONTINUE_TEXT_PLACE);
      resetProperty(P_ID::END_TEXT_PLACE);
      resetProperty(P_ID::BEGIN_HOOK_TYPE);
      resetProperty(P_ID::END_HOOK_TYPE);

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
            case P_ID::VOLTA_ENDING:
                  return QVariant::fromValue(QList<int>());
            case P_ID::ANCHOR:
                  return int(Anchor::MEASURE);
            case P_ID::BEGIN_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case P_ID::END_HOOK_TYPE:
                  return int(HookType::NONE);
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
                  return "";
            case P_ID::LINE_VISIBLE:
                  return true;
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
                  return int(PlaceText::ABOVE);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Volta::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(text());
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

