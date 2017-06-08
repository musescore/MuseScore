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

#include "pedal.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "measure.h"
#include "chordrest.h"

#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PedalSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());
      TextLineBaseSegment::layout();
      if (parent()) {     // for palette
            rypos() += score()->styleP(pedal()->placeBelow() ? StyleIdx::pedalPosBelow : StyleIdx::pedalPosAbove);
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());

                  if (pedal()->placeBelow()) {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = d + minDistance;
                        }
                  else {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = -(d + minDistance);
                        }
                  }
            else
                  adjustReadPos();
            }
      }


//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLineBase(s)
      {
      setLineWidth(score()->styleS(StyleIdx::pedalLineWidth));
      setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::pedalLineStyle)));
      resetProperty(P_ID::BEGIN_TEXT_ALIGN);
      resetProperty(P_ID::CONTINUE_TEXT_ALIGN);
      resetProperty(P_ID::END_TEXT_ALIGN);
      resetProperty(P_ID::BEGIN_HOOK_HEIGHT);
      resetProperty(P_ID::END_HOOK_HEIGHT);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(XmlReader& e)
      {
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Pedal::createLineSegment()
      {
      return new PedalSegment(score());
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Pedal::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? StyleIdx::pedalPosAbove : StyleIdx::pedalPosBelow);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Pedal::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_WIDTH:
                  return score()->styleV(StyleIdx::pedalLineWidth);

            case P_ID::ALIGN:
                  return QVariant::fromValue(Align::LEFT | Align::BASELINE);

            case P_ID::LINE_STYLE:
                  return score()->styleV(StyleIdx::pedalLineStyle);

            case P_ID::BEGIN_TEXT_OFFSET:
                  return score()->styleV(StyleIdx::pedalBeginTextOffset).toPointF();

            case P_ID::BEGIN_TEXT_ALIGN:
            case P_ID::CONTINUE_TEXT_ALIGN:
            case P_ID::END_TEXT_ALIGN:
                  return score()->styleV(StyleIdx::pedalTextAlign);

            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleV(StyleIdx::pedalHookHeight);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Pedal::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::PLACEMENT:
                  return StyleIdx::pedalPlacement;
            case P_ID::BEGIN_FONT_FACE:
                  return StyleIdx::pedalFontFace;
            case P_ID::BEGIN_FONT_SIZE:
                  return StyleIdx::pedalFontSize;
            case P_ID::BEGIN_FONT_BOLD:
                  return StyleIdx::pedalFontBold;
            case P_ID::BEGIN_FONT_ITALIC:
                  return StyleIdx::pedalFontItalic;
            case P_ID::BEGIN_FONT_UNDERLINE:
                  return StyleIdx::pedalFontUnderline;
            case P_ID::BEGIN_TEXT_ALIGN:
            case P_ID::CONTINUE_TEXT_ALIGN:
            case P_ID::END_TEXT_ALIGN:
                  return StyleIdx::pedalTextAlign;
            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return StyleIdx::pedalHookHeight;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }


//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF Pedal::linePos(Grip grip, System** sys) const
      {
      qreal x;
      qreal nhw = score()->noteHeadWidth();
      System* s = nullptr;
      if (grip == Grip::START) {
            ChordRest* c = static_cast<ChordRest*>(startElement());
            s = c->segment()->system();
            x = c->pos().x() + c->segment()->pos().x() + c->segment()->measure()->pos().x();
            if (c->type() == ElementType::REST && c->durationType() == TDuration::DurationType::V_MEASURE)
                  x -= c->x();
            if (beginHookType() == HookType::HOOK_45)
                  x += nhw * .5;
            }
      else {
            Element* e = endElement();
            ChordRest* c = static_cast<ChordRest*>(endElement());
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
                  if (c->type() == ElementType::REST && c->durationType() == TDuration::DurationType::V_MEASURE)
                        x -= c->x();
                  }
            if (!s) {
                  int t = tick2();
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

