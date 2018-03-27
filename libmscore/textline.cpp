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
                  rypos() = sh + score()->styleP(StyleIdx::textLinePosBelow) * mag();
                  }
            else
                  rypos() = score()->styleP(StyleIdx::textLinePosAbove) * mag();
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
            else
                  adjustReadPos();
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
      setBeginTextPlace(PlaceText::AUTO);
      setContinueTextPlace(PlaceText::AUTO);
      setEndTextPlace(PlaceText::AUTO);
      setBeginHookHeight(Spatium(1.5));
      setEndHookHeight(Spatium(1.5));

      resetProperty(P_ID::BEGIN_TEXT_ALIGN);
      resetProperty(P_ID::CONTINUE_TEXT_ALIGN);
      resetProperty(P_ID::END_TEXT_ALIGN);

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

QVariant TextLine::getProperty(P_ID propertyId) const
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

bool TextLine::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_ID::PLACEMENT:
                  if (val != getProperty(propertyId)) {
                        // reverse hooks
                        setBeginHookHeight(-beginHookHeight());
                        setEndHookHeight(-endHookHeight());
                        }
                  TextLineBase::setProperty(propertyId, val);
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

QVariant TextLine::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::PLACEMENT:
                  return int(Placement::ABOVE);
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT:
            case P_ID::END_TEXT:
                  return "";
            case P_ID::LINE_VISIBLE:
                  return true;
            case P_ID::BEGIN_TEXT_OFFSET:
            case P_ID::CONTINUE_TEXT_OFFSET:
            case P_ID::END_TEXT_OFFSET:
                  return QPointF(0,0);
            case P_ID::BEGIN_HOOK_TYPE:
            case P_ID::END_HOOK_TYPE:
                  return int(HookType::NONE);
            case P_ID::BEGIN_TEXT_PLACE:
            case P_ID::CONTINUE_TEXT_PLACE:
            case P_ID::END_TEXT_PLACE:
                  return int(PlaceText::AUTO);
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return Spatium(1.5);
            case P_ID::BEGIN_TEXT_ALIGN:
            case P_ID::END_TEXT_ALIGN:
            case P_ID::CONTINUE_TEXT_ALIGN:
                  return QVariant::fromValue(Align::LEFT | Align::TOP);
            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }



}     // namespace Ms

