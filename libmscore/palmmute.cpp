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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PalmMuteSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());
      TextLineBaseSegment::layout();
      if (parent()) {     // for palette
            rypos() += score()->styleP(palmMute()->placeBelow() ? StyleIdx::palmMutePosBelow : StyleIdx::palmMutePosAbove);
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());

                  if (palmMute()->placeBelow()) {
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
//   PalmMute
//---------------------------------------------------------

PalmMute::PalmMute(Score* s)
   : TextLineBase(s)
      {
      initSubStyle(SubStyleId::PALM_MUTE);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void PalmMute::read(XmlReader& e)
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

void PalmMute::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));

      for (const StyledProperty* spp = styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp)
            writeProperty(xml, spp->propertyIdx);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* PalmMute::createLineSegment()
      {
      return new PalmMuteSegment(score());
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void PalmMute::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? StyleIdx::palmMutePosAbove : StyleIdx::palmMutePosBelow);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant PalmMute::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::LINE_WIDTH:
                  return score()->styleV(StyleIdx::palmMuteLineWidth);

            case P_ID::ALIGN:
                  return QVariant::fromValue(Align::LEFT | Align::BASELINE);

            case P_ID::LINE_STYLE:
                  return score()->styleV(StyleIdx::palmMuteLineStyle);

            case P_ID::BEGIN_TEXT_OFFSET:
                  return score()->styleV(StyleIdx::palmMuteBeginTextOffset).toPointF();

            case P_ID::BEGIN_TEXT_ALIGN:
            case P_ID::CONTINUE_TEXT_ALIGN:
            case P_ID::END_TEXT_ALIGN:
                  return score()->styleV(StyleIdx::palmMuteTextAlign);

            case P_ID::BEGIN_HOOK_HEIGHT:
            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleV(StyleIdx::palmMuteHookHeight);

            case P_ID::BEGIN_FONT_ITALIC:
                  return score()->styleV(StyleIdx::palmMuteFontItalic);

            case P_ID::BEGIN_TEXT:
                  return score()->styleV(StyleIdx::palmMuteText);

            case P_ID::END_HOOK_TYPE:
                  return int(HookType::HOOK_90T);

            default:
                  return TextLineBase::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx PalmMute::getPropertyStyle(P_ID id) const
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
            case P_ID::BEGIN_TEXT:
                  return StyleIdx::palmMuteText;
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }


//---------------------------------------------------------
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF PalmMute::linePos(Grip grip, System** sys) const
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

