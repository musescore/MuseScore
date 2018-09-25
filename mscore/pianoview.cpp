//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "pianoview.h"
#include "libmscore/staff.h"
#include "piano.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/segment.h"
#include "libmscore/noteevent.h"

namespace Ms {

static const int MAP_OFFSET = 480;

//---------------------------------------------------------
//   pitch2y
//---------------------------------------------------------

static int pitch2y(int pitch)
      {
      static int tt[] = {
            12, 19, 25, 32, 38, 51, 58, 64, 71, 77, 84, 90
            };
      int y = (75 * keyHeight) - (tt[pitch % 12] + (7 * keyHeight) * (pitch / 12));
      if (y < 0)
            y = 0;
      return y;
      }

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

PianoItem::PianoItem(Note* n, NoteEvent* e)
   : QGraphicsRectItem(0), _note(n), _event(e)
      {
      setFlags(flags() | QGraphicsItem::ItemIsSelectable);
      setBrush(QBrush());
      updateValues();
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

QRectF PianoItem::updateValues()
      {
      QRectF r(rect().translated(pos()));
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      int pitch    = _note->pitch() + _event->pitch();
      int len      = ticks * _event->len() / 1000 + tieLen;

      setRect(0, 0, len, keyHeight/2);
      setSelected(_note->selected());

      setPos(_note->chord()->tick() + _event->ontime() * ticks / 1000 + MAP_OFFSET,
         pitch2y(pitch) + keyHeight / 4);

      return r | rect().translated(pos());
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PianoItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      painter->setPen(pen());
      painter->setBrush(isSelected() ? Qt::yellow : Qt::blue);
      painter->drawRect(boundingRect());
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

Pos PianoView::pix2pos(int x) const
      {
      x -= MAP_OFFSET;
      if (x < 0)
            x = 0;
      return Pos(staff->score()->tempomap(), staff->score()->sigmap(), x, _timeType);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int PianoView::pos2pix(const Pos& p) const
      {
      return p.time(_timeType) + MAP_OFFSET;
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (staff == 0)
            return;
      Score* _score = staff->score();
      setFrameShape(QFrame::NoFrame);

      QRectF r1;
      r1.setCoords(-1000000.0, 0.0, 480.0, 1000000.0);
      QRectF r2;
      r2.setCoords(ticks + MAP_OFFSET, 0.0, 1000000.0, 1000000.0);
      QColor bg(0x71, 0x8d, 0xbe);

      p->fillRect(r, bg);
      if (r.intersects(r1))
            p->fillRect(r.intersected(r1), bg.darker(150));
      if (r.intersects(r2))
            p->fillRect(r.intersected(r2), bg.darker(150));

      //
      // draw horizontal grid lines
      //
      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal kh = 13.0;
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      // int key = floor(y1 / 75);
      int key = floor(y1 / kh);
      qreal y = key * kh;

      for (; key < 75; ++key, y += kh) {
            if (y < y1)
                  continue;
            if (y > y2)
                  break;
            p->setPen(QPen((key % 7) == 5 ? Qt::lightGray : Qt::gray));
            p->drawLine(QLineF(x1, y, x2, y));
            }

      //
      // draw vertical grid lines
      //
      static const int mag[7] = {
            1, 1, 2, 5, 10, 20, 50
            };

      Pos pos1 = pix2pos(x1);
      Pos pos2 = pix2pos(x2);

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      int bar1, bar2, beat, tick;
      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      int mi = mag[magStep < 0 ? 0 : magStep];

      bar1 = (bar1 / mi) * mi;           // round down
      if (bar1 && mi >= 2)
            bar1 -= 1;
      bar2 = ((bar2 + mi - 1) / mi) * mi; // round up

      for (int bar = bar1; bar <= bar2;) {
            Pos stick(_score->tempomap(), _score->sigmap(), bar, 0, 0);
            if (magStep > 0) {
                  double x = double(pos2pix(stick));
                  if (x > 0) {
                        p->setPen(QPen(Qt::lightGray, 0.0));
                        p->drawLine(x, y1, x, y2);
                        }
                  else {
                        p->setPen(QPen(Qt::black, 0.0));
                        p->drawLine(x, y1, x, y1);
                        }
                  }
            else {
                  int z = stick.timesig().timesig().numerator();
                  for (int b = 0; b < z; b++) {
                        if (magStep == 0) {
                              Pos xx(_score->tempomap(), _score->sigmap(), bar, b, 0);
                              int xp = pos2pix(xx);
                              if (xp < 0)
                                    continue;
                              if (xp > 0) {
                                    p->setPen(QPen(b == 0 ? Qt::lightGray : Qt::gray, 0.0));
                                    p->drawLine(xp, y1, xp, y2);
                                    }
                              else {
                                    p->setPen(QPen(Qt::black, 0.0));
                                    p->drawLine(xp, y1, xp, y2);
                                    }
                              }
                        else {
                              int k;
                              if (magStep == -1)
                                    k = 2;
                              else if (magStep == -2)
                                    k = 4;
                              else if (magStep == -3)
                                    k = 8;
                              else if (magStep == -4)
                                    k = 16;
                              else
                                    k = 32;

                              int n = (MScore::division * 4) / stick.timesig().timesig().denominator();
                              for (int i = 0; i < k; ++i) {
                                    Pos xx(_score->tempomap(), _score->sigmap(), bar, b, (n * i)/ k);
                                    int xp = pos2pix(xx);
                                    if (xp < 0)
                                          continue;
                                    if (xp > 0) {
                                          p->setPen(QPen(i == 0 && b == 0 ? Qt::lightGray : Qt::gray, 0.0));
                                          p->drawLine(xp, y1, xp, y2);
                                          }
                                    else {
                                          p->setPen(QPen(Qt::black, 0.0));
                                          p->drawLine(xp, y1, xp, y2);
                                          }
                                    }
                              }
                        }
                  }
            if (bar == 0 && mi >= 2)
                  bar += (mi-1);
            else
                  bar += mi;
            }
      }

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

PianoView::PianoView()
   : QGraphicsView()
      {
      setFrameStyle(QFrame::NoFrame);
      setLineWidth(0);
      setMidLineWidth(0);
      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      _timeType = TType::TICKS;
      magStep   = 0;
      staff     = 0;
      chord     = 0;
      }

//---------------------------------------------------------
//   createLocators
//---------------------------------------------------------

void PianoView::createLocators()
      {
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            locatorLines[i] = new QGraphicsLineItem(QLineF(0.0, 0.0, 0.0, keyHeight * 75.0 * 5));
            QPen pen(lcColors[i]);
            pen.setWidth(2);
            locatorLines[i]->setPen(pen);
            locatorLines[i]->setZValue(1000+i);       // set stacking order
            locatorLines[i]->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            scene()->addItem(locatorLines[i]);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianoView::moveLocator(int i)
      {
      if (_locator[i].valid()) {
            locatorLines[i]->setVisible(true);
            qreal x = qreal(pos2pix(_locator[i]));
            locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      else
            locatorLines[i]->setVisible(false);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void PianoView::wheelEvent(QWheelEvent* event)
      {
      int step = event->delta() / 120;
      double xmag = transform().m11();
      double ymag = transform().m22();

      if (event->modifiers() == Qt::ControlModifier) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i) {
                        if (xmag > 10.0)
                              break;
                        scale(1.1, 1.0);
                        xmag *= 1.1;
                        }
                  }
            else {
                  for (int i = 0; i < -step; ++i) {
                        if (xmag < 0.001)
                              break;
                        scale(.9, 1.0);
                        xmag *= .9;
                        }
                  }
            emit magChanged(xmag, ymag);

            int tpix  = (480 * 4) * xmag;
            magStep = -5;
            if (tpix <= 4000)
                  magStep = -4;
            if (tpix <= 2000)
                  magStep = -3;
            if (tpix <= 1000)
                  magStep = -2;
            if (tpix <= 500)
                  magStep = -1;
            if (tpix <= 128)
                  magStep = 0;
            if (tpix <= 64)
                  magStep = 1;
            if (tpix <= 32)
                  magStep = 2;
            if (tpix <= 16)
                  magStep = 3;
            if (tpix <= 8)
                  magStep = 4;
            if (tpix <= 4)
                  magStep = 5;
            if (tpix <= 2)
                  magStep = 6;

            //
            // if xpos <= 0, then the scene is centered
            // there is no scroll bar anymore sending
            // change signals, so we have to do it here:
            //
            double xpos = -(mapFromScene(QPointF()).x());
            if (xpos <= 0)
                  emit xposChanged(xpos);
            }
      else if (event->modifiers() == Qt::ShiftModifier) {
            QWheelEvent we(event->pos(), event->delta(), event->buttons(), 0, Qt::Horizontal);
            QGraphicsView::wheelEvent(&we);
            }
      else if (event->modifiers() == 0) {
            QGraphicsView::wheelEvent(event);
            }
      else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i) {
                        if (ymag > 3.0)
                              break;
                        scale(1.0, 1.1);
                        ymag *= 1.1;
                        }
                  }
            else {
                  for (int i = 0; i < -step; ++i) {
                        if (ymag < 0.4)
                              break;
                        scale(1.0, .9);
                        ymag *= .9;
                        }
                  }
            emit magChanged(xmag, ymag);
            }
      }

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int PianoView::y2pitch(int y) const
      {
      int pitch;
      const int total = (10 * 7 + 5) * keyHeight;       // 75 Ganztonschritte
      y = total - y;
      int oct = (y / (7 * keyHeight)) * 12;
      static const char kt[] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2,
            3, 3, 3, 3, 3, 3, 3,
            4, 4, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6,
            7, 7, 7, 7, 7, 7,
            8, 8, 8, 8, 8, 8, 8,
            9, 9, 9, 9, 9, 9,
            10, 10, 10, 10, 10, 10, 10,
            11, 11, 11, 11, 11, 11, 11, 11, 11, 11
            };
      pitch = kt[y % 91] + oct;
      if (pitch < 0 || pitch > 127)
            pitch = -1;
      return pitch;
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoView::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF p(mapToScene(event->pos()));
      int pitch = y2pitch(int(p.y()));
      emit pitchChanged(pitch);
      int tick = int(p.x()) -480;
      if (tick < 0) {
            tick = 0;
            pos.setTick(tick);
            pos.setInvalid();
            }
      else
            pos.setTick(tick);
      emit posChanged(pos);
      QGraphicsView::mouseMoveEvent(event);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      pos.setInvalid();
      emit posChanged(pos);
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void PianoView::ensureVisible(int tick)
      {
      tick += MAP_OFFSET;
      QPointF pt = mapToScene(0, height() / 2);
      QGraphicsView::ensureVisible(qreal(tick), pt.y(), 240.0, 1.0);
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoView::setStaff(Staff* s, Pos* l)
      {
      staff    = s;
      _locator = l;
      setEnabled(staff != nullptr);
      if (!staff) {
            scene()->blockSignals(true);  // block changeSelection()
            scene()->clear();
            scene()->blockSignals(false);
            return;
            }

      pos.setContext(staff->score()->tempomap(), staff->score()->sigmap());
      Measure* lm = staff->score()->lastMeasure();
      ticks       = lm->tick() + lm->ticks();
      scene()->setSceneRect(0.0, 0.0, double(ticks + 960), keyHeight * 75);

      updateNotes();

      //
      // move to something interesting
      //
      QList<QGraphicsItem*> items = scene()->selectedItems();
      QRectF boundingRect;
      foreach (QGraphicsItem* item, items) {
            if (item->type() == PianoItemType)
                  boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
            }
      centerOn(boundingRect.center());
      horizontalScrollBar()->setValue(0);
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void PianoView::addChord(Chord* chord)
      {
      for (Chord* c : chord->graceNotes())
            addChord(c);
      for (Note* note : chord->notes()) {
            if (note->tieBack())
                  continue;
            for (NoteEvent& e : note->playEvents())
                  scene()->addItem(new PianoItem(note, &e));
            }
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoView::updateNotes()
      {
      scene()->blockSignals(true);  // block changeSelection()
      scene()->clearFocus();
      scene()->clear();
      createLocators();

      int staffIdx   = staff->idx();
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = staff->score()->firstSegment(st); s; s = s->next1(st)) {
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->isChord())
                        addChord(toChord(e));
                  }
            }
      for (int i = 0; i < 3; ++i)
            moveLocator(i);
      scene()->blockSignals(false);
      }
}

