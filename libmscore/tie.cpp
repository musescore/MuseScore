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
#include "hook.h"
#include "ledgerline.h"
#include "accidental.h"
#include "stem.h"
#include "tie.h"

namespace Ms {

Note* Tie::editStartNote;
Note* Tie::editEndNote;

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TieSegment::draw(QPainter* painter) const
      {
      // hide tie toward the second chord of a cross-measure value
      if (tie()->endNote() && tie()->endNote()->chord()->crossMeasure() == CrossMeasure::SECOND)
            return;

      QPen pen(curColor());
      qreal mag = staff() ? staff()->mag(tie()->tick()) : 1.0;

      //Replace generic Qt dash patterns with improved equivalents to show true dots (keep in sync with slur.cpp)
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
                  pen.setCapStyle(Qt::RoundCap); // True dots
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
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TieSegment::edit(EditData& ed)
      {
      SlurTie* sl = tie();

      if (ed.key == Qt::Key_X && !ed.modifiers) {
            sl->setSlurDirection(sl->up() ? Direction::DOWN : Direction::UP);
            sl->layout();
            return true;
            }
      if (ed.key == Qt::Key_Home && !ed.modifiers) {
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

      const size_t segments  = spanner()->spannerSegments().size();
      ups(ed.curGrip).off = QPointF();
      spanner()->layout();
      if (spanner()->spannerSegments().size() != segments) {
            const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();

            TieSegment* newSegment = toTieSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
            score()->endCmd();
            score()->startCmd();
            ed.view->startEdit(newSegment, ed.curGrip);
            triggerLayoutAll();
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

void TieSegment::computeBezier(QPointF shoulderOffset)
      {
      qreal _spatium = spatium();
      qreal shoulderW; // height as fraction of slur-length
      qreal shoulderH;

      QPointF tieStart = ups(Grip::START).p + ups(Grip::START).off;
      QPointF tieEnd = ups(Grip::END).p   + ups(Grip::END).off;

      QPointF tieEndNormalized = tieEnd - tieStart; // normalize to zero
      if (tieEndNormalized.x() == 0.0) {
            qDebug("zero tie");
            return;
            }

      qreal tieAngle = atan(tieEndNormalized.y() / tieEndNormalized.x()); // angle required from tie start to tie end--zero if horizontal
      QTransform t;
      t.rotateRadians(-tieAngle);  // rotate so that we are working with horizontal ties regardless of endpoint height difference
      tieEndNormalized = t.map(tieEndNormalized);  // apply that rotation
      shoulderOffset = t.map(shoulderOffset);  // also apply to shoulderOffset

      double smallH = 0.38; // I don't know what this means currently
      qreal tieWidthInSp = tieEndNormalized.x() / _spatium;
      shoulderH = tieWidthInSp * 0.4 * smallH;  // magic math?
      shoulderH = qBound(shoulderHeightMin, shoulderH, shoulderHeightMax);
      shoulderH *= _spatium;  // shoulderH is now canvas units
      shoulderW = .6;

      shoulderH -= shoulderOffset.y();

      if (!tie()->up())
            shoulderH = -shoulderH;

      qreal tieWidth = tieEndNormalized.x();
      qreal bezier1X = (tieWidth - tieWidth * shoulderW) * .5 + shoulderOffset.x();
      qreal bezier2X = bezier1X + tieWidth * shoulderW + shoulderOffset.x();

      QPointF tieDrag = QPointF(tieWidth * .5, 0.0);

      QPointF bezier1(bezier1X, -shoulderH);
      QPointF bezier2(bezier2X, -shoulderH);

      qreal w = score()->styleP(Sid::SlurMidWidth) - score()->styleP(Sid::SlurEndWidth);
      if (staff())
            w *= staff()->mag(tie()->tick());
      QPointF tieThickness(0.0, w);

      QPointF bezier1Offset = shoulderOffset + t.map(ups(Grip::BEZIER1).off);
      QPointF bezier2Offset = shoulderOffset + t.map(ups(Grip::BEZIER2).off);

      if (!shoulderOffset.isNull()) {
            QPointF invertedShoulder = t.inverted().map(shoulderOffset);
            ups(Grip::BEZIER1).off += invertedShoulder;
            ups(Grip::BEZIER2).off += invertedShoulder;
            }

      //-----------------------------------calculate p6
      QPointF bezier1Final = bezier1 + bezier1Offset;
      QPointF bezier2Final = bezier2 + bezier2Offset;
      QPointF bezierNormalized = bezier2Final - bezier1Final;

      qreal bezierAngle = atan(bezierNormalized.y() / bezierNormalized.x());  // in case bezier1 and bezier2 are not horizontal
      t.reset();
      t.rotateRadians(-bezierAngle);
      QPointF tieShoulder = QPointF(t.map(bezierNormalized).x() * .5, 0.0);

      t.rotateRadians(2 * bezierAngle);
      tieShoulder = t.map(tieShoulder) + bezier1Final - shoulderOffset;
      //-----------------------------------

      path = QPainterPath();
      path.moveTo(QPointF());
      path.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
      if (tie()->lineType() == 0)
            path.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, QPointF());

      tieThickness  = QPointF(0.0, 3.0 * w);
      shapePath = QPainterPath();
      shapePath.moveTo(QPointF());
      shapePath.cubicTo(bezier1 + bezier1Offset - tieThickness, bezier2 + bezier2Offset - tieThickness, tieEndNormalized);
      shapePath.cubicTo(bezier2 + bezier2Offset + tieThickness, bezier1 + bezier1Offset + tieThickness, QPointF());

      // translate back
      double y = tieStart.y();
      const double offsetFactor = 0.2;
      if (staff()->isTabStaff(slurTie()->tick()))
          y += (_spatium * (slurTie()->up() ? -offsetFactor : offsetFactor));
      t.reset();
      t.translate(tieStart.x(), y);
      t.rotateRadians(tieAngle);
      path = t.map(path);
      shapePath = t.map(shapePath);
      ups(Grip::BEZIER1).p = t.map(bezier1);
      ups(Grip::BEZIER2).p = t.map(bezier2);
      ups(Grip::END).p = t.map(tieEndNormalized) - ups(Grip::END).off;
      ups(Grip::DRAG).p = t.map(tieDrag);
      ups(Grip::SHOULDER).p = t.map(tieShoulder);

      _shape.clear();
      QPointF start;
      start = t.map(start);

      qreal minH = qAbs(3.0 * w);
      int nbShapes = 15;
      const CubicBezier b(tieStart, ups(Grip::BEZIER1).pos(), ups(Grip::BEZIER2).pos(), ups(Grip::END).pos());
      for (int i = 1; i <= nbShapes; i++) {
            const QPointF point = b.pointAtPercent(i/float(nbShapes));
            QRectF re = QRectF(start, point).normalized();
            if (re.height() < minH) {
                  tieWidthInSp = (minH - re.height()) * .5;
                  re.adjust(0.0, -tieWidthInSp, 0.0, tieWidthInSp);
                  }
            _shape.add(re);
            start = point;
            }
      }

//---------------------------------------------------------
//   layoutSegment
//    adjust the y-position of the tie. this is called before adjustX()
//    p1, p2  are in System coordinates
//---------------------------------------------------------

void TieSegment::layoutSegment(const QPointF& p1, const QPointF& p2)
      {
      autoAdjustOffset = QPointF();
      shoulderHeightMin = 0.4;
      shoulderHeightMax = 1.3;

      setPos(QPointF());
      ups(Grip::START).p = p1;
      ups(Grip::END).p = p2;
      
      //Adjust Y pos to staff type offset before other calculations
      if (staffType())
            rypos() += staffType()->yoffset().val() * spatium();

      if (isNudged() || isEdited())
            return;

      QRectF bbox;
      if (p1.y() == p2.y()) {
            // for horizontal ties we can estimate the bbox using simple math instead of having to call
            // computeBezier() which uses a whole lot of trigonometry to draw the entire tie
            bbox.setX(p1.x());
            bbox.setWidth(p2.x() - p1.x());

            // The following is ripped from computeBezier()
            // TODO: refactor this into its own method
            qreal shoulderHeight = bbox.width() * 0.4 * 0.38;
            shoulderHeight = qBound(shoulderHeightMin * spatium(), shoulderHeight, shoulderHeightMax * spatium());
            //////////
            qreal actualHeight = 2 * (shoulderHeight + styleP(Sid::SlurMidWidth)) / 3;

            bbox.setY(p1.y() - (slurTie()->up() ? actualHeight : 0));
            bbox.setHeight(actualHeight);
            }
      else {
            computeBezier();
            bbox = path.boundingRect();
            }

      // instead of the above if-else, the more "accurate" way to do this is:
      // computeBezier();
      // bbox = path.boundingRect();

      Tie* t = toTie(slurTie());
      qreal sp = spatium();

      // adjust position to avoid staff line if necessary
      Staff* st          = staff();

      bool collideAbove = false;
      bool collideBelow = false;

      if (slurTie()->isTie() && st && !st->isTabStaff(slurTie()->tick())) {
            // multinote chords with ties need special handling
            // otherwise, adjusted tie might crowd an unadjusted tie unnecessarily
            Note* sn  = t->startNote();
            t->setTick(t->startNote()->tick());
            Chord* sc = sn ? sn->chord() : 0;
            if (sc && sc->notes().size() > 1) {
                  for (Note* note : sc->notes()) {
                        if (note == sn || !note->tieFor())
                              continue;
                        if (note->line() == sn->line() - 1 && t->up() == note->tieFor()->up())
                              collideAbove = true;
                        if (note->line() == sn->line() + 1 && t->up() == note->tieFor()->up())
                              collideBelow = true;
                        }
                  }
            }

      if (st && !st->isTabStaff(slurTie()->tick())) {
            qreal ld = st->lineDistance(tick()) * sp;
            qreal staffLineOffset = 0.125; // sp
            staffLineOffset += (styleP(Sid::staffLineWidth) / 2) / ld;
            bool up = slurTie()->up();
            qreal tieWidth = (styleP(Sid::SlurMidWidth)) / ld;
            qreal topY = bbox.top() / ld;
            qreal bottomY = bbox.bottom() / ld;
            qreal endpointY = up ? bottomY : topY;
            if (endpointY > 0 && endpointY < (st->lines(tick()) - 1) * st->lineDistance(tick())) {
                  // tie endpoints are inside the staff and may require adjustment
                  std::vector<qreal> endpointAnchors;
                  for (int i = 0; i < st->lines(tick()); ++i) {
                        endpointAnchors.push_back((qreal)i - staffLineOffset);
                        endpointAnchors.push_back((qreal)i + staffLineOffset);
                        }

                  size_t endpointAnchorIndex = 0;
                  qreal extraAdjust = 0;
                  if (!tie()->isInside()) {
                        // Find the nearest endpoint anchor
                        for (size_t i = 0; i < endpointAnchors.size(); i++) {
                              if (qAbs(endpointAnchors[i] - endpointY) <= qAbs(endpointAnchors[endpointAnchorIndex] - endpointY))
                                    endpointAnchorIndex = i;
                              else
                                    break;
                              }

                        qreal currentOffset = endpointAnchors[endpointAnchorIndex] - endpointY;
                        topY += currentOffset;
                        bottomY += currentOffset;
                        // Adjust for tie apogee colliding with staff lines
                        qreal insideTieTop = up ? topY : (bottomY - tieWidth);
                        qreal insideTieBottom = up ? (topY + tieWidth) : bottomY;
                        qreal tieTopWithMargin = insideTieTop - staffLineOffset;
                        qreal tieBottomWithMargin = insideTieBottom + staffLineOffset;
                        if ((tieTopWithMargin < 0 && tieBottomWithMargin > 0) || (int)tieTopWithMargin != (int)tieBottomWithMargin) {
                              qreal oldAnchorY = endpointAnchors[endpointAnchorIndex];
                              if (up) {
                                    endpointAnchorIndex--;
                                    tieBottomWithMargin += endpointAnchors[endpointAnchorIndex] - oldAnchorY; // update position of tie bottom
                                    if (endpointAnchorIndex & 1) {  // endpoints just below a line can be adjusted downwards
                                          extraAdjust = ((int)(tieBottomWithMargin + 1) - tieBottomWithMargin); // how far the inside of the tie is from the staff line
                                          extraAdjust = qMax(extraAdjust, 0.0); // ensure downward adjustment

                                          // clamp endpoints to at least 0.5sp of staff line
                                          qreal endpointDistanceFromUpperLine = (endpointAnchors[endpointAnchorIndex] + extraAdjust) - (int)endpointY;
                                          if (endpointDistanceFromUpperLine < 0.5) {
                                                extraAdjust += 0.5 - endpointDistanceFromUpperLine;
                                                shoulderHeightMin = 3 * (0.5 + (tieWidth / 2) + (staffLineOffset / 2)) / 2;
                                                }
                                          }
                                    }
                              else { // tie is down
                                    endpointAnchorIndex++;
                                    tieTopWithMargin += endpointAnchors[endpointAnchorIndex] - oldAnchorY; // update position of tie top
                                    if (!(endpointAnchorIndex & 1)) { // endpoint just above a line can be adjusted upwards
                                          extraAdjust = ((int)tieTopWithMargin - tieTopWithMargin);
                                          extraAdjust = qMin(extraAdjust, 0.0);

                                          // clamp endpoints to at least 0.5sp of staff line
                                          qreal endpointDistanceFromLowerLine = (int)(endpointY + 1)
                                                      - (endpointAnchors[endpointAnchorIndex] + extraAdjust);
                                          if (endpointDistanceFromLowerLine < 0.5) {
                                                // add enough to extraAdjust so that it becomes endpointY + 0.25
                                                extraAdjust -= 0.5 - endpointDistanceFromLowerLine; //((int)endpointAnchors[endpointAnchorIndex] + 0.5) - (endpointAnchors[endpointAnchorIndex]);
                                                shoulderHeightMin = 3 * (0.5 + (tieWidth / 2) + (staffLineOffset / 2)) / 2;
                                                }
                                          }
                                    }
                              }
                        }
                  else { // inside-tie
                        endpointAnchorIndex = tie()->startNote()->line() + (tie()->up() ? 0 : 1);
                        if ((up && endpointAnchorIndex & 1)
                           || (!up && !(endpointAnchorIndex & 1))) {
                              // tie endpoint is right below the line, so let's adjust the height so that the top clears the line
                              shoulderHeightMin = 3 * (staffLineOffset + (tieWidth / 2)) / 2;
                              }
                        else {
                              qreal insideTieTop = up ? topY : (bottomY - tieWidth);
                              qreal insideTieBottom = up ? (topY + tieWidth) : bottomY;
                              qreal tieTopWithMargin = insideTieTop;// -0.125;// -staffLineOffset;
                              qreal tieBottomWithMargin = insideTieBottom;// +0.125;// +staffLineOffset;
                              if ((tieTopWithMargin < 0 && tieBottomWithMargin > 0) || (int)tieTopWithMargin != (int)tieBottomWithMargin) {
                                    //shoulderHeightMin = 3 * (staffLineOffset + (tieWidth / 2)) / 2;
                                    shoulderHeightMax = 1 - staffLineOffset - (tieWidth / 2);
                                    }
                              }
                        if ((up && collideBelow) || (!up && collideAbove))
                              shoulderHeightMin = 3 * (staffLineOffset + (tieWidth / 2)) / 2;
                        if ((up && collideAbove) || (!up && collideBelow))
                              shoulderHeightMax = 1 - staffLineOffset - (tieWidth / 2);
                        }

                  qreal currentOffset = endpointAnchors[endpointAnchorIndex] - endpointY;
                  setAutoAdjust(QPointF(0, (currentOffset + extraAdjust) * ld));
                  }
            }
      }

//---------------------------------------------------------
//   finalizeSegment
//    compute the bezier and adjust the bbox for the curve
//---------------------------------------------------------

void TieSegment::finalizeSegment()
      {
      computeBezier();
      setbbox(path.boundingRect());
      }

void TieSegment::adjustX()
      {
      Note* sn = tie()->startNote();
      Note* en = tie()->endNote();
      Chord* sc = sn ? sn->chord() : nullptr;
      Chord* ec = en ? en->chord() : nullptr;

      bool beginGrace = (sn && sn->noteType() != NoteType::NORMAL);
      bool endGrace = (en && en->noteType() != NoteType::NORMAL);
      qreal offsetMargin = !(beginGrace || endGrace) ? spatium() * 0.25 : spatium() * 0.10;
      qreal collisionYMargin = spatium() * 0.25;

      qreal xo;

      if (isNudged() || isEdited())
            return;

      // ADJUST LEFT GRIP -----------
      if (sc && ((spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::BEGIN))) {
            // grips are in system coordinates, normalize to note position
            QPointF p1 = ups(Grip::START).p + QPointF(system()->pos().x() - sn->canvasX() + sn->headWidth(), 0);
            xo = 0;
            if (tie()->isInside()) {  // only adjust for inside-style ties
                  // for cross-voice collisions, we need a list of all chords at this tick
                  std::vector<Chord*> chords;
                  int strack = sc->staffIdx() * VOICES;
                  int etrack = sc->staffIdx() * VOICES + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (Chord* ch = sc->measure()->findChord(sc->tick(), track))
                              chords.push_back(ch);
                        }

                  for (Chord* chord : chords) {
                        qreal chordOffset = chord->x() - sc->x() - sn->x() - sn->width(); // sn for right-offset notes, width() to normalize to zero
                        // adjust for hooks
                        if (chord->hook() && chord->hook()->visible()) {
                              qreal hookHeight = chord->hook()->bbox().height();
                              // turn the hook upside down for downstems
                              qreal hookY = chord->hook()->pos().y() - (chord->up() ? 0 : hookHeight);
                              if (p1.y() > hookY - collisionYMargin  && p1.y() < hookY + hookHeight + collisionYMargin )
                                    xo = qMax(xo, chord->hook()->x() + chord->hook()->width() + chordOffset);
                              }

                        // adjust for stems
                        if (chord->stem() && chord->stem()->visible()) {
                              qreal stemLen = chord->stem()->bbox().height();
                              qreal stemY = chord->stem()->pos().y() - (chord->up() ? stemLen : 0);
                              if (p1.y() > stemY - collisionYMargin  && p1.y() < stemY + stemLen + collisionYMargin )
                                    xo = qMax(xo, chord->stem()->x() + chord->stem()->width() + chordOffset);
                              }

                        // adjust for ledger lines
                        for (LedgerLine* currLedger = chord->ledgerLines(); currLedger; currLedger = currLedger->next()) {
                              // search through ledger lines and see if any are within .5sp of tie start
                              if (qAbs(p1.y() - currLedger->y()) < spatium() * 0.5) {
                                    xo = qMax(xo, (currLedger->x() + currLedger->len() + chordOffset));
                                    break;
                                    }
                              }

                        for (auto note : chord->notes()) {
                              // adjust for dots
                              if (note->dots().size() > 0) {
                                    qreal dotY = note->pos().y() + note->dots().last()->y();
                                    if (qAbs(p1.y() - dotY) < spatium() * 0.5)
                                          xo = qMax(xo, note->x() + note->dots().last()->x() + note->dots().last()->width() + chordOffset);
                                    }

                              // adjust for note collisions
                              if (note == sn)
                                    continue;
                              qreal noteTop = note->y() + note->bbox().top();
                              qreal noteHeight = note->height();
                              if (p1.y() > noteTop && p1.y() < noteTop + noteHeight)
                                    xo = qMax(xo, note->x() + note->width() + chordOffset);
                              }
                        }
                  }
            xo = (beginGrace || endGrace ? 0.0 : xo) + offsetMargin;
            ups(Grip::START).p += QPointF(xo, 0);
            }

      // ADJUST RIGHT GRIP ----------
      if (sc && ((spannerSegmentType() == SpannerSegmentType::SINGLE || spannerSegmentType() == SpannerSegmentType::END))) {
            // grips are in system coordinates, normalize to note position
            QPointF p2 = ups(Grip::END).p + QPointF(system()->pos().x() - en->canvasX(), 0);
            xo = 0;
            if (tie()->isInside()) {
                  // for inter-voice collisions, we need a list of all notes from all voices
                  std::vector<Chord*> chords;
                  int strack = ec->staffIdx() * VOICES;
                  int etrack = ec->staffIdx() * VOICES + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (Chord* ch = ec->measure()->findChord(ec->tick(), track))
                              chords.push_back(ch);
                        }

                  for (Chord* chord : chords) {
                        qreal chordOffset = (ec->x() + en->x()) - chord->x(); // en->x() for right-offset notes
                        for (LedgerLine* currLedger = chord->ledgerLines(); currLedger; currLedger = currLedger->next()) {
                              // search through ledger lines and see if any are within .5sp of tie end
                              if (qAbs(p2.y() - currLedger->y()) < spatium() * 0.5)
                                    xo = qMin(xo, currLedger->x() - chordOffset);
                              }

                        if (chord->stem() && chord->stem()->visible()) {
                              // adjust for stems
                              qreal stemLen = chord->stem()->bbox().height();
                              qreal stemY = chord->stem()->pos().y() - (chord->up() ? stemLen : 0);
                              if (p2.y() > stemY - collisionYMargin  && p2.y() < stemY + stemLen + collisionYMargin )
                                    xo = qMin(xo, chord->stem()->x() - chordOffset);
                              }

                        for (Note* note : chord->notes()) {
                              // adjust for accidentals
                              Accidental* acc = note->accidental();
                              if (acc && acc->visible()) {
                                    qreal accTop = (note->y() + acc->y()) + acc->bbox().top();
                                    qreal accHeight = acc->height();
                                    if (p2.y() >= accTop && p2.y() <= accTop + accHeight)
                                          xo = qMin(xo, note->x() + acc->x() - chordOffset);
                                    }

                              if (note == en)
                                    continue;
                              // adjust for shifted notes (such as intervals of unison or second)
                              qreal noteTop = note->y() + note->bbox().top();
                              qreal noteHeight = note->headHeight();
                              if (p2.y() >= noteTop - collisionYMargin  && p2.y() <= noteTop + noteHeight + collisionYMargin )
                                    xo = qMin(xo, note->x() - chordOffset);
                              }
                        }
                  }
            xo = (beginGrace || endGrace ? 0.0 : xo) - offsetMargin;
            ups(Grip::END).p += QPointF(xo, 0);
            }
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
      const StaffType* stt = useTablature ? staff()->staffType(tick()) : 0;
      qreal _spatium    = spatium();
      qreal hw          = startNote()->tabHeadWidth(stt);   // if stt == 0, defaults to headWidth()
      qreal __up        = _up ? -1.0 : 1.0;
      // y offset for ties inside chord margins (typically multi-note chords): lined up with note top or bottom margin
      //    or outside (typically single-note chord): overlaps note and is above/below it
      // Outside: Tab: uses font size and may be asymmetric placed above/below line (frets ON or ABOVE line)
      //          Std: assumes notehead is 1 sp high, 1/2 sp above and 1/2 below line; add 1/4 sp to it
      // Inside:  Tab: 1/2 of Outside offset
      //          Std: use a fixed percentage of note width
      qreal yOffOutside = useTablature
            ? (_up ? stt->fretBoxY() : stt->fretBoxY() + stt->fretBoxH()) * magS()
            : 0.75 * _spatium * __up;
      qreal yOffInside  = useTablature ? yOffOutside * 0.5 : hw * .3 * __up;

      Chord* sc = startNote()->chord();
      Chord* ec = endNote() ? endNote()->chord() : nullptr;
      sp->system1 = sc->measure()->system();
      if (!sp->system1) {
            Measure* m = sc->measure();
            qDebug("No system: measure is %d has %d count %d", m->isMMRest(), m->hasMMRest(), m->mmRestCount());
            }

      qreal x1, y1;
      qreal x2, y2;

      // determine attachment points
      // similar code is used in Chord::layoutPitched()
      // to allocate extra space to enforce minTieLength
      // so keep these in sync

      sp->p1    = sc->pos() + sc->segment()->pos() + sc->measure()->pos();

      //------p1
      x1 = startNote()->pos().x() + hw;
      y1 = startNote()->pos().y();
      qreal xo = 0;
      if (sc->notes().size() > 1 || (ec && ec->notes().size() > 1)) {
            _isInside = true;
            xo = 0;  // the offset for these will be decided in TieSegment::adjustX()
            }
      else {
            _isInside = false;
            if (sc->stem() && sc->up() && _up) {
                  // usually, outside ties start in the middle of the notehead, but
                  // for up-ties on up-stems, we'll start at the end of the notehead
                  // to avoid the stem
                  xo = 0;
                  }
            else {
                  // start in the middle of the notehead for outside notes
                  xo = -(hw / 2);
            }
      }
      y1 += isInside() ? yOffInside : yOffOutside;
      sp->p1 += QPointF(x1 + xo, y1);

      //------p2
      y2 = y1;
      xo = 0;
      if (!ec) {
            sp->p2 = sp->p1 + QPointF(_spatium * 3, 0.0);
            sp->system2 = sp->system1;
            return;
            }
      sp->p2 = ec->pos() + ec->segment()->pos() + ec->measure()->pos();
      sp->system2 = ec->measure()->system();

      // force tie to be horizontal except for cross-staff or if there is a difference of line (tpc, clef, tpc)
      bool horizontal = startNote()->line() == endNote()->line() && sc->vStaffIdx() == ec->vStaffIdx();

      hw = endNote()->tabHeadWidth(stt);
      x2 = endNote()->x();
      if (!horizontal)
            y2 = endNote()->pos().y() + (isInside() ? yOffInside : yOffOutside);
      if (isInside())
            xo = 0.0;
      else {
            if (ec->stem() && !ec->up() && !_up) {
                  // as before, xo should account for stems that could get in the way
                  xo = 0;
                  }
            else {
                  // start in the middle of the notehead for outside notes
                  xo = -(hw / 2);
                  }
            }
      sp->p2 += QPointF(x2 - xo, y2);

      // adjust for cross-staff
      if (sc->vStaffIdx() != vStaffIdx() && sp->system1) {
            qreal diff = sp->system1->staff(sc->vStaffIdx())->y() - sp->system1->staff(vStaffIdx())->y();
            sp->p1.ry() += diff;
            }
      if (ec->vStaffIdx() != vStaffIdx() && sp->system2) {
            qreal diff = sp->system2->staff(ec->vStaffIdx())->y() - sp->system2->staff(vStaffIdx())->y();
            sp->p2.ry() += diff;
            }
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

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line()) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void Tie::calculateDirection()
      {
      Chord* c1   = startNote()->chord();
      Chord* c2   = endNote()->chord();
      Measure* m1 = c1->measure();
      Measure* m2 = c2->measure();

      if (_slurDirection == Direction::AUTO) {
            std::vector<Note*> notes = c1->notes();
            size_t n = notes.size();
            // if there are multiple voices, the tie direction goes on stem side
            if (m1->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks()))
                  _up = c1->up();
            else if (m2->hasVoices(c2->staffIdx(), c2->tick(), c2->actualTicks()))
                _up = c2->up();
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
                  // first, find pivot point in chord (below which all ties curve down and above which all ties curve up)
                  Note* pivotPoint = nullptr;
                  bool multiplePivots = false;
                  for (size_t i = 0; i < n - 1; ++i) {
                        if (!notes[i]->tieFor())
                              continue; // don't include notes that don't have ties
                        for (size_t j = i + 1; j < n; ++j) {
                              if (!notes[j]->tieFor())
                                    continue;
                              int noteDiff = compareNotesPos(notes[i], notes[j]);
                              if (!multiplePivots && qAbs(noteDiff) <= 1) {
                                    // TODO: Fix unison ties somehow--if noteDiff == 0 then we need to determine which of the unison is 'lower'
                                    if (pivotPoint) {
                                          multiplePivots = true;
                                          pivotPoint = nullptr;
                                          }
                                    else
                                          pivotPoint = noteDiff < 0 ? notes[i] : notes[j];
                                    }
                              }
                        }
                  if (!pivotPoint) {
                        // if the pivot point was not found (either there are no unisons/seconds or there are more than one),
                        // determine if this note is in the lower or upper half of this chord
                        int notesAbove = 0, tiesAbove = 0;
                        int notesBelow = 0, tiesBelow = 0;
                        int unisonNotes = 0, unisonTies = 0;
                        for (size_t i = 0; i < n; ++i) {
                              if (notes[i] == startNote())
                                    // skip counting if this note is the current note or if this note doesn't have a tie
                                    continue;
                              int noteDiff = compareNotesPos(startNote(), notes[i]);
                              if (noteDiff == 0) {  // unison
                                    unisonNotes++;
                                    if (notes[i]->tieFor())
                                          unisonTies++;
                                    }
                              if (noteDiff < 0) { // the note is above startNote
                                    notesAbove++;
                                    if (notes[i]->tieFor())
                                          tiesAbove++;
                                    }
                              if (noteDiff > 0) { // the note is below startNote
                                    notesBelow++;
                                    if (notes[i]->tieFor())
                                          tiesBelow++;
                                    }
                              }

                        if (tiesAbove == 0 && tiesBelow == 0 && unisonTies == 0) {
                              // this is the only tie in the chord.
                              if (notesAbove == notesBelow)
                                    _up = !c1->up();
                              else
                                    _up = (notesAbove < notesBelow);
                              }
                        else if (tiesAbove == tiesBelow)
                              // this note is dead center, so its tie should go counter to the stem direction
                              _up = !c1->up();
                        else
                              _up = (tiesAbove < tiesBelow);
                        }
                  else if (pivotPoint == startNote())
                        // the current note is the lower of the only second or unison in the chord; tie goes down.
                        _up = false;
                  else {
                        // if lower than the pivot, tie goes down, otherwise up
                        int noteDiff = compareNotesPos(startNote(), pivotPoint);
                        _up = (noteDiff >= 0);
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
      // do not layout ties in tablature if not showing back-tied fret marks
      StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
      if (st && st->isTabStaff() && !st->showBackTied()) {
            if (!segmentsEmpty())
                  eraseSpannerSegments();
            return nullptr;
            }
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
                  if (c1->measure()->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
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
            segment->finalizeSegment();
            return segment;
            }
      calculateDirection();

      SlurPos sPos;
      slurPos(&sPos); // get unadjusted x values and determine inside or outside

      setPos(0, 0);

      int n;
      if (sPos.system1 != sPos.system2) {
            n = 2;
            sPos.p2 = QPointF(system->lastNoteRestSegmentX(true), sPos.p1.y());
            }
      else
            n = 1;

      fixupSegments(n);
      TieSegment* segment = segmentAt(0);
      segment->setSystem(system); // Needed to populate System.spannerSegments
      segment->layoutSegment(sPos.p1, sPos.p2); // adjust vertically
      segment->setSpannerSegmentType(sPos.system1 != sPos.system2 ? SpannerSegmentType::BEGIN : SpannerSegmentType::SINGLE);
      segment->adjustX(); // adjust horizontally for inside-style ties

      segment->finalizeSegment(); // compute bezier and set bbox
      return segment;
      }

//---------------------------------------------------------
//   layoutBack
//    layout the second SpannerSegment of a split slur
//---------------------------------------------------------

TieSegment* Tie::layoutBack(System* system)
      {
      // do not layout ties in tablature if not showing back-tied fret marks
      StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
      if (st->isTabStaff() && !st->showBackTied()) {
            if (!segmentsEmpty())
                  eraseSpannerSegments();
            return nullptr;
            }

      SlurPos sPos;
      slurPos(&sPos);

      fixupSegments(2);
      TieSegment* segment = segmentAt(1);
      segment->setSystem(system);

      qreal x = system->firstNoteRestSegmentX(true);

      segment->layoutSegment(QPointF(x, sPos.p2.y()), sPos.p2);
      segment->setSpannerSegmentType(SpannerSegmentType::END);
      segment->adjustX();
      segment->finalizeSegment();
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
//printf("tie::endEdit\n");
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
}

