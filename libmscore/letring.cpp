//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "letring.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "measure.h"
#include "chordrest.h"

#include "score.h"

namespace Ms {

static const ElementStyle letRingStyle {
      { Sid::letRingFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::letRingFontFace,                      Pid::CONTINUE_FONT_FACE     },
      { Sid::letRingFontFace,                      Pid::END_FONT_FACE          },
      { Sid::letRingFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::letRingFontSize,                      Pid::CONTINUE_FONT_SIZE     },
      { Sid::letRingFontSize,                      Pid::END_FONT_SIZE          },
      { Sid::letRingFontStyle,                     Pid::BEGIN_FONT_STYLE       },
      { Sid::letRingFontStyle,                     Pid::CONTINUE_FONT_STYLE    },
      { Sid::letRingFontStyle,                     Pid::END_FONT_STYLE         },
      { Sid::letRingTextAlign,                     Pid::BEGIN_TEXT_ALIGN       },
      { Sid::letRingTextAlign,                     Pid::CONTINUE_TEXT_ALIGN    },
      { Sid::letRingTextAlign,                     Pid::END_TEXT_ALIGN         },
      { Sid::letRingHookHeight,                    Pid::BEGIN_HOOK_HEIGHT      },
      { Sid::letRingHookHeight,                    Pid::END_HOOK_HEIGHT        },
      { Sid::letRingLineStyle,                     Pid::LINE_STYLE             },
      { Sid::letRingBeginTextOffset,               Pid::BEGIN_TEXT_OFFSET      },
      { Sid::letRingEndHookType,                   Pid::END_HOOK_TYPE          },
      { Sid::letRingLineWidth,                     Pid::LINE_WIDTH             },
      { Sid::letRingPlacement,                     Pid::PLACEMENT              },
      //{ Sid::letRingPosBelow,                      Pid::OFFSET                 },
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LetRingSegment::layout()
      {
      TextLineBaseSegment::layout();
      autoplaceSpannerSegment();
      }

//---------------------------------------------------------
//   LetRing
//---------------------------------------------------------

LetRing::LetRing(Score* s)
   : TextLineBase(s)
      {
      initElementStyle(&letRingStyle);
      resetProperty(Pid::LINE_VISIBLE);

      resetProperty(Pid::BEGIN_TEXT_PLACE);
      resetProperty(Pid::BEGIN_TEXT);
      resetProperty(Pid::CONTINUE_TEXT_PLACE);
      resetProperty(Pid::CONTINUE_TEXT);
      resetProperty(Pid::END_TEXT_PLACE);
      resetProperty(Pid::END_TEXT);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LetRing::read(XmlReader& e)
      {
      if (score()->mscVersion() < 301)
            e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement()) {
            if (readProperty(e.name(), e, Pid::LINE_WIDTH))
                  setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
            else if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//   
//   The removal of this function is potentially a temporary
//   change. For now, the intended behavior does no more than
//   the base write function and so we will just use that.
//
//   also see palmmute.cpp
//---------------------------------------------------------

/*
void LetRing::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);

      for (const StyledProperty& spp : *styledProperties()) {
            if (!isStyled(spp.pid))
                  writeProperty(xml, spp.pid);
            }

      TextLineBase::writeProperties(xml);
      xml.etag();
      }
*/

static const ElementStyle letRingSegmentStyle {
      //{ Sid::letRingPosBelow,       Pid::OFFSET       },
      { Sid::letRingMinDistance,    Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* LetRing::createLineSegment()
      {
      LetRingSegment* lr = new LetRingSegment(this, score());
      lr->setTrack(track());
      lr->initElementStyle(&letRingSegmentStyle);
      return lr;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant LetRing::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_WIDTH:
                  return score()->styleV(Sid::letRingLineWidth);

            case Pid::ALIGN:
                  return QVariant::fromValue(Align::LEFT | Align::BASELINE);

            case Pid::LINE_STYLE:
                  return score()->styleV(Sid::letRingLineStyle);

            case Pid::LINE_VISIBLE:
                  return true;

            case Pid::CONTINUE_TEXT_OFFSET:
            case Pid::END_TEXT_OFFSET:
                  return QPointF(0, 0);

            case Pid::BEGIN_FONT_STYLE:
                  return score()->styleV(Sid::letRingFontStyle);

            case Pid::BEGIN_TEXT:
                  return score()->styleV(Sid::letRingText);
            case Pid::CONTINUE_TEXT:
            case Pid::END_TEXT:
                  return "";

            case Pid::BEGIN_HOOK_TYPE:
                  return int(HookType::NONE);

            case Pid::BEGIN_TEXT_PLACE:
            case Pid::CONTINUE_TEXT_PLACE:
            case Pid::END_TEXT_PLACE:
                  return int(PlaceText::AUTO);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid LetRing::getPropertyStyle(Pid id) const
      {
      switch (id) {
            case Pid::PLACEMENT:
                  return Sid::letRingPlacement;
            case Pid::BEGIN_FONT_FACE:
                  return Sid::letRingFontFace;
            case Pid::BEGIN_FONT_SIZE:
            case Pid::CONTINUE_FONT_SIZE:
            case Pid::END_FONT_SIZE:
                  return Sid::letRingFontSize;
            case Pid::BEGIN_FONT_STYLE:
            case Pid::CONTINUE_FONT_STYLE:
            case Pid::END_FONT_STYLE:
                  return Sid::letRingFontStyle;
            case Pid::BEGIN_TEXT_ALIGN:
            case Pid::CONTINUE_TEXT_ALIGN:
            case Pid::END_TEXT_ALIGN:
                  return Sid::letRingTextAlign;
            case Pid::BEGIN_HOOK_HEIGHT:
            case Pid::END_HOOK_HEIGHT:
                  return Sid::letRingHookHeight;
            case Pid::BEGIN_TEXT:
                  return Sid::letRingText;
            default:
                  break;
            }
      return TextLineBase::getPropertyStyle(id);
      }


//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF LetRing::linePos(Grip grip, System** sys) const
      {
      qreal x = 0.0;
      qreal nhw = score()->noteHeadWidth();
      System* s = nullptr;
      if (grip == Grip::START) {
            ChordRest* c = toChordRest(startElement());
            if (!c) {
                  *sys = s;
                  return QPointF();
                  }
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->isRest() && c->durationType() == TDuration::DurationType::V_MEASURE)
                  x -= c->x();
            }
      else {
            Element* e = endElement();
            ChordRest* c = toChordRest(endElement());
            if (!e || e == startElement() || (endHookType() == HookType::HOOK_90)) {
                  // pedal marking on single note or ends with non-angled hook:
                  // extend to next note or end of measure
                  Segment* seg = nullptr;
                  if (!e)
                        seg = startSegment();
                  else
                        seg = c->segment();
                  if (seg) {
                        seg = seg->next();
                        for ( ; seg; seg = seg->next()) {
                              if (seg->segmentType() == SegmentType::ChordRest) {
                                    // look for a chord/rest in any voice on this staff
                                    bool crFound = false;
                                    int track = staffIdx() * VOICES;
                                    for (int i = 0; i < VOICES; ++i) {
                                          if (seg->element(track + i)) {
                                                crFound = true;
                                                break;
                                                }
                                          }
                                    if (crFound)
                                          break;
                                    }
                              else if (seg->segmentType() == SegmentType::EndBarLine) {
                                    break;
                                    }
                              }
                        }
                  if (seg) {
                        s = seg->system();
                        x = seg->pos().x() + seg->measure()->pos().x() - nhw * 2;
                        }
                  }
            else if (c) {
                  s = c->segment()->system();
                  x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
                  if (c->isRest() && c->durationType() == TDuration::DurationType::V_MEASURE)
                        x -= c->x();
                  }
            if (!s) {
                  Fraction t = tick2();
                  Measure* m = score()->tick2measure(t);
                  s = m->system();
                  x = m->tick2pos(t);
                  }
            x += nhw;
            }

      *sys = s;
      return QPointF(x, 0);
      }

}

