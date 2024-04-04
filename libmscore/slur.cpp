//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "chord.h"
#include "stem.h"
#include "slur.h"
#include "tie.h"
#include "part.h"
#include "navigate.h"
#include "articulation.h"

namespace Ms {

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(QPainter* painter) const
      {
      QPen pen(curColor());
      qreal mag = staff() ? staff()->mag(slur()->tick()) : 1.0;

      //Replace generic Qt dash patterns with improved equivalents to show true dots (keep in sync with tie.cpp)
      QVector<qreal> dotted     = { 0.01, 1.99 }; // tighter than Qt DotLine equivalent - woud be { 0.01, 2.99 }
      QVector<qreal> dashed     = { 3.00, 3.00 }; // Compensating for caps. Qt default DashLine is { 4.0, 2.0 }
      QVector<qreal> wideDashed = { 5.00, 6.00 };

      switch (slurTie()->lineType()) {
            case 0:
                  painter->setBrush(QBrush(pen.color()));
                  pen.setCapStyle(Qt::RoundCap);
                  pen.setJoinStyle(Qt::RoundJoin);
                  pen.setWidthF(score()->styleP(Sid::SlurEndWidth) * mag);
                  break;
            case 1:
                  painter->setBrush(Qt::NoBrush);
                  pen.setCapStyle(Qt::RoundCap); // round dots
                  pen.setDashPattern(dotted);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
                  break;
            case 2:
                  painter->setBrush(Qt::NoBrush);
                  pen.setDashPattern(dashed);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
                  break;
            case 3:
                  painter->setBrush(Qt::NoBrush);
                  pen.setDashPattern(wideDashed);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth) * mag);
                  break;
            }
      painter->setPen(pen);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   searchCR
//---------------------------------------------------------

static ChordRest* searchCR(Segment* segment, int startTrack, int endTrack)
      {
      // for (Segment* s = segment; s; s = s->next1MM(SegmentType::ChordRest)) {
      for (Segment* s = segment; s; s = s->next(SegmentType::ChordRest)) {     // restrict search to measure
            if (startTrack > endTrack) {
                  for (int t = startTrack-1; t >= endTrack; --t) {
                        if (s->element(t))
                              return toChordRest(s->element(t));
                        }
                  }
            else {
                  for (int t = startTrack; t < endTrack; ++t) {
                        if (s->element(t))
                              return toChordRest(s->element(t));
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(EditData& ed)
      {
      Slur* sl = slur();

      if (ed.key == Qt::Key_X) {
            sl->undoChangeProperty(Pid::SLUR_DIRECTION, QVariant::fromValue<Direction>(sl->up() ? Direction::DOWN : Direction::UP));
            sl->layout();
            return true;
            }
      if (ed.key == Qt::Key_Home) {
            ups(ed.curGrip).off = QPointF();          //TODO
            sl->layout();
            return true;
            }

      if (!((ed.modifiers & Qt::ShiftModifier) && (isSingleType() || (isBeginType() && ed.curGrip == Grip::START) || (isEndType() && ed.curGrip == Grip::END))))
            return false;

      ChordRest* cr = 0;
      ChordRest* e;
      ChordRest* e1;
      if (ed.curGrip == Grip::START) {
            e  = sl->startCR();
            e1 = sl->endCR();
            }
      else {
            e  = sl->endCR();
            e1 = sl->startCR();
            }

      if (ed.key == Qt::Key_Left)
            cr = prevChordRest(e);
      else if (ed.key == Qt::Key_Right)
            cr = nextChordRest(e);
      else if (ed.key == Qt::Key_Up) {
            Part* part     = e->part();
            int startTrack = part->startTrack();
            int endTrack   = e->track();
            cr = searchCR(e->segment(), endTrack, startTrack);
            }
      else if (ed.key == Qt::Key_Down) {
            int startTrack = e->track() + 1;
            Part* part     = e->part();
            int endTrack   = part->endTrack();
            cr = searchCR(e->segment(), startTrack, endTrack);
            }
      if (cr && cr != e1)
            changeAnchor(ed, cr);
      return true;
      }

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(EditData& ed, Element* element)
      {
      ChordRest* cr = element->isChordRest() ? toChordRest(element) : nullptr;
      ChordRest* scr = spanner()->startCR();
      ChordRest* ecr = spanner()->endCR();
      if (!cr || !scr || !ecr)
            return;

      // save current start/end elements
      for (ScoreElement* e : spanner()->linkList()) {
            Spanner* sp = toSpanner(e);
            score()->undoStack()->push1(new ChangeStartEndSpanner(sp, sp->startElement(), sp->endElement()));
            }

      if (ed.curGrip == Grip::START) {
            spanner()->undoChangeProperty(Pid::SPANNER_TICK, cr->tick());
            Fraction ticks = ecr->tick() - cr->tick();
            spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
            int diff = cr->track() - spanner()->track();
            for (auto e : spanner()->linkList()) {
                  Spanner* s = toSpanner(e);
                  s->undoChangeProperty(Pid::TRACK, s->track() + diff);
                  }
            scr = cr;
            }
      else {
            Fraction ticks = cr->tick() - scr->tick();
            spanner()->undoChangeProperty(Pid::SPANNER_TICKS, ticks);
            int diff = cr->track() - spanner()->track();
            for (auto e : spanner()->linkList()) {
                  Spanner* s = toSpanner(e);
                  s->undoChangeProperty(Pid::SPANNER_TRACK2, s->track() + diff);
                  }
            ecr = cr;
            }

      // update start/end elements (which could be grace notes)
      for (ScoreElement* lsp : spanner()->linkList()) {
            Spanner* sp = static_cast<Spanner*>(lsp);
            if (sp == spanner()) {
                  score()->undo(new ChangeSpannerElements(sp, scr, ecr));
                  }
            else {
                  Element* se = 0;
                  Element* ee = 0;
                  if (scr) {
                        QList<ScoreElement*> sel = scr->linkList();
                        for (ScoreElement* lcr : qAsConst(sel)) {
                              Element* le = toElement(lcr);
                              if (le->score() == sp->score() && le->track() == sp->track()) {
                                    se = le;
                                    break;
                                    }
                              }
                        }
                  if (ecr) {
                        QList<ScoreElement*> sel = ecr->linkList();
                        for (ScoreElement* lcr : qAsConst(sel)) {
                              Element* le = toElement(lcr);
                              if (le->score() == sp->score() && le->track() == sp->track2()) {
                                    ee = le;
                                    break;
                                    }
                              }
                        }
                  score()->undo(new ChangeStartEndSpanner(sp, se, ee));
                  sp->layout();
                  }
            }

      const size_t segments  = spanner()->spannerSegments().size();
      ups(ed.curGrip).off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();
            SlurSegment* newSegment = toSlurSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
            ed.view->startEdit(newSegment, ed.curGrip);
            triggerLayout();
            }
      }

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void SlurSegment::computeBezier(QPointF p6o)
      {
      qreal _spatium  = spatium();
      qreal shoulderW;              // height as fraction of slur-length
      qreal shoulderH;
      //
      // pp1 and pp2 are the end points of the slur
      //
      QPointF pp1 = ups(Grip::START).p + ups(Grip::START).off;
      QPointF pp2 = ups(Grip::END).p   + ups(Grip::END).off;

      QPointF p2 = pp2 - pp1;
      if (qFuzzyIsNull(p2.x()) && qFuzzyIsNull(p2.y())) {
            Measure* m1 = slur()->startCR()->segment()->measure();
            Measure* m2 = slur()->endCR()->segment()->measure();
            qDebug("zero slur at tick %d(%d) track %d in measure %d-%d  tick %d ticks %d",
               m1->tick().ticks(), tick().ticks(), track(), m1->no(), m2->no(), slur()->tick().ticks(), slur()->ticks().ticks());
            slur()->setBroken(true);
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
            shoulderH = (dd + smallH) * _spatium + _extraHeight;
            if (d > 18.0)
                  shoulderW = 0.7; // 0.8;
            else if (d > 10)
                  shoulderW = 0.6; // 0.7;
            else
                  shoulderW = 0.5; // 0.6;
            }

      shoulderH -= p6o.y();

      if (!slur()->up())
            shoulderH = -shoulderH;

      qreal c    = p2.x();
      qreal c1   = (c - c * shoulderW) * .5 + p6o.x();
      qreal c2   = c1 + c * shoulderW       + p6o.x();

      QPointF p5 = QPointF(c * .5, 0.0);

      QPointF p3(c1, -shoulderH);
      QPointF p4(c2, -shoulderH);

      qreal w = score()->styleP(Sid::SlurMidWidth) - score()->styleP(Sid::SlurEndWidth);
      if (staff())
            w *= staff()->mag(slur()->tick());
      if ((c2 - c1) <= _spatium)
            w *= .5;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ups(Grip::BEZIER1).off);
      QPointF p4o = p6o + t.map(ups(Grip::BEZIER2).off);

      if (!p6o.isNull()) {
            QPointF p6i = t.inverted().map(p6o);
            ups(Grip::BEZIER1).off += p6i ;
            ups(Grip::BEZIER2).off += p6i;
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

      path = QPainterPath();
      path.moveTo(QPointF());
      path.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
      if (slur()->lineType() == 0)
            path.cubicTo(p4 +p4o + th, p3 + p3o + th, QPointF());

      th = QPointF(0.0, 3.0 * w);
      shapePath = QPainterPath();
      shapePath.moveTo(QPointF());
      shapePath.cubicTo(p3 + p3o - th, p4 + p4o - th, p2);
      shapePath.cubicTo(p4 +p4o + th, p3 + p3o + th, QPointF());

      // translate back
      t.reset();
      t.translate(pp1.x(), pp1.y());
      t.rotateRadians(sinb);
      path                  = t.map(path);
      shapePath             = t.map(shapePath);
      ups(Grip::BEZIER1).p  = t.map(p3);
      ups(Grip::BEZIER2).p  = t.map(p4);
      ups(Grip::END).p      = t.map(p2) - ups(Grip::END).off;
      ups(Grip::DRAG).p     = t.map(p5);
      ups(Grip::SHOULDER).p = t.map(p6);

      _shape.clear();
      QPointF start = pp1;
      int nbShapes  = 32;  // (pp2.x() - pp1.x()) / _spatium;
      qreal minH    = qAbs(3 * w);
      const CubicBezier b(pp1, ups(Grip::BEZIER1).pos(), ups(Grip::BEZIER2).pos(), ups(Grip::END).pos());
      for (int i = 1; i <= nbShapes; i++) {
            const QPointF point = b.pointAtPercent(i/float(nbShapes));
            QRectF re     = QRectF(start, point).normalized();
            if (re.height() < minH) {
                  qreal d1 = (minH - re.height()) * .5;
                  re.adjust(0.0, -d1, 0.0, d1);
                  }
            _shape.add(re);
            start = point;
            }
      }

//---------------------------------------------------------
//   layoutSegment
//---------------------------------------------------------

void SlurSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      setPos(QPointF());
      ups(Grip::START).p = p1;
      ups(Grip::END).p   = p2;
      _extraHeight = 0.0;

      //Adjust Y pos to staff type yOffset before other calculations
      if (staffType())
            rypos() += staffType()->yoffset().val() * spatium();

      computeBezier();

      if (autoplace() && system()) {
            bool up = slur()->up();
            Segment* ls = system()->lastMeasure()->last();
            Segment* fs = system()->firstMeasure()->first();
            Segment* ss = slur()->startSegment();
            Segment* es = slur()->endSegment();
            QPointF pp1 = ups(Grip::START).p;
            QPointF pp2 = ups(Grip::END).p;
            qreal slurMaxMove = spatium();
            bool intersection = false;
            qreal gdist = 0.0;
            qreal minDistance = score()->styleS(Sid::SlurMinDistance).val() * spatium();
            for (int tries = 1; true; ++tries) {
                  for (Segment* s = fs; s && s != ls; s = s->next1()) {
                        if (!s->enabled())
                              continue;
                        // skip start and end segments on assumption start and end points were placed well already
                        // this avoids overcorrection on collision with own ledger lines and accidentals
                        // it also avoids issues where slur appears to be attached to a note in a different voice
                        if (s == ss || s == es)
                              continue;
                        // allow slurs to cross barlines
                        if (s->segmentType() & SegmentType::BarLineType)
                              continue;
                        qreal x1 = s->x() + s->measure()->x();
                        qreal x2 = x1 + s->width();
                        if (pp1.x() > x2)
                              continue;
                        if (pp2.x() < x1)
                              break;
                        const Shape& segShape = s->staffShape(staffIdx()).translated(s->pos() + s->measure()->pos());
                        if (!intersection)
                              intersection = segShape.intersects(_shape);
                        if (up) {
                              //QPointF pt = QPointF(s->x() + s->measure()->x(), s->staffShape(staffIdx()).top() + s->y() + s->measure()->y());
                              qreal dist = _shape.minVerticalDistance(segShape);
                              if (dist > 0.0)
                                    gdist = qMax(gdist, dist);
                              }
                        else {
                              //QPointF pt = QPointF(s->x() + s->measure()->x(), s->staffShape(staffIdx()).bottom() + s->y() + s->measure()->y());
                              qreal dist = segShape.minVerticalDistance(_shape);
                              if (dist > 0.0)
                                    gdist = qMax(gdist, dist);
                              }
                        }
                  if (!intersection || gdist <= slurMaxMove || tries >= 2)
                        break;
                  // slur would be moved too far
                  // try again with a steeper curve
                  _extraHeight += gdist;
                  computeBezier();
                  intersection = false;
                  gdist = 0.0;
                  }
            if (intersection && gdist > 0.0) {
                  qreal min = minDistance + gdist;
                  rypos() += up ? -min : min;
                  }
            }
      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool SlurSegment::isEdited() const
      {
      for (int i = 0; i < int(Grip::GRIPS); ++i) {
            if (!_ups[i].off.isNull())
                  return true;
            }
      return false;
      }

Slur::Slur(const Slur& s)
   : SlurTie(s)
      {
      _sourceStemArrangement = s._sourceStemArrangement;
      }

//---------------------------------------------------------
//   fixArticulations
//---------------------------------------------------------

static qreal fixArticulations(qreal yo, Chord* c, qreal _up, bool stemSide = false)
      {
      //
      // handle special case of tenuto and staccato
      // yo = current offset of slur from chord position
      // return unchanged position, or position of outmost "close" articulation
      //
#if 1
      for (Articulation* a : c->articulations()) {
            if (!a->layoutCloseToNote() || !a->addToSkyline())
                  continue;
            // skip if articulation on stem side but slur is not or vice versa
            if ((a->up() == c->up()) != stemSide)
                  continue;
            if (a->up())
                  yo = qMin(yo, a->y() + (a->height() + c->score()->spatium() * .3) * _up);
            else
                  yo = qMax(yo, a->y() + (a->height() + c->score()->spatium() * .3) * _up);
            }
      return yo;
#else
      const QVector<Articulation*>& al = c->articulations();
      if (al.size() >= 2) {
            Articulation* a = al.at(1);
            if (a->up() == c->up() && !stemSide)
                  return yo;
            else if (a->layoutCloseToNote())
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      else if (al.size() >= 1) {
            Articulation* a = al.at(0);
            if (a->up() == c->up() && !stemSide)
                  return yo;
            else if (a->layoutCloseToNote())
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      return yo;
#endif
      }

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Slur::slurPosChord(SlurPos* sp)
      {
      Chord* stChord;
      Chord* enChord ;
      if (startChord()->isGraceAfter()){     // grace notes after, coming in reverse order
            stChord = endChord();
            enChord = startChord();
            _up = false;
            }
      else {
            stChord = startChord();
            enChord = endChord();
            }
      Note* _startNote = stChord->downNote();
      Note* _endNote   = enChord->downNote();
      qreal hw         = _startNote->bboxRightPos();
      qreal __up       = _up ? -1.0 : 1.0;
      qreal _spatium = spatium();

      Measure* measure = endChord()->measure();
      sp->system1 = measure->system();
      if (!sp->system1) {             // DEBUG
            qDebug("no system1");
            return;
            }
      Q_ASSERT(sp->system1);
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
      sp->p1 = stChord->pagePos() - pp + QPointF(xo, yo);

      //------p2
      if ((enChord->notes().size() > 1) || (enChord->stem() && !enChord->up() && !_up)) {
            xo = _endNote->x() - hw * 0.12;
            yo = _endNote->pos().y() + hw * .3 * __up;
            }
      else {
            xo = _endNote->x() + hw * 0.15;
            yo = _endNote->pos().y() + _spatium * .75 * __up;
            }
      sp->p2 = enChord->pagePos() - pp + QPointF(xo, yo);
      }

//---------------------------------------------------------
//   slurPos
//    calculate position of start- and endpoint of slur
//    relative to System() position
//---------------------------------------------------------

void Slur::slurPos(SlurPos* sp)
      {
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

      bool useTablature  = staff() && staff()->isTabStaff(endCR()->tick());
      bool staffHasStems = true;     // assume staff uses stems
      const StaffType* stt = 0;
      if (useTablature) {
            stt           = staff()->staffType(tick());
            staffHasStems = stt->stemThrough();   // if tab with stems beside, stems do not count for slur pos
            }

      // start and end cr, chord, and note
      ChordRest* scr = startCR();
      ChordRest* ecr = endCR();
      Chord* sc      = 0;
      Note* note1    = 0;
      if (scr->isChord()) {
            sc    = toChord(scr);
            note1 = _up ? sc->upNote() : sc->downNote();
            }
      Chord* ec = 0;
      Note* note2 = 0;
      if (ecr->isChord()) {
            ec   = toChord(ecr);
            note2 = _up ? ec->upNote() : ec->downNote();
            }

      sp->system1 = scr->measure()->system();
      sp->system2 = ecr->measure()->system();

      if (sp->system1 == 0) {
            qDebug("no system1");
            return;
            }

      sp->p1 = scr->pos() + scr->segment()->pos() + scr->measure()->pos();
      sp->p2 = ecr->pos() + ecr->segment()->pos() + ecr->measure()->pos();

      // adjust for cross-staff
      if (scr->vStaffIdx() != vStaffIdx() && sp->system1) {
            qreal diff = sp->system1->staff(scr->vStaffIdx())->y() - sp->system1->staff(vStaffIdx())->y();
            sp->p1.ry() += diff;
            }
      if (ecr->vStaffIdx() != vStaffIdx() && sp->system2) {
            qreal diff = sp->system2->staff(ecr->vStaffIdx())->y() - sp->system2->staff(vStaffIdx())->y();
            sp->p2.ry() += diff;
            }

      // account for centering or other adjustments (other than mirroring)
      if (note1 && !note1->mirror())
            sp->p1.rx() += note1->x();
      if (note2 && !note2->mirror())
            sp->p2.rx() += note2->x();

      qreal xo, yo;

      Stem* stem1 = sc && staffHasStems ? sc->stem() : 0;
      Stem* stem2 = ec && staffHasStems ? ec->stem() : 0;

      enum class SlurAnchor : char {
            NONE, STEM
            };
      SlurAnchor sa1 = SlurAnchor::NONE;
      SlurAnchor sa2 = SlurAnchor::NONE;
      // if slur is 'embedded' between either stem or both (as it might happen with voices)
      // link corresponding slur end to stem position
      if ((scr->up() == ecr->up()) && !scr->beam() && !ecr->beam() && (_up == scr->up())) {
            // both chords are facing same direction and slur is also in same direction
            // and no beams
            if (stem1)
                  sa1 = SlurAnchor::STEM;
            if (stem2)
                  sa2 = SlurAnchor::STEM;
            }
      // also link start of slur to stem if start chord & slur are in same direction and there is a hook
      if (scr->up() == _up && stem1 && sc->hook()) {
            sa1 = SlurAnchor::STEM;
            // if end chord is in same direction, link end of slur to stem too
            if (ecr->up() == scr->up() && stem2 && (!ecr->beam() || !ecr->beam()->cross()))
                  sa2 = SlurAnchor::STEM;
            }

      qreal __up = _up ? -1.0 : 1.0;
      qreal hw1 = note1 ? note1->tabHeadWidth(stt) : scr->width();      // if stt == 0, tabHeadWidth()
      qreal hw2 = note2 ? note2->tabHeadWidth(stt) : ecr->width();      // defaults to headWidth()
      QPointF pt;
      switch (sa1) {
            case SlurAnchor::STEM:        //sc can't be null
                  {
                  // place slur starting point at stem end point
                  pt = sc->stemPos() - sc->pagePos() + sc->stem()->p2();
                  if (useTablature)                   // in tabs, stems are centred on note:
                        pt.rx() = hw1 * 0.5 + (note1 ? note1->bboxXShift() : 0.0);          // skip half notehead to touch stem, anatoly-os: incorrect. half notehead width is not always the stem position
                  // clear the stem (x)
                  // allow slight overlap (y) as per Gould
                  // don't allow overlap with hook if not disabling the autoplace checks against start/end segments in SlurSegment::layoutSegment()
                  qreal yadj = -0.25;     // sc->hook() ? 0.25 : -0.25;
                  yadj *= _spatium * __up;
                  pt += QPointF(0.35 * _spatium, yadj);
                  // account for articulations
                  pt.ry() = fixArticulations(pt.y(), sc, __up, true);
                  sp->p1 += pt;
                  }
                  break;
            case SlurAnchor::NONE:
                  break;
            }
      switch (sa2) {
            case SlurAnchor::STEM:        //ec can't be null
                  {
                  pt = ec->stemPos() - ec->pagePos() + ec->stem()->p2();
                  if (useTablature)
                        pt.rx() = hw2 * 0.5;
                  // don't allow overlap with beam
                  qreal yadj = ec->beam() ? 0.75 : -0.25;
                  yadj *= _spatium * __up;
                  pt += QPointF(-0.35 * _spatium, yadj);
                  // account for articulations
                  pt.ry() = fixArticulations(pt.y(), ec, __up, true);
                  sp->p2 += pt;
                  }
                  break;
            case SlurAnchor::NONE:
                  break;
            }

      //
      // default position:
      //    horizontal: middle of notehead
      //    vertical:   _spatium * .4 above/below notehead
      //
      //------p1
      // Compute x0, y0 and stemPos
      if (sa1 == SlurAnchor::NONE || sa2 == SlurAnchor::NONE) { // need stemPos if sa2 == SlurAnchor::NONE
            bool stemPos = false;   // p1 starts at chord stem side

            // default positions
            xo = hw1 * .5 + (note1 ? note1->bboxXShift() : 0.0);
            if (note1)
                  yo = note1->pos().y();
            else if (_up)
                  yo = scr->bbox().top();
            else
                  yo = scr->bbox().top() + scr->height();
            yo += _spatium * .9 * __up;

            // adjustments for stem and/or beam

            if (stem1) { //sc not null
                  Beam* beam1 = sc->beam();
                  if (beam1 && beam1->cross()) {
                        // TODO: stem direction is not finalized, so we cannot use it here
                        yo = fixArticulations(yo, sc, __up, false);
                        }
                  else if (beam1 && (beam1->elements().back() != sc) && (sc->up() == _up)) {
                        // start chord is beamed but not the last chord of beam group
                        // and slur direction is same as start chord (stem side)

                        // in these cases, layout start of slur to stem

                        qreal sh = stem1->height() + _spatium;
                        if (_up)
                              yo = sc->downNote()->pos().y() - sh;
                        else
                              yo = sc->upNote()->pos().y() + sh;
                        xo       = stem1->pos().x();

                        // account for articulations
                        yo = fixArticulations(yo, sc, __up, true);

                        // force end of slur to layout to stem as well,
                        // if start and end chords have same stem direction
                        stemPos = true;
                        }
                  else {
                        // start chord is not beamed or is last chord of beam group
                        // or slur direction is opposite that of start chord

                        // at this point slur is in default position relative to note on slur side
                        // but we may need to make further adjustments

                        // if stem and slur are both up
                        // we need to clear stem horizontally
                        if (sc->up() && _up)
                              xo = hw1 + _spatium * .3;

                        //
                        // handle case: stem up   - stem down
                        //              stem down - stem up
                        //
                        if ((sc->up() != ecr->up()) && (sc->up() == _up)) {
                              // start and end chord have opposite direction
                              // and slur direction is same as start chord
                              // (so slur starts on stem side)

                              // float the start point along the stem to follow direction of movement
                              // see for example Gould p. 111

                              // get position of note on slur side for start & end chords
                              Note* n1  = sc->up() ? sc->upNote() : sc->downNote();
                              Note* n2  = 0;
                              if (ec)
                                    n2 = ec->up() ? ec->upNote() : ec->downNote();

                              // differential in note positions
                              qreal yd  = (n2 ? n2->pos().y() : ecr->pos().y()) - n1->pos().y();
                              yd *= .5;

                              // float along stem according to differential
                              qreal sh = stem1->height();
                              if (_up && yd < 0.0)
                                    yo = qMax(yo + yd, sc->downNote()->pos().y() - sh - _spatium);
                              else if (!_up && yd > 0.0)
                                    yo = qMin(yo + yd, sc->upNote()->pos().y() + sh + _spatium);

                              // account for articulations
                              yo = fixArticulations(yo, sc, __up, true);

                              // we may wish to force end to align to stem as well,
                              // if it is in same direction
                              // (but it won't be, so this assignment should have no effect)
                              stemPos = true;
                              }
                        else {
                              // avoid articulations
                              yo = fixArticulations(yo, sc, __up, sc->up() == _up);
                              }
                        }
                  }
            else if (sc) {
                  // avoid articulations
                  yo = fixArticulations(yo, sc, __up, sc->up() == _up);
                  }

            if (sa1 == SlurAnchor::NONE)
                  sp->p1 += QPointF(xo, yo);

            //------p2
            if (sa2 == SlurAnchor::NONE) {

                  // default positions
                  xo = hw2 * .5 + (note2 ? note2->bboxXShift() : 0.0);
                  if (note2)
                        yo = note2->pos().y();
                  else if (_up)
                        yo = endCR()->bbox().top();
                  else
                        yo = endCR()->bbox().top() + endCR()->height();
                  yo += _spatium * .9 * __up;

                  // adjustments for stem and/or beam

                  if (stem2) { //ec can't be null
                        Beam* beam2 = ec->beam();
                        if (beam2 && beam2->cross()) {
                              // TODO: stem direction is not finalized, so we cannot use it here
                              yo = fixArticulations(yo, ec, __up, false);
                              }
                        else if ((stemPos && (scr->up() == ec->up()))
                           || (beam2
                             && (!beam2->elements().empty())
                             && (beam2->elements().front() != ec)
                             && (ec->up() == _up)
                             && sc && (sc->noteType() == NoteType::NORMAL)
                             )
                              ) {

                              // slur start was laid out to stem and start and end have same direction
                              // OR
                              // end chord is beamed but not the first chord of beam group
                              // and slur direction is same as end chord (stem side)
                              // and start chordrest is not a grace chord

                              // in these cases, layout end of slur to stem

                              qreal sh = stem2->height() + _spatium;
                              if (_up)
                                    yo = ec->downNote()->pos().y() - sh;
                              else
                                    yo = ec->upNote()->pos().y() + sh;
                              xo = stem2->pos().x();

                              // account for articulations
                              yo = fixArticulations(yo, ec, __up, true);
                              }
                        else
                              {
                              // slur was not aligned to stem or start and end have different direction
                              // AND
                              // end chord is not beamed or is first chord of beam group
                              // or slur direction is opposite that of end chord

                              // if stem and slur are both down,
                              // we need to clear stem horizontally
                              if (!ec->up() && !_up)
                                    xo = -_spatium * .3 + note2->x();

                              //
                              // handle case: stem up   - stem down
                              //              stem down - stem up
                              //
                              if ((scr->up() != ec->up()) && (ec->up() == _up)) {
                                    // start and end chord have opposite direction
                                    // and slur direction is same as end chord
                                    // (so slur end on stem side)

                                    // float the end point along the stem to follow direction of movement
                                    // see for example Gould p. 111

                                    Note* n1 = 0;
                                    if (sc)
                                          n1 = sc->up() ? sc->upNote() : sc->downNote();
                                    Note* n2 = ec->up() ? ec->upNote() : ec->downNote();

                                    qreal yd = n2->pos().y() - (n1 ? n1->pos().y() : startCR()->pos().y());
                                    yd *= .5;

                                    qreal mh = stem2->height();
                                    if (_up && yd > 0.0)
                                          yo = qMax(yo - yd, ec->downNote()->pos().y() - mh - _spatium);
                                    else if (!_up && yd < 0.0)
                                          yo = qMin(yo - yd, ec->upNote()->pos().y() + mh + _spatium);

                                    // account for articulations
                                    yo = fixArticulations(yo, ec, __up, true);
                                    }
                              else {
                                    // avoid articulations
                                    yo = fixArticulations(yo, ec, __up, ec->up() == _up);
                                    }

                              }
                        }
                  else if (ec) {
                        // avoid articulations
                        yo = fixArticulations(yo, ec, __up, ec->up() == _up);
                        }

                  sp->p2 += QPointF(xo, yo);
                  }
            }
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(Score* s)
   : SlurTie(s)
      {
      setAnchor(Anchor::CHORD);
      }

//---------------------------------------------------------
//   calcStemArrangement
//---------------------------------------------------------

int calcStemArrangement(Element* start, Element* end)
      {
      return (start && toChord(start)->stem() && toChord(start)->stem()->up() ? 2 : 0)
           + (end && end->isChord() && toChord(end)->stem() && toChord(end)->stem()->up() ? 4 : 0);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slur::write(XmlWriter& xml) const
      {
      if (broken()) {
            qDebug("broken slur not written");
            return;
            }
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      if (xml.clipboardmode())
            xml.tag("stemArr", calcStemArrangement(startElement(), endElement()));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Slur::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "stemArr") {
            _sourceStemArrangement = e.readInt();
            return true;
            }
      return SlurTie::readProperties(e);
      }

//---------------------------------------------------------
//   chordsHaveTie
//---------------------------------------------------------

static bool chordsHaveTie(Chord* c1, Chord* c2)
      {
      size_t n = c1->notes().size();
      for (size_t i1 = 0; i1 < n; ++i1) {
            Note* n1 = c1->notes().at(i1);
            size_t n2 = c2->notes().size();
            for (size_t i2 = 0; i2 < n2; ++i2) {
                  Note* n3 = c2->notes().at(i2);
                  if (n1->tieFor() && n1->tieFor() == n3->tieBack())
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
      for (Segment* seg = c1->segment(); seg; seg = seg->next(SegmentType::ChordRest)) {
            Element* e = seg->element(c1->track());
            if (!e || !e->isChord())
                  continue;
            Chord* c = toChord(e);
            if (c->up() != up)
                  return true;
            if (seg == c2->segment())
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   layoutSystem
//    layout slurSegment for system
//---------------------------------------------------------

SpannerSegment* Slur::layoutSystem(System* system)
      {
      Fraction stick = system->firstMeasure()->tick();
      Fraction etick = system->lastMeasure()->endTick();

      SlurSegment* slurSegment = toSlurSegment(getNextLayoutSystemSegment(system, [this]() { return new SlurSegment(score()); }));

      SpannerSegmentType sst;
      if (tick() >= stick) {
            //
            // this is the first call to layoutSystem,
            // processing the first line segment
            //
            if (track2() == -1)
                  setTrack2(track());
            if (startCR() == 0 || startCR()->measure() == 0) {
                  qDebug("Slur::layout(): track %d-%d  %p - %p tick %d-%d null start anchor",
                     track(), track2(), startCR(), endCR(), tick().ticks(), tick2().ticks());
                  return slurSegment;
                  }
            if (endCR() == 0) {     // sanity check
                  setEndElement(startCR());
                  setTick2(tick());
                  }
            switch (_slurDirection) {
                  case Direction::UP:
                        _up = true;
                        break;
                  case Direction::DOWN:
                        _up = false;
                        break;
                  case Direction::AUTO:
                        {
                        //
                        // assumption:
                        // slurs have only chords or rests as start/end elements
                        //
                        if (startCR() == 0 || endCR() == 0) {
                              _up = true;
                              break;
                              }
                        Chord* c1 = startCR()->isChord() ? toChord(startCR()) : 0;
                        Chord* c2 = endCR()->isChord()   ? toChord(endCR())   : 0;

                        if (_sourceStemArrangement != -1) {
                              if (_sourceStemArrangement != calcStemArrangement(c1, c2)) {
                                    // copy & paste from incompatible stem arrangement, so reset bezier points
                                    for (int g = 0; g < (int)Ms::Grip::GRIPS; ++g) {
                                          slurSegment->ups((Ms::Grip)g) = UP();
                                          }
                                    }
                              }

                        if (c1 && c1->beam() && c1->beam()->cross()) {
                              // TODO: stem direction is not finalized, so we cannot use it here
                              _up = true;
                              break;
                              }

                        _up = !(startCR()->up());

                        Measure* m1 = startCR()->measure();
#if 0
                        // the following code was in place until 3.6,
                        // to force "long" slurs (duration > one measure) above
                        // but it's much too aggressive - one measure isn't necessarily long
                        if ((endCR()->tick() - startCR()->tick()) > m1->ticks()) // long slurs are always above
                              _up = true;
#endif
                        if (c1 && c2 && isDirectionMixture(c1, c2) && !c1->isGrace()) {
                              // slurs go above if start and end note have different stem directions,
                              // but grace notes are exceptions
                              _up = true;
                              }
                        else if (m1->hasVoices(startCR()->staffIdx(), tick(), ticks()) && c1 && !c1->isGrace()) {
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
            sst = tick2() < etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
            }
      else if (tick() < stick && tick2() >= etick)
            sst = SpannerSegmentType::MIDDLE;
      else
            sst = SpannerSegmentType::END;
      slurSegment->setSpannerSegmentType(sst);

      SlurPos sPos;
      slurPos(&sPos);

      switch (sst) {
            case SpannerSegmentType::SINGLE:
                  slurSegment->layoutSegment(sPos.p1, sPos.p2);
                  break;
            case SpannerSegmentType::BEGIN:
                  slurSegment->layoutSegment(sPos.p1, QPointF(system->lastNoteRestSegmentX(true), sPos.p1.y()));
                  break;
            case SpannerSegmentType::MIDDLE: {
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal x2 = system->lastNoteRestSegmentX(true);
                  qreal y  = staffIdx() > system->staves()->size() ? system->y() : system->staff(staffIdx())->y();
                  slurSegment->layoutSegment(QPointF(x1, y), QPointF(x2, y));
                  }
                  break;
            case SpannerSegmentType::END:
                  slurSegment->layoutSegment(QPointF(system->firstNoteRestSegmentX(true), sPos.p2.y()), sPos.p2);
                  break;
            }

      return slurSegment;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout()
      {
      if (track2() == -1)
            setTrack2(track());

      qreal _spatium = spatium();

      if (score() == gscore || tick() == Fraction(-1,1)) {
            //
            // when used in a palette, slur has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            SlurSegment* s;
            if (spannerSegments().empty()) {
                  s = new SlurSegment(score());
                  s->setTrack(track());
                  add(s);
                  }
            else {
                  s = frontSegment();
                  }
            s->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            s->layoutSegment(QPointF(0, 0), QPointF(_spatium * 6, 0));
            setbbox(frontSegment()->bbox());
            return;
            }

      if (startCR() == 0 || startCR()->measure() == 0) {
            qDebug("track %d-%d  %p - %p tick %d-%d null start anchor",
               track(), track2(), startCR(), endCR(), tick().ticks(), tick2().ticks());
            return;
            }
      if (endCR() == 0) {     // sanity check
            qDebug("no end CR for %d", (tick()+ticks()).ticks());
            setEndElement(startCR());
            setTick2(tick());
            }
      switch (_slurDirection) {
            case Direction::UP:
                  _up = true;
                  break;
            case Direction::DOWN:
                  _up = false;
                  break;
            case Direction::AUTO:
                  {
                  //
                  // assumption:
                  // slurs have only chords or rests as start/end elements
                  //
                  if (startCR() == 0 || endCR() == 0) {
                        _up = true;
                        break;
                        }
                  Measure* m1 = startCR()->measure();

                  Chord* c1 = startCR()->isChord() ? toChord(startCR()) : 0;
                  Chord* c2 = endCR()->isChord()   ? toChord(endCR())   : 0;

                  _up = !(startCR()->up());

                  if ((endCR()->tick() - startCR()->tick()) > m1->ticks()) {
                        // long slurs are always above
                        _up = true;
                        }
                  else
                        _up = !(startCR()->up());

                  if (c1 && c2 && isDirectionMixture(c1, c2) && (c1->noteType() == NoteType::NORMAL)) {
                        // slurs go above if start and end note have different stem directions,
                        // but grace notes are exceptions
                        _up = true;
                        }
                  else if (m1->hasVoices(startCR()->staffIdx(), tick(), ticks()) && c1 && c1->noteType() == NoteType::NORMAL) {
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

      const QList<System*>& sl = score()->systems();
      ciSystem is = sl.begin();
      while (is != sl.end()) {
            if (*is == sPos.system1)
                  break;
            ++is;
            }
      if (is == sl.end())
            qDebug("Slur::layout  first system not found");
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      unsigned nsegs = 1;
      for (ciSystem iis = is; iis != sl.end(); ++iis) {
            if ((*iis)->vbox())
                  continue;
            if (*iis == sPos.system2)
                  break;
            ++nsegs;
            }

      fixupSegments(nsegs);

      for (int i = 0; is != sl.end(); ++i, ++is) {
            System* system  = *is;
            if (system->vbox()) {
                  --i;
                  continue;
                  }
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  segment->layoutSegment(sPos.p1, sPos.p2);
                  }
            // case 2: start segment
            else if (i == 0) {
                  segment->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  qreal x = system->bbox().width();
                  segment->layoutSegment(sPos.p1, QPointF(x, sPos.p1.y()));
                  }
            // case 3: middle segment
            else if (i != 0 && system != sPos.system2) {
                  segment->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
                  qreal x1 = system->firstNoteRestSegmentX(true);
                  qreal x2 = system->bbox().width();
                  qreal y  = staffIdx() > system->staves()->size() ? system->y() : system->staff(staffIdx())->y();
                  segment->layoutSegment(QPointF(x1, y), QPointF(x2, y));
                  }
            // case 4: end segment
            else {
                  segment->setSpannerSegmentType(SpannerSegmentType::END);
                  qreal x = system->firstNoteRestSegmentX(true);
                  segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
                  }
            if (system == sPos.system2)
                  break;
            }
      setbbox(spannerSegments().empty() ? QRectF() : frontSegment()->bbox());
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(int n)
      {
      Element::setTrack(n);
      for (SpannerSegment* ss : spannerSegments())
            ss->setTrack(n);
      }
}

