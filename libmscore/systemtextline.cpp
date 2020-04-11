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
#include "systemtextline.h"
#include "staff.h"
#include "system.h"

namespace Ms {

//---------------------------------------------------------
//   systemTextLineStyle
//---------------------------------------------------------

static const ElementStyle systemTextLineStyle {
      { Sid::systemTextLineFontFace,             Pid::BEGIN_FONT_FACE         },
      { Sid::systemTextLineFontFace,             Pid::CONTINUE_FONT_FACE      },
      { Sid::systemTextLineFontFace,             Pid::END_FONT_FACE           },
      { Sid::systemTextLineFontSize,             Pid::BEGIN_FONT_SIZE         },
      { Sid::systemTextLineFontSize,             Pid::CONTINUE_FONT_SIZE      },
      { Sid::systemTextLineFontSize,             Pid::END_FONT_SIZE           },
      { Sid::systemTextLineFontStyle,            Pid::BEGIN_FONT_STYLE        },
      { Sid::systemTextLineFontStyle,            Pid::CONTINUE_FONT_STYLE     },
      { Sid::systemTextLineFontStyle,            Pid::END_FONT_STYLE          },
      { Sid::systemTextLineTextAlign,            Pid::BEGIN_TEXT_ALIGN        },
      { Sid::systemTextLineTextAlign,            Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::systemTextLineTextAlign,            Pid::END_TEXT_ALIGN          },
      { Sid::systemTextLinePlacement,            Pid::PLACEMENT               },
      { Sid::systemTextLinePosAbove,             Pid::OFFSET                  },
      };

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

SystemTextLineSegment::SystemTextLineSegment(Spanner* sp, Score* s)
   : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SystemTextLineSegment::layout()
      {
      TextLineBaseSegment::layout();
      if (isStyled(Pid::OFFSET))
            roffset() = systemTextLine()->propertyDefault(Pid::OFFSET).toPointF();
      autoplaceSpannerSegment();
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

SystemTextLine::SystemTextLine(Score* s)
   : TextLineBase(s, ElementFlag::SYSTEM)
      {
      initElementStyle(&systemTextLineStyle);

      setBeginText("");
      setContinueText("");
      setEndText("");
      setBeginTextOffset(QPointF(0,0));
      setContinueTextOffset(QPointF(0,0));
      setEndTextOffset(QPointF(0,0));
      setLineVisible(true);

      setBeginHookType(HookType::NONE);
      setEndHookType(HookType::NONE);
      setBeginHookHeight(Spatium(1.5));
      setEndHookHeight(Spatium(1.5));

      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::END_TEXT_PLACE);
      }

SystemTextLine::SystemTextLine(const SystemTextLine& tl)
   : TextLineBase(tl)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SystemTextLine::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      // other styled properties are included in TextLineBase pids list
      writeProperty(xml, Pid::PLACEMENT);
      writeProperty(xml, Pid::OFFSET);
      TextLineBase::writeProperties(xml);
      xml.etag();
      }

static const ElementStyle systemTextLineSegmentStyle {
      { Sid::systemTextLinePosAbove,      Pid::OFFSET       },
      { Sid::systemTextLineMinDistance,   Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* SystemTextLine::createLineSegment()
      {
      SystemTextLineSegment* seg = new SystemTextLineSegment(this, score());
      seg->setTrack(track());
      // note-anchored line segments are relative to system not to staff
      if (anchor() == Spanner::Anchor::NOTE)
            seg->setFlag(ElementFlag::ON_STAFF, false);
      seg->initElementStyle(&systemTextLineSegmentStyle);
      return seg;
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid SystemTextLineSegment::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET) {
            if (spanner()->anchor() == Spanner::Anchor::NOTE)
                  return Sid::NOSTYLE;
            else
                  return spanner()->placeAbove() ? Sid::systemTextLinePosAbove : Sid::systemTextLinePosBelow;
            }
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid SystemTextLine::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET) {
            if (anchor() == Spanner::Anchor::NOTE)
                  return Sid::NOSTYLE;
            else
                  return placeAbove() ? Sid::systemTextLinePosAbove : Sid::systemTextLinePosBelow;
            }
      return TextLineBase::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SystemTextLine::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::systemTextLinePlacement);
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
                  return int(PlaceText::LEFT);
            case Pid::BEGIN_HOOK_HEIGHT:
            case Pid::END_HOOK_HEIGHT:
                  return Spatium(1.5);
            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }



}     // namespace Ms


