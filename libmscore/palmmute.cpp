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

#include "palmmute.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "measure.h"
#include "chordrest.h"

#include "score.h"

namespace Ms {

static const ElementStyle palmMuteStyle {
      { Sid::palmMuteFontFace,                      Pid::BEGIN_FONT_FACE        },
      { Sid::palmMuteFontFace,                      Pid::CONTINUE_FONT_FACE     },
      { Sid::palmMuteFontFace,                      Pid::END_FONT_FACE          },
      { Sid::palmMuteFontSize,                      Pid::BEGIN_FONT_SIZE        },
      { Sid::palmMuteFontSize,                      Pid::CONTINUE_FONT_SIZE     },
      { Sid::palmMuteFontSize,                      Pid::END_FONT_SIZE          },
      { Sid::palmMuteFontStyle,                     Pid::BEGIN_FONT_STYLE       },
      { Sid::palmMuteFontStyle,                     Pid::CONTINUE_FONT_STYLE    },
      { Sid::palmMuteFontStyle,                     Pid::END_FONT_STYLE         },
      { Sid::palmMuteTextAlign,                     Pid::BEGIN_TEXT_ALIGN       },
      { Sid::palmMuteTextAlign,                     Pid::CONTINUE_TEXT_ALIGN    },
      { Sid::palmMuteTextAlign,                     Pid::END_TEXT_ALIGN         },
      { Sid::palmMuteHookHeight,                    Pid::BEGIN_HOOK_HEIGHT      },
      { Sid::palmMuteHookHeight,                    Pid::END_HOOK_HEIGHT        },
      { Sid::palmMutePosBelow,                      Pid::OFFSET                 },
      { Sid::palmMuteLineStyle,                     Pid::LINE_STYLE             },
      { Sid::palmMuteBeginTextOffset,               Pid::BEGIN_TEXT_OFFSET      },
      { Sid::palmMuteEndHookType,                   Pid::END_HOOK_TYPE          },
      { Sid::palmMuteLineWidth,                     Pid::LINE_WIDTH             },
      { Sid::palmMutePlacement,                     Pid::PLACEMENT              },
      { Sid::palmMutePosBelow,                      Pid::OFFSET                 },
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PalmMuteSegment::layout()
      {
      TextLineBaseSegment::layout();
      autoplaceSpannerSegment();
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid PalmMuteSegment::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return spanner()->placeAbove() ? Sid::palmMutePosAbove : Sid::palmMutePosBelow;
      return TextLineBaseSegment::getPropertyStyle(pid);
      }

Sid PalmMute::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::palmMutePosAbove : Sid::palmMutePosBelow;
      return TextLineBase::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   PalmMute
//---------------------------------------------------------

PalmMute::PalmMute(Score* s)
   : TextLineBase(s)
      {
      initElementStyle(&palmMuteStyle);
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

void PalmMute::read(XmlReader& e)
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
//   also see letring.cpp
//---------------------------------------------------------

/*
void PalmMute::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);

      for (const StyledProperty& spp : *styledProperties()) {
            if(!isStyled(spp.pid))
                  writeProperty(xml, spp.pid);
            }

      TextLineBase::writeProperties(xml);
      xml.etag();
      }
*/

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle palmMuteSegmentStyle {
      { Sid::palmMutePosBelow,                      Pid::OFFSET                 },
      { Sid::palmMuteMinDistance,                   Pid::MIN_DISTANCE           },
      };

LineSegment* PalmMute::createLineSegment()
      {
      PalmMuteSegment* pms = new PalmMuteSegment(this, score());
      pms->setTrack(track());
      pms->initElementStyle(&palmMuteSegmentStyle);
      return pms;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant PalmMute::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::LINE_WIDTH:
                  return score()->styleV(Sid::palmMuteLineWidth);

            case Pid::ALIGN:
                  return QVariant::fromValue(Align::LEFT | Align::BASELINE);

            case Pid::LINE_STYLE:
                  return score()->styleV(Sid::palmMuteLineStyle);

            case Pid::LINE_VISIBLE:
                  return true;

            case Pid::CONTINUE_TEXT_OFFSET:
            case Pid::END_TEXT_OFFSET:
                  return QPointF(0, 0);

//TODOws            case Pid::BEGIN_FONT_ITALIC:
//                  return score()->styleV(Sid::palmMuteFontItalic);

            case Pid::BEGIN_TEXT:
                  return score()->styleV(Sid::palmMuteText);
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
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF PalmMute::linePos(Grip grip, System** sys) const
      {
      qreal x = 0.0;
      qreal nhw = score()->noteHeadWidth();
      System* s = nullptr;
      if (grip == Grip::START) {
            ChordRest* c = toChordRest(startElement());
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->isRest() && c->durationType() == TDuration::DurationType::V_MEASURE)
                  x -= c->x();
            }
      else {
            Element* e = endElement();
            ChordRest* c = toChordRest(endElement());
            if (!e || e == startElement() || (endHookType() == HookType::HOOK_90)) {
                  // palmMute marking on single note or ends with non-angled hook:
                  // extend to next note or end of measure
                  Segment* seg = nullptr;
                  if (!e)
                        seg = startSegment();
                  else
                        seg = c->segment();
                  if (seg) {
                        seg = seg->next();
                        for ( ; seg; seg = seg->next()) {
                              if (seg->isChordRestType()) {
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
                  if (c->type() == ElementType::REST && c->durationType() == TDuration::DurationType::V_MEASURE)
                        x -= c->x();
                  }
            if (!s) {
                  Fraction t = tick2();
                  Measure* m = score()->tick2measure(t);
                  s = m->system();
                  x = m->tick2pos(t);
                  }
            if (endHookType() == HookType::HOOK_45)
                  x += nhw * .5;
            else
                  x += nhw;
            }

      *sys = s;
      return QPointF(x, 0);
      }

}

