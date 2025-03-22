//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "chordview.h"
#include "musescore.h"
#include "piano.h"

#include "libmscore/chord.h"
#include "libmscore/mscore.h"
#include "libmscore/note.h"
#include "libmscore/noteevent.h"
#include "libmscore/score.h"

namespace Ms {

static const int CHORD_MAP_OFFSET = 50;
static const int grip = 7;

//---------------------------------------------------------
//   GripItem
//---------------------------------------------------------

GripItem::GripItem(int type, ChordView* v)
   : QGraphicsRectItem()
      {
      _gripType = type;
      _view     = v;
      setFlags(flags() | ItemIsSelectable | ItemIsFocusable
         | ItemIgnoresTransformations | ItemIsMovable);
      setRect(-grip, -grip, grip*2, grip*2);
      setPos(0, 0);
      _event = 0;
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void GripItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
      {
      QPen pen(MScore::defaultColor);
      pen.setWidthF(2.0);
      p->setPen(pen);
      p->setBrush(isSelected() ? QBrush(Qt::blue) : Qt::NoBrush);
      p->drawRect(-grip, -grip, grip*2, grip*2);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void GripItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
      {
      QPointF np = mapToScene(event->pos());
      if (_event) {
            NoteEvent* ne = _event->event();
            if (_gripType == 0) {
                  qreal x = ChordView::pix2pos(np.x());
                  int x2  = ne->ontime() + ne->len();
                  ne->setOntime(x);
                  ne->setLen(x2 - x);
                  _event->setPos(ChordView::pos2pix(x), _event->pos().y());
                  setPos(0.0, pos().y());
                  GripItem* rg = _view->rightGrip();
                  rg->setPos(x2-x, rg->pos().y());
                  }
            else {
                  qreal x = np.x() - _event->pos().x();
                  setPos(x, pos().y());
                  _event->event()->setLen(x);
                  }
            _view->setDirty(true);
            _event->update();
            }
      }

//---------------------------------------------------------
//   ChordItem
//---------------------------------------------------------

ChordItem::ChordItem(ChordView* v, Note* n, NoteEvent* e)
   : QGraphicsRectItem(), _view(v), _note(n), _event(e)
      {
      setFlags(flags() | ItemIsSelectable | ItemIsMovable);
      _current  = false;
      int pitch = _event->pitch() + n->pitch();
      int len   = _event->len();
      setRect(0, 0, len, keyHeight/2);
      setBrush(QBrush());
      setSelected(n->selected());
      setData(0, QVariant::fromValue<void*>(n));

      setPos(ChordView::pos2pix(e->ontime()), ChordView::pitch2y(pitch) + keyHeight / 4);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ChordItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
      {
      QPointF np = mapToScene(event->pos());
      int pitch  = ChordView::y2pitch(np.y());
      int y = ChordView::pitch2y(pitch);
      setPos(pos().x(), y + keyHeight / 4);
      _event->setPitch(pitch - _note->pitch());
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ChordItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      int len = _event->len();
      painter->setPen(pen());
      if (_view->curNote() == _note)
            painter->setBrush(_current ? Qt::yellow : Qt::blue);
      else
            painter->setBrush(Qt::gray);
      painter->drawRect(0.0, 0.0, len, keyHeight / 2);
      }

//---------------------------------------------------------
//   setCurrent
//---------------------------------------------------------

void ChordItem::setCurrent(bool val)
      {
      _current = val;
      update();
      }

//---------------------------------------------------------
//   ChordView
//---------------------------------------------------------

ChordView::ChordView()
   : QGraphicsView()
      {
      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      // setMouseTracking(true);
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      magStep   = 2;
      chord     = 0;
      _curNote  = 0;
      _locator  = 0;
      _pos      = 0;
      locatorLine = nullptr;
      _evenGrid = true;
      lg        = 0;
      rg        = 0;
      curEvent  = 0;
      ticks     = 1000;
      _dirty    = false;

      scale(6.0, 1.0);
      QAction* a = getAction("delete");
      addAction(a);
      connect(a, SIGNAL(triggered()), SLOT(deleteItem()));
      connect(scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void ChordView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (chord == 0)
            return;
      QRectF r1(-DBL_MAX,                 0.0,           DBL_MAX, DBL_MAX);
      QRectF r2(ticks + CHORD_MAP_OFFSET, 0.0,           DBL_MAX, DBL_MAX);
      QRectF r3(-DBL_MAX,                 127*keyHeight, DBL_MAX, keyHeight);
      QRectF r4(ticks + CHORD_MAP_OFFSET, 127*keyHeight, DBL_MAX, keyHeight);

      QColor bg(0x71, 0x8d, 0xbe);
      QColor bg1 = bg.darker(150);
      QColor bg2 = bg.lighter(150);
      QColor bg3 = bg.darker(75);

      p->fillRect(r, bg);
      p->fillRect(r.intersected(r1), bg1);
      p->fillRect(r.intersected(r2), bg1);

      foreach (const Note* n, chord->notes()) {
            p->fillRect(QRect(CHORD_MAP_OFFSET, (127 - n->pitch()) * keyHeight,
               1000, keyHeight), bg2);
            }

      p->fillRect(r.intersected(r3), bg3);
      p->fillRect(r.intersected(r4), bg3);

      //---------------------------------------------------
      // draw horizontal grid lines
      //---------------------------------------------------

      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      int key = floor(y1 / keyHeight);
      qreal y = key * keyHeight;

      for (; key < 127; ++key, y += keyHeight) {
            if (y < y1)
                  continue;
            if (y > y2)
                  break;
            p->setPen(QPen((key % 6) == 5 ? Qt::lightGray : Qt::gray));
            p->drawLine(QLineF(x1, y, x2, y));
            }

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      if (_evenGrid) {
            for (int x = 0; x <= 1000; x += 50) {
                  if (x % 250)
                        p->setPen(Qt::lightGray);
                  else
                        p->setPen(Qt::black);
                  p->drawLine(pos2pix(x), y1, pos2pix(x), y2);
                  }
            }
      else {
            double step1 = 1000.0 / 3;
            double step2 = step1 / 4;
            for (int i = 0; i < 3; ++i) {
                  p->setPen(Qt::lightGray);
                  for (int k = 1; k < 4; ++k) {
                        int x = (int)lrint(i * step1 + k * step2);
                        p->drawLine(pos2pix(x), y1, pos2pix(x), y2);
                        }
                  p->setPen(Qt::black);
                  int x = (int)lrint(i * step1);
                  p->drawLine(pos2pix(x), y1, pos2pix(x), y2);
                  }
            }
      }

//---------------------------------------------------------
//   setChord
//---------------------------------------------------------

void ChordView::setChord(Chord* c)
      {
      chord    = c;
      _dirty   = false;
      _pos     = 0;
      _locator = 0;

      scene()->blockSignals(true);
      scene()->clear();
      locatorLine = new QGraphicsLineItem(QLineF(0.0, 0.0, 0.0, keyHeight * 127.0 * 5));
      QPen pen(Qt::red);
      pen.setWidth(2);
      locatorLine->setPen(pen);
      locatorLine->setZValue(1000);       // set stacking order
      locatorLine->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
      scene()->addItem(locatorLine);

      curEvent = 0;
      _curNote  = 0;
      foreach(Note* note, c->notes()) {
            if (note->selected() && _curNote == 0)
                  _curNote = note;
            int n = note->playEvents().size();
            for (int i = 0; i < n; ++i) {
                  NoteEvent* e = &note->playEvents()[i];
                  ChordItem* item = new ChordItem(this, note, e);
                  if (_curNote == note && curEvent == 0)
                        curEvent = item;
                  scene()->addItem(item);
                  }
            }
      lg = new GripItem(0, this);
      rg = new GripItem(1, this);
      lg->setZValue(100);
      rg->setZValue(101);
      lg->setVisible(false);
      rg->setVisible(false);
      scene()->addItem(lg);
      scene()->addItem(rg);

      scene()->blockSignals(false);
      setCurItem(curEvent);
      // selectionChanged();

      scene()->setSceneRect(0.0, 0.0, double(ticks + CHORD_MAP_OFFSET * 2), keyHeight * 127);

      moveLocator();

      //
      // move to something interesting
      //
      QList<QGraphicsItem*> items = scene()->selectedItems();
      QRectF boundingRect;
      foreach(QGraphicsItem* item, items) {
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
            }
      centerOn(boundingRect.center());
      scene()->update();
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void ChordView::moveLocator()
      {
      if (_locator >= 0) {
            locatorLine->setVisible(true);
            qreal x = qreal(pos2pix(_locator));
            locatorLine->setPos(QPointF(x, 0.0));
            }
      else
            locatorLine->setVisible(false);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ChordView::wheelEvent(QWheelEvent* event)
      {
      int step    = event->angleDelta().y() / 120;
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

            int tpix  = 1000 * xmag;
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            QWheelEvent we(event->position(), event->globalPosition(), event->pixelDelta().transposed(), event->angleDelta().transposed(),
                           event->buttons(), Qt::NoModifier, Qt::ScrollPhase::NoScrollPhase, false);
#else
            QWheelEvent we(event->pos(), event->delta(), event->buttons(), 0, Qt::Horizontal);
#endif
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

int ChordView::y2pitch(int y)
      {
      return 127 - (y / keyHeight);
      }

//---------------------------------------------------------
//   pitch2y
//---------------------------------------------------------

int ChordView::pitch2y(int pitch)
      {
      return keyHeight * (127 - pitch);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ChordView::mousePressEvent(QMouseEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            QPointF p(mapToScene(event->pos()));
            int pitch = y2pitch(int(p.y()));
            int tick  = int(p.x()) - CHORD_MAP_OFFSET;
            int tcks = 1000 - tick;
            foreach(const NoteEvent& e, _curNote->playEvents()) {
                  if (e.pitch() != pitch)
                        continue;
                  if (tick >= e.ontime() && tick < e.offtime()) {
                        return;     // cannot place an event here
                        }
                  int nticks = e.ontime() - tick;
                  if (nticks > 0)
                        tcks = qMin(tcks, nticks);
                  }

            NoteEvent ne;
            ne.setPitch(pitch);
            ne.setOntime(tick);
            ne.setLen(tcks);
            _curNote->playEvents().append(ne);
            NoteEvent* pne = &_curNote->playEvents()[_curNote->playEvents().size()-1];
            ChordItem* item = new ChordItem(this, _curNote, pne);
            scene()->addItem(item);
            setCurItem(item);
            _dirty = true;
            }
      QGraphicsView::mousePressEvent(event);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ChordView::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF p(mapToScene(event->pos()));
      int pitch = y2pitch(int(p.y()));
      emit pitchChanged(pitch);
      int tick = int(p.x()) - CHORD_MAP_OFFSET;
      if (tick < 0)
            _pos = -1;
      else
            _pos = tick;
      emit posChanged(_pos);
      QGraphicsView::mouseMoveEvent(event);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void ChordView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      emit posChanged(-1);
      _pos = -1;
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void ChordView::ensureVisible(int tick)
      {
      tick += CHORD_MAP_OFFSET;
      QPointF pt = mapToScene(0, height() / 2);
      QGraphicsView::ensureVisible(qreal(tick), pt.y(), 240.0, 1.0);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int ChordView::pos2pix(int pos)
      {
      return pos + CHORD_MAP_OFFSET;
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

int ChordView::pix2pos(int pix)
      {
      return pix - CHORD_MAP_OFFSET;
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void ChordView::selectionChanged()
      {
      QList<QGraphicsItem*> items = scene()->selectedItems();
      if (items.isEmpty())
            setCurItem(0);
      else {
            QGraphicsItem* item = items[0];
            if (item->type() == ChordTypeItem) {
                  ChordItem* ci = static_cast<ChordItem*>(item);
                  setCurItem(ci);
                  }
            }
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void ChordView::deleteItem()
      {
      QList<QGraphicsItem*> items = scene()->selectedItems();
      foreach(QGraphicsItem* item, items) {
            if (item->type() == ChordTypeItem) {
                  ChordItem* ci = static_cast<ChordItem*>(item);
                  if (curEvent == ci)
                        setCurItem(0);
                  NoteEvent* event = ci->event();
                  ci->note()->playEvents().removeOne(*event);

                  scene()->removeItem(item);
                  delete item;
                  _dirty = true;
                  }
            }
      }

//---------------------------------------------------------
//   setCurItem
//---------------------------------------------------------

void ChordView::setCurItem(ChordItem* item)
      {
      if (curEvent)
            curEvent->setCurrent(false);
      curEvent = item;
      bool visible = item != 0;
      lg->setVisible(visible);
      rg->setVisible(visible);
      lg->setEvent(item);
      rg->setEvent(item);
      lg->setParentItem(item);
      rg->setParentItem(item);
      if (visible) {
            lg->setPos(0,   keyHeight / 4);
            rg->setPos(item->event()->len(), keyHeight / 4);
            item->setCurrent(true);
            if (item->note() != _curNote) {
                  _curNote = item->note();
                  _curNote->score()->select(_curNote);
                  _curNote->score()->update();
                  }
            }
      }
}

