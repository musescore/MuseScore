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
#include "part.h"

namespace Ms {

Element* SlurTie::editEndElement;
Element* SlurTie::editStartElement;
QList<SlurOffsets> SlurTie::editUps;

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(Score* score)
   : SpannerSegment(score)
      {
      autoAdjustOffset = QPointF();
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : SpannerSegment(b)
      {
      for (int i = 0; i < int(GripSlurSegment::GRIPS); ++i)
            ups[i] = b.ups[i];
      path = b.path;
      autoAdjustOffset = QPointF();
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      Element::move(s);
      for (int k = 0; k < int(GripSlurSegment::GRIPS); ++k)
            ups[k].p += s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(QPainter* painter) const
      {
      // hide tie toward the second chord of a cross-measure value
      if ((slurTie()->type() == Element::Type::TIE)
         && (static_cast<Tie*>(slurTie())->endNote())
         && (static_cast<Tie*>(slurTie())->endNote()->chord()->crossMeasure() == CrossMeasure::SECOND))
            return;

      QPen pen(curColor());
      switch (slurTie()->lineType()) {
            case 0:
                  painter->setBrush(QBrush(pen.color()));
                  pen.setCapStyle(Qt::RoundCap);
                  pen.setJoinStyle(Qt::RoundJoin);
                  pen.setWidthF(point(score()->styleS(StyleIdx::SlurEndWidth)));
                  break;
            case 1:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(point(score()->styleS(StyleIdx::SlurDottedWidth)));
                  pen.setStyle(Qt::DotLine);
                  break;
            case 2:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(point(score()->styleS(StyleIdx::SlurDottedWidth)));
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

void SlurSegment::updateGrips(int* n, int* defaultGrip, QRectF* r) const
      {
      *n = int(GripSlurSegment::GRIPS);
      *defaultGrip = int(GripSlurSegment::END);
      QPointF p(pagePos());
      for (int i = 0; i < int(GripSlurSegment::GRIPS); ++i)
            r[i].translate(ups[i].p + ups[i].off * spatium() + p);
      }

//---------------------------------------------------------
//   searchCR
//---------------------------------------------------------

static ChordRest* searchCR(Segment* segment, int startTrack, int endTrack)
      {
      // for (Segment* s = segment; s; s = s->next1MM(Segment::Type::ChordRest)) {
      for (Segment* s = segment; s; s = s->next(Segment::Type::ChordRest)) {     // restrict search to measure
            if (startTrack > endTrack) {
                  for (int t = startTrack-1; t >= endTrack; --t) {
                        if (s->element(t))
                              return static_cast<ChordRest*>(s->element(t));
                        }
                  }
            else {
                  for (int t = startTrack; t < endTrack; ++t) {
                        if (s->element(t))
                              return static_cast<ChordRest*>(s->element(t));
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(MuseScoreView* viewer, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      Slur* sl = static_cast<Slur*>(slurTie());

      if (key == Qt::Key_X) {
            sl->setSlurDirection(sl->up() ? MScore::Direction::DOWN : MScore::Direction::UP);
            sl->layout();
            return true;
            }
      if (key == Qt::Key_Home) {
            ups[curGrip].off = QPointF();
            sl->layout();
            return true;
            }
      if (slurTie()->type() != Element::Type::SLUR)
            return false;

      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SpannerSegmentType::SINGLE)
              || (spannerSegmentType() == SpannerSegmentType::BEGIN && curGrip == int(GripSlurSegment::START))
              || (spannerSegmentType() == SpannerSegmentType::END && curGrip == int(GripSlurSegment::END))
            )))
            return false;

      ChordRest* cr = 0;
      ChordRest* e  = curGrip == int(GripSlurSegment::START) ? sl->startCR() : sl->endCR();
      ChordRest* e1 = curGrip == int(GripSlurSegment::START) ? sl->endCR() : sl->startCR();

      if (key == Qt::Key_Left)
            cr = prevChordRest(e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest(e);
      else if (key == Qt::Key_Up) {
            Part* part     = e->staff()->part();
            int startTrack = part->startTrack();
            int endTrack   = e->track();
            cr = searchCR(e->segment(), endTrack, startTrack);
            }
      else if (key == Qt::Key_Down) {
            int startTrack = e->track() + 1;
            Part* part     = e->staff()->part();
            int endTrack   = part->endTrack();
            cr = searchCR(e->segment(), startTrack, endTrack);
            }
      if (cr && cr != e1)
            changeAnchor(viewer, curGrip, cr);
      return true;
      }

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(MuseScoreView* viewer, int curGrip, Element* element)
      {
      if (curGrip == int(GripSlurSegment::START)) {
            spanner()->setStartElement(element);
            switch(spanner()->anchor()) {
                  case Spanner::Anchor::NOTE: {
                        Tie* tie = static_cast<Tie*>(spanner());
                        tie->startNote()->setTieFor(0);
                        tie->setStartNote(static_cast<Note*>(element));
                        static_cast<Note*>(element)->setTieFor(tie);
                        break;
                        }
                  case Spanner::Anchor::CHORD:
                        spanner()->setTick(static_cast<Chord*>(element)->tick());
                        spanner()->setStartChord(static_cast<Chord*>(element));
                        break;
                  case Spanner::Anchor::SEGMENT:
                  case Spanner::Anchor::MEASURE:
                        qDebug("SlurSegment::changeAnchor: bad anchor");
                        break;
                  }
            }
      else {
            spanner()->setEndElement(element);
            switch(spanner()->anchor()) {
                  case Spanner::Anchor::NOTE: {
                        Tie* tie = static_cast<Tie*>(spanner());
                        tie->endNote()->setTieBack(0);
                        tie->setEndNote(static_cast<Note*>(element));
                        static_cast<Note*>(element)->setTieBack(tie);
                        break;
                        }
                  case Spanner::Anchor::CHORD:
                        spanner()->setTick2(static_cast<Chord*>(element)->tick());
                        spanner()->setTrack2(element->track());
                        spanner()->setEndChord(static_cast<Chord*>(element));
                        break;

                  case Spanner::Anchor::SEGMENT:
                  case Spanner::Anchor::MEASURE:
                        qDebug("SlurSegment::changeAnchor: bad anchor");
                        break;
                  }
            }

      int segments  = spanner()->spannerSegments().size();
      ups[curGrip].off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            QList<SpannerSegment*>& ss = spanner()->spannerSegments();

            SlurSegment* newSegment = static_cast<SlurSegment*>(curGrip == int(GripSlurSegment::END) ? ss.back() : ss.front());
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
            case SpannerSegmentType::SINGLE:
                  if (grip == int(GripSlurSegment::START))
                        return p1;
                  else if (grip == int(GripSlurSegment::END))
                        return p2;
                  break;

            case SpannerSegmentType::BEGIN:
                  if (grip == int(GripSlurSegment::START))
                        return p1;
                  else if (grip == int(GripSlurSegment::END))
                        return system()->abbox().topRight();
                  break;

            case SpannerSegmentType::MIDDLE:
                  if (grip == int(GripSlurSegment::START))
                        return sp;
                  else if (grip == int(GripSlurSegment::END))
                        return system()->abbox().topRight();
                  break;

            case SpannerSegmentType::END:
                  if (grip == int(GripSlurSegment::START))
                        return sp;
                  else if (grip == int(GripSlurSegment::END))
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
      switch((GripSlurSegment)n) {
            case GripSlurSegment::START:
            case GripSlurSegment::END:
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
      switch((GripSlurSegment)n) {
            case GripSlurSegment::START:
            case GripSlurSegment::END:
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

      if (ed.curGrip == int(GripSlurSegment::START) || ed.curGrip == int(GripSlurSegment::END)) {
            slurTie()->computeBezier(this);
            //
            // move anchor for slurs/ties
            //
            SpannerSegmentType st = spannerSegmentType();
            if (
               (ed.curGrip == int(GripSlurSegment::START)  && (st == SpannerSegmentType::SINGLE || st == SpannerSegmentType::BEGIN))
               || (ed.curGrip == int(GripSlurSegment::END) && (st == SpannerSegmentType::SINGLE || st == SpannerSegmentType::END))
               ) {
                  Spanner* spanner = slurTie();
                  Qt::KeyboardModifiers km = qApp->keyboardModifiers();
                  Note* note = static_cast<Note*>(ed.view->elementNear(ed.pos));
                  if (note && note->type() == Element::Type::NOTE &&
                     ((ed.curGrip == int(GripSlurSegment::END) && note->chord()->tick() > slurTie()->tick())
                      || (ed.curGrip == int(GripSlurSegment::START) && note->chord()->tick() < slurTie()->tick2()))
                     ) {
                        if (ed.curGrip == int(GripSlurSegment::END) && spanner->type() == Element::Type::TIE) {
                              Tie* tie = static_cast<Tie*>(spanner);
                              if (tie->startNote()->pitch() == note->pitch()) {
                                    ed.view->setDropTarget(note);
                                    if (note != tie->endNote()) {
                                          changeAnchor(ed.view, ed.curGrip, note);
                                          return;
                                          }
                                    }
                              }
                        else if (spanner->type() != Element::Type::TIE && km != (Qt::ShiftModifier | Qt::ControlModifier)) {
                              Chord* c = note->chord();
                              ed.view->setDropTarget(note);
                              if (c != spanner->endCR()) {
                                    changeAnchor(ed.view, ed.curGrip, c);
                                    QPointF p1 = ed.pos - ups[ed.curGrip].p - canvasPos();
                                    ups[ed.curGrip].off = p1 / _spatium;
                                    slurTie()->layout();
                                    }
                              }
                        }
                  else
                        ed.view->setDropTarget(0);
                  }
            }
      else if (ed.curGrip == int(GripSlurSegment::BEZIER1) || ed.curGrip == int(GripSlurSegment::BEZIER2))
            slurTie()->computeBezier(this);
      else if (ed.curGrip == int(GripSlurSegment::SHOULDER)) {
            ups[ed.curGrip].off = QPointF();
            slurTie()->computeBezier(this, ed.delta);
            }
      else if (ed.curGrip == int(GripSlurSegment::DRAG)) {
            ups[int(GripSlurSegment::DRAG)].off = QPointF();
            setUserOff(userOff() + ed.delta);
            }

      // if this SlurSegment was automatically adjusted to avoid collision
      // lock this edit by resetting SlurSegment to default position
      // and incorporating previous adjustment into user offset
      QPointF offset = getAutoAdjust();
      if (!offset.isNull()) {
            setAutoAdjust(0.0, 0.0);
            setUserOff(userOff() + offset);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurSegment::write(Xml& xml, int no) const
      {
      if (ups[int(GripSlurSegment::START)].off.isNull()
         && ups[int(GripSlurSegment::END)].off.isNull()
         && ups[int(GripSlurSegment::BEZIER1)].off.isNull()
         && ups[int(GripSlurSegment::BEZIER2)].off.isNull()
         && userOff().isNull()
         && visible()
         && (color() == Qt::black)
            )
            return;

      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));

      writeProperty(xml, P_ID::SLUR_UOFF1);
      writeProperty(xml, P_ID::SLUR_UOFF2);
      writeProperty(xml, P_ID::SLUR_UOFF3);
      writeProperty(xml, P_ID::SLUR_UOFF4);
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
                  ups[int(GripSlurSegment::START)].off = e.readPoint();
            else if (tag == "o2")
                  ups[int(GripSlurSegment::BEZIER1)].off = e.readPoint();
            else if (tag == "o3")
                  ups[int(GripSlurSegment::BEZIER2)].off = e.readPoint();
            else if (tag == "o4")
                  ups[int(GripSlurSegment::END)].off = e.readPoint();
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
      QPointF pp1 = ss->ups[int(GripSlurSegment::START)].p + ss->ups[int(GripSlurSegment::START)].off * _spatium;
      QPointF pp2 = ss->ups[int(GripSlurSegment::END)].p   + ss->ups[int(GripSlurSegment::END)].off   * _spatium;

      QPointF p2 = pp2 - pp1;
      if ((p2.x() == 0.0) && (p2.y() == 0.0)) {
            Measure* m1 = startCR()->segment()->measure();
            Measure* m2 = endCR()->segment()->measure();
            qDebug("zero slur at tick %d(%d) track %d in measure %d-%d",
               m1->tick(), tick(), track(), m1->no(), m2->no());
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

      qreal w = (score()->styleS(StyleIdx::SlurMidWidth).val() - score()->styleS(StyleIdx::SlurEndWidth).val()) * _spatium;
      if (((c2 - c1) / _spatium) <= _spatium)
            w *= .5;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ss->ups[int(GripSlurSegment::BEZIER1)].off * _spatium);
      QPointF p4o = p6o + t.map(ss->ups[int(GripSlurSegment::BEZIER2)].off * _spatium);

      if (!p6o.isNull()) {
            QPointF p6i = t.inverted().map(p6o) / _spatium;
            ss->ups[int(GripSlurSegment::BEZIER1)].off += p6i ;
            ss->ups[int(GripSlurSegment::BEZIER2)].off += p6i;
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
      ss->ups[int(GripSlurSegment::BEZIER1)].p  = t.map(p3);
      ss->ups[int(GripSlurSegment::BEZIER2)].p  = t.map(p4);
      ss->ups[int(GripSlurSegment::END)].p      = t.map(p2) - ss->ups[int(GripSlurSegment::END)].off * _spatium;
      ss->ups[int(GripSlurSegment::DRAG)].p     = t.map(p5);
      ss->ups[int(GripSlurSegment::SHOULDER)].p = t.map(p6);
      }

//---------------------------------------------------------
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void SlurSegment::layout(const QPointF& p1, const QPointF& p2)
      {
      ups[int(GripSlurSegment::START)].p = p1;
      ups[int(GripSlurSegment::END)].p   = p2;
      slurTie()->computeBezier(this);
      QRectF bbox = path.boundingRect();

      // adjust position to avoid staff line if necessary
      qreal sp = spatium();
      qreal minDistance = 0.5;
      Staff* st = staff();
      autoAdjustOffset = QPointF();
      if (bbox.height() < minDistance * 2 * sp && st) {
            // slur/tie is fairly flat
            bool up = slurTie()->up();
            qreal staffY = system()->staff(staffIdx())->y();
            qreal ld = st->lineDistance() * sp;
            qreal topY = (bbox.top() - staffY) / ld;
            qreal bottomY = (bbox.bottom() - staffY) / ld;
            int lineY = up ? qRound(topY) : qRound(bottomY);
            if (lineY >= 0 && lineY < st->lines() * st->lineDistance()) {
                  // on staff
                  if (qAbs(topY - lineY) < minDistance && qAbs(bottomY - lineY) < minDistance) {
                        // too close to line
                        if (!isNudged() && !isEdited()) {
                              // user has not nudged or edited
                              qreal offY;
                              if (up)
                                    offY = (lineY - minDistance) - topY;
                              else
                                    offY = (lineY + minDistance) - bottomY;
                              setAutoAdjust(0.0, offY * sp);
                              bbox = path.boundingRect();
                              }
                        }
                  }
            }
      if (system() && staffIdx() != -1)
            setPos(QPointF(0.0, -system()->staff(staffIdx())->y()));
      setbbox(path.boundingRect());
      adjustReadPos();
      }

//---------------------------------------------------------
//   setAutoAdjust
//---------------------------------------------------------

void SlurSegment::setAutoAdjust(const QPointF& offset)
      {
      QPointF diff = offset - autoAdjustOffset;
      if (!diff.isNull()) {
            path.translate(diff);
            shapePath.translate(diff);
            for (int i = 0; i < int(GripSlurSegment::GRIPS); ++i)
                  ups[i].p += diff;
            autoAdjustOffset = offset;
            }
      }

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool SlurSegment::isEdited() const
      {
      for (int i = 0; i < int(GripSlurSegment::GRIPS); ++i) {
            if (!ups[i].off.isNull())
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Spanner(s)
      {
      _slurDirection = MScore::Direction::AUTO;
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
            if (a->articulationType() == ArticulationType::Tenuto || a->articulationType() == ArticulationType::Staccato)
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      else if (al.size() >= 1) {
            Articulation* a = al.at(0);
            if (a->articulationType() == ArticulationType::Tenuto || a->articulationType() == ArticulationType::Staccato)
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
      Chord* stChord;
      Chord* enChord ;
      if (startChord()->isGraceAfter()){     // grace notes after, coming in reverse order
            stChord = endChord();
            enChord = startChord();
            _up = false;
            }
      else{
            stChord = startChord();
            enChord = endChord();
            }
      Note* _startNote = stChord->downNote();
      Note* _endNote   = enChord->downNote();
      qreal hw         = _startNote->headWidth();
      qreal __up       = _up ? -1.0 : 1.0;
      qreal _spatium = spatium();

      Measure* measure = endChord()->measure();
      sp->system1 = measure->system();
      if (!sp->system1)             // DEBUG
            return;
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
//      if (anchor() == Anchor::CHORD) {
//            slurPosChord(sp);
//            return;
//            }
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

      ChordRest* scr = startCR();
      ChordRest* ecr = endCR();
      Chord* sc      = 0;
      Note* note1    = 0;
      if (startCR()->type() == Element::Type::CHORD) {
            sc = static_cast<Chord*>(startCR());
            note1 = _up ? sc->upNote() : sc->downNote();
            }
      Chord* ec = 0;
      Note* note2 = 0;
      if (endCR()->type() == Element::Type::CHORD) {
            ec   = static_cast<Chord*>(endCR());
            note2 = _up ? ec->upNote() : ec->downNote();
            }

      sp->system1 = scr->measure()->system();
      sp->system2 = ecr->measure()->system();
      if (sp->system1 == 0 || sp->system2 == 0)
            return;

      sp->p1 = scr->pagePos() - sp->system1->pagePos();
      sp->p2 = ecr->pagePos() - sp->system2->pagePos();
      // account for centering or other adjustments (other than mirroring)
      if (note1 && !note1->mirror())
            sp->p1.rx() += note1->x();
      if (note2 && !note2->mirror())
            sp->p2.rx() += note2->x();

      qreal xo, yo;

      Stem* stem1 = sc?sc->stem():0;
      Stem* stem2 = ec?ec->stem():0;

      enum class SlurAnchor : char {
            NONE, STEM
            };
      SlurAnchor sa1 = SlurAnchor::NONE;
      SlurAnchor sa2 = SlurAnchor::NONE;
      if ((scr->up() == ecr->up()) && !scr->beam() && !ecr->beam() && (_up == scr->up())) {
            if (stem1)
                  sa1 = SlurAnchor::STEM;
            if (stem2)
                  sa2 = SlurAnchor::STEM;
            }

      qreal __up = _up ? -1.0 : 1.0;
      switch (sa1) {
            case SlurAnchor::STEM: //sc can't be null
                  sp->p1 += sc->stemPos() - sc->pagePos() + sc->stem()->p2();
                  sp->p1 += QPointF(0.35 * _spatium, 0.25 * _spatium);
                  break;
            case SlurAnchor::NONE:
                  break;
            }
      switch(sa2) {
            case SlurAnchor::STEM: //ec can't be null
                  sp->p2 += ec->stemPos() - ec->pagePos() + ec->stem()->p2();
                  sp->p2 += QPointF(-0.35 * _spatium, 0.25 * _spatium);
                  break;
            case SlurAnchor::NONE:
                  break;
            }

      //
      // default position:
      //    horizontal: middle of note head
      //    vertical:   _spatium * .4 above/below note head
      //
      //------p1
      bool stemPos = false;   // p1 starts at chord stem side
      qreal hw = note1 ? note1->headWidth() : startCR()->width();
      xo = hw * .5;
      if (note1)
            yo = note1->pos().y();
      else if(_up)
            yo = startCR()->bbox().top();
      else
            yo = startCR()->bbox().top() + startCR()->height();
      yo += _spatium * .9 * __up;

      if (stem1) { //sc not null
            Beam* beam1 = sc->beam();
            if (beam1 && (beam1->elements().back() != sc) && (sc->up() == _up)) {
                  qreal sh = stem1->height() + _spatium;
                  if (_up)
                        yo = sc->downNote()->pos().y() - sh;
                  else
                        yo = sc->upNote()->pos().y() + sh;
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
      else if (sc && sc->up() != _up)
            yo = fixArticulations(yo, sc, __up);

      if (sa1 == SlurAnchor::NONE)
            sp->p1 += QPointF(xo, yo);

      //------p2
      hw = note2 ? note2->headWidth() : endCR()->width();
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
                 && sc && (sc->noteType() == NoteType::NORMAL)
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
      else if (ec && ec->up() != _up)
            yo = fixArticulations(yo, ec, __up);

      if (sa2 == SlurAnchor::NONE)
            sp->p2 += QPointF(xo, yo);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      if(track() != track2() && track2() != -1)
            xml.tag("track2", track2());
      int idx = 0;
      foreach(const SpannerSegment* ss, spannerSegments())
            ((SlurSegment*)ss)->write(xml, idx++);
      if (_slurDirection != MScore::Direction::AUTO)
            xml.tag("up", int(_slurDirection));
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
      score()->undoChangeProperty(this, P_ID::LINE_TYPE, t);
      }

//---------------------------------------------------------
//   undoSetSlurDirection
//---------------------------------------------------------

void SlurTie::undoSetSlurDirection(MScore::Direction d)
      {
      score()->undoChangeProperty(this, P_ID::SLUR_DIRECTION, int(d));
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurTie::reset()
      {
      score()->undoChangeProperty(this, P_ID::USER_OFF, QPointF());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant SlurTie::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::LINE_TYPE:      return lineType();
            case P_ID::SLUR_DIRECTION: return int(slurDirection());
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
            case P_ID::LINE_TYPE:      setLineType(v.toInt()); break;
            case P_ID::SLUR_DIRECTION: setSlurDirection(MScore::Direction(v.toInt())); break;
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
            case P_ID::LINE_TYPE:
                  return 0;
            case P_ID::SLUR_DIRECTION:
                  return int(MScore::Direction::AUTO);
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
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->getProperty(propertyId);
            case P_ID::SLUR_UOFF1:
                  return ups[int(GripSlurSegment::START)].off;
            case P_ID::SLUR_UOFF2:
                  return ups[int(GripSlurSegment::BEZIER1)].off;
            case P_ID::SLUR_UOFF3:
                  return ups[int(GripSlurSegment::BEZIER2)].off;
            case P_ID::SLUR_UOFF4:
                  return ups[int(GripSlurSegment::END)].off;
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
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->setProperty(propertyId, v);
            case P_ID::SLUR_UOFF1:
                  ups[int(GripSlurSegment::START)].off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF2:
                  ups[int(GripSlurSegment::BEZIER1)].off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF3:
                  ups[int(GripSlurSegment::BEZIER2)].off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF4:
                  ups[int(GripSlurSegment::END)].off = v.toPointF();
                  break;
            default:
                  return SpannerSegment::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant SlurSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::LINE_TYPE:
            case P_ID::SLUR_DIRECTION:
                  return slurTie()->propertyDefault(id);
            case P_ID::SLUR_UOFF1:
            case P_ID::SLUR_UOFF2:
            case P_ID::SLUR_UOFF3:
            case P_ID::SLUR_UOFF4:
                  return QPointF();
            default:
                  return SpannerSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void SlurSegment::reset()
      {
      score()->undoChangeProperty(this, P_ID::USER_OFF,   QPointF());
      score()->undoChangeProperty(this, P_ID::SLUR_UOFF1, QPointF());
      score()->undoChangeProperty(this, P_ID::SLUR_UOFF2, QPointF());
      score()->undoChangeProperty(this, P_ID::SLUR_UOFF3, QPointF());
      score()->undoChangeProperty(this, P_ID::SLUR_UOFF4, QPointF());

      parent()->reset();
      parent()->layout();
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
      if (!xml.canWrite(this)) return;
      xml.stag(QString("Slur id=\"%1\"").arg(xml.spannerId(this)));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(XmlReader& e)
      {
      setTrack(e.track());      // set staff
      e.addSpanner(e.intAttribute("id"), this);
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
      if (track2() == -1)
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
      for (Segment* seg = c1->segment(); seg; seg = seg->next(Segment::Type::ChordRest)) {
            Chord* c = static_cast<Chord*>(seg->element(c1->track()));
            if (!c || c->type() != Element::Type::CHORD)
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
            s->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            s->layout(QPointF(0, 0), QPointF(_spatium * 6, 0));
            setbbox(frontSegment()->bbox());
            return;
            }

      if (startCR() == 0) {
            qDebug("Slur::layout(): track %d-%d  %p - %p tick %d-%d null start anchor",
               track(), track2(), startCR(), endCR(), tick(), tick2());
            return;
            }
      if (endCR() == 0) {
            setEndElement(startCR());
            setTick2(tick());
            }
      switch (_slurDirection) {
            case MScore::Direction::UP:
                  _up = true;
                  break;
            case MScore::Direction::DOWN:
                  _up = false;
                  break;
            case MScore::Direction::AUTO:
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

                  Chord* c1 = (startCR()->type() == Element::Type::CHORD) ? static_cast<Chord*>(startCR()) : 0;
                  Chord* c2 = (endCR()->type() == Element::Type::CHORD) ? static_cast<Chord*>(endCR()) : 0;

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
                  else if (m1->mstaff(startCR()->staffIdx())->hasVoices && c1 && c1->noteType() == NoteType::NORMAL) {
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
                  segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
                  segment->layout(sPos.p1, sPos.p2);
                  }
            // case 2: start segment
            else if (i == 0) {
                  segment->setSpannerSegmentType(SpannerSegmentType::BEGIN);
                  qreal x = system->bbox().width();
                  segment->layout(sPos.p1, QPointF(x, sPos.p1.y()));
                  }
            // case 3: middle segment
            else if (i != 0 && system != sPos.system2) {
                  segment->setSpannerSegmentType(SpannerSegmentType::MIDDLE);
                  qreal x1 = firstNoteRestSegmentX(system) - _spatium;
                  qreal x2 = system->bbox().width();
                  qreal y  = system->staff(staffIdx())->y();
                  segment->layout(QPointF(x1, y), QPointF(x2, y));
                  }
            // case 4: end segment
            else {
                  segment->setSpannerSegmentType(SpannerSegmentType::END);
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
            if (mb->type() == Element::Type::MEASURE) {
                  const Measure* measure = static_cast<const Measure*>(mb);
                  for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->segmentType() == Segment::Type::ChordRest) {
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
//   startEdit
//---------------------------------------------------------

void SlurTie::startEdit(MuseScoreView* view, const QPointF& pt)
      {
      Spanner::startEdit(view, pt);

      editStartElement = startElement();
      editEndElement   = endElement();

      editUps.clear();
      foreach (SpannerSegment* s, spannerSegments()) {
            SlurOffsets o;
            SlurSegment* ss = static_cast<SlurSegment*>(s);
            o.o[0] = ss->getProperty(P_ID::SLUR_UOFF1).toPointF();
            o.o[1] = ss->getProperty(P_ID::SLUR_UOFF2).toPointF();
            o.o[2] = ss->getProperty(P_ID::SLUR_UOFF3).toPointF();
            o.o[3] = ss->getProperty(P_ID::SLUR_UOFF4).toPointF();
            editUps.append(o);
            }
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SlurTie::endEdit()
      {
      Spanner::endEdit();
      if (type() == Element::Type::SLUR) {
            if ((editStartElement != startElement()) || (editEndElement != endElement())) {
                  //
                  // handle parts:
                  //    search new start/end elements
                  //
                  for (Element* e : linkList()) {
                        Spanner* spanner = static_cast<Spanner*>(e);
                        if (spanner == this)
                              score()->undo()->push1(new ChangeStartEndSpanner(this, editStartElement, editEndElement));
                        else {
                              Element* se = 0;
                              Element* ee = 0;
                              if (startElement()) {
                                    QList<Element*> sel = startElement()->linkList();
                                    for (Element* e : sel) {
                                          if (e->score() == spanner->score() && e->track() == spanner->track()) {
                                                se = e;
                                                break;
                                                }
                                          }
                                    }
                              if (endElement()) {
                                    QList<Element*> sel = endElement()->linkList();
                                    for (Element* e : sel) {
                                          if (e->score() == spanner->score() && e->track() == spanner->track2()) {
                                                ee = e;
                                                break;
                                                }
                                          }
                                    }
                              score()->undo(new ChangeStartEndSpanner(spanner, se, ee));
                              }
                        }
                  }
            }
      if (spannerSegments().size() != editUps.size()) {
            qDebug("SlurTie::endEdit(): segment size changed %d != %d", spannerSegments().size(), editUps.size());
            return;
            }
      for (int i = 0; i < editUps.size(); ++i) {
            SpannerSegment* ss = segments[i];
            SlurOffsets o = editUps[i];
            score()->undoPropertyChanged(ss, P_ID::SLUR_UOFF1, o.o[0]);
            score()->undoPropertyChanged(ss, P_ID::SLUR_UOFF2, o.o[1]);
            score()->undoPropertyChanged(ss, P_ID::SLUR_UOFF3, o.o[2]);
            score()->undoPropertyChanged(ss, P_ID::SLUR_UOFF4, o.o[3]);
            }
      }

}

