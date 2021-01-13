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
#include "undo.h"
#include "musescoreCore.h"

namespace Ms {


//---------------------------------------------------------
//   textLineSegmentStyle
//---------------------------------------------------------

static const ElementStyle textLineSegmentStyle {
      { Sid::textLinePosAbove,      Pid::OFFSET       },
      { Sid::textLineMinDistance,   Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   systemTextLineSegmentStyle
//---------------------------------------------------------

      static const ElementStyle systemTextLineSegmentStyle {
      { Sid::systemTextLinePosAbove,      Pid::OFFSET       },
      { Sid::systemTextLineMinDistance,   Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   textLineStyle
//---------------------------------------------------------

static const ElementStyle textLineStyle {
//       { Sid::textLineSystemFlag,                 Pid::SYSTEM_FLAG             },
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
//   systemTextLineStyle
//---------------------------------------------------------

static const ElementStyle systemTextLineStyle {
//       { Sid::systemTextLineSystemFlag,           Pid::SYSTEM_FLAG             },
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

TextLineSegment::TextLineSegment(Spanner* sp, Score* s, bool system)
   : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      setSystemFlag(system);
      if (systemFlag())
            initElementStyle(&systemTextLineSegmentStyle);
      else
            initElementStyle(&textLineSegmentStyle);
      }

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

Element* TextLineSegment::propertyDelegate(Pid pid)
      {
      if (pid == Pid::SYSTEM_FLAG)
            return static_cast<TextLine*>(spanner());
      return TextLineBaseSegment::propertyDelegate(pid);
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

TextLine::TextLine(Score* s, bool system)
   : TextLineBase(s)
      {
      setSystemFlag(system);

      initStyle();

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

      initElementStyle(&textLineStyle);

      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::END_TEXT_PLACE);
      }

TextLine::TextLine(const TextLine& tl)
   : TextLineBase(tl)
      {
      }

//---------------------------------------------------------
//   initStyle
//---------------------------------------------------------

void TextLine::initStyle()
      {
      if (systemFlag())
            initElementStyle(&systemTextLineStyle);
      else
            initElementStyle(&textLineStyle);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      if (systemFlag())
            xml.stag(QString("TextLine"), this, QString("system=\"1\""));
      else
            xml.stag(this);
      // other styled properties are included in TextLineBase pids list
      writeProperty(xml, Pid::PLACEMENT);
      writeProperty(xml, Pid::OFFSET);
      TextLineBase::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(XmlReader& e)
      {
      bool system =  e.intAttribute("system", 0) == 1;
      setSystemFlag(system);
      initStyle();
      TextLineBase::read(e);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      TextLineSegment* seg = new TextLineSegment(this, score(), systemFlag());
      seg->setTrack(track());
      // note-anchored line segments are relative to system not to staff
      if (anchor() == Spanner::Anchor::NOTE)
            seg->setFlag(ElementFlag::ON_STAFF, false);

      if (systemFlag())
            seg->initElementStyle(&systemTextLineSegmentStyle);
      else
            seg->initElementStyle(&textLineSegmentStyle);

      return seg;
      }

//---------------------------------------------------------
//   getTextLinePos
//---------------------------------------------------------

Sid TextLineSegment::getTextLinePos(bool above) const
      {
      if (systemFlag())
            return above ? Sid::systemTextLinePosAbove : Sid::systemTextLinePosBelow;
      else
            return above ? Sid::textLinePosAbove : Sid::textLinePosBelow;
      }

Sid TextLine::getTextLinePos(bool above) const
      {
      if (systemFlag())
            return above ? Sid::systemTextLinePosAbove : Sid::systemTextLinePosBelow;
      else
            return above ? Sid::textLinePosAbove : Sid::textLinePosBelow;
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
                  return getTextLinePos(spanner()->placeAbove());
            }
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid TextLine::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET) {
            if (anchor() == Spanner::Anchor::NOTE)
                  return Sid::NOSTYLE;
            else
                  return getTextLinePos(placeAbove());
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
                  if (systemFlag())
                        return score()->styleV(Sid::textLinePlacement);
                  else
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

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLine::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::PLACEMENT:
                  setPlacement(Placement(v.toInt()));
                  break;
            default:
                  return TextLineBase::setProperty(id, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void TextLine::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (id == Pid::SYSTEM_FLAG) {
            score()->undo(new ChangeTextLineProperty(this, v));
            for (SpannerSegment* s : spannerSegments()) {
                  score()->undo(new ChangeTextLineProperty(s, v));
                  triggerLayout();
                  }
            MuseScoreCore::mscoreCore->updateInspector();
            return;
            }
      TextLineBase::undoChangeProperty(id, v, ps);
      }

//---------------------------------------------------------
//   layoutSystem
//    layout spannersegment for system
//---------------------------------------------------------

SpannerSegment* TextLine::layoutSystem(System* system)
      {
      TextLineSegment* tls = toTextLineSegment(TextLineBase::layoutSystem(system));

      if (tls->spanner()) {
            for (SpannerSegment* ss : tls->spanner()->spannerSegments()) {
                  ss->setFlag(ElementFlag::SYSTEM, systemFlag());
                  ss->setTrack(systemFlag() ? 0 : track());
            }
            tls->spanner()->setFlag(ElementFlag::SYSTEM, systemFlag());
            tls->spanner()->setTrack(systemFlag() ? 0 : track());
            }

      return tls;
      }

}     // namespace Ms

