//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "note.h"
#include "chord.h"
#include "xml.h"
#include "slur.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "system.h"
#include "segment.h"
#include "staff.h"
#include "navigate.h"
#include "articulation.h"
#include "undo.h"
#include "stem.h"
#include "beam.h"
#include "mscore.h"
#include "page.h"
#include "tie.h"

namespace Ms {

Note* Tie::editStartNote;
Note* Tie::editEndNote;

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void Tie::computeBezier(SlurSegment* ss, QPointF p6o)
      {
      qreal _spatium  = spatium();
      qreal shoulderW;              // height as fraction of slur-length
      qreal shoulderH;

      //
      // pp1      start of slur
      // pp2      end of slur
      // pp3      bezier 1
      // pp4      bezier 2
      // pp5      drag
      // pp6      shoulder
      //
      QPointF pp1 = ss->ups(Grip::START).p + ss->ups(Grip::START).off * _spatium;
      QPointF pp2 = ss->ups(Grip::END).p   + ss->ups(Grip::END).off   * _spatium;

      QPointF p2 = pp2 - pp1;       // normalize to zero
      if (p2.x() == 0.0) {
            qDebug("zero tie");
            return;
            }

      qreal sinb = atan(p2.y() / p2.x());
      QTransform t;
      t.rotateRadians(-sinb);
      p2  = t.map(p2);
      p6o = t.map(p6o);

      double smallH = 0.38;
      qreal d   = p2.x() / _spatium;
      shoulderH = d * 0.4 * smallH;
      shoulderH = qBound(0.4, shoulderH, 1.3);
      shoulderH *= _spatium;
      shoulderW = .6;

      shoulderH -= p6o.y();

      if (!up())
            shoulderH = -shoulderH;

      qreal c    = p2.x();
      qreal c1   = (c - c * shoulderW) * .5 + p6o.x();
      qreal c2   = c1 + c * shoulderW       + p6o.x();

      QPointF p5 = QPointF(c * .5, 0.0);

      QPointF p3(c1, -shoulderH);
      QPointF p4(c2, -shoulderH);

      qreal w = (score()->styleS(StyleIdx::SlurMidWidth).val() - score()->styleS(StyleIdx::SlurEndWidth).val()) * _spatium;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ss->ups(Grip::BEZIER1).off * _spatium);
      QPointF p4o = p6o + t.map(ss->ups(Grip::BEZIER2).off * _spatium);

      if(!p6o.isNull()) {
            QPointF p6i = t.inverted().map(p6o) / _spatium;
            ss->ups(Grip::BEZIER1).off += p6i ;
            ss->ups(Grip::BEZIER2).off += p6i;
            }

      //-----------------------------------calculate p6
      QPointF pp3  = p3 + p3o;
      QPointF pp4  = p4 + p4o;
      QPointF ppp4 = pp4 - pp3;

      qreal r2 = atan(ppp4.y() / ppp4.x());
      t.reset();
      t.rotateRadians(-r2);
      QPointF p6  = QPointF(t.map(ppp4).x() * .5, 0.0);

      t.rotateRadians(2 * r2);
      p6 = t.map(p6) + pp3 - p6o;
      //-----------------------------------

      ss->path = QPainterPath();
      ss->path.moveTo(QPointF());
      ss->path.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
      if (lineType() == 0)
            ss->path.cubicTo(p4 +p4o + th, p3 + p3o + th, QPointF());

      th = QPointF(0.0, 3.0 * w);
      ss->shapePath = QPainterPath();
      ss->shapePath.moveTo(QPointF());
      ss->shapePath.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
      ss->shapePath.cubicTo(p4 +p4o + th, p3 + p3o + th, QPointF());

      // translate back
      t.reset();
      t.translate(pp1.x(), pp1.y());
      t.rotateRadians(sinb);
      ss->path                  = t.map(ss->path);
      ss->shapePath             = t.map(ss->shapePath);
      ss->ups(Grip::BEZIER1).p  = t.map(p3);
      ss->ups(Grip::BEZIER2).p  = t.map(p4);
      ss->ups(Grip::END).p      = t.map(p2) - ss->ups(Grip::END).off * _spatium;
      ss->ups(Grip::DRAG).p     = t.map(p5);
      ss->ups(Grip::SHOULDER).p = t.map(p6);

      QPointF staffOffset;
      if (ss->system() && ss->track() >= 0)
            staffOffset = QPointF(0.0, -ss->system()->staff(ss->staffIdx())->y());

      ss->path.translate(staffOffset);
      ss->shapePath.translate(staffOffset);
      }

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Tie::slurPos(SlurPos* sp)
      {
      bool useTablature = staff() != nullptr && staff()->isTabStaff();
      StaffType* stt = nullptr;
      if (useTablature)
            stt = staff()->staffType();
      qreal _spatium    = spatium();
      qreal hw          = startNote()->tabHeadWidth(stt);   // if stt == 0, defaults to headWidth()
      qreal __up        = _up ? -1.0 : 1.0;
      // y offset for ties inside chord margins (typically multi-note chords): lined up with note top or bottom margin
      //    or outside (typically single-note chord): overlaps note and is above/below it
      // Outside: Tab: uses font size and may be asymmetric placed above/below line (frets ON or ABOVE line)
      //          Std: assumes notehead is 1 sp high, 1/2 sp above and 1/2 below line; add 1/4 sp to it
      // Inside:  Tab: 1/2 of Outside offset
      //          Std: use a fixed pecentage of note width
      qreal yOffOutside = useTablature
            ? (_up ? stt->fretBoxY() : stt->fretBoxY() + stt->fretBoxH()) * magS()
            : 0.75 * _spatium * __up;
      qreal yOffInside  = useTablature ? yOffOutside * 0.5 : hw * .3 * __up;

      Chord* sc   = startNote()->chord();
      Q_ASSERT(sc);
      sp->system1 = sc->measure()->system();
      if (!sp->system1) {
            Measure* m = sc->measure();
            qDebug("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
            }
      Q_ASSERT(sp->system1);

      qreal xo;
      qreal yo;
      bool shortStart = false;

      // determine attachment points
      // similar code is used in Chord::layoutPitched()
      // to allocate extra space to enforce minTieLength
      // so keep these in sync

      //------p1
      if ((sc->notes().size() > 1) || (sc->stem() && (sc->up() == _up))) {
            xo = startNote()->x() + hw * 1.12;
            yo = startNote()->pos().y() + yOffInside;
            shortStart = true;
            }
      else {
            xo = startNote()->x() + hw * 0.65;
            yo = startNote()->pos().y() + yOffOutside;
            }
      sp->p1 = sc->pagePos() - sp->system1->pagePos() + QPointF(xo, yo);

      //------p2
      if (endNote() == 0) {
            sp->p2 = sp->p1 + QPointF(_spatium * 3, 0.0);
            sp->system2 = sp->system1;
            return;
            }
      Chord* ec   = endNote()->chord();
      sp->system2 = ec->measure()->system();
      if (!sp->system2) {
            qDebug("Tie::slurPos no system2");
            sp->system2 = sp->system1;
            }
      hw = endNote()->tabHeadWidth(stt);
      if ((ec->notes().size() > 1) || (ec->stem() && !ec->up() && !_up))
            xo = endNote()->x() - hw * 0.12;
      else if (shortStart)
            xo = endNote()->x() + hw * 0.15;
      else
            xo = endNote()->x() + hw * 0.35;
      sp->p2 = ec->pagePos() - sp->system2->pagePos() + QPointF(xo, yo);
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
   : SlurTie(s)
      {
      setAnchor(Anchor::NOTE);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tie::write(Xml& xml) const
      {
      xml.stag(QString("Tie id=\"%1\"").arg(xml.spannerId(this)));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(XmlReader& e)
      {
      e.addSpanner(e.intAttribute("id"), this);
      while (e.readNextStartElement()) {
            if (SlurTie::readProperties(e) || Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      if (score()->mscVersion() <= 114 && spannerSegments().size() == 1) {
            // ignore manual adjustments to single-segment ties in older scores
            SlurSegment* ss = frontSegment();
            QPointF zeroP;
            ss->ups(Grip::START).off     = zeroP;
            ss->ups(Grip::BEZIER1).off   = zeroP;
            ss->ups(Grip::BEZIER2).off   = zeroP;
            ss->ups(Grip::END).off       = zeroP;
            ss->setUserOff(zeroP);
            ss->setUserOff2(zeroP);
            }
      }

//---------------------------------------------------------
//   calculateDirection
//---------------------------------------------------------

void Tie::calculateDirection()
      {
      Chord* c1   = startNote()->chord();
      Chord* c2   = endNote()->chord();
      Measure* m1 = c1->measure();
      Measure* m2 = c2->measure();

      if (_slurDirection == Direction::AUTO) {
            std::vector<Note*> notes = c1->notes();
            int n = notes.size();
            if (m1->mstaff(c1->staffIdx())->hasVoices || m2->mstaff(c2->staffIdx())->hasVoices) {
                  // in polyphonic passage, ties go on the stem side
                  _up = c1->up();
                  }
            else if (n == 1) {
                  //
                  // single note
                  //
                  if (c1->up() != c2->up()) {
                        // if stem direction is mixed, always up
                        _up = true;
                        }
                  else
                        _up = !c1->up();
                  }
            else {
                  //
                  // chords
                  //
                  QList<int> ties;
                  int idx = 0;
                  int noteIdx = -1;
                  for (int i = 0; i < n; ++i) {
                        if (notes[i]->tieFor()) {
                              ties.append(notes[i]->line());
                              if (notes[i] == startNote()) {
                                    idx = ties.size() - 1;
                                    noteIdx = i;
                                    }
                              }
                        }
                  if (idx == 0) {
                        if (ties.size() == 1)         // if just one tie
                              _up = noteIdx != 0;     // it is up if not the bottom note of the chord
                        else                          // if several ties and this is the bottom one (idx == 0)
                              _up = false;            // it is down
                        }
                  else if (idx == ties.size() - 1)
                        _up = true;
                  else {
                        if (ties[idx] <= 4)
                              _up = ((ties[idx-1] - ties[idx]) <= 1) || ((ties[idx] - ties[idx+1]) > 1);
                        else
                              _up = ((ties[idx-1] - ties[idx]) <= 1) && ((ties[idx] - ties[idx+1]) > 1);
                        }
                  }
            }
      else
            _up = _slurDirection == Direction::UP ? true : false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tie::layout()
      {
      //
      //    show short bow
      //
      if (startNote() == 0 || endNote() == 0) {
            if (startNote() == 0) {
                  qDebug("Tie::layout(): no start note");
                  return;
                  }
            Chord* c1 = startNote()->chord();
            if (_slurDirection == Direction::AUTO) {
                  if (c1->measure()->mstaff(c1->staffIdx())->hasVoices) {
                        // in polyphonic passage, ties go on the stem side
                        _up = c1->up();
                        }
                  else
                        _up = !c1->up();
                  }
            else
                  _up = _slurDirection == Direction::UP ? true : false;
            fixupSegments(1);
            SlurSegment* segment = segmentAt(0);
            segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            segment->setSystem(startNote()->chord()->segment()->measure()->system());
            SlurPos sPos;
            slurPos(&sPos);
            segment->layoutSegment(sPos.p1, sPos.p2);
            return;
            }

      calculateDirection();

      qreal w   = startNote()->headWidth();
      qreal xo1 = w * 1.12;
      qreal h   = w * 0.3;
      qreal yo  = _up ? -h : h;

      QPointF off1(xo1, yo);
      QPointF off2(0.0, yo);

      // TODO: cleanup

      SlurPos sPos;
      slurPos(&sPos);

      // p1, p2, s1, s2

      const QList<System*>& systems = score()->systems();
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      int sysIdx1 = systems.indexOf(sPos.system1);
      if (sysIdx1 == -1) {
            qDebug("system not found");
            for (System* s : systems)
                  qDebug("   search %p in %p", sPos.system1, s);
            return;
            }

      int sysIdx2     = systems.indexOf(sPos.system2);
      if (sysIdx2 < 0)
            sysIdx2 = sysIdx1;
      unsigned nsegs  = sysIdx2 - sysIdx1 + 1;
      fixupSegments(nsegs);

      int i = 0;
      for (uint ii = 0; ii < nsegs; ++ii) {
            System* system = systems[sysIdx1++];
            if (system->vbox())
                  continue;
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->layoutSegment(sPos.p1, sPos.p2);
                  segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = system->bbox().width();
                  segment->layoutSegment(sPos.p1, QPointF(x, sPos.p1.y()));
                  segment->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  }
            // case 4: end segment
            else {
                  qreal x = firstNoteRestSegmentX(system);

                  segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
                  segment->setSpannerSegmentType(SpannerSegmentType::END);
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Tie::startEdit(MuseScoreView* v, const QPointF& p)
      {
      editStartNote = startNote();
      editEndNote = endNote();
      SlurTie::startEdit(v, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Tie::endEdit()
      {
      if (editStartNote != startNote() || editEndNote != endNote()) {
            score()->undoStack()->push1(new ChangeSpannerElements(this, editStartNote, editEndNote));
            }
      SlurTie::endEdit();
      score()->setLayoutAll();
      }

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
      {
      setStartElement(note);
      setParent(note);
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

Note* Tie::startNote() const
      {
      Q_ASSERT(!startElement() || startElement()->type() == Element::Type::NOTE);
      return static_cast<Note*>(startElement());
      }

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Tie::endNote() const
      {
      Q_ASSERT(!endElement() || endElement()->type() == Element::Type::NOTE);
      return static_cast<Note*>(endElement());
      }

}

