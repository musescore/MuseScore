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
      QPointF pp1 = ss->ups[GRIP_START].p + ss->ups[GRIP_START].off * _spatium;
      QPointF pp2 = ss->ups[GRIP_END].p   + ss->ups[GRIP_END].off   * _spatium;

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
      if (shoulderH > 1.3)            // maximum tie shoulder height
            shoulderH = 1.3;
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

      qreal w = (score()->styleS(ST_SlurMidWidth).val() - score()->styleS(ST_SlurEndWidth).val()) * _spatium;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ss->ups[GRIP_BEZIER1].off * _spatium);
      QPointF p4o = p6o + t.map(ss->ups[GRIP_BEZIER2].off * _spatium);

      if(!p6o.isNull()) {
            QPointF p6i = t.inverted().map(p6o) / _spatium;
            ss->ups[GRIP_BEZIER1].off += p6i ;
            ss->ups[GRIP_BEZIER2].off += p6i;
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
      ss->path                 = t.map(ss->path);
      ss->shapePath            = t.map(ss->shapePath);
      ss->ups[GRIP_BEZIER1].p  = t.map(p3);
      ss->ups[GRIP_BEZIER2].p  = t.map(p4);
      ss->ups[GRIP_END].p      = t.map(p2) - ss->ups[GRIP_END].off * _spatium;
      ss->ups[GRIP_DRAG].p     = t.map(p5);
      ss->ups[GRIP_SHOULDER].p = t.map(p6);
      }

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Tie::slurPos(SlurPos* sp)
      {
      qreal hw   = startNote()->headWidth();
      qreal __up = _up ? -1.0 : 1.0;
      qreal _spatium = spatium();

      Chord* sc   = startNote()->chord();
      Q_ASSERT(sc);
      sp->system1 = sc->measure()->system();
      if (!sp->system1) {
            Measure* m = sc->measure();
            printf("no system: measure is %d has %d count %d\n", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
            }
      Q_ASSERT(sp->system1);

      qreal xo;
      qreal yo;

      //------p1
      if ((sc->notes().size() > 1) || (sc->stem() && (sc->up() == _up))) {
            xo = startNote()->x() + hw * 1.12;
            yo = startNote()->pos().y() + hw * .3 * __up;
            }
      else {
            xo = startNote()->x() + hw * 0.85;
            yo = startNote()->pos().y() + _spatium * .75 * __up;
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
      if ((ec->notes().size() > 1) || (ec->stem() && !ec->up() && !_up))
            xo = endNote()->x() - hw * 0.12;
      else
            xo = endNote()->x() + hw * 0.15;
      sp->p2 = ec->pagePos() - sp->system2->pagePos() + QPointF(xo, yo);
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
   : SlurTie(s)
      {
      setAnchor(ANCHOR_NOTE);
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
//   write
//---------------------------------------------------------

void Tie::write(Xml& xml) const
      {
      xml.stag(QString("Tie id=\"%1\"").arg(id()));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(XmlReader& e)
      {
      setId(e.intAttribute("id"));
      while (e.readNextStartElement()) {
            if (SlurTie::readProperties(e) || Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tie::layout()
      {
      qreal _spatium = spatium();

      //
      //    show short bow
      //
      if (startNote() == 0 || endNote() == 0) {
            if (startNote() == 0) {
                  qDebug("Tie::layout(): no start note");
                  return;
                  }
            Chord* c1 = startNote()->chord();
            if (_slurDirection == MScore::AUTO) {
                  if (c1->measure()->mstaff(c1->staffIdx())->hasVoices) {
                        // in polyphonic passage, ties go on the stem side
                        _up = c1->up();
                        }
                  else
                        _up = !c1->up();
                  }
            else
                  _up = _slurDirection == MScore::UP ? true : false;
            fixupSegments(1);
            SlurSegment* segment = segmentAt(0);
            segment->setSpannerSegmentType(SEGMENT_SINGLE);
            segment->setSystem(startNote()->chord()->segment()->measure()->system());
            SlurPos sPos;
            slurPos(&sPos);
            segment->layout(sPos.p1, sPos.p2);
            return;
            }

      Chord* c1   = startNote()->chord();
      Chord* c2   = endNote()->chord();
      Measure* m1 = c1->measure();
      Measure* m2 = c2->measure();

      if (_slurDirection == MScore::AUTO) {
            QList<Note*> notes = c1->notes();
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
                  for (int i = 0; i < n; ++i) {
                        if (notes[i]->tieFor()) {
                              ties.append(notes[i]->line());
                              if (notes[i] == startNote())
                                    idx = ties.size() - 1;
                              }
                        }
                  if (idx == 0)
                        _up = false;
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
            _up = _slurDirection == MScore::UP ? true : false;

      qreal w   = startNote()->headWidth();
      qreal xo1 = w * 1.12;
      qreal h   = w * 0.3;
      qreal yo  = _up ? -h : h;

      QPointF off1(xo1, yo);
      QPointF off2(0.0, yo);

      QPointF ppos(pagePos());

      // TODO: cleanup

      SlurPos sPos;
      slurPos(&sPos);

      // p1, p2, s1, s2

      QList<System*>* systems = score()->systems();
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      int sysIdx1 = systems->indexOf(sPos.system1);
      if (sysIdx1 == -1) {
            qDebug("system not found");
            foreach(System* s, *systems)
                  qDebug("   search %p in %p", sPos.system1, s);
            return;
            }

      int sysIdx2     = systems->indexOf(sPos.system2);
      if (sysIdx2 < 0)
            sysIdx2 = sysIdx1;
      unsigned nsegs  = sysIdx2 - sysIdx1 + 1;
      fixupSegments(nsegs);

      int i = 0;
      for (uint ii = 0; ii < nsegs; ++ii) {
            System* system = (*systems)[sysIdx1++];
            if (system->isVbox())
                  continue;
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->layout(sPos.p1, sPos.p2);
                  segment->setSpannerSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = system->bbox().width();
                  segment->layout(sPos.p1, QPointF(x, sPos.p1.y()));
                  segment->setSpannerSegmentType(SEGMENT_BEGIN);
                  }
            // case 4: end segment
            else {
                  qreal x = firstNoteRestSegmentX(system) - 2 * _spatium;

                  segment->layout(QPointF(x, sPos.p2.y()), sPos.p2);
                  segment->setSpannerSegmentType(SEGMENT_END);
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
            score()->undo()->push1(new ChangeSpannerElements(this, editStartNote, editEndNote));
            }
      SlurTie::endEdit();
      score()->setLayoutAll(true);
      }

}

