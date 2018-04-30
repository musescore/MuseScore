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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LetRingSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());
      TextLineBaseSegment::layout();
      if (parent()) {     // for palette
            rypos() += score()->styleP(letRing()->placeBelow() ? Sid::letRingPosBelow : Sid::letRingPosAbove);
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());

                  if (letRing()->placeBelow()) {
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
            }
      }

//---------------------------------------------------------
//   LetRing
//---------------------------------------------------------

LetRing::LetRing(Score* s)
   : TextLineBase(s)
      {
      initSubStyle(SubStyleId::LET_RING);
      resetProperty(Pid::LINE_VISIBLE);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LetRing::read(XmlReader& e)
      {
      int id = e.intAttribute("id", -1);
      e.addSpanner(id, this);
      while (e.readNextStartElement()) {
            if (!TextLineBase::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LetRing::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));

      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* LetRing::createLineSegment()
      {
      return new LetRingSegment(score());
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void LetRing::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? Sid::letRingPosAbove : Sid::letRingPosBelow);
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

            case Pid::BEGIN_TEXT_OFFSET:
                  return score()->styleV(Sid::letRingBeginTextOffset).toPointF();

            case Pid::BEGIN_TEXT_ALIGN:
            case Pid::CONTINUE_TEXT_ALIGN:
            case Pid::END_TEXT_ALIGN:
                  return score()->styleV(Sid::letRingTextAlign);

            case Pid::BEGIN_HOOK_HEIGHT:
            case Pid::END_HOOK_HEIGHT:
                  return score()->styleV(Sid::letRingHookHeight);

            case Pid::BEGIN_FONT_ITALIC:
                  return score()->styleV(Sid::letRingFontItalic);

            case Pid::BEGIN_TEXT:
                  return score()->styleV(Sid::letRingText);

            case Pid::END_HOOK_TYPE:
                  return int(HookType::HOOK_90T);

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
                  return Sid::letRingFontSize;
            case Pid::BEGIN_FONT_BOLD:
                  return Sid::letRingFontBold;
            case Pid::BEGIN_FONT_ITALIC:
                  return Sid::letRingFontItalic;
            case Pid::BEGIN_FONT_UNDERLINE:
                  return Sid::letRingFontUnderline;
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
      return Sid::NOSTYLE;
      }


//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF LetRing::linePos(Grip grip, System** sys) const
      {
      qreal x;
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
                  int t = tick2();
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

