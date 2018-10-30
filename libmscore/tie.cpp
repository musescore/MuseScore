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

#include "measure.h"
#include "score.h"
#include "system.h"
#include "undo.h"
#include "chord.h"
#include "tie.h"

namespace Ms {

Note* Tie::editStartNote;
Note* Tie::editEndNote;

//---------------------------------------------------------
//   updateGrips
//    return grip rectangles in page coordinates
//---------------------------------------------------------

void TieSegment::updateGrips(EditData& ed) const
      {
      QPointF p(pagePos());
      p -= QPointF(0.0, system()->staff(staffIdx())->y());   // ??
      for (int i = 0; i < int(Grip::GRIPS); ++i)
            ed.grip[i].translate(_ups[i].p + _ups[i].off + p);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TieSegment::draw(QPainter* painter) const
      {
      // hide tie toward the second chord of a cross-measure value
      if (tie()->endNote() && tie()->endNote()->chord()->crossMeasure() == CrossMeasure::SECOND)
            return;

      QPen pen(curColor());
      switch (slurTie()->lineType()) {
            case 0:
                  painter->setBrush(QBrush(pen.color()));
                  pen.setCapStyle(Qt::RoundCap);
                  pen.setJoinStyle(Qt::RoundJoin);
                  pen.setWidthF(score()->styleP(Sid::SlurEndWidth));
                  break;
            case 1:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth));
                  pen.setStyle(Qt::DotLine);
                  break;
            case 2:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth));
                  pen.setStyle(Qt::DashLine);
                  break;
            case 3:
                  painter->setBrush(Qt::NoBrush);
                  pen.setWidthF(score()->styleP(Sid::SlurDottedWidth));
                  pen.setStyle(Qt::CustomDashLine);
                  QVector<qreal> dashes { 5.0, 5.0 };
                  pen.setDashPattern(dashes);
                  break;
            }
      painter->setPen(pen);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TieSegment::edit(EditData& ed)
      {
      SlurTie* sl = tie();

      if (ed.key == Qt::Key_X) {
            sl->setSlurDirection(sl->up() ? Direction::DOWN : Direction::UP);
            sl->layout();
            return true;
            }
      if (ed.key == Qt::Key_Home) {
            ups(ed.curGrip).off = QPointF();
            sl->layout();
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void TieSegment::changeAnchor(EditData& ed, Element* element)
      {
      if (ed.curGrip == Grip::START) {
            spanner()->setStartElement(element);
            Note* note = toNote(element);
            if (note->chord()->tick() <= tie()->endNote()->chord()->tick()) {
                  tie()->startNote()->setTieFor(0);
                  tie()->setStartNote(note);
                  note->setTieFor(tie());
                  }
            }
      else {
            spanner()->setEndElement(element);
            Note* note = toNote(element);
            // do not allow backward ties
            if (note->chord()->tick() >= tie()->startNote()->chord()->tick()) {
                  tie()->endNote()->setTieBack(0);
                  tie()->setEndNote(note);
                  note->setTieBack(tie());
                  }
            }

      int segments  = spanner()->spannerSegments().size();
      ups(ed.curGrip).off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            QList<SpannerSegment*>& ss = spanner()->spannerSegments();

            TieSegment* newSegment = toTieSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
            score()->endCmd();
            score()->startCmd();
            ed.view->startEdit(newSegment, ed.curGrip);
            score()->setLayoutAll();
            }
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TieSegment::editDrag(EditData& ed)
      {
      Grip g = ed.curGrip;
      ups(g).off += ed.delta;

      if (g == Grip::START || g == Grip::END) {
            computeBezier();
            //
            // move anchor for slurs/ties
            //
            if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
                  Spanner* spanner = tie();
                  Element* e = ed.view->elementNear(ed.pos);
                  Note* note = (e && e->isNote()) ? toNote(e) : nullptr;
                  if (note && ((g == Grip::END && note->tick() > tie()->tick()) || (g == Grip::START && note->tick() < tie()->tick2()))) {
                        if (g == Grip::END) {
                              Tie* tie = toTie(spanner);
                              if (tie->startNote()->pitch() == note->pitch()
                                 && tie->startNote()->chord()->tick() < note->chord()->tick()) {
                                    ed.view->setDropTarget(note);
                                    if (note != tie->endNote()) {
                                          changeAnchor(ed, note);
                                          return;
                                          }
                                    }
                              }
                        }
                  else
                        ed.view->setDropTarget(0);
                  }
            }
      else if (g == Grip::BEZIER1 || g == Grip::BEZIER2)
            computeBezier();
      else if (g == Grip::SHOULDER) {
            ups(g).off = QPointF();
            computeBezier(ed.delta);
            }
      else if (g == Grip::DRAG) {
            ups(Grip::DRAG).off = QPointF();
            roffset() += ed.delta;
            }

      // if this SlurSegment was automatically adjusted to avoid collision
      // lock this edit by resetting SlurSegment to default position
      // and incorporating previous adjustment into user offset
      QPointF offset = getAutoAdjust();
      if (!offset.isNull()) {
            setAutoAdjust(0.0, 0.0);
            roffset() += offset;
            }
      }

//---------------------------------------------------------
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void TieSegment::computeBezier(QPointF p6o)
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
      QPointF pp1 = ups(Grip::START).p + ups(Grip::START).off;
      QPointF pp2 = ups(Grip::END).p   + ups(Grip::END).off;

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

      if (!tie()->up())
            shoulderH = -shoulderH;

      qreal c    = p2.x();
      qreal c1   = (c - c * shoulderW) * .5 + p6o.x();
      qreal c2   = c1 + c * shoulderW       + p6o.x();

      QPointF p5 = QPointF(c * .5, 0.0);

      QPointF p3(c1, -shoulderH);
      QPointF p4(c2, -shoulderH);

      qreal w = score()->styleP(Sid::SlurMidWidth) - score()->styleP(Sid::SlurEndWidth);
      QPointF th(0.0, w);    // thickness of slur

      QPointF p3o = p6o + t.map(ups(Grip::BEZIER1).off);
      QPointF p4o = p6o + t.map(ups(Grip::BEZIER2).off);

      if(!p6o.isNull()) {
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
      if (tie()->lineType() == 0)
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

//      QPointF staffOffset;
//      if (system() && track() >= 0)
//            staffOffset = QPointF(0.0, -system()->staff(staffIdx())->y());

//      path.translate(staffOffset);
//      shapePath.translate(staffOffset);

      QPainterPath p;
      p.moveTo(QPointF());
      p.cubicTo(p3 + p3o, p4 + p4o, p2);
      _shape.clear();
      QPointF start;
      start = t.map(start);

      qreal minH = qAbs(3.0 * w);
      int nbShapes = 15;
      for (int i = 1; i <= nbShapes; i++) {
            QPointF point = t.map(p.pointAtPercent(i/float(nbShapes)));
            QRectF re = QRectF(start, point).normalized();
            if (re.height() < minH) {
                  d = (minH - re.height()) * .5;
                  re.adjust(0.0, -d, 0.0, d);
                  }
            _shape.add(re);
            start = point;
            }
      }

//---------------------------------------------------------
//   layout
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void TieSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      if (autoplace()) {
            for (UP& up : _ups)
                  up.off = QPointF();
            }
      ups(Grip::START).p = p1;
      ups(Grip::END).p   = p2;
      computeBezier();

      QRectF bbox = path.boundingRect();

      // adjust position to avoid staff line if necessary
      Staff* st          = staff();
      bool reverseAdjust = false;

      if (slurTie()->isTie() && st && !st->isTabStaff(slurTie()->tick())) {
            // multinote chords with ties need special handling
            // otherwise, adjusted tie might crowd an unadjusted tie unnecessarily
            Tie* t    = toTie(slurTie());
            Note* sn  = t->startNote();
            Chord* sc = sn ? sn->chord() : 0;

            // normally, the adjustment moves ties according to their direction (eg, up if tie is up)
            // but we will reverse this for notes within chords when appropriate
            // for two-note chords, it looks better to have notes on spaces tied outside the lines

            if (sc) {
                  size_t notes = sc->notes().size();
                  bool onLine = !(sn->line() & 1);
                  if ((onLine && notes > 1) || (!onLine && notes > 2))
                        reverseAdjust = true;
                  }
            }
      qreal sp          = spatium();
      qreal minDistance = 0.5;
      autoAdjustOffset  = QPointF();
      if (bbox.height() < minDistance * 2 * sp && st && !st->isTabStaff(slurTie()->tick())) {
            // slur/tie is fairly flat
            bool up       = slurTie()->up();
            qreal ld      = st->lineDistance(tick()) * sp;
            qreal topY    = bbox.top() / ld;
            qreal bottomY = bbox.bottom() / ld;
            int lineY     = up ? qRound(topY) : qRound(bottomY);
            if (lineY >= 0 && lineY < st->lines(tick()) * st->lineDistance(tick())) {
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

      setbbox(bbox);
      }

//---------------------------------------------------------
//   setAutoAdjust
//---------------------------------------------------------

void TieSegment::setAutoAdjust(const QPointF& offset)
      {
      QPointF diff = offset - autoAdjustOffset;
      if (!diff.isNull()) {
            path.translate(diff);
            shapePath.translate(diff);
            _shape.translate(diff);
            for (int i = 0; i < int(Grip::GRIPS); ++i)
                  _ups[i].p += diff;
            autoAdjustOffset = offset;
            }
      }

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool TieSegment::isEdited() const
      {
      for (int i = 0; i < int(Grip::GRIPS); ++i) {
            if (!_ups[i].off.isNull())
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   slurPos
//    Calculate position of start- and endpoint of slur
//    relative to System() position.
//---------------------------------------------------------

void Tie::slurPos(SlurPos* sp)
      {
      bool useTablature = staff() && staff()->isTabStaff(tick());
      StaffType* stt    = useTablature ? staff()->staffType(tick()) : 0;
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
      sp->system1 = sc->measure()->system();
      if (!sp->system1) {
            Measure* m = sc->measure();
            qDebug("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
            }

      qreal xo;
      qreal yo;
      bool shortStart = false;

      // determine attachment points
      // similar code is used in Chord::layoutPitched()
      // to allocate extra space to enforce minTieLength
      // so keep these in sync

      sp->p1    = sc->pos() + sc->segment()->pos() + sc->measure()->pos();

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
      sp->p1 += QPointF(xo, yo);

      //------p2
      if (endNote() == 0) {
            sp->p2 = sp->p1 + QPointF(_spatium * 3, 0.0);
            sp->system2 = sp->system1;
            return;
            }
      Chord* ec = endNote()->chord();
      sp->p2    = ec->pos() + ec->segment()->pos() + ec->measure()->pos();
      if ((sc->measure() == sp->system1->lastMeasure()) && (ec->measure() != sc->measure()))
            sp->system2 = nullptr;
      else
            sp->system2 = ec->measure()->system();

      hw = endNote()->tabHeadWidth(stt);
      if ((ec->notes().size() > 1) || (ec->stem() && !ec->up() && !_up))
            xo = endNote()->x() - hw * 0.12;
      else if (shortStart)
            xo = endNote()->x() + hw * 0.15;
      else
            xo = endNote()->x() + hw * 0.35;
      sp->p2 += QPointF(xo, yo);
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

void Tie::write(XmlWriter& xml) const
      {
      xml.stag(this);
      SlurTie::writeProperties(xml);
      xml.etag();
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
            size_t n = notes.size();
            if (m1->hasVoices(c1->staffIdx()) || m2->hasVoices(c2->staffIdx())) {
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
                  for (size_t i = 0; i < n; ++i) {
                        if (notes[i]->tieFor()) {
                              ties.append(notes[i]->line());
                              if (notes[i] == startNote()) {
                                    idx = ties.size() - 1;
                                    noteIdx = int(i);
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
//   layoutFor
//    layout the first SpannerSegment of a slur
//---------------------------------------------------------

TieSegment* Tie::layoutFor(System* system)
      {
      //
      //    show short bow
      //
      if (startNote() == 0 || endNote() == 0) {
            if (startNote() == 0) {
                  qDebug("no start note");
                  return 0;
                  }
            Chord* c1 = startNote()->chord();
            if (_slurDirection == Direction::AUTO) {
                  if (c1->measure()->hasVoices(c1->staffIdx())) {
                        // in polyphonic passage, ties go on the stem side
                        _up = c1->up();
                        }
                  else
                        _up = !c1->up();
                  }
            else
                  _up = _slurDirection == Direction::UP ? true : false;
            fixupSegments(1);
            TieSegment* segment = segmentAt(0);
            segment->setSpannerSegmentType(SpannerSegmentType::SINGLE);
            segment->setSystem(startNote()->chord()->segment()->measure()->system());
            SlurPos sPos;
            slurPos(&sPos);
            segment->layoutSegment(sPos.p1, sPos.p2);
            return segment;
            }
      calculateDirection();

      SlurPos sPos;
      slurPos(&sPos);

      setPos(0, 0);

      int n;
      if (sPos.system1 != sPos.system2) {
            n = 2;
            sPos.p2 = QPointF(system->width(), sPos.p1.y());
            }
      else
            n = 1;

      fixupSegments(n);
      TieSegment* segment = segmentAt(0);
      segment->setSystem(system); // Needed to populate System.spannerSegments
      segment->layoutSegment(sPos.p1, sPos.p2);
      segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
      return segment;
      }

//---------------------------------------------------------
//   layoutBack
//    layout the second SpannerSegment of a split slur
//---------------------------------------------------------

TieSegment* Tie::layoutBack(System* system)
      {
      SlurPos sPos;
      slurPos(&sPos);

      fixupSegments(2);
      TieSegment* segment = segmentAt(1);
      segment->setSystem(system);

      qreal x;
      Segment* seg = endNote()->chord()->segment()->prev();
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
            x = seg->measure()->pos().x() + seg->pos().x() + width;
            }
      else
            x = 0.0;

      segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
      segment->setSpannerSegmentType(SpannerSegmentType::END);
      return segment;
      }

#if 0
//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Tie::startEdit(EditData& ed)
      {
      printf("tie start edit %p %p\n", editStartNote, editEndNote);
      editStartNote = startNote();
      editEndNote   = endNote();
      SlurTie::startEdit(ed);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Tie::endEdit(EditData& ed)
      {
      printf("tie::endEdit\n");
//      if (editStartNote != startNote() || editEndNote != endNote()) {
//            score()->undoStack()->push1(new ChangeSpannerElements(this, editStartNote, editEndNote));
//            }
      SlurTie::endEdit(ed);
      }
#endif

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
      Q_ASSERT(!startElement() || startElement()->type() == ElementType::NOTE);
      return toNote(startElement());
      }

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Tie::endNote() const
      {
      return toNote(endElement());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Tie::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "TieSegment") {
            int idx = e.intAttribute("no", 0);
            int n = spannerSegments().size();
            for (int i = n; i < idx; ++i)
                  add(new TieSegment(score()));
            TieSegment* segment = new TieSegment(score());
            segment->setAutoplace(false);
            segment->read(e);
            add(segment);
            }
      else if (!SlurTie::readProperties(e))
            return false;
      return true;
      }


}

