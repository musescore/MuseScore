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
      rypos() = score()->styleP(Sid::voltaY) * mag();
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

QVariant VoltaSegment::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::BEGIN_HOOK_TYPE:
            case Pid::END_HOOK_TYPE:
            case Pid::VOLTA_ENDING:
                  return spanner()->getProperty(id);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
                  return spanner()->getProperty(id);
            }
      return TextLineBaseSegment::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool VoltaSegment::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::BEGIN_HOOK_TYPE:
            case Pid::END_HOOK_TYPE:
            case Pid::VOLTA_ENDING:
                  return spanner()->setProperty(id, v);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
                  return spanner()->setProperty(id, v);
            }
      return TextLineBaseSegment::setProperty(id, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant VoltaSegment::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::BEGIN_HOOK_TYPE:
            case Pid::END_HOOK_TYPE:
            case Pid::VOLTA_ENDING:
                  return volta()->propertyDefault(id);
            default:
                  break;
            }
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
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
      resetProperty(Pid::BEGIN_TEXT);
      resetProperty(Pid::CONTINUE_TEXT);
      resetProperty(Pid::END_TEXT);
      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::END_TEXT_PLACE);
      resetProperty(Pid::BEGIN_HOOK_TYPE);
      resetProperty(Pid::END_HOOK_TYPE);

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

QVariant Volta::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
                  return QVariant::fromValue(endings());
            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Volta::setProperty(Pid propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
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

QVariant Volta::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::VOLTA_ENDING:
                  return QVariant::fromValue(QList<int>());
            case Pid::ANCHOR:
                  return int(Anchor::MEASURE);
            case Pid::BEGIN_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case Pid::END_HOOK_TYPE:
                  return int(HookType::NONE);
            case Pid::BEGIN_TEXT:
            case Pid::CONTINUE_TEXT:
            case Pid::END_TEXT:
                  return "";
            case Pid::LINE_VISIBLE:
                  return true;
            case Pid::BEGIN_TEXT_PLACE:
            case Pid::CONTINUE_TEXT_PLACE:
            case Pid::END_TEXT_PLACE:
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

