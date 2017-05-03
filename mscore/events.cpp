//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreview.h"
#include "magbox.h"
#include "musescore.h"
#include "seq.h"
#include "texttools.h"
#include "fotomode.h"
#include "libmscore/score.h"
#include "libmscore/keysig.h"
#include "libmscore/segment.h"
#include "libmscore/utils.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/stafflines.h"

namespace Ms {

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool ScoreView::event(QEvent* event)
      {
      if (event->type() == QEvent::KeyPress && editData.element) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
                  if (editData.element->isText())
                        return true;
                  bool rv = true;
                  if (ke->key() == Qt::Key_Tab) {
                        rv = editData.element->nextGrip(editData);
                        updateGrips();
                        _score->update();
                        if (rv)
                              return true;
                        }
                  else if (ke->key() == Qt::Key_Backtab)
                        rv = editData.element->prevGrip(editData);
                  updateGrips();
                  _score->update();
                  if (rv)
                        return true;
                  }
            }
//      else if (event->type() == CloneDrag) {
//TODO:drag            Element* e = static_cast<CloneEvent*>(event)->element();
//            cloneElement(e);
//            }
      else if (event->type() == QEvent::Gesture) {
            return gestureEvent(static_cast<QGestureEvent*>(event));
            }
      else if (event->type() == QEvent::MouseButtonPress && qApp->focusWidget() != this) {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton)
                  this->setFocus();
            }
      return QWidget::event(event);
      }

//---------------------------------------------------------
//   gestureEvent
//    fired on touchscreen gestures as well as Mac touchpad gestures
//---------------------------------------------------------

bool ScoreView::gestureEvent(QGestureEvent *event)
      {
      if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
            // Zoom in/out when receiving a pinch gesture
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);

            static qreal magStart = 1.0;
            if (pinch->state() == Qt::GestureStarted) {
                  magStart = lmag();
                  }
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
                  // On Windows, totalScaleFactor() contains the net magnification.
                  // On OS X, totalScaleFactor() is 1, and scaleFactor() contains the net magnification.
                  qreal value = pinch->totalScaleFactor();
                  if (value == 1) {
                        value = pinch->scaleFactor();
                        }
                  zoom(magStart*value, pinch->centerPoint());
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ScoreView::wheelEvent(QWheelEvent* event)
       {
#define PIXELSSTEPSFACTOR 5

      QPoint pixelsScrolled = event->pixelDelta();
      QPoint stepsScrolled  = event->angleDelta();

      int dx = 0, dy = 0, n = 0;
      qreal nReal = 0.0;

      if (!pixelsScrolled.isNull()) {
            dx = pixelsScrolled.x();
            dy = pixelsScrolled.y();
            nReal = static_cast<qreal>(dy) / PIXELSSTEPSFACTOR;
            }
      else if (!stepsScrolled.isNull()) {
            dx = static_cast<qreal>(stepsScrolled.x()) * qMax(2, width() / 10) / 120;
            dy = static_cast<qreal>(stepsScrolled.y()) * qMax(2, height() / 10) / 120;
            nReal = static_cast<qreal>(stepsScrolled.y()) / 120;
            }

      n = (int) nReal;

      //this functionality seems currently blocked by the context menu
      if (event->buttons() & Qt::RightButton) {
            bool up = n > 0;
            if (!up)
                  n = -n;
            score()->startCmd();
            for (int i = 0; i < n; ++i)
                  score()->upDown(up, UpDownMode::CHROMATIC);
            score()->endCmd();
            return;
            }

      if (event->modifiers() & Qt::ControlModifier) { // Windows touch pad pinches also execute this
            QApplication::sendPostedEvents(this, 0);
            zoomStep(nReal, event->pos());
            return;
            }

      //make shift+scroll go horizontally
      if (event->modifiers() & Qt::ShiftModifier && dx == 0) {
            dx = dy;
            dy = 0;
            }

      if (dx == 0 && dy == 0)
            return;

      constraintCanvas(&dx, &dy);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();

      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit viewRectChanged();
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void ScoreView::resizeEvent(QResizeEvent* /*ev*/)
      {
      if (_magIdx != MagIdx::MAG_FREE)
            setMag(mscore->getMag(this));
      }

//---------------------------------------------------------
//   focusInEvent
//---------------------------------------------------------

void ScoreView::focusInEvent(QFocusEvent* event)
      {
      if (this != mscore->currentScoreView())
         mscore->setCurrentScoreView(this);

      if (mscore->splitScreen()) {
            if (!focusFrame) {
                  focusFrame = new QFocusFrame;
                  QPalette p(focusFrame->palette());
                  p.setColor(QPalette::WindowText, Qt::blue);
                  focusFrame->setPalette(p);
                  }
            focusFrame->setWidget(static_cast<QWidget*>(this));
            }
      QWidget::focusInEvent(event);
      }

//---------------------------------------------------------
//   focusOutEvent
//---------------------------------------------------------

void ScoreView::focusOutEvent(QFocusEvent* event)
      {
      if (focusFrame)
            focusFrame->setWidget(0);
      QWidget::focusOutEvent(event);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ScoreView::mouseMoveEvent(QMouseEvent* me)
      {
      if (editData.buttons == Qt::NoButton)
            return;
      switch (state) {
            case ViewState::NORMAL:
                  if (!editData.element && (me->modifiers() & Qt::ShiftModifier))
                        changeState(ViewState::LASSO);
                  else if (editData.element && !(me->modifiers()))
                        changeState(ViewState::DRAG_OBJECT);
                  else
                        changeState(ViewState::DRAG);
                  break;

            case ViewState::NOTE_ENTRY:
                  dragNoteEntry(me);
                  break;

            case ViewState::DRAG:
            case ViewState::FOTO_DRAG:
                  dragScoreView(me);
                  break;

            case ViewState::DRAG_OBJECT:
            case ViewState::FOTO_DRAG_OBJECT:
                  doDragElement(me);
                  break;

            case ViewState::DRAG_EDIT:
            case ViewState::FOTO_DRAG_EDIT:
                  doDragEdit(me);
                  break;

            case ViewState::LASSO:
                  doDragLasso(me);
                  break;

            default:
                  break;
            }
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void ScoreView::mouseReleaseEvent(QMouseEvent*)
      {
      editData.buttons = Qt::NoButton;
      if (seq)
            seq->stopNoteTimer();
      switch (state) {
            case ViewState::DRAG:
            case ViewState::DRAG_OBJECT:
            case ViewState::LASSO:
                  changeState(ViewState::NORMAL);
                  break;
            case ViewState::DRAG_EDIT:
                  changeState(ViewState::EDIT);
                  break;
            case ViewState::FOTO_DRAG:
            case ViewState::FOTO_DRAG_EDIT:
            case ViewState::FOTO_DRAG_OBJECT:
                  changeState(ViewState::FOTO);
                  break;
            case ViewState::NORMAL:
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   mousePressEventNormal
//    handle mouse press event for NORMAL mode
//---------------------------------------------------------

void ScoreView::mousePressEventNormal(QMouseEvent* ev)
      {
      Qt::KeyboardModifiers keyState = ev->modifiers();
      SelectType st = SelectType::SINGLE;
      if (keyState == Qt::NoModifier)
            st = SelectType::SINGLE;
      else if (keyState & Qt::ShiftModifier)
            st = SelectType::RANGE;
      else if (keyState & Qt::ControlModifier)
            st = SelectType::ADD;
      Element* e = editData.element;
      if (e) {
            if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
                  cloneElement(e);
                  return;
                  }
            if (e->isKeySig() && (keyState != Qt::ControlModifier) && st == SelectType::SINGLE) {
                  // special case: select for all staves
                  Segment* s = toKeySig(e)->segment();
                  bool first = true;
                  for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
                        Element* e = s->element(staffIdx * VOICES);
                        if (e) {
                              e->score()->select(e, first ? SelectType::SINGLE : SelectType::ADD);
                              first = false;
                              }
                        }
                  }
            else {
                  if (st == SelectType::ADD && e->selected())
                        e->score()->deselect(e);
                  else
                        e->score()->select(e, st, -1);
                  }
            if (e->isText())
                  changeState(ViewState::EDIT);
            else if (e->isNote())
                  mscore->play(e);
            if (e) {
                  _score = e->score();
                  _score->setUpdateAll();
                  }
            }
      else {
            // special case: chacke if measure is selected
            int staffIdx;
            Measure* m = _score->pos2measure(editData.startMove, &staffIdx, 0, 0, 0);
            if (m && m->staffLines(staffIdx)->abbox().contains(editData.startMove)) {
                  _score->select(m, st, staffIdx);
                  _score->setUpdateAll();
                  }
            else
                  _score->deselectAll();
            }
      _score->update();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ScoreView::mousePressEvent(QMouseEvent* ev)
      {
      editData.startMove = toLogical(ev->pos());
      editData.buttons   = ev->buttons();
      Element* e         = elementNear(editData.startMove);
      qDebug("%s", e ? e->name() : "--");

      switch (state) {
            case ViewState::NORMAL:
                  if (ev->button() == Qt::RightButton)   // context menu?
                        break;
                  editData.element = e;
                  mousePressEventNormal(ev);
                  break;

            case ViewState::FOTO:
                  editData.element = _foto;
                  if (editElementDragTransition(ev))
                        changeState(ViewState::FOTO_DRAG_EDIT);
                  else if (_foto->canvasBoundingRect().contains(editData.startMove))
                        changeState(ViewState::FOTO_DRAG_OBJECT);
                  else
                        changeState(ViewState::FOTO_DRAG);
                  break;

            case ViewState::NOTE_ENTRY:
                  noteEntryButton(ev);
                  break;

            case ViewState::EDIT:
                  if (editElementDragTransition(ev))
                        changeState(ViewState::DRAG_EDIT);
                  else {
                        endEdit();
                        editData.element = e;
                        if (e && e->isText()) {
                              changeState(ViewState::EDIT);
                              _score->update();
                              mscore->endCmd();
                              }
                        else {
                              changeState(ViewState::NORMAL);
                              mousePressEventNormal(ev);
                              }
                        }
                  break;

            default:
                  qDebug("mousePressEvent in state %d", int(state));
                  break;
            }
      }

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void ScoreView::mouseDoubleClickEvent(QMouseEvent* me)
      {
      if (state == ViewState::NORMAL) {
            QPointF p = toLogical(me->pos());
            Element* e = elementNear(p);
            if (e && e->isEditable()) {
                  startEditMode(e);
                  changeState(ViewState::EDIT);
                  }
            }
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void ScoreView::keyPressEvent(QKeyEvent* ev)
      {
      if (state != ViewState::EDIT)
            return;
      editData.view      = this;
      editData.key       = ev->key();
      editData.modifiers = ev->modifiers();
      editData.s         = ev->text();

      if (MScore::debugMode)
            qDebug("keyPressEvent key 0x%02x(%c) mod 0x%04x <%s> nativeKey 0x%02x scancode %d",
               editData.key, editData.key, int(editData.modifiers), qPrintable(editData.s), ev->nativeVirtualKey(), ev->nativeScanCode());
      _score->startCmd();
      _keyPressEvent(ev);
      _score->endCmd();
      }

void ScoreView::_keyPressEvent(QKeyEvent* ev)
      {
      if (editData.element->isLyrics()) {
            if (editKeyLyrics(ev))
                  return;
            }
      else if (editData.element->isHarmony()) {
            if (editData.key == Qt::Key_Space && !(editData.modifiers & CONTROL_MODIFIER)) {
                  harmonyBeatsTab(true, editData.modifiers & Qt::ShiftModifier);
                  return;
                  }
            }
      else if (editData.element->isFiguredBass()) {
            if (editData.key == Qt::Key_Space && !(editData.modifiers & CONTROL_MODIFIER)) {
                  figuredBassTab(false, editData.modifiers & Qt::ShiftModifier);
                  return;
                  }
            }

#ifdef Q_OS_WIN // Japenese IME on Windows needs to know when Contrl/Alt/Shift/CapsLock is pressed while in predit
      if (editData.element->isText()) {
            Text* text = toText(editData.element);
            if (text->cursor(editData)->format()->preedit() && QGuiApplication::inputMethod()->locale().script() == QLocale::JapaneseScript &&
                ((editData.key == Qt::Key_Control || (editData.modifiers & Qt::ControlModifier)) ||
                 (editData.key == Qt::Key_Alt     || (editData.modifiers & Qt::AltModifier)) ||
                 (editData.key == Qt::Key_Shift   || (editData.modifiers & Qt::ShiftModifier)) ||
                 (editData.key == Qt::Key_CapsLock))) {
                  return; // musescore will ignore this key event so that the IME can handle it
                  }
            }
#endif

      if (!((editData.modifiers & Qt::ShiftModifier) && (editData.key == Qt::Key_Backtab))) {
            if (editData.element->edit(editData)) {
                  if (editData.element->isText())
                        mscore->textTools()->updateTools(editData);
                  updateGrips();
                  return;
                  }
            }
      QPointF delta;
      qreal _spatium = editData.element->spatium();

      qreal xval, yval;
      if (editData.element->isBeam()) {
            xval = 0.25 * _spatium;
            if (editData.modifiers & Qt::ControlModifier)
                  xval = _spatium;
            else if (editData.modifiers & Qt::AltModifier)
                  xval = 4 * _spatium;
            }
      else {
            xval = MScore::nudgeStep * _spatium;
            if (editData.modifiers & Qt::ControlModifier)
                  xval = MScore::nudgeStep10 * _spatium;
            else if (editData.modifiers & Qt::AltModifier)
                  xval = MScore::nudgeStep50 * _spatium;
            }
      yval = xval;

      if (mscore->vRaster()) {
            qreal vRaster = _spatium / MScore::vRaster();
            if (yval < vRaster)
                  yval = vRaster;
            }
      if (mscore->hRaster()) {
            qreal hRaster = _spatium / MScore::hRaster();
            if (xval < hRaster)
                  xval = hRaster;
            }
      // TODO: if raster, then xval/yval should be multiple of raster

      switch (editData.key) {
            case Qt::Key_Left:
                  delta = QPointF(-xval, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(xval, 0);
                  break;
            case Qt::Key_Up:
                  delta = QPointF(0, -yval);
                  break;
            case Qt::Key_Down:
                  delta = QPointF(0, yval);
                  break;
            default:
                  ev->ignore();
                  return;
            }
      editData.init();
      editData.delta   = delta;
      editData.view    = this;
      editData.hRaster = mscore->hRaster();
      editData.vRaster = mscore->vRaster();
      if (editData.curGrip != Grip::NO_GRIP && int(editData.curGrip) < editData.grips)
            editData.pos = editData.grip[int(editData.curGrip)].center() + delta;
      editData.element->startEditDrag(editData);
      editData.element->editDrag(editData);
      editData.element->endEditDrag(editData);
      updateGrips();
      }

//---------------------------------------------------------
//   keyReleaseEvent
//---------------------------------------------------------

void ScoreView::keyReleaseEvent(QKeyEvent* ev)
      {
      if (state == ViewState::EDIT) {
            auto modifiers = Qt::ControlModifier | Qt::ShiftModifier;
            if (editData.element->isText() && ((ev->modifiers() & modifiers) == 0)) {
                  Text* text = toText(editData.element);
                  text->endHexState();
                  ev->accept();
                  update();
                  }
            }
      }

//---------------------------------------------------------
//   contextPopup
//---------------------------------------------------------

void ScoreView::contextMenuEvent(QContextMenuEvent* ev)
      {
      QPoint gp          = ev->globalPos();
      editData.startMove = toLogical(ev->pos());
      Element* e         = elementNear(editData.startMove);
      if (e) {
            if (!e->selected()) {
                  // bool control = (ev->modifiers() & Qt::ControlModifier) ? true : false;
                  // _score->select(e, control ? SelectType::ADD : SelectType::SINGLE, 0);
                  // editData.element = e;
                  // select(ev);
                  }
            if (seq)
                  seq->stopNotes();       // stop now because we dont get a mouseRelease event
            objectPopup(gp, e);
            }
      else {
            int staffIdx;
            Measure* m = _score->pos2measure(editData.startMove, &staffIdx, 0, 0, 0);
            if (m && m->staffLines(staffIdx)->abbox().contains(editData.startMove))
                  measurePopup(gp, m);
            else {
                  QMenu* popup = new QMenu();
                  popup->addAction(getAction("edit-style"));
                  popup->addAction(getAction("page-settings"));
                  popup->addAction(getAction("load-style"));
                  _score->update();
                  popup->popup(gp);
                  }
            }
      ev->accept();
      }

//---------------------------------------------------------
//   escapeCmd
//---------------------------------------------------------

void ScoreView::escapeCmd()
      {
      switch (state) {
            case ViewState::EDIT:
                  endEdit();
                  changeState(ViewState::NORMAL);
                  break;
            case ViewState::FOTO:
            case ViewState::NOTE_ENTRY:
            case ViewState::PLAY:
                  changeState(ViewState::NORMAL);
                  break;
            default:
                  break;
            }
      }

}    // namespace Ms

