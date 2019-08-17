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
//   textLineStyle
//---------------------------------------------------------

static const ElementStyle textLineStyle {
      { Sid::textLineFontFace,                   Pid::BEGIN_FONT_FACE         },
      { Sid::textLineFontFace,                   Pid::CONTINUE_FONT_FACE      },
      { Sid::textLineFontFace,                   Pid::END_FONT_FACE           },
      { Sid::textLineFontSize,                   Pid::BEGIN_FONT_SIZE         },
      { Sid::textLineFontSize,                   Pid::CONTINUE_FONT_SIZE      },
      { Sid::textLineFontSize,                   Pid::END_FONT_SIZE           },
      { Sid::textLineFontStyle,                  Pid::BEGIN_FONT_STYLE        },
      { Sid::textLineFontStyle,                  Pid::CONTINUE_FONT_STYLE     },
      { Sid::textLineFontStyle,                  Pid::END_FONT_STYLE          },
      { Sid::textLineTextAlign,                  Pid::BEGIN_TEXT_ALIGN        },
      { Sid::textLineTextAlign,                  Pid::CONTINUE_TEXT_ALIGN     },
      { Sid::textLineTextAlign,                  Pid::END_TEXT_ALIGN          },
      { Sid::textLinePlacement,                  Pid::PLACEMENT               },
      { Sid::textLinePosAbove,                   Pid::OFFSET                  },
      };

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Spanner* sp, Score* s)
   : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout()
      {
      TextLineBaseSegment::layout();
      if (isStyled(Pid::OFFSET))
            roffset() = textLine()->propertyDefault(Pid::OFFSET).toPointF();
      autoplaceSpannerSegment();
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : TextLineBase(s)
      {
      initElementStyle(&textLineStyle);

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

TextLine::TextLine(const TextLine& tl)
   : TextLineBase(tl)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(XmlWriter& xml) const
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

static const ElementStyle textLineSegmentStyle {
      { Sid::textLinePosAbove,      Pid::OFFSET       },
      { Sid::textLineMinDistance,   Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      TextLineSegment* seg = new TextLineSegment(this, score());
      seg->setTrack(track());
      // note-anchored line segments are relative to system not to staff
      if (anchor() == Spanner::Anchor::NOTE)
            seg->setFlag(ElementFlag::ON_STAFF, false);
      seg->initElementStyle(&textLineSegmentStyle);
      return seg;
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TextLineSegment::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET) {
            if (spanner()->anchor() == Spanner::Anchor::NOTE)
                  return Sid::NOSTYLE;
            else
                  return spanner()->placeAbove() ? Sid::textLinePosAbove : Sid::textLinePosBelow;
            }
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid TextLine::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET) {
            if (anchor() == Spanner::Anchor::NOTE)
                  return Sid::NOSTYLE;
            else
                  return placeAbove() ? Sid::textLinePosAbove : Sid::textLinePosBelow;
            }
      return TextLineBase::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant TextLine::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::PLACEMENT:
                  return score()->styleV(Sid::textLinePlacement);
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

