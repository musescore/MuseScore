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

#include "score.h"
#include "textline.h"
#include "staff.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Score* s)
   : TextLineBaseSegment(s)
      {
      setPlacement(Placement::ABOVE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());

      TextLineBaseSegment::layout();
      if (parent()) {
            if (textLine()->placeBelow()) {
                  qreal sh = staff() ? staff()->height() : 0.0;
                  rypos() = sh + score()->styleP(Sid::textLinePosBelow) * mag();
                  }
            else
                  rypos() = score()->styleP(Sid::textLinePosAbove) * mag();
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());
                  if (textLine()->placeAbove()) {
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
            }
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : TextLineBase(s)
      {
      setPlacement(Placement::ABOVE);
      setBeginText("");
      setContinueText("");
      setEndText("");
      setBeginTextOffset(QPointF(0,0));
      setContinueTextOffset(QPointF(0,0));
      setEndTextOffset(QPointF(0,0));
      setLineVisible(true);
      setBeginHookType(HookType::NONE);
      setEndHookType(HookType::NONE);
      setBeginTextPlace(PlaceText::LEFT);
//      setBeginTextPlace(PlaceText::AUTO);
      setContinueTextPlace(PlaceText::AUTO);
      setEndTextPlace(PlaceText::AUTO);
      setBeginHookHeight(Spatium(1.5));
      setEndHookHeight(Spatium(1.5));

      resetProperty(Pid::BEGIN_TEXT_ALIGN);
      resetProperty(Pid::CONTINUE_TEXT_ALIGN);
      resetProperty(Pid::END_TEXT_ALIGN);

      initSubStyle(SubStyleId::TEXTLINE);
      }

TextLine::TextLine(const TextLine& tl)
   : TextLineBase(tl)
      {
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      TextLineSegment* seg = new TextLineSegment(score());
      // note-anchored line segments are relative to system not to staff
      if (anchor() == Spanner::Anchor::NOTE)
            seg->setFlag(ElementFlag::ON_STAFF, false);
      return seg;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant TextLine::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLine::setProperty(Pid propertyId, const QVariant& val)
      {
      return TextLineBase::setProperty(propertyId, val);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLine::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::PLACEMENT:
                  return int(Placement::ABOVE);
            case Pid::BEGIN_TEXT:
            case Pid::CONTINUE_TEXT:
            case Pid::END_TEXT:
                  return "";
            case Pid::LINE_VISIBLE:
                  return true;
            case Pid::BEGIN_TEXT_OFFSET:
            case Pid::CONTINUE_TEXT_OFFSET:
            case Pid::END_TEXT_OFFSET:
                  return QPointF(0,0);
            case Pid::BEGIN_HOOK_TYPE:
            case Pid::END_HOOK_TYPE:
                  return int(HookType::NONE);
            case Pid::BEGIN_TEXT_PLACE:
            case Pid::CONTINUE_TEXT_PLACE:
            case Pid::END_TEXT_PLACE:
                  return int(PlaceText::AUTO);
            case Pid::BEGIN_HOOK_HEIGHT:
            case Pid::END_HOOK_HEIGHT:
                  return Spatium(1.5);
            case Pid::BEGIN_TEXT_ALIGN:
            case Pid::END_TEXT_ALIGN:
            case Pid::CONTINUE_TEXT_ALIGN:
//                  return QVariant::fromValue(Align::LEFT | Align::TOP);
                  return QVariant::fromValue(Align::LEFT | Align::VCENTER);
            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }



}     // namespace Ms

