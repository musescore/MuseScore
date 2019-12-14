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
#include "tourhandler.h"
#include "scoreaccessibility.h"
#include "libmscore/score.h"
#include "libmscore/keysig.h"
#include "libmscore/segment.h"
#include "libmscore/utils.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/stafflines.h"
#include "libmscore/chord.h"
#include "libmscore/shadownote.h"
#include "libmscore/repeatlist.h"
#include "libmscore/select.h"
#include "libmscore/staff.h"

namespace Ms {

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool ScoreView::event(QEvent* event)
      {
      switch (event->type()) {
            case QEvent::KeyPress: {
                  QKeyEvent* ke = static_cast<QKeyEvent*>(event);
                  const int key = ke->key();
                  if (key != Qt::Key_Tab && key != Qt::Key_Backtab)
                        break;

                  if (textEditMode()) {
                        // block Tab/Backtab in text editing mode
                        return true;
                        }

                  if (editMode() || editData.grips) {
                        if (ke->key() == Qt::Key_Tab)
                              editData.element->nextGrip(editData);
                        else
                              editData.element->prevGrip(editData);
                        updateGrips();
                        _score->update();
                        return true;
                        }
                  }
                  break;
            case QEvent::ShortcutOverride: {
                  QKeyEvent* ke = static_cast<QKeyEvent*>(event);
                  switch (ke->key()) {
                        case Qt::Key_Left:
                        case Qt::Key_Right:
                        case Qt::Key_Up:
                        case Qt::Key_Down: {
                              if (!editData.grips)
                                    break;
                              const auto m = ke->modifiers();
                              // KeypadModifier is necessary on MacOS as arrow keys seem to always
                              // trigger that modifier there. However it would probably be appropriate
                              // to allow it on other systems too.
                              constexpr auto allowedModifiers = Qt::ShiftModifier | Qt::KeypadModifier;
                              if ((m & ~allowedModifiers) == 0) {
                                    ke->accept();
                                    return true;
                                    }
                              }
                              break;
                        default:
                              break;
                        }
                  }
                  break;
            case QEvent::Gesture:
                  return gestureEvent(static_cast<QGestureEvent*>(event));
            case QEvent::MouseButtonPress:
                  if (qApp->focusWidget() != this) {
                        QMouseEvent* me = static_cast<QMouseEvent*>(event);
                        if (me->button() == Qt::LeftButton)
                              this->setFocus();
                        }
                  break;
            default:
                  break;
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
      emit sizeChanged();

      // The score may need to be repositioned now.
      // So figure out how far it needs to move in each direction...
      int dx = 0, dy = 0;
      constraintCanvas(&dx, &dy);

      if (dx == 0 && dy == 0)
            return;

      // ...and adjust its position accordingly.
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();

      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit viewRectChanged();
      emit offsetChanged(_matrix.dx(), _matrix.dy());
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
//   startTextEditingOnMouseRelease
//---------------------------------------------------------

bool ScoreView::startTextEditingOnMouseRelease(QMouseEvent* mouseEvent)
      {
      if (!editData.element || mouseEvent->button() != Qt::LeftButton)
            return false;

      if (!(editData.element->isEditable() && editData.element->isTextBase()))
            return false;

      TextBase* textBase = toTextBase(editData.element);

      if (!editData.element->canvasBoundingRect().contains(toLogical(mouseEvent->pos()))) {
            // mouse up OUTSIDE textbase - clear any priming
            textBase->setPrimed(false);
            return false;
            }

      if (!textBase->isPrimed()) {
            // mouse up INSIDE textBase - set priming
            textBase->setPrimed(true);
            return false;
      }

      //mouse up INSIDE primed textBase - start editing
      startEditMode(textBase);
      setCursor(QCursor(Qt::IBeamCursor));
      textBase->setPrimed(false);
      return true;
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void ScoreView::mouseReleaseEvent(QMouseEvent* mouseEvent)
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
                  if (editData.element && editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit) {
                        changeState(ViewState::NORMAL);
                        break;
                        }
                  changeState(ViewState::EDIT);
                  break;
            case ViewState::FOTO_DRAG:
            case ViewState::FOTO_DRAG_EDIT:
            case ViewState::FOTO_DRAG_OBJECT:
                  changeState(ViewState::FOTO);
                  break;
            case ViewState::NORMAL:
                  if (startTextEditingOnMouseRelease(mouseEvent))
                        break;

                  if (modifySelection) {
                        _score->select(elementToSelect);
                        modifySelection = false;
                        elementToSelect = nullptr;
                        _score->update();
                        mscore->endCmd();
                        }
                  break;
            case ViewState::EDIT:
            case ViewState::NOTE_ENTRY:
            case ViewState::PLAY:
            case ViewState::ENTRY_PLAY:
            case ViewState::FOTO:
                  break;
            case ViewState::FOTO_LASSO:
                  changeState(ViewState::FOTO);
                  break;
            }
      }

//---------------------------------------------------------
//   mousePressEventNormal
//    handle mouse press event for NORMAL mode
//---------------------------------------------------------

void ScoreView::mousePressEventNormal(QMouseEvent* ev)
      {
      _score->masterScore()->cmdState().reset();      // DEBUG: should not be necessary
      modifySelection = false;
      elementToSelect = nullptr;

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
                        Element* ee = s->element(staffIdx * VOICES);
                        if (ee) {
                              ee->score()->select(ee, first ? SelectType::SINGLE : SelectType::ADD);
                              first = false;
                              }
                        }
                  }
            else {
                  if (st == SelectType::ADD) {
                        // e is the top element in stacking order,
                        // but we want to replace it with "first non-measure element after a selected element"
                        // (if such an element exists)
                        QList<Element*> ll = elementsNear(editData.startMove);
                        bool found = false;
                        for (Element* ee : ll) {
                              if (found) {
                                    e = ee;
                                    if (!e->isMeasure())
                                          break;
                                    }
                              else if (ee->selected()) {
                                    found = true;
                                    ee->score()->deselect(ee);
                                    e = nullptr;
                                    }
                              }
                        }
                  if (e) {
                        if (!e->selected())
                              e->score()->select(e, st, -1);
                        else if (st != SelectType::ADD) {
                              modifySelection = true;
                              elementToSelect = e;
                              }
                        }
                  }
            if (e && e->isNote()) {
                  e->score()->updateCapo();
                  mscore->play(e);
                  }
            if (e) {
                  _score = e->score();
                  _score->setUpdateAll();
                  }
            }
      else {
            // special case: check if measure is selected
            int staffIdx;
            Measure* m = _score->pos2measure(editData.startMove, &staffIdx, 0, 0, 0);
            if (m) {
                  QRectF r = m->staffLines(staffIdx)->canvasBoundingRect();
                  if (_score->staff(staffIdx)->lines(m->tick()) == 1) {
                        r.setHeight(m->spatium() * 2);
                        r.translate(0.0, -m->spatium());
                        }
                  if (r.contains(editData.startMove)) {
                        //TourHandler::startTour("select-tour");
                        _score->select(m, st, staffIdx);
                        _score->setUpdateAll();
                        }
                  else if (st != SelectType::ADD)
                        modifySelection = true;
                  }
            else if (st != SelectType::ADD)
                  modifySelection = true;
            }
      _score->update();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ScoreView::mousePressEvent(QMouseEvent* ev)
      {

      if (tripleClickPending) {
            if (textEditMode()) {
                  TextBase* textBase = toTextBase(editData.element);
                  textBase->multiClickSelect(editData, MultiClick::Triple);
                  mscore->textTools()->updateTools(editData);
                  textBase->endHexState(editData);
                  update();
                  return;
                  }
            }

      editData.startMovePixel = ev->pos();
      editData.startMove = toLogical(ev->pos());
      editData.lastPos   = editData.startMove;
      editData.pos       = editData.startMove;
      editData.buttons   = ev->buttons();
      editData.modifiers = qApp->keyboardModifiers();

      bool gripFound = false;
      if (editData.element && editData.grips && ev->button() == Qt::LeftButton) {
            switch (state) {
                  case ViewState::NORMAL:
                  case ViewState::EDIT:
                  case ViewState::FOTO:
                        {
                        const qreal a = editData.grip[0].width() * 0.5;
                        for (int i = 0; i < editData.grips; ++i) {
                              if (editData.grip[i].adjusted(-a, -a, a, a).contains(editData.startMove)) {
                                    editData.curGrip = Grip(i);
                                    updateGrips();
                                    score()->update();
                                    gripFound = true;
                                    break;
                                    }
                              }

                        if (!gripFound)
                              editData.curGrip = Grip::NO_GRIP;
                        }
                        break;
                  default:
                        break;
                  }
            }

      switch (state) {
            case ViewState::NORMAL:
                  if (gripFound)
                        break;
                  if (ev->button() == Qt::RightButton)   // context menu?
                        break;

                  if (editData.element
                      && editData.element->isEditable()
                      && editData.element->isTextBase()
                      && !editData.element->canvasBoundingRect().contains(toLogical(ev->pos()))) {
                        // mouse down OUTSIDE of textBase - clear priming -
                        toTextBase(editData.element)->setPrimed(false);
                        }

                  setEditElement(elementNear(editData.startMove));
                  mousePressEventNormal(ev);
                  break;

            case ViewState::FOTO: {
                  if (ev->buttons() & Qt::RightButton)
                        break;
                  setEditElement(_foto);

                  if (gripFound)
                        changeState(ViewState::FOTO_DRAG_EDIT);
                  else if (_foto->canvasBoundingRect().contains(editData.startMove))
                        changeState(ViewState::FOTO_DRAG_OBJECT);
                  else if (ev->modifiers() & Qt::ShiftModifier)
                        changeState(ViewState::FOTO_LASSO);
                  else
                        changeState(ViewState::FOTO_DRAG);
                  }
                  break;

            case ViewState::NOTE_ENTRY: {
                  _score->startCmd();
                  bool restMode = _score->inputState().rest();
                  if (ev->button() == Qt::RightButton)
                        _score->inputState().setRest(!restMode);
                  _score->putNote(editData.startMove, ev->modifiers() & Qt::ShiftModifier, ev->modifiers() & Qt::ControlModifier);
                  if (ev->button() == Qt::RightButton)
                        _score->inputState().setRest(restMode);
                  _score->endCmd();
                  if (_score->inputState().cr())
                        adjustCanvasPosition(_score->inputState().cr(), false);
                  shadowNote->setVisible(false);
                  }
                  break;

            case ViewState::EDIT: {
                  if (gripFound)
                        break;
                  if (!editData.element->canvasBoundingRect().contains(editData.startMove)) {
                        changeState(ViewState::NORMAL);
                        // changeState may trigger layout and destroy some elements
                        // so we should search elementNear after changeState.
                        setEditElement(elementNear(editData.startMove));
                        mousePressEventNormal(ev);
                        }
                  else {
                        editData.element->mousePress(editData);
                        score()->update();
                        if (editData.element->isTextBase() && mscore->textTools())
                              mscore->textTools()->updateTools(editData);
                        }
                  }
                  break;

            case ViewState::PLAY: {
                  Element* e = elementNear(editData.startMove);
                  if (seq && e && (e->isNote() || e->isRest())) {
                        if (e->isNote())
                              e = e->parent();
                        ChordRest* cr = toChordRest(e);
                        seq->seek(seq->score()->repeatList().tick2utick(cr->tick().ticks()));
                        }
                  }
                  break;

            default:
                  qDebug("mousePressEvent in state %d", int(state));
                  break;
            }
      }

//---------------------------------------------------------
//   adjustCursor
//---------------------------------------------------------
void ScoreView::adjustCursorForTextEditing(QMouseEvent* mouseEvent)
      {
      if (!editData.element)
            return;

      if (!(state == ViewState::EDIT || state == ViewState::DRAG_EDIT))
            return;

      if (!editData.element->isTextBase())
            return;

      if (editData.element->canvasBoundingRect().contains(toLogical(mouseEvent->pos())))
            setCursor(QCursor(Qt::IBeamCursor));
      else
            setCursor(QCursor(Qt::ArrowCursor));
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ScoreView::mouseMoveEvent(QMouseEvent* me)
      {
      modifySelection = false;
      adjustCursorForTextEditing(me);

      if (state != ViewState::NOTE_ENTRY && editData.buttons == Qt::NoButton)
            return;

      // start some drag operations after a minimum of movement:
      bool drag = (me->pos() - editData.startMovePixel).manhattanLength() > 4;

      switch (state) {
            case ViewState::NORMAL:
                  if (!drag)
                        return;
                  if (!editData.element && (me->modifiers() & Qt::ShiftModifier)) {
                        changeState(ViewState::LASSO);
                        break;
                        }
                  if (editData.element) {
                        if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit && editData.curGrip != Grip::NO_GRIP) {
                              score()->startCmd();
                              editData.element->startEditDrag(editData);
                              changeState(ViewState::DRAG_EDIT);
                              break;
                              }
                        if (editData.element->isMovable()) {
                              if (editData.element->normalModeEditBehavior() == Element::EditBehavior::Edit)
                                    endEdit();
                              changeState(ViewState::DRAG_OBJECT);
                              break;
                              }
                        }
                  changeState(ViewState::DRAG);
                  break;

            case ViewState::NOTE_ENTRY: {
                  QPointF p = toLogical(me->pos());
                  QRectF r(shadowNote->canvasBoundingRect());
                  setShadowNote(p);
                  r |= shadowNote->canvasBoundingRect();
                  update(toPhysical(r).adjusted(-2, -2, 2, 2));
                  }
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

            case ViewState::EDIT:
                  if (!drag)
                        return;
                  score()->startCmd();
                  editData.element->startEditDrag(editData);
                  changeState(ViewState::DRAG_EDIT);
                  break;

            case ViewState::PLAY:
                  if (drag)
                        dragScoreView(me);
                  break;

            case ViewState::FOTO_LASSO:
                  doDragFoto(me);
                  break;

            default:
                  break;
            }
      update();
      }

void ScoreView:: tripleClickTimeOut()
      {
      tripleClickPending = false;
      }

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void ScoreView::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
      {
      QTimer::singleShot(QApplication::doubleClickInterval(), this, SLOT(tripleClickTimeOut()));
      tripleClickPending = true;

      if (textEditMode()) {
            // double click on a textBase element that is being edited - select word
            TextBase* textBase = toTextBase(editData.element);
            textBase->multiClickSelect(editData, MultiClick::Double);
            mscore->textTools()->updateTools(editData);
            textBase->endHexState(editData);
            update();
            return;
            }

      if (state != ViewState::NORMAL)
            return;

      Element* clickedElement = elementNear(toLogical(mouseEvent->pos()));

      if (!(clickedElement && clickedElement->isEditable()))
            return;

      startEditMode(clickedElement);

      if (clickedElement->isTextBase()) {
            setCursor(QCursor(Qt::IBeamCursor));
            toTextBase(clickedElement)->setPrimed(false);
            }
      }

//---------------------------------------------------------
//   ScoreViewCmdContext
//---------------------------------------------------------

class ScoreViewCmdContext {
      Score* s;
      ScoreView* view;
      bool _updateGrips = false;

   public:
      ScoreViewCmdContext(ScoreView* v, bool updateGrips)
         : s(v->score()), view(v), _updateGrips(updateGrips)
            {
            s->startCmd();
            }

      ~ScoreViewCmdContext()
            {
            s->endCmd();
            if (_updateGrips)
                  view->updateGrips();
            }
      };

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void ScoreView::keyPressEvent(QKeyEvent* ev)
      {
      editData.key       = ev->key();
      editData.modifiers = ev->modifiers();
      editData.s         = ev->text();

      if (state != ViewState::EDIT) {
            const bool shiftModifier = ev->modifiers() & Qt::ShiftModifier;
            if (editData.grips && !(shiftModifier && ev->key() == Qt::Key_Backtab)) {
                  switch (ev->key()) {
                        case Qt::Key_Left:
                        case Qt::Key_Right:
                        case Qt::Key_Up:
                        case Qt::Key_Down:
                              // Move focus to default grip if arrow keys are pressed and no grip is focused
                              if (editData.grips && editData.curGrip == Grip::NO_GRIP)
                                    editData.curGrip = editData.element->defaultGrip();
                              break;
                        default:
                              break;
                        }

                  ScoreViewCmdContext ctx(this, /* updateGrips */ true);

                  if (!editData.element->edit(editData))
                        handleArrowKeyPress(ev);
                  }
            return;
            }

      if (MScore::debugMode)
            qDebug("keyPressEvent key 0x%02x(%c) mod 0x%04x <%s> nativeKey 0x%02x scancode %d",
               editData.key, editData.key, int(editData.modifiers), qPrintable(editData.s), ev->nativeVirtualKey(), ev->nativeScanCode());

      if (editData.element->isLyrics()) {
            if (editKeyLyrics())
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
      else if (editData.element->isSticking()) {
            if (editKeySticking())
                  return;
            }

      ScoreViewCmdContext cc(this, editData.grips);
      const bool textEdit = textEditMode();

#ifdef Q_OS_WIN // Japenese IME on Windows needs to know when Ctrl/Alt/Shift/CapsLock is pressed while in predit
      if (textEdit) {
            TextBase* text = toTextBase(editData.element);
            if (text->cursor(editData)->format()->preedit() && QGuiApplication::inputMethod()->locale().script() == QLocale::JapaneseScript &&
                ((editData.key == Qt::Key_Control || (editData.modifiers & Qt::ControlModifier)) ||
                 (editData.key == Qt::Key_Alt     || (editData.modifiers & Qt::AltModifier)) ||
                 (editData.key == Qt::Key_Shift   || (editData.modifiers & Qt::ShiftModifier)) ||
                 (editData.key == Qt::Key_CapsLock))) {
                  return; // musescore will ignore this key event so that the IME can handle it
                  }
            }
#endif

      if (!( (editData.modifiers & Qt::ShiftModifier) && (editData.key == Qt::Key_Backtab) )) {
            if (editData.element->edit(editData)) {
                  if (state != ViewState::EDIT) {
                        // textTab or other function may have terminated edit mode
                        mscore->endCmd();
                        return;
                        }
                  if (textEdit)
                        mscore->textTools()->updateTools(editData);
                  return;
                  }
            }

      const bool handled = handleArrowKeyPress(ev);
      if (!handled)
            ev->ignore();
      }

//---------------------------------------------------------
//   handleArrowKeys
//---------------------------------------------------------

bool ScoreView::handleArrowKeyPress(const QKeyEvent* ev)
      {
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

      switch (ev->key()) {
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
                  return false;
            }
      editData.delta   = delta;
      editData.hRaster = mscore->hRaster();
      editData.vRaster = mscore->vRaster();
      if (editData.curGrip != Grip::NO_GRIP && int(editData.curGrip) < editData.grips)
            editData.pos = editData.grip[int(editData.curGrip)].center() + delta;
      editData.element->startEditDrag(editData);
      editData.element->editDrag(editData);
      editData.element->endEditDrag(editData);
      return true;
      }

//---------------------------------------------------------
//   keyReleaseEvent
//---------------------------------------------------------

void ScoreView::keyReleaseEvent(QKeyEvent* ev)
      {
      if (textEditMode()) {
            auto modifiers = Qt::ControlModifier | Qt::ShiftModifier;
            if ((ev->modifiers() & modifiers) == 0) {
                  TextBase* text = toTextBase(editData.element);
                  text->endHexState(editData);
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
      if (state == ViewState::NOTE_ENTRY) {
            // Do not show context menu in note input mode.
            return;
            }
      if (state == ViewState::FOTO) {
            fotoContextPopup(ev);
            return;
            }
      QPoint gp          = ev->globalPos();
      editData.startMove = toLogical(ev->pos());
      editData.buttons   = Qt::NoButton;
      Element* e         = nullptr;
      if (ev->reason() == QContextMenuEvent::Keyboard)
            e = score()->selection().element();
      else
            e = elementNear(editData.startMove);
      if (e) {
            if (!e->selected()) {
                  // bool control = (ev->modifiers() & Qt::ControlModifier) ? true : false;
                  // _score->select(e, control ? SelectType::ADD : SelectType::SINGLE, 0);
                  // editData.element = e;
                  // select(ev);
                  }
            if (seq)
                  seq->stopNotes();       // stop now because we don’t get a mouseRelease event
            objectPopup(gp, e);
            }
      else {
            int staffIdx;
            Measure* m = nullptr;
            if (ev->reason() == QContextMenuEvent::Keyboard) {
                  // find measure based on selection
                  m = score()->selection().findMeasure();
                  }
            else {
                  // find nearest measure based on mouse pointer location
                  m = _score->pos2measure(editData.startMove, &staffIdx, 0, 0, 0);
                  // but only use it if mouse pointer is within the staff
                  if (m && !m->staffLines(staffIdx)->canvasBoundingRect().contains(editData.startMove))
                        m = nullptr;
                  }
            if (m)
                  measurePopup(ev, m);
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
                  changeState(ViewState::NORMAL);
                  break;
            case ViewState::FOTO:
            case ViewState::NOTE_ENTRY:
            case ViewState::PLAY:
                  changeState(ViewState::NORMAL);
                  break;
            case ViewState::NORMAL:
                  _score->deselectAll();
                  update();
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   stateName
//---------------------------------------------------------

static const char* stateName(ViewState s)
      {
      switch (s) {
            case ViewState::NORMAL:             return "NORMAL";
            case ViewState::DRAG:               return "DRAG";
            case ViewState::DRAG_OBJECT:        return "DRAG_OBJECT";
            case ViewState::EDIT:               return "EDIT";
            case ViewState::DRAG_EDIT:          return "DRAG_EDIT";
            case ViewState::LASSO:              return "LASSO";
            case ViewState::NOTE_ENTRY:         return "NOTE_ENTRY";
            case ViewState::PLAY:               return "PLAY";
            case ViewState::ENTRY_PLAY:         return "ENTRY_PLAY";
            case ViewState::FOTO:               return "FOTO";
            case ViewState::FOTO_DRAG:          return "FOTO_DRAG";
            case ViewState::FOTO_DRAG_EDIT:     return "FOTO_DRAG_EDIT";
            case ViewState::FOTO_DRAG_OBJECT:   return "FOTO_DRAG_OBJECT";
            case ViewState::FOTO_LASSO:         return "FOTO_LASSO";
            }
      return "";
      }

//---------------------------------------------------------
//   seqStopped
//---------------------------------------------------------

void ScoreView::seqStopped()
      {
      changeState(ViewState::NORMAL);
      }

//---------------------------------------------------------
//   changeState
//---------------------------------------------------------

void ScoreView::changeState(ViewState s)
      {
//      if (state == ViewState::EDIT && s == ViewState::EDIT) {
//            startEdit();
//            return;
//            }
      if (s == ViewState::PLAY && !seq)
            return;
      if (s == state)
            return;

      qDebug("changeState %s  -> %s", stateName(state), stateName(s));
      //
      //    end current state
      //
      switch (state) {
            case ViewState::NOTE_ENTRY:
                  endNoteEntry();
                  break;
            case ViewState::DRAG:
                  break;
            case ViewState::FOTO_DRAG_OBJECT:
                  for (Element* e : _score->selection().elements()) {
                        e->endDrag(editData);
                        e->triggerLayout();
                        }
                  setDropTarget(0); // this also resets dropAnchor
                  _score->endCmd();
                  break;
            case ViewState::DRAG_OBJECT:
                  endDrag();
                  break;
            case ViewState::FOTO_DRAG_EDIT:
            case ViewState::DRAG_EDIT:
                  endDragEdit();
                  break;
            case ViewState::FOTO:
                  if (s != ViewState::FOTO_DRAG && s != ViewState::FOTO_DRAG_EDIT && s != ViewState::FOTO_DRAG_OBJECT)
                        stopFotomode();
                  break;
            case ViewState::LASSO:
                  endLasso();
                  break;
            case ViewState::FOTO_LASSO:
                  endFotoDrag();
                  break;
            case ViewState::PLAY:
                  seq->stop();
                  break;
            case ViewState::EDIT:
                  setMouseTracking(false);
                  break;
            default:
                  break;
            }
      //
      //    start new state
      //
      switch (s) {
            case ViewState::NORMAL:
                  if (state == ViewState::EDIT) {
                        _blockShowEdit = true;  // otherwise may jump on clicking outside the text element being edited
                        endEdit();
                        _blockShowEdit = false;
                        }
                  setCursor(QCursor(Qt::ArrowCursor));
                  break;
            case ViewState::DRAG:
                  setCursor(QCursor(Qt::SizeAllCursor));
                  break;
            case ViewState::FOTO_DRAG_OBJECT:
                  _score->select(_foto);
                  // fall through
            case ViewState::DRAG_OBJECT:
                  setCursor(QCursor(Qt::ArrowCursor));
                  startDrag();
                  break;
            case ViewState::FOTO_DRAG:
                  setCursor(QCursor(Qt::SizeAllCursor));
                  break;
            case ViewState::NOTE_ENTRY:
                  setCursor(QCursor(Qt::ArrowCursor));
                  startNoteEntry();
                  break;
            case ViewState::DRAG_EDIT:
            case ViewState::FOTO_DRAG_EDIT:
                  setCursor(QCursor(Qt::ArrowCursor));
                  break;
            case ViewState::FOTO:
                  setCursor(QCursor(Qt::ArrowCursor));
                  if (state != ViewState::FOTO_DRAG && state != ViewState::FOTO_DRAG_EDIT && state != ViewState::FOTO_DRAG_OBJECT)
                        startFotomode();
                  if (state == ViewState::FOTO_DRAG_OBJECT)
                        startEdit();
                  break;
            case ViewState::EDIT:
                  if (state != ViewState::DRAG_EDIT)
                        startEdit();
                  break;
            case ViewState::LASSO:
                  break;
            case ViewState::PLAY:
                  seq->start();
                  break;
            case ViewState::ENTRY_PLAY:
                  break;
            case ViewState::FOTO_LASSO:
                  startFotoDrag();
                  break;
            }

      state = s;
      mscore->changeState(mscoreState());
      if (mscoreState() & STATE_ALLTEXTUAL_EDIT)
            setMouseTracking(true);
      }

//---------------------------------------------------------
//   inputMethodEvent
//---------------------------------------------------------

void ScoreView::inputMethodEvent(QInputMethodEvent* event)
      {
      if (textEditMode())
            toTextBase(editData.element)->inputTransition(editData, event);
      }

}    // namespace Ms

