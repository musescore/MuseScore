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
      setFlag(ElementFlag::ON_STAFF, true);
      autoAdjustOffset = QPointF();
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : SpannerSegment(b)
      {
      for (int i = 0; i < int(Grip::GRIPS); ++i) {
            _ups[i]   = b._ups[i];
            _ups[i].p = QPointF();
            }
      path = b.path;
      autoAdjustOffset = QPointF();
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      Element::move(s);
      for (int k = 0; k < int(Grip::GRIPS); ++k)
            _ups[k].p += s;
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

void SlurSegment::updateGrips(Grip* defaultGrip, QVector<QRectF>& r) const
      {
      *defaultGrip = Grip::END;
      QPointF p(pagePos());
      p -= QPointF(0.0, system()->staff(staffIdx())->y());   // ??
      for (int i = 0; i < int(Grip::GRIPS); ++i)
            r[i].translate(_ups[i].p + _ups[i].off * spatium() + p);
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

bool SlurSegment::edit(MuseScoreView* viewer, Grip curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      Slur* sl = static_cast<Slur*>(slurTie());

      if (key == Qt::Key_X) {
            sl->setSlurDirection(sl->up() ? Direction::DOWN : Direction::UP);
            sl->layout();
            return true;
            }
      if (key == Qt::Key_Home) {
            ups(curGrip).off = QPointF();
            sl->layout();
            return true;
            }
      if (slurTie()->type() != Element::Type::SLUR)
            return false;

      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SpannerSegmentType::SINGLE)
              || (spannerSegmentType() == SpannerSegmentType::BEGIN && curGrip == Grip::START)
              || (spannerSegmentType() == SpannerSegmentType::END && curGrip == Grip::END)
            )))
            return false;

      ChordRest* cr = 0;
      ChordRest* e  = curGrip == Grip::START ? sl->startCR() : sl->endCR();
      ChordRest* e1 = curGrip == Grip::START ? sl->endCR() : sl->startCR();

      if (key == Qt::Key_Left)
            cr = prevChordRest(e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest(e);
      else if (key == Qt::Key_Up) {
            Part* part     = e->part();
            int startTrack = part->startTrack();
            int endTrack   = e->track();
            cr = searchCR(e->segment(), endTrack, startTrack);
            }
      else if (key == Qt::Key_Down) {
            int startTrack = e->track() + 1;
            Part* part     = e->part();
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

void SlurSegment::changeAnchor(MuseScoreView* viewer, Grip curGrip, Element* element)
      {
      if (curGrip == Grip::START) {
            spanner()->setStartElement(element);
            switch (spanner()->anchor()) {
                  case Spanner::Anchor::NOTE: {
                        Tie* tie = static_cast<Tie*>(spanner());
                        Note* note = static_cast<Note*>(element);
                        if (note->chord()->tick() <= tie->endNote()->chord()->tick()) {
                              tie->startNote()->setTieFor(0);
                              tie->setStartNote(note);
                              note->setTieFor(tie);
                              }
                        break;
                        }
                  case Spanner::Anchor::CHORD:
                        spanner()->setTick(static_cast<Chord*>(element)->tick());
                        spanner()->setTrack(element->track());
                        break;
                  case Spanner::Anchor::SEGMENT:
                  case Spanner::Anchor::MEASURE:
                        qDebug("SlurSegment::changeAnchor: bad anchor");
                        break;
                  }
            }
      else {
            spanner()->setEndElement(element);
            switch (spanner()->anchor()) {
                  case Spanner::Anchor::NOTE: {
                        Tie* tie = static_cast<Tie*>(spanner());
                        Note* note = static_cast<Note*>(element);
                        // do not allow backward ties
                        if (note->chord()->tick() >= tie->startNote()->chord()->tick()) {
                              tie->endNote()->setTieBack(0);
                              tie->setEndNote(note);
                              note->setTieBack(tie);
                              }
                        break;
                        }
                  case Spanner::Anchor::CHORD:
                        spanner()->setTick2(static_cast<Chord*>(element)->tick());
                        spanner()->setTrack2(element->track());
                        break;

                  case Spanner::Anchor::SEGMENT:
                  case Spanner::Anchor::MEASURE:
                        qDebug("SlurSegment::changeAnchor: bad anchor");
                        break;
                  }
            }

      int segments  = spanner()->spannerSegments().size();
      ups(curGrip).off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            QList<SpannerSegment*>& ss = spanner()->spannerSegments();

            SlurSegment* newSegment = static_cast<SlurSegment*>(curGrip == Grip::END ? ss.back() : ss.front());
            score()->endCmd();
            score()->startCmd();
            viewer->startEdit(newSegment, curGrip);
            score()->setLayoutAll();
            }
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(Grip grip) const
      {
      SlurPos spos;
      slurTie()->slurPos(&spos);

      QPointF sp(system()->pagePos());
      QPointF p1(spos.p1 + spos.system1->pagePos());
      QPointF p2(spos.p2 + spos.system2->pagePos());
      switch (spannerSegmentType()) {
            case SpannerSegmentType::SINGLE:
                  if (grip == Grip::START)
                        return p1;
                  else if (grip == Grip::END)
                        return p2;
                  break;

            case SpannerSegmentType::BEGIN:
                  if (grip == Grip::START)
                        return p1;
                  else if (grip == Grip::END)
                        return system()->abbox().topRight();
                  break;

            case SpannerSegmentType::MIDDLE:
                  if (grip == Grip::START)
                        return sp;
                  else if (grip == Grip::END)
                        return system()->abbox().topRight();
                  break;

            case SpannerSegmentType::END:
                  if (grip == Grip::START)
                        return sp;
                  else if (grip == Grip::END)
                        return p2;
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF SlurSegment::getGrip(Grip n) const
      {
      switch (n) {
            case Grip::START:
            case Grip::END:
                  return (ups(n).p - gripAnchor(n)) / spatium() + ups(n).off;
            default:
                  return ups(n).off;
            }
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void SlurSegment::setGrip(Grip n, const QPointF& pt)
      {
      switch (n) {
            case Grip::START:
            case Grip::END:
                  ups(n).off = ((pt * spatium()) - (ups(n).p - gripAnchor(n))) / spatium();
                  break;
            default:
                  ups(n).off = pt;
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
      ups(ed.curGrip).off += (ed.delta / _spatium);

      if (ed.curGrip == Grip::START || ed.curGrip == Grip::END) {
            slurTie()->computeBezier(this);
            //
            // move anchor for slurs/ties
            //
            SpannerSegmentType st = spannerSegmentType();
            if (
               (ed.curGrip == Grip::START  && (st == SpannerSegmentType::SINGLE || st == SpannerSegmentType::BEGIN))
               || (ed.curGrip == Grip::END && (st == SpannerSegmentType::SINGLE || st == SpannerSegmentType::END))
               ) {
                  Spanner* spanner = slurTie();
                  Qt::KeyboardModifiers km = qApp->keyboardModifiers();
                  Note* note = static_cast<Note*>(ed.view->elementNear(ed.pos));
                  if (note && note->type() == Element::Type::NOTE &&
                     ((ed.curGrip == Grip::END && note->chord()->tick() > slurTie()->tick())
                      || (ed.curGrip == Grip::START && note->chord()->tick() < slurTie()->tick2()))
                     ) {
                        if (ed.curGrip == Grip::END && spanner->type() == Element::Type::TIE) {
                              Tie* tie = static_cast<Tie*>(spanner);
                              if (tie->startNote()->pitch() == note->pitch()
                                 && tie->startNote()->chord()->tick() < note->chord()->tick()) {
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
//                                    QPointF p1 = ed.pos - ups(ed.curGrip).p - canvasPos();
//                                    ups(ed.curGrip).off = p1 / _spatium;
                                    slurTie()->layout();
                                    }
                              }
                        }
                  else
                        ed.view->setDropTarget(0);
                  }
            }
      else if (ed.curGrip == Grip::BEZIER1 || ed.curGrip == Grip::BEZIER2)
            slurTie()->computeBezier(this);
      else if (ed.curGrip == Grip::SHOULDER) {
            ups(ed.curGrip).off = QPointF();
            slurTie()->computeBezier(this, ed.delta);
            }
      else if (ed.curGrip == Grip::DRAG) {
            ups(Grip::DRAG).off = QPointF();
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

void SlurSegment::writeSlur(Xml& xml, int no) const
      {
      if (ups(Grip::START).off.isNull()
         && ups(Grip::END).off.isNull()
         && ups(Grip::BEZIER1).off.isNull()
         && ups(Grip::BEZIER2).off.isNull()
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
                  ups(Grip::START).off = e.readPoint();
            else if (tag == "o2")
                  ups(Grip::BEZIER1).off = e.readPoint();
            else if (tag == "o3")
                  ups(Grip::BEZIER2).off = e.readPoint();
            else if (tag == "o4")
                  ups(Grip::END).off = e.readPoint();
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
      QPointF pp1 = ss->ups(Grip::START).p + ss->ups(Grip::START).off * _spatium;
      QPointF pp2 = ss->ups(Grip::END).p   + ss->ups(Grip::END).off   * _spatium;

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
      if ((c2 - c1) <= _spatium)
            w *= .5;
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ss->ups(Grip::BEZIER1).off * _spatium);
      QPointF p4o = p6o + t.map(ss->ups(Grip::BEZIER2).off * _spatium);

      if (!p6o.isNull()) {
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
      ss->path                 = t.map(ss->path);
      ss->shapePath            = t.map(ss->shapePath);
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
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void SlurSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      ups(Grip::START).p = p1;
      ups(Grip::END).p   = p2;
      slurTie()->computeBezier(this);
      QRectF bbox = path.boundingRect();

      // adjust position to avoid staff line if necessary
      Staff* st = staff();
      bool reverseAdjust = false;
      if (slurTie()->type() == Element::Type::TIE && st && !st->isTabStaff()) {
            // multinote chords with ties need special handling
            // otherwise, adjusted tie might crowd an unadjusted tie unnecessarily
            Tie* t = static_cast<Tie*>(slurTie());
            Note* sn = t->startNote();
            Chord* sc = sn ? sn->chord() : 0;
            // normally, the adjustment moves ties according to their direction (eg, up if tie is up)
            // but we will reverse this for notes within chords when appropriate
            // for two-note chords, it looks better to have notes on spaces tied outside the lines
            if (sc) {
                  int notes = sc->notes().size();
                  bool onLine = !(sn->line() & 1);
                  if ((onLine && notes > 1) || (!onLine && notes > 2))
                        reverseAdjust = true;
                  }
            }
      qreal sp = spatium();
      qreal minDistance = 0.5;
      autoAdjustOffset = QPointF();
      if (bbox.height() < minDistance * 2 * sp && st && !st->isTabStaff()) {
            // slur/tie is fairly flat
            bool up = slurTie()->up();
            qreal ld = st->lineDistance() * sp;
            qreal topY = bbox.top() / ld;
            qreal bottomY = bbox.bottom() / ld;
            int lineY = up ? qRound(topY) : qRound(bottomY);
            if (lineY >= 0 && lineY < st->lines() * st->lineDistance()) {
                  // on staff
                  if (qAbs(topY - lineY) < minDistance && qAbs(bottomY - lineY) < minDistance) {
                        // too close to line
                        if (!isNudged() && !isEdited()) {
                              // user has not nudged or edited
                              qreal offY;
                              if (up != reverseAdjust)      // exclusive or
                                    offY = (lineY - minDistance) - topY;
                              else
                                    offY = (lineY + minDistance) - bottomY;
                              setAutoAdjust(0.0, offY * sp);
                              bbox = path.boundingRect();
                              }
                        }
                  }
            }
      setbbox(path.boundingRect());
      if ((staffIdx() > 0) && score()->mscVersion() < 201 && !readPos().isNull()) {
            QPointF staffOffset;
            if (system() && track() >= 0)
                  staffOffset = QPointF(0.0, system()->staff(staffIdx())->y());
            setReadPos(readPos() + staffOffset);
            }
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
            for (int i = 0; i < int(Grip::GRIPS); ++i)
                  _ups[i].p += diff;
            autoAdjustOffset = offset;
            }
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

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Spanner(s)
      {
      _slurDirection = Direction::AUTO;
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
      const QVector<Articulation*>& al = c->articulations();
      if (al.size() >= 2) {
            Articulation* a = al.at(1);
            if (a->up() == c->up())
                  return yo;
            else if (a->articulationType() == ArticulationType::Tenuto || a->articulationType() == ArticulationType::Staccato)
                  return a->y() + (a->height() + c->score()->spatium() * .3) * _up;
            }
      else if (al.size() >= 1) {
            Articulation* a = al.at(0);
            if (a->up() == c->up())
                  return yo;
            else if (a->articulationType() == ArticulationType::Tenuto || a->articulationType() == ArticulationType::Staccato)
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

      bool        useTablature      = staff() != nullptr && staff()->isTabStaff();
      bool        staffHasStems     = true;     // assume staff uses stems
      StaffType*  stt               = nullptr;
      if (useTablature) {
            stt               = staff()->staffType();
            staffHasStems     = stt->stemThrough();   // if tab with stems beside, stems do not count for slur pos
            }

      // start and end cr, chord, and note
      ChordRest* scr = startCR();
      ChordRest* ecr = endCR();
      Chord* sc      = 0;
      Note* note1    = 0;
      if (scr->type() == Element::Type::CHORD) {
            sc = static_cast<Chord*>(scr);
            note1 = _up ? sc->upNote() : sc->downNote();
            }
      Chord* ec = 0;
      Note* note2 = 0;
      if (ecr->type() == Element::Type::CHORD) {
            ec   = static_cast<Chord*>(ecr);
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

      qreal __up = _up ? -1.0 : 1.0;
      qreal hw1 = note1 ? note1->tabHeadWidth(stt) : scr->width();      // if stt == 0, tabHeadWidth()
      qreal hw2 = note2 ? note2->tabHeadWidth(stt) : ecr->width();      // defaults to headWidth()
      QPointF pt;
      switch (sa1) {
            case SlurAnchor::STEM:        //sc can't be null
                  // place slur starting point at stem base point
                  pt = sc->stemPos() - sc->pagePos() + sc->stem()->p2();
                  if (useTablature)                   // in tabs, stems are centred on note:
                        pt.rx() = hw1 * 0.5;          // skip half note head to touch stem
                  sp->p1 += pt;
                  sp->p1 += QPointF(0.35 * _spatium, 0.25 * _spatium);  // clear the stem (x) and the note head (y)
                  break;
            case SlurAnchor::NONE:
                  break;
            }
      switch (sa2) {
            case SlurAnchor::STEM:        //ec can't be null
                  pt = ec->stemPos() - ec->pagePos() + ec->stem()->p2();
                  if (useTablature)
                        pt.rx() = hw2 * 0.5;
                  sp->p2 += pt;
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
      // Compute x0, y0 and stemPos
      if (sa1 == SlurAnchor::NONE || sa2 == SlurAnchor::NONE) { // need stemPos if sa2 == SlurAnchor::NONE
            bool stemPos = false;   // p1 starts at chord stem side

            // default positions
            xo = hw1 * .5;
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
                  if (beam1 && (beam1->elements().back() != sc) && (sc->up() == _up)) {
                        // start chord is beamed but not the last chord of beam group
                        // and slur direction is same as start chord (stem side)

                        // in these cases, layout start of slur to stem

                        qreal sh = stem1->height() + _spatium;
                        if (_up)
                              yo = sc->downNote()->pos().y() - sh;
                        else
                              yo = sc->upNote()->pos().y() + sh;
                        xo       = stem1->pos().x();

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

                              // we may wish to force end to align to stem as well,
                              // if it is in same direction
                              // (but it won't be, so this assignment should have no effect)
                              stemPos = true;
                              }
                        else if (sc->up() != _up) {
                              // slur opposite direction from chord
                              // avoid articulations
                              yo = fixArticulations(yo, sc, __up);
                              }
                        }
                  }
            else if (sc && sc->up() != _up) {
                  // slur opposite direction from chord
                  // avoid articulations
                  yo = fixArticulations(yo, sc, __up);
                  }

            if (sa1 == SlurAnchor::NONE)
                  sp->p1 += QPointF(xo, yo);

            //------p2
            if (sa2 == SlurAnchor::NONE) {

                  // default positions
                  xo = hw2 * .5;
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
                        if ((stemPos && (scr->up() == ec->up()))
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
                                    }
                              else if (ec->up() != _up) {
                                    // slur opposite direction from chord
                                    // avoid articulations
                                    yo = fixArticulations(yo, ec, __up);
                                    }

                              }
                        }
                  else if (ec && ec->up() != _up) {
                        // slur opposite direction from chord
                        // avoid articulations
                        yo = fixArticulations(yo, ec, __up);
                        }

                  sp->p2 += QPointF(xo, yo);
                  }
            }
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
            ((SlurSegment*)ss)->writeSlur(xml, idx++);
      if (_slurDirection != Direction::AUTO)
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
            // in v1.x "visible" is a property of the segment only;
            // we must ensure that it propagates also to the parent element
            // That's why the visibility is set after adding the segment
            // to the corresponding spanner
            if (score()->mscVersion() <= 114)
                  segment->SpannerSegment::setVisible(segment->visible());
            }
      else if (tag == "up")
            _slurDirection = Direction(e.readInt());
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

void SlurTie::undoSetSlurDirection(Direction d)
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
            case P_ID::SLUR_DIRECTION: setSlurDirection(Direction(v.toInt())); break;
            default:
                  return Spanner::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
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
                  return int(Direction::AUTO);
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
                  return ups(Grip::START).off;
            case P_ID::SLUR_UOFF2:
                  return ups(Grip::BEZIER1).off;
            case P_ID::SLUR_UOFF3:
                  return ups(Grip::BEZIER2).off;
            case P_ID::SLUR_UOFF4:
                  return ups(Grip::END).off;
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
                  ups(Grip::START).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF2:
                  ups(Grip::BEZIER1).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF3:
                  ups(Grip::BEZIER2).off = v.toPointF();
                  break;
            case P_ID::SLUR_UOFF4:
                  ups(Grip::END).off = v.toPointF();
                  break;
            default:
                  return SpannerSegment::setProperty(propertyId, v);
            }
      score()->setLayoutAll();
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
      if (!xml.canWrite(this))
            return;
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
            qDebug("Slur::layout(): track %d-%d  %p - %p tick %d-%d null start anchor",
               track(), track2(), startCR(), endCR(), tick(), tick2());
            return;
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
                  qreal x1 = firstNoteRestSegmentX(system);
                  qreal x2 = system->bbox().width();
                  qreal y  = system->staff(staffIdx())->y();
                  segment->layoutSegment(QPointF(x1, y), QPointF(x2, y));
                  }
            // case 4: end segment
            else {
                  segment->setSpannerSegmentType(SpannerSegmentType::END);
                  qreal x = firstNoteRestSegmentX(system);
                  segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
                  }
            if (system == sPos.system2)
                  break;
            }
      setbbox(spannerSegments().empty() ? QRectF() : frontSegment()->bbox());
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//    in System() coordinates
//    returns the position just after the last non-chordrest segment
//---------------------------------------------------------

qreal SlurTie::firstNoteRestSegmentX(System* system)
      {
      foreach(const MeasureBase* mb, system->measures()) {
            if (mb->type() == Element::Type::MEASURE) {
                  const Measure* measure = static_cast<const Measure*>(mb);
                  for (const Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->segmentType() == Segment::Type::ChordRest) {
                              // first CR found; back up to previous segment
                              seg = seg->prev();
                              if (seg) {
                                    // find maximum width
                                    qreal width = 0.0;
                                    int n = score()->nstaves();
                                    for (int i = 0; i < n; ++i) {
                                          if (!system->staff(i)->show())
                                                continue;
                                          Element* e = seg->element(i * VOICES);
                                          if (e)
                                                width = qMax(width, e->width());
                                          }
                                    return seg->measure()->pos().x() + seg->pos().x() + width;
                                    }
                              else
                                    return 0.0;
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
                  if (!delSegments.empty()) {
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
                  for (ScoreElement* e : linkList()) {
                        Spanner* spanner = static_cast<Spanner*>(e);
                        if (spanner == this)
                              score()->undo()->push1(new ChangeStartEndSpanner(this, editStartElement, editEndElement));
                        else {
                              Element* se = 0;
                              Element* ee = 0;
                              if (startElement()) {
                                    QList<ScoreElement*> sel = startElement()->linkList();
                                    for (ScoreElement* eee : sel) {
                                          Element* e = static_cast<Element*>(eee);
                                          if (e->score() == spanner->score() && e->track() == spanner->track()) {
                                                se = e;
                                                break;
                                                }
                                          }
                                    }
                              if (endElement()) {
                                    QList<ScoreElement*> sel = endElement()->linkList();
                                    for (ScoreElement* eee : sel) {
                                          Element* e = static_cast<Element*>(eee);
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
      score()->setLayoutAll();
      }

}

