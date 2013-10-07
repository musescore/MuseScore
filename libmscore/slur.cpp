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

#include "note.h"
#include "chord.h"
#include "xml.h"
#include "slur.h"
#include "tie.h"
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

namespace Ms {

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(Score* score)
   : SpannerSegment(score)
      {
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : SpannerSegment(b)
      {
      for (int i = 0; i < SLUR_GRIPS; ++i)
            ups[i] = b.ups[i];
      path = b.path;
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      move(s);
      for (int k = 0; k < SLUR_GRIPS; ++k)
            ups[k].p += s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(QPainter* painter) const
      {
      // hide tie toward the second chord of a cross-measure value
      if ((slurTie()->type() == TIE)
         && (static_cast<Tie*>(slurTie())->endNote())
         && (static_cast<Tie*>(slurTie())->endNote()->chord()->crossMeasure() == CROSSMEASURE_SECOND))
            return;

      QPen pen(curColor());
      switch (slurTie()->lineType()) {
            case 0:
                  painter->setBrush(QBrush(QColor(curColor())));
                  pen.setCapStyle(Qt::RoundCap);
                  pen.setJoinStyle(Qt::RoundJoin);
                  pen.setWidthF(point(score()->styleS(ST_SlurEndWidth)));
                  break;
            case 1:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(point(score()->styleS(ST_SlurDottedWidth)));
                  pen.setStyle(Qt::DotLine);
                  break;
            case 2:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(point(score()->styleS(ST_SlurDottedWidth)));
                  pen.setStyle(Qt::DashLine);
                  break;
            }
      painter->setPen(pen);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   updateGrips
//    return grip rectangles in page coordinates
//---------------------------------------------------------

void SlurSegment::updateGrips(int* n, QRectF* r) const
      {
      *n = SLUR_GRIPS;
      QPointF p(pagePos());
      for (int i = 0; i < SLUR_GRIPS; ++i)
            r[i].translate(ups[i].p + ups[i].off * spatium() + p);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(MuseScoreView* viewer, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      Slur* sl = static_cast<Slur*>(slurTie());

      if (key == Qt::Key_X) {
            sl->setSlurDirection(sl->up() ? MScore::DOWN : MScore::UP);
            sl->layout();
            return true;
            }
      if (slurTie()->type() != SLUR)
            return false;


      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SEGMENT_SINGLE)
              || (spannerSegmentType() == SEGMENT_BEGIN && curGrip == GRIP_START)
              || (spannerSegmentType() == SEGMENT_END && curGrip == GRIP_END)
            )))
            return false;

      ChordRest* cr = 0;
      ChordRest* e  = curGrip == GRIP_START ? sl->startCR() : sl->endCR();
      ChordRest* e1 = curGrip == GRIP_START ? sl->endCR() : sl->startCR();

      if (key == Qt::Key_Left)
            cr = prevChordRest(e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest(e);

      if (cr == 0 || cr == e1)
            return true;
      changeAnchor(viewer, curGrip, cr);
      return true;
      }

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(MuseScoreView* viewer, int curGrip, Element* element)
      {
      if (curGrip == GRIP_START) {
            spanner()->setStartElement(element);
            if (spanner()->anchor() == Spanner::ANCHOR_NOTE) {
                  Tie* tie = static_cast<Tie*>(spanner());
                  tie->startNote()->setTieFor(0);
                  tie->setStartNote(static_cast<Note*>(element));
                  static_cast<Note*>(element)->setTieFor(tie);
                  }
            else if (spanner()->anchor() == Spanner::ANCHOR_SEGMENT)
                  spanner()->setTick(static_cast<Chord*>(element)->tick());
            }
      else {
            spanner()->setEndElement(element);
            if (spanner()->anchor() == Spanner::ANCHOR_NOTE) {
                  Tie* tie = static_cast<Tie*>(spanner());
                  tie->endNote()->setTieBack(0);
                  tie->setEndNote(static_cast<Note*>(element));
                  static_cast<Note*>(element)->setTieBack(tie);
                  }
            else if (spanner()->anchor() == Spanner::ANCHOR_SEGMENT)
                  spanner()->setTick2(static_cast<Chord*>(element)->tick());
            }

      int segments  = spanner()->spannerSegments().size();
      ups[curGrip].off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            QList<SpannerSegment*>& ss = spanner()->spannerSegments();

            SlurSegment* newSegment = static_cast<SlurSegment*>(curGrip == GRIP_END ? ss.back() : ss.front());
            score()->endCmd();
            score()->startCmd();
            viewer->startEdit(newSegment, curGrip);
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(int grip) const
      {
      SlurPos spos;
      slurTie()->slurPos(&spos);

      QPointF sp(system()->pagePos());
      QPointF p1(spos.p1 + spos.system1->pagePos());
      QPointF p2(spos.p2 + spos.system2->pagePos());
      switch (spannerSegmentType()) {
            case SEGMENT_SINGLE:
                  if (grip == GRIP_START)
                        return p1;
                  else if (grip == GRIP_END)
                        return p2;
                  break;

            case SEGMENT_BEGIN:
                  if (grip == GRIP_START)
                        return p1;
                  else if (grip == GRIP_END)
                        return system()->abbox().topRight();
                  break;

            case SEGMENT_MIDDLE:
                  if (grip == GRIP_START)
                        return sp;
                  else if (grip == GRIP_END)
                        return system()->abbox().topRight();
                  break;

            case SEGMENT_END:
                  if (grip == GRIP_START)
                        return sp;
                  else if (grip == GRIP_END)
                        return p2;
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF SlurSegment::getGrip(int n) const
      {
      switch(n) {
            case GRIP_START:
            case GRIP_END:
                  return (ups[n].p - gripAnchor(n)) / spatium() + ups[n].off;
            default:
                  return ups[n].off;
            }
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void SlurSegment::setGrip(int n, const QPointF& pt)
      {
      switch(n) {
            case GRIP_START:
            case GRIP_END:
                  ups[n].off = ((pt * spatium()) - (ups[n].p - gripAnchor(n))) / spatium();
                  break;
            default:
                  ups[n].off = pt;
                  break;
            }
      slurTie()->layout();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurSegment::editDrag(const EditData& ed)
      {
      qreal _spatium = spatium();
      ups[ed.curGrip].off += (ed.delta / _spatium);
      if (ed.curGrip == GRIP_START || ed.curGrip == GRIP_END) {
            slurTie()->computeBezier(this);
            //
            // move anchor for slurs
            //
            Spanner* spanner = static_cast<Spanner*>(slurTie());
            Element* e = ed.view->elementNear(ed.pos);
            SpannerSegmentType st = spannerSegmentType();
            if (
               (ed.curGrip == GRIP_START  && (st == SEGMENT_SINGLE || st == SEGMENT_BEGIN))
               || (ed.curGrip == GRIP_END && (st == SEGMENT_SINGLE || st == SEGMENT_END))
               ) {
                  if (e && e->type() == NOTE) {
                        Note* note = static_cast<Note*>(e);
                        Chord* chord = note->chord();
                        if ((spanner->type() == SLUR)
                           && ((ed.curGrip == GRIP_END && chord != spanner->endCR())
                           || (ed.curGrip == GRIP_START && chord != spanner->startCR())))
                              {
                              changeAnchor(ed.view, ed.curGrip, chord);
                              QPointF p1 = ed.pos - ups[ed.curGrip].p - pagePos();
                              ups[ed.curGrip].off = p1 / _spatium;
                              return;
                              }
                        if (ed.curGrip == GRIP_END && spanner->type() == TIE) {
                              Tie* tie = static_cast<Tie*>(spanner);
                              if (tie->startNote()->pitch() == note->pitch()) {
                                    ed.view->setDropTarget(note);
                                    if (note != tie->endNote()) {
                                          changeAnchor(ed.view, ed.curGrip, note);
                                          // tie->endNote()->setTieBack(0);
                                          // tie->setEndNote(note);
                                          // note->setTieBack(tie);
                                          return;
                                          }
                                    }
                              }
                        }
                  else
                        ed.view->setDropTarget(0);
                  }
            }
      else if (ed.curGrip == GRIP_BEZIER1 || ed.curGrip == GRIP_BEZIER2)
            slurTie()->computeBezier(this);
      else if (ed.curGrip == GRIP_SHOULDER) {
            ups[ed.curGrip].off = QPointF();
            slurTie()->computeBezier(this, ed.delta);
            }
      else if (ed.curGrip == GRIP_DRAG) {
            ups[GRIP_DRAG].off = QPointF();
            setUserOff(userOff() + ed.delta);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurSegment::write(Xml& xml, int no) const
      {
      if (ups[GRIP_START].off.isNull()
         && ups[GRIP_END].off.isNull()
         && ups[GRIP_BEZIER1].off.isNull()
         && ups[GRIP_BEZIER2].off.isNull()
         && userOff().isNull()
         && visible()
         && (color() == Qt::black)
            )
            return;

      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));
      if (!(ups[GRIP_START].off.isNull()))
            xml.tag("o1", ups[GRIP_START].off);
      if (!(ups[GRIP_BEZIER1].off.isNull()))
            xml.tag("o2", ups[GRIP_BEZIER1].off);
      if (!(ups[GRIP_BEZIER2].off.isNull()))
            xml.tag("o3", ups[GRIP_BEZIER2].off);
      if (!(ups[GRIP_END].off.isNull()))
            xml.tag("o4", ups[GRIP_END].off);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurSegment::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "o1")
                  ups[GRIP_START].off = e.readPoint();
            else if (tag == "o2")
                  ups[GRIP_BEZIER1].off = e.readPoint();
            else if (tag == "o3")
                  ups[GRIP_BEZIER2].off = e.readPoint();
            else if (tag == "o4")
                  ups[GRIP_END].off = e.readPoint();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void Slur::computeBezier(SlurSegment* ss, QPointF p6o)
      {
      qreal _spatium  = spatium();
      qreal shoulderW;              // height as fraction of slur-length
      qreal shoulderH;
      //
      // p1 and p2 are the end points of the slur
      //
      QPointF pp1 = ss->ups[GRIP_START].p + ss->ups[GRIP_START].off * _spatium;
      QPointF pp2 = ss->ups[GRIP_END].p   + ss->ups[GRIP_END].off   * _spatium;

      QPointF p2 = pp2 - pp1;
      if ((p2.x() == 0.0) && (p2.y() == 0.0)) {
            Measure* m1 = startCR()->segment()->measure();
            Measure* m2 = endCR()->segment()->measure();
            qDebug("zero slur id%d at tick %d(%d) track %d in measure %d-%d",
               id(), m1->tick(), tick(), track(), m1->no(), m2->no());
            return;
            }

      qreal sinb = atan(p2.y() / p2.x());
      QTransform t;
      t.rotateRadians(-sinb);
      p2  = t.map(p2);
      p6o = t.map(p6o);

      double smallH = 0.5;
      qreal d = p2.x() / _spatium;
      if (d <= 2.0) {
            shoulderH = d * 0.5 * smallH * _spatium;
            shoulderW = .6;
            }
      else {
            qreal dd = log10(1.0 + (d - 2.0) * .5) * 2.0;
            if (dd > 3.0)
                  dd = 3.0;
            shoulderH = (dd + smallH) * _spatium;
            if (d > 18.0)
                  shoulderW = 0.7; // 0.8;
            else if (d > 10)
                  shoulderW = 0.6; // 0.7;
            else
                  shoulderW = 0.5; // 0.6;
            }

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
      if (((c2 - c1) / _spatium) <= _spatium)
            w *= .5;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ss->ups[GRIP_BEZIER1].off * _spatium);
      QPointF p4o = p6o + t.map(ss->ups[GRIP_BEZIER2].off * _spatium);

      if (!p6o.isNull()) {
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
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void SlurSegment::layout(const QPointF& p1, const QPointF& p2)
      {
      ups[GRIP_START].p = p1;
      ups[GRIP_END].p   = p2;
      slurTie()->computeBezier(this);
      setbbox(path.boundingRect());
      adjustReadPos();
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Spanner(s)
      {
      _slurDirection = MScore::AUTO;
      _up            = true;
      _lineType      = 0;     // default is solid
      }

SlurTie::SlurTie(const SlurTie& t)
   : Spanner(t)
      {
      _up            = t._up;
      _slurDirection = t._slurDirection;
      _lineType      = t._lineType;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   fixArticulations
//---------------------------------------------------------

static qreal fixArticulations(qreal yo, Chord* c, qreal _up)
      {
      //
      // handle special case of tenuto and staccato;
      //
      const QList<Articulation*>& al = c->articulations();
      if (al.size() >= 2) {
            Articulation* a = al.at(1);
            if (a->articulationType() == Articulation_Tenuto || a->articulationType() == Articulation_Staccato)
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      else if (al.size() >= 1) {
            Articulation* a = al.at(0);
            if (a->articulationType() == Articulation_Tenuto || a->articulationType() == Articulation_Staccato)
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      return yo;
      }

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Slur::slurPosChord(SlurPos* sp)
      {
      Note* _startNote = startChord()->downNote();
      Note* _endNote   = endChord()->downNote();
      qreal hw         = _startNote->headWidth();
      qreal __up       = _up ? -1.0 : 1.0;
      qreal _spatium = spatium();

      sp->system1 = endChord()->measure()->system();
      sp->system2 = sp->system1;
      QPointF pp(sp->system1->pagePos());

      qreal xo;
      qreal yo;

      //------p1
      if (_up) {
            xo = _startNote->x() + hw * 1.12;
            yo = _startNote->pos().y() + hw * .3 * __up;
            }
      else {
            xo = _startNote->x() + hw * 0.4;
            yo = _startNote->pos().y() + _spatium * .75 * __up;
            }
      sp->p1 = startChord()->pagePos() - pp + QPointF(xo, yo);

      //------p2
      if ((endChord()->notes().size() > 1) || (endChord()->stem() && !endChord()->up() && !_up)) {
            xo = _endNote->x() - hw * 0.12;
            yo = _endNote->pos().y() + hw * .3 * __up;
            }
      else {
            xo = _endNote->x() + hw * 0.15;
            yo = _endNote->pos().y() + _spatium * .75 * __up;
            }
      sp->p2 = endChord()->pagePos() - pp + QPointF(xo, yo);
      }

//---------------------------------------------------------
//   slurPos
//    calculate position of start- and endpoint of slur
//    relative to System() position
//---------------------------------------------------------

void Slur::slurPos(SlurPos* sp)
      {
      if (anchor() == ANCHOR_CHORD) {
            slurPosChord(sp);
            return;
            }
      qreal _spatium = spatium();

      if (endCR() == 0) {
            sp->p1 = startCR()->pagePos();
            sp->p1.rx() += startCR()->width();
            sp->p2 = sp->p1;
            sp->p2.rx() += 5 * _spatium;
            sp->system1 = startCR()->measure()->system();
            sp->system2 = sp->system1;
            return;
            }

      ChordRest* scr   = startCR();
      ChordRest* ecr   = endCR();
      Chord* sc   = 0;
      Note* note1 = 0;
      if (startCR()->type() == CHORD) {
            sc = static_cast<Chord*>(startCR());
            note1 = _up ? sc->upNote() : sc->downNote();
            }
      Chord* ec = 0;
      Note* note2 = 0;
      if (endCR()->type() == CHORD) {
            ec   = static_cast<Chord*>(endCR());
            note2 = _up ? ec->upNote() : ec->downNote();
            }

      sp->system1 = scr->measure()->system();
      sp->system2 = ecr->measure()->system();
      sp->p1      = scr->pagePos() - sp->system1->pagePos();
      sp->p2      = ecr->pagePos() - sp->system2->pagePos();

      qreal xo, yo;

      Stem* stem1 = sc?sc->stem():0;
      Stem* stem2 = ec?ec->stem():0;

      enum SlurAnchor {
            SA_NONE, SA_STEM
            };
      SlurAnchor sa1 = SA_NONE;
      SlurAnchor sa2 = SA_NONE;
      if ((scr->up() == ecr->up()) && !scr->beam() && !ecr->beam() && (_up == scr->up())) {
            if (stem1)
                  sa1 = SA_STEM;
            if (stem2)
                  sa2 = SA_STEM;
            }

      qreal __up = _up ? -1.0 : 1.0;
      qreal hw   = note1 ? note1->headWidth() : startCR()->width();
      switch (sa1) {
            case SA_STEM: //sc can't be null
                  sp->p1 += sc->stemPosBeam() - sc->pagePos() + sc->stem()->p2();
                  sp->p1 += QPointF(0.35 * _spatium, 0.25 * _spatium);
                  break;
            case SA_NONE:
                  break;
            }
      switch(sa2) {
            case SA_STEM: //ec can't be null
                  sp->p2 += ec->stemPosBeam() - ec->pagePos() + ec->stem()->p2();
                  sp->p2 += QPointF(-0.35 * _spatium, 0.25 * _spatium);
                  break;
            case SA_NONE:
                  break;
            }

      //
      // default position:
      //    horizontal: middle of note head
      //    vertical:   _spatium * .4 above/below note head
      //
      //------p1
      bool stemPos = false;   // p1 starts at chord stem side
      if (note1)
            yo = note1->pos().y();
      else if(_up)
            yo = startCR()->bbox().top();
      else
            yo = startCR()->bbox().top() + startCR()->height();
      yo += _spatium * .9 * __up;
      xo = hw * .5;

      if (stem1) { //sc not null
            Beam* beam1 = sc->beam();
            if (beam1 && (beam1->elements().back() != sc) && (sc->up() == _up)) {
                  qreal sh = stem1->height() + _spatium;
                  yo       = sc->downNote()->pos().y() + sh * __up;
                  xo       = stem1->pos().x();
                  stemPos  = true;
                  }
            else {
                  if (sc->up() && _up)
                        xo = hw + _spatium * .3;
                  //
                  // handle case: stem up   - stem down
                  //              stem down - stem up
                  //
                  if ((sc->up() != ecr->up()) && (sc->up() == _up)) {
                        Note* n1  = sc->up() ? sc->downNote() : sc->upNote();
                        Note* n2  = 0;
                        if(ec)
                              n2 = ec->up() ? ec->downNote() : ec->upNote();
                        qreal yd  = (n2 ? n2->pos().y() : endCR()->pos().y()) - n1->pos().y();

                        yd *= .5;

                        qreal sh = stem1->height();    // limit y move
                        if (yd > 0.0) {
                              if (yd > sh)
                                    yd = sh;
                              }
                        else {
                              if (yd < - sh)
                                    yd = -sh;
                              }
                        stemPos = true;
                        if ((_up && (yd < -_spatium)) || (!_up && (yd > _spatium)))
                              yo += yd;
                        }
                  else if (sc->up() != _up)
                        yo = fixArticulations(yo, sc, __up);
                  }
            }

      if (sa1 == SA_NONE)
            sp->p1 += QPointF(xo, yo);

      //------p2
      xo = hw * .5;
      if (note2)
            yo = note2->pos().y();
      else if(_up)
            yo = endCR()->bbox().top();
      else
            yo = endCR()->bbox().top() + endCR()->height();
      yo += _spatium * .9 * __up;

      if (stem2) { //ec can't be null
            Beam* beam2 = ec->beam();
            if ((stemPos && (scr->up() == ec->up()))
               || (beam2
                 && (!beam2->elements().isEmpty())
                 && (beam2->elements().front() != ec)
                 && (ec->up() == _up)
                 && sc && (sc->noteType() == NOTE_NORMAL)
                 )
                  ) {
                  qreal sh = stem2->height() + _spatium;
                  if (_up)
                        yo = ec->downNote()->pos().y() - sh;
                  else
                        yo = ec->upNote()->pos().y() + sh;
                  xo = stem2->pos().x();
                  }
            else if (!ec->up() && !_up)
                  xo = -_spatium * .3 + note2->x();
            //
            // handle case: stem up   - stem down
            //              stem down - stem up
            //
            if ((scr->up() != ec->up()) && (ec->up() == _up)) {
                  Note* n1 = 0;
                  if(sc)
                        sc->up() ? sc->downNote() : sc->upNote();
                  Note* n2 = ec->up() ? ec->downNote() : ec->upNote();
                  qreal yd = n2->pos().y() - (n1 ? n1->pos().y() : startCR()->pos().y());

                  yd *= .5;

                  qreal mh = stem2->height();    // limit y move
                  if (yd > 0.0) {
                        if (yd > mh)
                              yd = mh;
                        }
                  else {
                        if (yd < - mh)
                              yd = -mh;
                        }

                  if ((_up && (yd > _spatium)) || (!_up && (yd < -_spatium)))
                        yo -= yd;
                  }
            else if (ec->up() != _up)
                  yo = fixArticulations(yo, ec, __up);
            }

      if (sa2 == SA_NONE)
            sp->p2 += QPointF(xo, yo);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      int idx = 0;
      foreach(const SpannerSegment* ss, spannerSegments())
            ((SlurSegment*)ss)->write(xml, idx++);
      if (_slurDirection)
            xml.tag("up", _slurDirection);
      if (_lineType)
            xml.tag("lineType", _lineType);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "SlurSegment") {
            int idx = e.intAttribute("no", 0);
            int n = spannerSegments().size();
            for (int i = n; i < idx; ++i)
                  add(new SlurSegment(score()));
            SlurSegment* segment = new SlurSegment(score());
            segment->read(e);
            add(segment);
            }
      else if (tag == "up")
            _slurDirection = MScore::Direction(e.readInt());
      else if (tag == "lineType")
            _lineType = e.readInt();
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   undoSetLineType
//---------------------------------------------------------

void SlurTie::undoSetLineType(int t)
      {
      score()->undoChangeProperty(this, P_LINE_TYPE, t);
      }

//---------------------------------------------------------
//   undoSetSlurDirection
//---------------------------------------------------------

void SlurTie::undoSetSlurDirection(MScore::Direction d)
      {
      score()->undoChangeProperty(this, P_SLUR_DIRECTION, d);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTie::reset()
      {
      score()->undoChangeProperty(this, P_USER_OFF, QPointF());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurTie::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_LINE_TYPE:      return lineType();
            case P_SLUR_DIRECTION: return slurDirection();
            default:
                  return Spanner::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurTie::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_LINE_TYPE:      setLineType(v.toInt()); break;
            case P_SLUR_DIRECTION: setSlurDirection(MScore::Direction(v.toInt())); break;
            default:
                  return Spanner::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurTie::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_LINE_TYPE:
                  return 0;
            case P_SLUR_DIRECTION:
                  return MScore::AUTO;
            default:
                  return Spanner::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurSegment::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_LINE_TYPE:
            case P_SLUR_DIRECTION:
                  return slurTie()->getProperty(propertyId);
            default:
                  return SpannerSegment::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool SlurSegment::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_LINE_TYPE:
            case P_SLUR_DIRECTION:
                  return slurTie()->setProperty(propertyId, v);
            default:
                  return SpannerSegment::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_LINE_TYPE:
            case P_SLUR_DIRECTION:
                  return slurTie()->propertyDefault(id);
            default:
                  return SpannerSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurSegment::reset()
      {
      score()->undoChangeProperty(this, P_USER_OFF, QPointF());
      score()->undo(new ChangeSlurOffsets(this, QPointF(), QPointF(), QPointF(), QPointF()));
      for (int i = 0; i < SLUR_GRIPS; ++i)
            ups[i].off = QPointF();
      parent()->reset();
      parent()->layout();
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(Score* s)
   : SlurTie(s)
      {
      setId(-1);
      setAnchor(ANCHOR_SEGMENT);
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::~Slur()
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slur::write(Xml& xml) const
      {
      xml.stag(QString("Slur id=\"%1\"").arg(id()));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(XmlReader& e)
      {
      setTrack(e.track());      // set staff
      setId(e.intAttribute("id"));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "track2")
                  setTrack2(e.readInt());
            else if (tag == "startTrack")       // obsolete
                  setTrack(e.readInt());
            else if (tag == "endTrack")         // obsolete
                  e.readInt();
            else if (!SlurTie::readProperties(e))
                  e.unknown();
            }
      if(track2() == -1)
            setTrack2(track());
      }

//---------------------------------------------------------
//   chordsHaveTie
//---------------------------------------------------------

static bool chordsHaveTie(Chord* c1, Chord* c2)
      {
      int n1 = c1->notes().size();
      for (int i1 = 0; i1 < n1; ++i1) {
            Note* n1 = c1->notes().at(i1);
            int n2 = c2->notes().size();
            for (int i2 = 0; i2 < n2; ++i2) {
                  Note* n2 = c2->notes().at(i2);
                  if (n1->tieFor() && n1->tieFor() == n2->tieBack())
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   directionMixture
//---------------------------------------------------------

static bool isDirectionMixture(Chord* c1, Chord* c2)
      {
      bool up = c1->up();
      for (Segment* seg = c1->segment(); seg; seg = seg->next(Segment::SegChordRest)) {
            Chord* c = static_cast<Chord*>(seg->element(c1->track()));
            if (!c || c->type() != Element::CHORD)
                  continue;
            if (c->up() != up)
                  return true;
            if (seg == c2->segment())
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout()
      {
      if (anchor() == ANCHOR_CHORD) {
            layoutChord();
            return;
            }
      if (track2() == -1)
            setTrack2(track());

      qreal _spatium = spatium();

      if (score() == gscore || tick() == -1) {
            //
            // when used in a palette, slur has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            SlurSegment* s;
            if (spannerSegments().isEmpty()) {
                  s = new SlurSegment(score());
                  s->setTrack(track());
                  add(s);
                  }
            else {
                  s = frontSegment();
                  }
            s->setSpannerSegmentType(SEGMENT_SINGLE);
            s->layout(QPointF(0, 0), QPointF(_spatium * 6, 0));
            setbbox(frontSegment()->bbox());
            return;
            }

      if (anchor() == Spanner::ANCHOR_SEGMENT) {
            computeStartElement();
            computeEndElement();
            }

      if (startCR() == 0 || endCR() == 0) {
            qDebug("Slur::layout(): id %d  track %d-%d  %p - %p tick %d-%d null anchor",
               id(), track(), track2(), startCR(), endCR(), tick(), tick2());
            return;
            }
      switch (_slurDirection) {
            case MScore::UP:
                  _up = true;
                  break;
            case MScore::DOWN:
                  _up = false;
                  break;
            case MScore::AUTO:
                  {
                  //
                  // assumption:
                  // slurs have only chords or rests as start/end elements
                  //
                  if (startCR() == 0 || endCR() == 0) {
                        _up = true;
                        break;
                        }
                  Measure* m1    = startCR()->measure();

                  Chord* c1 = (startCR()->type() == CHORD) ? static_cast<Chord*>(startCR()) : 0;
                  Chord* c2 = (endCR()->type() == CHORD) ? static_cast<Chord*>(endCR()) : 0;

                  _up = !(startCR()->up());

                  if ((endCR()->tick() - startCR()->tick()) > m1->ticks()) {
                        // long slurs are always above
                        _up = true;
                        }
                  else
                        _up = !(startCR()->up());

                  if (c1 && c2 && isDirectionMixture(c1, c2) && (c1->noteType() == NOTE_NORMAL)) {
                        // slurs go above if start and end note have different stem directions,
                        // but grace notes are exceptions
                        _up = true;
                        }
                  else if (m1->mstaff(startCR()->staffIdx())->hasVoices && c1 && c1->noteType() == NOTE_NORMAL) {
                        // in polyphonic passage, slurs go on the stem side
                        _up = startCR()->up();
                        }
                  else if (c1 && c2 && chordsHaveTie(c1, c2)) {
                        // could confuse slur with tie, put slur on stem side
                        _up = startCR()->up();
                        }
                  }
                  break;
            }

      SlurPos sPos;
      slurPos(&sPos);

      QList<System*>* sl = score()->systems();
      iSystem is = sl->begin();
      while (is != sl->end()) {
            if (*is == sPos.system1)
                  break;
            ++is;
            }
      if (is == sl->end())
            qDebug("Slur::layout  first system not found");
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      unsigned nsegs = 1;
      for (iSystem iis = is; iis != sl->end(); ++iis) {
            if ((*iis)->isVbox())
                  continue;
            if (*iis == sPos.system2)
                  break;
            ++nsegs;
            }

      fixupSegments(nsegs);

      for (int i = 0; is != sl->end(); ++i, ++is) {
            System* system  = *is;
            if (system->isVbox()) {
                  --i;
                  continue;
                  }
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->setSpannerSegmentType(SEGMENT_SINGLE);
                  segment->layout(sPos.p1, sPos.p2);
                  }
            // case 2: start segment
            else if (i == 0) {
                  segment->setSpannerSegmentType(SEGMENT_BEGIN);
                  qreal x = system->bbox().width();
                  segment->layout(sPos.p1, QPointF(x, sPos.p1.y()));
                  }
            // case 3: middle segment
            else if (i != 0 && system != sPos.system2) {
                  segment->setSpannerSegmentType(SEGMENT_MIDDLE);
                  qreal x1 = firstNoteRestSegmentX(system) - _spatium;
                  qreal x2 = system->bbox().width();
                  qreal y  = system->staff(staffIdx())->y();
                  segment->layout(QPointF(x1, y), QPointF(x2, y));
                  }
            // case 4: end segment
            else {
                  segment->setSpannerSegmentType(SEGMENT_END);
                  qreal x = firstNoteRestSegmentX(system) - _spatium;
                  segment->layout(QPointF(x, sPos.p2.y()), sPos.p2);
                  }
            if (system == sPos.system2)
                  break;
            }
      setbbox(spannerSegments().isEmpty() ? QRectF() : frontSegment()->bbox());
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//    in System() coordinates
//---------------------------------------------------------

qreal SlurTie::firstNoteRestSegmentX(System* system)
      {
      foreach(const MeasureBase* mb, system->measures()) {
            if (mb->type() == MEASURE) {
                  const Measure* measure = static_cast<const Measure*>(mb);
                  for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->segmentType() == Segment::SegChordRest) {
                              return seg->pos().x() + seg->measure()->pos().x();
                              }
                        }
                  }
            }
      qDebug("firstNoteRestSegmentX: did not find segment");
      return 0.0;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(int n)
      {
      Element::setTrack(n);
      foreach(SpannerSegment* ss, spannerSegments())
            ss->setTrack(n);
      }

//---------------------------------------------------------
//   fixupSegments
//---------------------------------------------------------

void SlurTie::fixupSegments(unsigned nsegs)
      {
      unsigned onsegs = spannerSegments().size();
      if (nsegs > onsegs) {
            for (unsigned i = onsegs; i < nsegs; ++i) {
                  SlurSegment* s;
                  if (!delSegments.isEmpty()) {
                        s = delSegments.dequeue();
                        }
                  else {
                        s = new SlurSegment(score());
                        }
                  s->setTrack(track());
                  add(s);
                  }
            }
      else if (nsegs < onsegs) {
            for (unsigned i = nsegs; i < onsegs; ++i) {
                  SlurSegment* s = takeLastSegment();
                  s->setSystem(0);
                  delSegments.enqueue(s);  // cannot delete: used in SlurSegment->edit()
                  }
            }
      }

//---------------------------------------------------------
//   layoutChord
//---------------------------------------------------------

void Slur::layoutChord()
      {
      qreal _spatium = spatium();

      //
      //    show short bow
      //
      if (startChord() == 0 || endChord() == 0) {
            qDebug("no start/end chord");
            return;
            }

      Chord* c1 = startChord();
//      Chord* c2 = endChord();
      Note* startNote = c1->upNote();
      // Note* endNote = c2->upNote();

      if (_slurDirection == MScore::AUTO)
            _up = false;
      else
            _up = _slurDirection == MScore::UP ? true : false;

      qreal w   = startNote->headWidth();
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

}

