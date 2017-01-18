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
//   getProperty
//---------------------------------------------------------

QVariant PedalSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
                  return pedal()->getProperty(id);
            default:
                  return TextLineBaseSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool PedalSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
                  return pedal()->setProperty(id, v);
            default:
                  return TextLineBaseSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant PedalSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
                  return pedal()->propertyDefault(id);
            default:
                  return TextLineBaseSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags PedalSegment::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
                  return pedal()->propertyFlags(id);

            default:
                  return TextLineBaseSegment::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void PedalSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
            case P_ID::LINE_STYLE:
            case P_ID::PLACEMENT:
                  return pedal()->resetProperty(id);

            default:
                  return TextLineBaseSegment::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void PedalSegment::styleChanged()
      {
      pedal()->styleChanged();
      }

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLineBase(s)
      {
      setBeginHookHeight(Spatium(-1.2));
      setEndHookHeight(Spatium(-1.2));

      setLineWidth(score()->styleS(StyleIdx::pedalLineWidth));
      lineWidthStyle = PropertyFlags::STYLED;
      setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::pedalLineStyle)));
      lineStyleStyle = PropertyFlags::STYLED;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(XmlReader& e)
      {
      if (score()->mscVersion() >= 110) {
            // setBeginSymbol(SymId::noSym);
            setEndHook(false);
            }
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")          // obsolete
                  e.skipCurrentElement();
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyFlags::UNSTYLED;
                  }
            else if (tag == "lineStyle") {
                  setLineStyle(Qt::PenStyle(e.readInt()));
                  lineStyleStyle = PropertyFlags::UNSTYLED;
                  }
            else if (!TextLineBase::readProperties(e))
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
//   setProperty
//---------------------------------------------------------

bool Pedal::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_ID::PLACEMENT:
                  if (val != getProperty(propertyId)) {
                        // reverse hooks
                        setBeginHookHeight(-beginHookHeight());
                        setEndHookHeight(-endHookHeight());
                        }
                  setPlacement(Placement(val.toInt()));
                  break;

            case P_ID::LINE_WIDTH:
                  lineWidthStyle = PropertyFlags::UNSTYLED;
                  TextLineBase::setProperty(propertyId, val);
                  break;

            case P_ID::LINE_STYLE:
                  lineStyleStyle = PropertyFlags::UNSTYLED;
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

QVariant Pedal::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_WIDTH:
                  return score()->styleV(StyleIdx::pedalLineWidth);

            case P_ID::LINE_STYLE:
                  return int(score()->styleI(StyleIdx::pedalLineStyle));

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags Pedal::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return lineWidthStyle;

            case P_ID::LINE_STYLE:
                  return lineStyleStyle;

            default:
                  return TextLineBase::propertyFlags(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Pedal::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  setLineWidth(score()->styleS(StyleIdx::pedalLineWidth));
                  lineWidthStyle = PropertyFlags::STYLED;
                  break;

            case P_ID::LINE_STYLE:
                  setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::pedalLineStyle)));
                  lineStyleStyle = PropertyFlags::STYLED;
                  break;

            default:
                  return TextLineBase::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Pedal::styleChanged()
      {
      if (lineWidthStyle == PropertyFlags::STYLED)
            setLineWidth(score()->styleS(StyleIdx::pedalLineWidth));
      if (lineStyleStyle == PropertyFlags::STYLED)
            setLineStyle(Qt::PenStyle(score()->styleI(StyleIdx::pedalLineStyle)));
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
            if (beginHook() && beginHookType() == HookType::HOOK_45)
                  x += nhw * .5;
            }
      else {
            Element* e = endElement();
            ChordRest* c = static_cast<ChordRest*>(endElement());
            if (!e || e == startElement() || (endHook() && endHookType() == HookType::HOOK_90)) {
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
                              if (seg->segmentType() == Segment::Type::ChordRest) {
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
                              else if (seg->segmentType() == Segment::Type::EndBarLine) {
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
            if (endHook() && endHookType() == HookType::HOOK_45)
                  x += nhw * .5;
            else
                  x += nhw;
            }

      *sys = s;
      return QPointF(x, 0);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Pedal::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_WIDTH:
                  return StyleIdx::pedalLineWidth;
            case P_ID::LINE_STYLE:
                  return StyleIdx::pedalLineStyle;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

}

