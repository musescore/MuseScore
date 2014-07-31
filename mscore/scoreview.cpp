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

/**
 \file
 Implementation of most part of class ScoreView.
*/

#include "globals.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "musescore.h"
#include "seq.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/page.h"
#include "libmscore/xml.h"
#include "libmscore/text.h"
#include "libmscore/note.h"
#include "libmscore/dynamic.h"
#include "libmscore/pedal.h"
#include "libmscore/volta.h"
#include "libmscore/ottava.h"
#include "libmscore/noteline.h"
#include "libmscore/trill.h"
#include "libmscore/hairpin.h"
#include "libmscore/image.h"
#include "libmscore/part.h"
#include "libmscore/icon.h"
#include "editdrumset.h"
#include "editstaff.h"
#include "splitstaff.h"
#include "libmscore/barline.h"
#include "libmscore/system.h"
#include "magbox.h"
#include "libmscore/measure.h"
#include "drumroll.h"
#include "libmscore/lyrics.h"
#include "libmscore/figuredbass.h"
#include "textpalette.h"
#include "libmscore/undo.h"
#include "libmscore/slur.h"
#include "libmscore/harmony.h"
#include "libmscore/navigate.h"
#include "libmscore/stringdata.h"
#include "libmscore/shadownote.h"
#include "libmscore/sym.h"
#include "libmscore/lasso.h"
#include "libmscore/box.h"
#include "libmscore/textframe.h"
#include "texttools.h"
#include "libmscore/clef.h"
#include "scoretab.h"
#include "measureproperties.h"
#include "textcursor.h"
#include "navigator.h"
#include "continuouspanel.h"

#include "libmscore/pitchspelling.h"
#include "libmscore/notedot.h"
#include "libmscore/articulation.h"
#include "libmscore/tuplet.h"
#include "libmscore/stafftext.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/spanner.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/excerpt.h"
#include "libmscore/stafftype.h"
#include "libmscore/repeatlist.h"

#include "inspector/inspector.h"

namespace Ms {

extern QErrorMessage* errorMessage;

#if 0
// a useful enum for scale steps (could be moved to libmscore/pitchspelling.h)
enum {
      STEP_NONE      = -1,
      STEP_C,
      STEP_D,
      STEP_E,
      STEP_F,
      STEP_G,
      STEP_A,
      STEP_B
};
#endif

static const QEvent::Type CloneDrag = QEvent::Type(QEvent::User + 1);

//---------------------------------------------------------
//   CloneEvent
//---------------------------------------------------------

class CloneEvent : public QEvent {
      Element* _element;

   public:
      CloneEvent(Element* e) : QEvent(CloneDrag) { _element = e->clone(); }
      ~CloneEvent() { delete _element; }
      Element* element() const { return _element; }
      };

//---------------------------------------------------------
//   stateNames
//---------------------------------------------------------

static const char* stateNames[] = {
      "Normal", "Drag", "DragObject", "Edit", "DragEdit",
      "Lasso",  "NoteEntry", "Mag", "Play", "Search", "EntryPlay",
      "Fotomode"
      };

//---------------------------------------------------------
//   MagTransition
//---------------------------------------------------------

class MagTransition1 : public QEventTransition
      {
   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e))
                  return false;
            return !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            bool b1 = me->button() == Qt::LeftButton;
            ScoreView* c = static_cast<ScoreView*>(eventSource());
            c->zoomStep(b1 ? 2 : -1, me->pos());
            }
   public:
      MagTransition1(QObject* obj, QState* target)
         : QEventTransition(obj, QEvent::MouseButtonPress) {
            setTargetState(target);
            }
      };

class MagTransition2 : public QEventTransition
      {
   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e)) {
                  qDebug("MagTransition2: wrong event");
                  return false;
                  }
            return bool(QApplication::keyboardModifiers() & Qt::ShiftModifier);
            }
      virtual void onTransition(QEvent* e) {
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            bool b1 = me->button() == Qt::LeftButton;
            ScoreView* c = static_cast<ScoreView*>(eventSource());
            c->zoomStep(b1 ? 2 : -1, me->pos());
            }
   public:
      MagTransition2(QObject* obj)
         : QEventTransition(obj, QEvent::MouseButtonPress) {}
      };

//---------------------------------------------------------
//   ContextTransition
//---------------------------------------------------------

class ContextTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QContextMenuEvent* me = static_cast<QContextMenuEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            canvas->contextPopup(me);
            }
   public:
      ContextTransition(ScoreView* c)
         : QEventTransition(c, QEvent::ContextMenu), canvas(c) {}
      };

//---------------------------------------------------------
//   EditTransition
//---------------------------------------------------------

class EditTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event) || canvas->getEditObject())
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            QPointF p = canvas->toLogical(me->pos());
            Element* e = canvas->elementNear(p);
            if (e && e->isEditable())
                  canvas->setEditObject(e);
            return e && e->isEditable();
            }
   public:
      EditTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonDblClick, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   SeekTransition
//---------------------------------------------------------

class SeekTransition : public QMouseEventTransition
      {
      ScoreView* canvas;
      ChordRest* cr;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            QPointF p = canvas->toLogical(me->pos());
            Element* e = canvas->elementNear(p);
            if (e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST)) {
                  if (e->type() == Element::Type::NOTE)
                        e = e->parent();
                  cr = static_cast<ChordRest*>(e);
                  return true;
                  }
            return false;
            }
      virtual void onTransition(QEvent*) {
            if (seq)
                  seq->seek(seq->score()->repeatList()->tick2utick(cr->tick()));
            canvas->setCursorVisible(true);
            }
   public:
      SeekTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditKeyTransition
//---------------------------------------------------------

class EditKeyTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QKeyEvent* ke = static_cast<QKeyEvent*>(we->event());
            canvas->editKey(ke);
            }
   public:
      EditKeyTransition(ScoreView* c)
         : QEventTransition(c, QEvent::KeyPress), canvas(c) {}
      };

//---------------------------------------------------------
//   ScoreViewLassoTransition
//---------------------------------------------------------

class ScoreViewLassoTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            if (!(me->modifiers() & Qt::ShiftModifier))
                  return false;
            return !canvas->mousePress(me);
            }
   public:
      ScoreViewLassoTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
// ElementDragTransition
//---------------------------------------------------------

class ElementDragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->testElementDragTransition(me);
            }
   public:
      ElementDragTransition(ScoreView* c, QState* target)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditDragEditTransition
//---------------------------------------------------------

class EditDragEditTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->editElementDragTransition(me);
            }
   public:
      EditDragEditTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditPasteTransition
//---------------------------------------------------------

class EditPasteTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->onEditPasteTransition(me);
            }
   public:
      EditPasteTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::MidButton), canvas(c) {
            }
      };

//---------------------------------------------------------
//   EditInputTransition
//---------------------------------------------------------

class EditInputTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            canvas->editInputTransition(static_cast<QInputMethodEvent*>(we->event()));
            return true;
            }
   public:
      EditInputTransition(ScoreView* c)
         : QEventTransition(c, QEvent::InputMethod), canvas(c) {}
      };

//---------------------------------------------------------
//   EditDragTransition
//---------------------------------------------------------

class EditDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            return canvas->editScoreViewDragTransition(me);
            }
   public:
      EditDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditSelectTransition
//---------------------------------------------------------

class EditSelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            return canvas->editSelectTransition(me);
            }
   public:
      EditSelectTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   SelectTransition
//---------------------------------------------------------

class SelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->mousePress(me);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->select(me);
            }
   public:
      SelectTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {}
      };

//---------------------------------------------------------
//   DeSelectTransition
//---------------------------------------------------------

class DeSelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->mousePress(me);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->select(me);
            }
   public:
      DeSelectTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonRelease, Qt::LeftButton), canvas(c) {}
      };

//---------------------------------------------------------
//   NoteEntryDragTransition
//---------------------------------------------------------

class NoteEntryDragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->dragNoteEntry(me);
            }
   public:
      NoteEntryDragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   NoteEntryButtonTransition
//---------------------------------------------------------

class NoteEntryButtonTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->noteEntryButton(me);
            }
   public:
      NoteEntryButtonTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseButtonPress), canvas(c) {}
      };

//---------------------------------------------------------
//   DragElementTransition
//---------------------------------------------------------

class DragElementTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragElement(me);
            }
   public:
      DragElementTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   DragEditTransition
//---------------------------------------------------------

class DragEditTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragEdit(me);
            }
   public:
      DragEditTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   DragLassoTransition
//---------------------------------------------------------

class DragLassoTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragLasso(me);
            }
   public:
      DragLassoTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   eventTest
//---------------------------------------------------------

bool CommandTransition::eventTest(QEvent* e)
      {
      if (e->type() != QEvent::Type(QEvent::User+1))
            return false;
      CommandEvent* ce = static_cast<CommandEvent*>(e);
      return ce->value == val;
      }

//---------------------------------------------------------
//   ScoreViewDragTransition
//---------------------------------------------------------

bool ScoreViewDragTransition::eventTest(QEvent* event)
      {
      if (!QMouseEventTransition::eventTest(event))
            return false;
      QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
      QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
      if (me->modifiers() & Qt::ShiftModifier)
            return false;
      return !canvas->mousePress(me);
      }

ScoreViewDragTransition::ScoreViewDragTransition(ScoreView* c, QState* target)
   : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c)
      {
      setTargetState(target);
      }

//---------------------------------------------------------
//   onTransition
//---------------------------------------------------------

void DragTransition::onTransition(QEvent* e)
      {
      QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
      QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
      canvas->dragScoreView(me);
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QWidget* parent)
   : QWidget(parent)
      {
      setAcceptDrops(true);
      setAttribute(Qt::WA_OpaquePaintEvent);
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setAutoFillBackground(false);

      _score      = 0;
      _omrView    = 0;
      dropTarget  = 0;

      setContextMenuPolicy(Qt::DefaultContextMenu);

      double mag  = preferences.mag;
      _matrix     = QTransform(mag, 0.0, 0.0, mag, 0.0, 0.0);
      imatrix     = _matrix.inverted();
      _magIdx     = preferences.mag == 1.0 ? MagIdx::MAG_100 : MagIdx::MAG_FREE;
      focusFrame  = 0;
      dragElement = 0;
      curElement  = 0;
      _bgColor    = Qt::darkBlue;
      _fgColor    = Qt::white;
      fgPixmap    = 0;
      bgPixmap    = 0;
      curGrip     = -1;
      defaultGrip = -1;
      lasso       = new Lasso(_score);
      _foto       = new Lasso(_score);

      _cursor     = new PositionCursor(this);
      _cursor->setType(CursorType::POS);
      _continuousPanel = new ContinuousPanel(this);
      _continuousPanel->setVisible(true);

      shadowNote  = 0;
      grips       = 0;
      editObject  = 0;
      addSelect   = false;

      _curLoopIn  = new PositionCursor(this);
      _curLoopIn->setType(CursorType::LOOP_IN);
      _curLoopOut = new PositionCursor(this);
      _curLoopOut->setType(CursorType::LOOP_OUT);

      //---setup state machine-------------------------------------------------
      sm          = new QStateMachine(this);
      QState* stateActive = new QState;
      for (int i = 0; i < STATES; ++i) {
            states[i] = new QState(stateActive);
            states[i]->setObjectName(stateNames[i]);
            connect(states[i], SIGNAL(entered()), SLOT(enterState()));
            connect(states[i], SIGNAL(exited()), SLOT(exitState()));
            }

      if (converterMode)      // HACK
            return;

      connect(stateActive, SIGNAL(entered()), SLOT(enterState()));
      connect(stateActive, SIGNAL(exited()), SLOT(exitState()));

      CommandTransition* ct;
      QEventTransition* evt;
      QState* s;

      //----------------------------------
      //    setup normal state
      //----------------------------------

      s = states[NORMAL];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new ContextTransition(this));                          // context menu
      EditTransition* et = new EditTransition(this, states[EDIT]);            // ->edit
      connect(et, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(et);
      s->addTransition(new SelectTransition(this));                           // select
      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
//      s->addTransition(new DeSelectTransition(this));                         // deselect
//      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
      s->addTransition(new ScoreViewDragTransition(this, states[DRAG]));      // ->stateDrag
      s->addTransition(new ScoreViewLassoTransition(this, states[LASSO]));    // ->stateLasso
      s->addTransition(new ElementDragTransition(this, states[DRAG_OBJECT])); // ->stateDragObject
      s->addTransition(new CommandTransition("note-input", states[NOTE_ENTRY])); // ->noteEntry
      ct = new CommandTransition("escape", s);                                // escape
      connect(ct, SIGNAL(triggered()), SLOT(deselectAll()));
      s->addTransition(ct);
      ct = new CommandTransition("edit", states[EDIT]);                       // ->edit harmony/slur/lyrics
      connect(ct, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(ct);
      s->addTransition(new CommandTransition("mag", states[MAG]));            // ->mag
      s->addTransition(new CommandTransition("play", states[PLAY]));          // ->play
      s->addTransition(new CommandTransition("fotomode", states[FOTOMODE]));  // ->fotomode
      ct = new CommandTransition("paste", 0);                                 // paste
      connect(ct, SIGNAL(triggered()), SLOT(normalPaste()));
      s->addTransition(ct);
      ct = new CommandTransition("copy", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(normalCopy()));
      s->addTransition(ct);
      ct = new CommandTransition("cut", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(normalCut()));
      s->addTransition(ct);

      //----------------------------------
      //    setup mag state
      //----------------------------------

      s = states[MAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      s->addTransition(new MagTransition1(this, states[NORMAL]));
      s->addTransition(new MagTransition2(this));

      //----------------------------------
      // setup drag element state
      //----------------------------------

      s = states[DRAG_OBJECT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      QEventTransition* cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      s->addTransition(new DragElementTransition(this));
      connect(s, SIGNAL(entered()), SLOT(startDrag()));
      connect(s, SIGNAL(exited()), SLOT(endDrag()));

      //----------------------------------
      //    setup edit state
      //----------------------------------

      s = states[EDIT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));

      ct = new CommandTransition("escape", states[NORMAL]);                   // ->NORMAL
      connect(ct, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(ct);

      s->addTransition(new EditKeyTransition(this));                          // key events

      et = new EditTransition(this, s);                                       // ->EDIT
      connect(et, SIGNAL(triggered()), SLOT(endStartEdit()));
      s->addTransition(et);

      s->addTransition(new EditDragEditTransition(this, states[DRAG_EDIT]));  // ->DRAG_EDIT

      evt = new EditDragTransition(this, states[DRAG]);                       // ->DRAG
      connect(evt, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(evt);

      evt = new EditSelectTransition(this, states[NORMAL]);                   // ->NORMAL
      connect(evt, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(evt);

      s->addTransition(new EditInputTransition(this));                        // compose text

      s->addTransition(new EditPasteTransition(this));                        // paste text

      ct = new CommandTransition("copy", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(editCopy()));
      s->addTransition(ct);

      ct = new CommandTransition("paste", 0);                                 // paste
      connect(ct, SIGNAL(triggered()), SLOT(editPaste()));
      s->addTransition(ct);

      //----------------------------------
      //    setup drag edit state
      //----------------------------------

      s = states[DRAG_EDIT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[EDIT]);
      s->addTransition(cl);
      s->addTransition(new DragEditTransition(this));
      connect(s, SIGNAL(exited()), SLOT(endDragEdit()));

      // setup lasso state
      s = states[LASSO];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);            // ->normal
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      s->addTransition(new class DragLassoTransition(this));                  // drag
      connect(s, SIGNAL(exited()), SLOT(endLasso()));

      //----------------------setup note entry state
      s = states[NOTE_ENTRY];
      s->assignProperty(this, "cursor", QCursor(Qt::UpArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));      // ->normal
      s->addTransition(new CommandTransition("note-input", states[NORMAL]));  // ->normal
      connect(s, SIGNAL(entered()), SLOT(startNoteEntry()));
      connect(s, SIGNAL(exited()), SLOT(endNoteEntry()));
      s->addTransition(new NoteEntryDragTransition(this));                    // mouse drag
      s->addTransition(new NoteEntryButtonTransition(this));                  // mouse button
      s->addTransition(new CommandTransition("play", states[ENTRY_PLAY]));    // ->entryPlay

      // setup normal drag canvas state
      s = states[DRAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      connect(s, SIGNAL(entered()), SLOT(deselectAll()));
      s->addTransition(new DragTransition(this));

      //----------------------setup play state
      if (seq) {
            s = states[PLAY];
            // s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
            s->addTransition(new CommandTransition("play", states[NORMAL]));
            s->addTransition(new CommandTransition("escape", states[NORMAL]));
            s->addTransition(new SeekTransition(this, states[PLAY]));
            QSignalTransition* st = new QSignalTransition(seq, SIGNAL(stopped()));
            st->setTargetState(states[NORMAL]);
            s->addTransition(st);
            connect(s, SIGNAL(entered()), mscore, SLOT(setPlayState()));
            connect(s, SIGNAL(entered()), seq, SLOT(start()));
            connect(s, SIGNAL(exited()),  seq, SLOT(stop()));

            QState* s1 = new QState(s);
            s1->setObjectName("play-normal");
            connect(s1, SIGNAL(entered()), SLOT(enterState()));
            connect(s1, SIGNAL(exited()), SLOT(exitState()));
            QState* s2 = new QState(s);
            connect(s2, SIGNAL(entered()), SLOT(enterState()));
            connect(s2, SIGNAL(exited()), SLOT(exitState()));
            s2->setObjectName("play-drag");

            s1->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
            s1->addTransition(new ScoreViewDragTransition(this, s2));      // ->stateDrag

            // drag during play state
            s2->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
            cl = new QEventTransition(this, QEvent::MouseButtonRelease);
            cl->setTargetState(s1);
            s2->addTransition(cl);
            connect(s2, SIGNAL(entered()), SLOT(deselectAll()));
            s2->addTransition(new DragTransition(this));

            s->setInitialState(s1);
            s->addTransition(new ScoreViewDragTransition(this, s2));

            // setup editPlay state
            s = states[ENTRY_PLAY];
            s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
            s->addTransition(new CommandTransition("play", states[NOTE_ENTRY]));
            s->addTransition(new CommandTransition("escape", states[NOTE_ENTRY]));
            st = new QSignalTransition(seq, SIGNAL(stopped()));
            st->setTargetState(states[NOTE_ENTRY]);
            s->addTransition(st);
            connect(s, SIGNAL(entered()), mscore, SLOT(setPlayState()));
            connect(s, SIGNAL(entered()), seq, SLOT(start()));
            connect(s, SIGNAL(exited()),  seq, SLOT(stop()));
            }

      setupFotoMode();

      sm->addState(stateActive);
      stateActive->setInitialState(states[NORMAL]);
      sm->setInitialState(stateActive);

      sm->start();

      grabGesture(Qt::PinchGesture);      // laptop pad

      //-----------------------------------------------------------------------

      if (MScore::debugMode)
            setMouseTracking(true);

      if (preferences.bgUseColor)
            setBackground(MScore::bgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.bgWallpaper);
            setBackground(pm);
            }
      if (preferences.fgUseColor)
            setForeground(preferences.fgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.fgWallpaper);
            if (pm == 0 || pm->isNull())
                  qDebug("no valid pixmap %s", qPrintable(preferences.fgWallpaper));
            setForeground(pm);
            }

      connect(getAction("loop"), SIGNAL(toggled(bool)), SLOT(loopToggled(bool)));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(Score* s)
      {
      if (_score) {
            _score->removeViewer(this);
            disconnect(s, SIGNAL(posChanged(POS, int)), this, SLOT(posChanged(POS,int)));
            }

      _score = s;
      _score->addViewer(this);

      if (shadowNote == 0) {
            shadowNote = new ShadowNote(_score);
            shadowNote->setVisible(false);
            }
      else
            shadowNote->setScore(_score);
      lasso->setScore(s);
      _foto->setScore(s);
      _continuousPanel->setScore(s);

      if (s) {
            _curLoopIn->move(s->pos(POS::LEFT));
            _curLoopOut->move(s->pos(POS::RIGHT));
            loopToggled(getAction("loop")->isChecked());

            connect(s, SIGNAL(posChanged(POS,unsigned)), SLOT(posChanged(POS,unsigned)));
            connect(this, SIGNAL(viewRectChanged()), this, SLOT(updateContinuousPanel()));

            s->setLayoutMode(LayoutMode::PAGE);
            s->setLayoutAll(true);
            s->update();
            }
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::~ScoreView()
      {
      if (_score)
            _score->removeViewer(this);
      delete lasso;
      delete _foto;
      delete _cursor;
      delete _continuousPanel;
      delete _curLoopIn;
      delete _curLoopOut;
      delete bgPixmap;
      delete fgPixmap;
      delete shadowNote;
      }

//---------------------------------------------------------
//   objectPopup
//    the menu can be extended by Elements with
//      genPropertyMenu()/propertyAction() methods
//---------------------------------------------------------

void ScoreView::objectPopup(const QPoint& pos, Element* obj)
      {
      // show tuplet properties if number is clicked:
      if (obj->type() == Element::Type::TEXT && obj->parent() && obj->parent()->type() == Element::Type::TUPLET) {
            obj = obj->parent();
            if (!obj->selected())
                  obj->score()->select(obj, SelectType::SINGLE, 0);
            }

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);
      QAction* a = popup->addSeparator();
      a->setText(obj->userName());

      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));

      QMenu* selMenu = popup->addMenu(tr("Select"));
      selMenu->addAction(getAction("select-similar"));
      selMenu->addAction(getAction("select-similar-staff"));
      selMenu->addAction(getAction("select-similar-range"));
      a = selMenu->addAction(tr("More..."));
      a->setData("select-dialog");
      popup->addSeparator();
      a = getAction("edit-element");
      popup->addAction(a);
      a->setEnabled(obj->isEditable());

      createElementPropertyMenu(obj, popup);

      popup->addSeparator();
      a = popup->addAction(tr("Help"));
      popup->addSeparator();
      a->setData("help");
      a = popup->addAction(tr("Debugger"));
      a->setData("list");
      a = popup->exec(pos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste") {
            // these actions are already activated
            return;
            }
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "help")
            mscore->helpBrowser(obj->name());
      else if (cmd == "edit") {
            if (obj->isEditable()) {
                  startEdit(obj);
                  return;
                  }
            }
      else if (cmd == "select-similar")
            mscore->selectSimilar(obj, false);
      else if (cmd == "select-similar-staff")
            mscore->selectSimilar(obj, true);
      else if (cmd == "select-similar-range")
            mscore->selectSimilarInRange(obj);
      else if (cmd == "select-dialog")
            mscore->selectElementDialog(obj);
      else {
            _score->startCmd();
            elementPropertyAction(cmd, obj);
            _score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   measurePopup
//---------------------------------------------------------

void ScoreView::measurePopup(const QPoint& gpos, Measure* obj)
      {
      int staffIdx;
      int pitch;
      Segment* seg;
      QPointF offset;

      if (!_score->pos2measure(data.startMove, &staffIdx, &pitch, &seg, &offset))
            return;
      if (staffIdx == -1) {
            qDebug("ScoreView::measurePopup: staffIdx == -1!");
            return;
            }

      Staff* staff = _score->staff(staffIdx);

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addSeparator();
      a->setText(tr("Staff"));
      a = popup->addAction(tr("Edit Drumset..."));
      a->setData("edit-drumset");
      a->setEnabled(staff->part()->instr()->drumset() != 0);

      if (staff->part()->instr()->drumset()) {
            a = popup->addAction(tr("Drumroll Editor..."));
            a->setData("drumroll");
            }
      else {
            a = popup->addAction(tr("Pianoroll Editor..."));
            a->setData("pianoroll");
            }

      a = popup->addAction(tr("Staff Properties..."));
      a->setData("staff-properties");
      a = popup->addAction(tr("Split Staff..."));
      a->setData("staff-split");

      a = popup->addSeparator();
      a->setText(tr("Measure"));
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addAction(getAction("delete"));
      popup->addAction(getAction("insert-measure"));
      popup->addSeparator();

      popup->addAction(tr("Measure Properties..."))->setData("props");
      popup->addSeparator();

      popup->addAction(tr("Object Debugger"))->setData("list");

      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste" || cmd == "insert-measure"
         || cmd == "select-similar"
         || cmd == "delete") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
      else if (cmd == "edit") {
            if (obj->isEditable()) {
                  startEdit(obj);
                  return;
                  }
            }
      else if (cmd == "edit-drumset") {
            EditDrumset drumsetEdit(staff->part()->instr()->drumset(), this);
            drumsetEdit.exec();
            }
      else if (cmd == "drumroll") {
            _score->endCmd();
            mscore->editInDrumroll(staff);
            }
      else if (cmd == "pianoroll") {
            _score->endCmd();
            mscore->editInPianoroll(staff);
            }
      else if (cmd == "staff-properties") {
            EditStaff editStaff(staff, this);
            connect(&editStaff, SIGNAL(instrumentChanged()), mscore, SLOT(instrumentChanged()));
            editStaff.exec();
            }
      else if (cmd == "staff-split") {
            SplitStaff splitStaff(this);
            if (splitStaff.exec())
                  _score->splitStaff(staffIdx, splitStaff.getSplitPoint());
            }
      else if (cmd == "props") {
            MeasureProperties im(obj);
            im.exec();
            }
      _score->setLayoutAll(true);
      if (_score->undo()->active()) {
            _score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void ScoreView::resizeEvent(QResizeEvent* /*ev*/)
      {
      if (_magIdx == MagIdx::MAG_PAGE_WIDTH || _magIdx == MagIdx::MAG_PAGE || _magIdx == MagIdx::MAG_DBL_PAGE) {
            double m = mscore->getMag(this);
            setMag(m);
            }
      update();
      }

//---------------------------------------------------------
//   updateGrips
//    if (curGrip == -1) then initialize to element
//    default grip
//---------------------------------------------------------

void ScoreView::updateGrips()
      {
      if (editObject == 0)
            return;

      double dx = 1.5 / _matrix.m11();
      double dy = 1.5 / _matrix.m22();

      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));

      qreal w   = 8.0 / _matrix.m11();
      qreal h   = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < MAX_GRIPS; ++i)
            grip[i] = r;

      editObject->updateGrips(&grips, &defaultGrip, grip);

      // updateGrips returns grips in page coordinates,
      // transform to view coordinates:

      Element* page = editObject;
      while (page->parent())
            page = page->parent();
      QPointF pageOffset(page->pos());
      for (int i = 0; i < grips; ++i) {
            grip[i].translate(pageOffset);
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));
            }

      if (curGrip == -1)
            curGrip = defaultGrip;

      QPointF anchor = editObject->gripAnchor(curGrip);
      if (!anchor.isNull())
            setDropAnchor(QLineF(anchor + pageOffset, grip[curGrip].center()));
      else
            setDropTarget(0); // this also resets dropAnchor
      score()->addRefresh(editObject->canvasBoundingRect());
      }

//---------------------------------------------------------
//   setEditPos
//---------------------------------------------------------

void ScoreView::setEditPos(const QPointF& pt)
      {
      editObject->setGrip(curGrip, pt);
      updateGrips();
      _score->end();
      }

//---------------------------------------------------------
//   setBackground
//---------------------------------------------------------

void ScoreView::setBackground(QPixmap* pm)
      {
      delete bgPixmap;
      bgPixmap = pm;
      update();
      }

void ScoreView::setBackground(const QColor& color)
      {
      delete bgPixmap;
      bgPixmap = 0;
      _bgColor = color;
      update();
      }

//---------------------------------------------------------
//   setForeground
//---------------------------------------------------------

void ScoreView::setForeground(QPixmap* pm)
      {
      delete fgPixmap;
      fgPixmap = pm;
      update();
      }

void ScoreView::setForeground(const QColor& color)
      {
      delete fgPixmap;
      fgPixmap = 0;
      _fgColor = color;
      update();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void ScoreView::dataChanged(const QRectF& r)
      {
      update(_matrix.mapRect(r).toRect());  // generate paint event
      }

//---------------------------------------------------------
//   updateAll
//---------------------------------------------------------

void ScoreView::updateAll()
      {
      update();
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor during playback
//---------------------------------------------------------

void ScoreView::moveCursor(int tick)
      {
      Measure* measure = score()->tick2measureMM(tick);
      if (measure == 0)
            return;
      int offset = 0;

      qreal x;
      Segment* s;
      for (s = measure->first(Segment::Type::ChordRest); s;) {
            int t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            int t2;
            Segment* ns = s->next(Segment::Type::ChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  // measure->width is not good enough because of courtesy keysig, timesig
                  Segment* seg = measure->findSegment(Segment::Type::EndBarLine, measure->tick() + measure->ticks());
                  if(seg)
                        x2 = seg->canvasPos().x();
                  else
                        x2 = measure->canvasPos().x() + measure->width(); //safety, should not happen
                  }
            t1 += offset;
            t2 += offset;
            if (tick >= t1 && tick < t2) {
                  int   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1) / dt;
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      QColor c(MScore::selectColor[0]);
      c.setAlpha(50);
      _cursor->setColor(c);
      _cursor->setTick(tick);

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffYpage(0) + system->page()->pos().y();
      double _spatium = score()->spatium();

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      qreal mag = _spatium / (MScore::DPI * SPATIUM20);
      double w  = _spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag);
      double h  = 6 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;

      for (int i = 0; i < _score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !_score->staff(i)->show())
                  continue;
            y2 = ss->y() + ss->bbox().height();
            }
      h += y2;
      x -= _spatium;
      y -= 3 * _spatium;

      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
      if (mscore->panDuringPlayback())
            adjustCanvasPosition(measure, true);
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor in note input mode
//---------------------------------------------------------

void ScoreView::moveCursor()
      {
      const InputState& is = _score->inputState();
      Segment* segment = is.segment();
      if (segment && score()->styleB(StyleIdx::createMultiMeasureRests) && segment->measure()->hasMMRest()) {
            Measure* m = segment->measure()->mmRest();
            segment = m->findSegment(Segment::Type::ChordRest, m->tick());
            }
      if (!segment)
            return;

      int track    = is.track() == -1 ? 0 : is.track();
      int voice    = track % VOICES;
      int staffIdx = track / VOICES;

      QColor c(MScore::selectColor[voice]);
      c.setAlpha(50);
      _cursor->setColor(c);
      _cursor->setTick(segment->tick());

      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            // or this measure was skipped by a multi measure rest
            return;
            }
      double x        = segment->canvasPos().x();
      double y        = system->staffYpage(staffIdx) + system->page()->pos().y();
      double _spatium = score()->spatium();

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      double h;
      qreal mag       = _spatium / (MScore::DPI * SPATIUM20);
      double w        = _spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag);
      Staff* staff    = _score->staff(staffIdx);
      double lineDist = staff->staffType()->lineDistance().val() * _spatium;
      int lines       = staff->lines();
      int strg        = is.string();
      x              -= _spatium;
      // if on a TAB staff and InputState::_string makes sense,
      // draw cursor around single string
      if (staff->isTabStaff() && strg > VISUAL_STRING_NONE && strg < lines) {
            h = lineDist * 1.5;       // 1 space above strg for letters and 1/2 sp. below strg for numbers
            y += lineDist * (strg-1); // start 1 sp. above strg to include letter position
            // look for a note on this string in this staff
            bool        done  = false;
            Segment*    seg   = is.segment();
            int         minTrack = (is.track() / VOICES) * VOICES;
            int         maxTrack = minTrack + VOICES;
            // get the physical string corresponding to current visual string
            strg = staff->staffType()->visualStringToPhys(strg);
            for (int track = minTrack; track < maxTrack; track++) {
                  Element* e = seg->element(track);
                  if (e != nullptr && e->type() == Element::Type::CHORD)
                        for (Note* n : static_cast<Chord*>(e)->notes())
                              // if note found on this string, make it current
                              if (n->string() == strg) {
                                    _score->select(n);
                                    done = true;
                                    break;
                                    }
                  if (done)
                        break;
                  }
      }
      // otherwise, draw cursor across whole staff
      else {
            h = (lines - 1) * lineDist + 4 * _spatium;
            y -= 2.0 * _spatium;
            }
      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
      }

//---------------------------------------------------------
//   cursorTick
//---------------------------------------------------------

int ScoreView::cursorTick() const
      {
      return _cursor->tick();
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void ScoreView::setCursorOn(bool val)
      {
      if (_cursor && (_cursor->visible() != val)) {
            _cursor->setVisible(val);
            update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
            }
      }

//---------------------------------------------------------
//   setLoopCursor
//    adjust the cursor shape and position to mark the loop
//    isInPos is used to adjust the x position of In vs Out mark
//---------------------------------------------------------

void ScoreView::setLoopCursor(PositionCursor *curLoop, int tick, bool isInPos)
      {
      //
      // set mark height for whole system
      //
      Measure* measure = score()->tick2measure(tick);
      if (measure == 0)
            return;
      qreal x;
      int offset = 0;

      Segment* s;
      for (s = measure->first(Segment::Type::ChordRest); s;) {
            int t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            int t2;
            Segment* ns = s->next(Segment::Type::ChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  x2 = measure->canvasPos().x() + measure->width();
                  }
            t1 += offset;
            t2 += offset;
            if (tick >= t1 && tick < t2) {
                  int   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1) / dt;
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffYpage(0) + system->page()->pos().y();
      double _spatium = score()->spatium();

      qreal mag = _spatium / (MScore::DPI * SPATIUM20);
      double w  = (_spatium * 2.0 + score()->scoreFont()->width(SymId::noteheadBlack, mag))/3;
      double h  = 6 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;

      for (int i = 0; i < _score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show() || !_score->staff(i)->show())
                  continue;
            y2 = ss->y() + ss->bbox().height();
            }
      h += y2;
      y -= 3 * _spatium;

      if (isInPos) {
            x = x - _spatium + w/1.5;
            }
      else {
            x = x - _spatium;
            }
      curLoop->setTick(tick);
      update(_matrix.mapRect(curLoop->rect()).toRect().adjusted(-1,-1,1,1));
      curLoop->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(curLoop->rect()).toRect().adjusted(-1,-1,1,1));
      }

//---------------------------------------------------------
//   setShadowNote
//---------------------------------------------------------

void ScoreView::setShadowNote(const QPointF& p)
      {
      const InputState& is = _score->inputState();
      Position pos;
      if (!score()->getPosition(&pos, p, is.voice())) {
            shadowNote->setVisible(false);
            return;
            }

      shadowNote->setVisible(true);
      Staff* staff      = score()->staff(pos.staffIdx);
      shadowNote->setMag(staff->mag());
      const Instrument* instr     = staff->part()->instr();
      NoteHead::Group noteheadGroup = NoteHead::Group::HEAD_NORMAL;
      int line                    = pos.line;
      NoteHead::Type noteHead       = is.duration().headType();

      if (instr->useDrumset()) {
            Drumset* ds  = instr->drumset();
            int pitch    = is.drumNote();
            if (pitch >= 0 && ds->isValid(pitch)) {
                  line     = ds->line(pitch);
                  noteheadGroup = ds->noteHead(pitch);
                  }
            }
      shadowNote->setLine(line);
      SymId s;
      if (is.rest()) {
            int yo;
            TDuration d(is.duration());
            Rest rest(gscore, d.type());
            rest.setDuration(d.fraction());
            s = rest.getSymbol(is.duration().type(), 0, staff->lines(), &yo);
            }
      else
            s = Note::noteHead(0, noteheadGroup, noteHead);
      shadowNote->setSym(s);
      shadowNote->layout();
      shadowNote->setPos(pos.pos);
      }

//---------------------------------------------------------
//   paintElement
//---------------------------------------------------------

static void paintElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      QPointF pos(e->canvasPos());
      p->translate(pos);
      e->draw(p);
      p->translate(-pos);
      }

//---------------------------------------------------------
//   paintEvent
//    Note: desktop background and paper background are not
//    scaled
//---------------------------------------------------------

void ScoreView::paintEvent(QPaintEvent* ev)
      {
      if (!_score)
            return;
      QPainter vp(this);
      vp.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      vp.setRenderHint(QPainter::TextAntialiasing, true);

      paint(ev->rect(), vp);

      vp.setTransform(_matrix);
      vp.setClipping(false);

      _curLoopIn->paint(&vp);
      _curLoopOut->paint(&vp);
      _cursor->paint(&vp);

      if (_score->layoutMode() == LayoutMode::LINE)
            _continuousPanel->paint(ev->rect(), vp);

      lasso->draw(&vp);
      if (fotoMode())
            _foto->draw(&vp);
      shadowNote->draw(&vp);

      if (dragElement)
            dragElement->scanElements(&vp, paintElement, false);
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / vp.worldTransform().m11(), Qt::DotLine);
            vp.setPen(pen);
            vp.drawLine(dropAnchor);

            qreal d = 4.0 / vp.worldTransform().m11();
            QRectF r(-d, -d, 2 * d, 2 * d);

            vp.setBrush(QBrush(QColor(80, 0, 0)));
            vp.setPen(Qt::NoPen);
            r.moveCenter(dropAnchor.p1());
            vp.drawEllipse(r);
            r.moveCenter(dropAnchor.p2());
            vp.drawEllipse(r);
            }

      if (grips) {
            if (grips == 6) {       // HACK: this are grips of a slur
                  QPolygonF polygon(grips+1);
                  polygon[0] = QPointF(grip[int(GripSlurSegment::START)].center());
                  polygon[1] = QPointF(grip[int(GripSlurSegment::BEZIER1)].center());
                  polygon[2] = QPointF(grip[int(GripSlurSegment::SHOULDER)].center());
                  polygon[3] = QPointF(grip[int(GripSlurSegment::BEZIER2)].center());
                  polygon[4] = QPointF(grip[int(GripSlurSegment::END)].center());
                  polygon[5] = QPointF(grip[int(GripSlurSegment::DRAG)].center());
                  polygon[6] = QPointF(grip[int(GripSlurSegment::START)].center());
                  QPen pen(MScore::frameMarginColor, 0.0);
                  vp.setPen(pen);
                  vp.drawPolyline(polygon);
                  }
            QPen pen(MScore::defaultColor, 0.0);
            vp.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  if (i == curGrip && hasFocus())
                        vp.setBrush(MScore::selectColor[0]);
                  else
                        vp.setBrush(Qt::NoBrush);
                  vp.drawRect(grip[i]);
                  }
            if (editObject) {      // if object is moved, it may not be covered by bsp
                  paintElement(&vp, editObject);
                  }
            }
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void ScoreView::drawBackground(QPainter* p, const QRectF& r) const
      {
      if (score()->printing()) {
            p->fillRect(r, Qt::white);
            return;
            }
      if (fgPixmap == 0 || fgPixmap->isNull())
            p->fillRect(r, _fgColor);
      else {
            p->drawTiledPixmap(r, *fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }
      }

//---------------------------------------------------------
//   paintPageBorder
//---------------------------------------------------------

void ScoreView::paintPageBorder(QPainter& p, Page* page)
      {
      //add a black border to pages
      QRectF r(page->canvasBoundingRect());
      p.setBrush(Qt::NoBrush);
      p.setPen(QPen(QColor(0,0,0,102), 1));
      p.drawRect(r);

#if 0
      QRectF r(page->bbox());
      qreal x1 = r.x();
      qreal y1 = r.y();
      qreal x2 = x1 + r.width();
      qreal y2 = y1 + r.height();

      const QColor c1("#befbbefbbefb");
      const QColor c2("#79e779e779e7");
      int h1, h2, s1, s2, v1, v2;
      int bw = 6;
      c2.getHsv(&h1, &s1, &v1);
      c1.getHsv(&h2, &s2, &v2);

      if (page->no() & 1) {
            int bbw = bw/2-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw/2; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x1+i, y1, x1+i, y2));
                  }
            c1.getHsv(&h1, &s1, &v1);
            c2.getHsv(&h2, &s2, &v2);

            p.fillRect(x2-bw, y1, bw, bw, MScore::bgColor);
            p.fillRect(x2-bw, y2-bw, bw, bw, MScore::bgColor);

            bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x2-bw+i, y1+i+1, x2-bw+i, y2-i-1));
                  }
            }
      else {
            c2.getHsv(&h1, &s1, &v1);
            c1.getHsv(&h2, &s2, &v2);

            p.fillRect(x1, y1, bw, bw, MScore::bgColor);
            p.fillRect(x1, y2-bw, bw, bw, MScore::bgColor);
            int bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x1+i, y1+(bw-i), x1+i, y2-(bw-i)-1));
                  }
            c1.getHsv(&h1, &s1, &v1);
            c2.getHsv(&h2, &s2, &v2);

            bw/=2;
            bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x2-bw+i, y1, x2-bw+i, y2));
                  }
            }
#endif
      if (_score->showPageborders()) {
            // show page margins
            p.setBrush(Qt::NoBrush);
            p.setPen(MScore::frameMarginColor);
            QRectF f(page->canvasBoundingRect());
            f.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
            p.drawRect(f);
            if (!page->isOdd()) {
                  QRectF f(page->canvasBoundingRect());
                  p.drawLine(f.right(), 0.0, f.right(), f.bottom());
                  }
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint(const QRect& r, QPainter& p)
      {
      p.save();
      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(r, _fgColor);
      else {
            p.drawTiledPixmap(r, *fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }

      p.setTransform(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(r));

      QRegion r1(r);
      if (_score->layoutMode() == LayoutMode::LINE) {
            Page* page = _score->pages().front();
            QList<Element*> ell = page->items(fr);
            qStableSort(ell.begin(), ell.end(), elementLessThan);
            drawElements(p, ell);
            }
      else {
            foreach (Page* page, _score->pages()) {
                  if (!score()->printing())
                        paintPageBorder(p, page);
                  QRectF pr(page->abbox().translated(page->pos()));
                  if (pr.right() < fr.left())
                        continue;
                  if (pr.left() > fr.right())
                        break;
                  QList<Element*> ell = page->items(fr.translated(-page->pos()));
                  qStableSort(ell.begin(), ell.end(), elementLessThan);
                  QPointF pos(page->pos());
                  p.translate(pos);
                  drawElements(p, ell);
                  p.translate(-pos);
                  r1 -= _matrix.mapRect(pr).toAlignedRect();
                  }
            }
      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      //
      // frame text in edit mode, except for text in a text frame
      //
      if (editObject && editObject->isText()
         && !(editObject->parent() && editObject->parent()->type() == Element::Type::TBOX)) {
            Element* e = editObject;
            while (e->parent())
                  e = e->parent();
            Text* text = static_cast<Text*>(editObject);
            QRectF r = text->pageRectangle().translated(e->pos()); // abbox();
            qreal w = 2.0 / matrix().m11(); // give the frame some padding, like in 1.3
            r.adjust(-w,-w,w,w);
            p.setPen(QPen(QBrush(Qt::lightGray), 2.0 / matrix().m11()));  // 2 pixel pen size
            p.setBrush(QBrush(Qt::NoBrush));
            p.drawRect(r);
            }
      const Selection& sel = _score->selection();
      if (sel.isRange()) {
            Segment* ss = sel.startSegment();
            Segment* es = sel.endSegment();
            if (!ss)
                  return;
            p.setBrush(Qt::NoBrush);

            QPen pen;
            pen.setColor(MScore::selectColor[0]);
            pen.setWidthF(2.0 / p.matrix().m11());

            pen.setStyle(Qt::SolidLine);

            p.setPen(pen);
            double _spatium = score()->spatium();
            double x2      = ss->pagePos().x() - _spatium;
            int staffStart = sel.staffStart();
            int staffEnd   = sel.staffEnd();

            System* system2 = ss->measure()->system();
            QPointF pt      = ss->pagePos();
            double y        = pt.y();
            SysStaff* ss1   = system2->staff(staffStart);

            // find last visible staff:
            int lastStaff = 0;
            for (int i = staffEnd-1; i >= 0; --i) {
                  if (score()->staff(i)->show()) {
                        lastStaff = i;
                        break;
                        }
                  }
            SysStaff* ss2 = system2->staff(lastStaff);

            double y1 = ss1->y() - 2 * _spatium + y;
            double y2 = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;

            // drag vertical start line
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));

            System* system1 = system2;
            double x1;

            for (Segment* s = ss; s && (s != es);) {
                  Segment* ns = s->next1MM();
                  system1  = system2;
                  system2  = s->measure()->system();
                  pt       = s->pagePos();
                  x1  = x2;
                  x2  = pt.x() + _spatium * 2;

                  // HACK for whole measure rest:
                  if (ns == 0 || ns == es) {    // last segment?
                        Element* e = s->element(staffStart * VOICES);
                        if (e && e->type() == Element::Type::REST && static_cast<Rest*>(e)->durationType().type() == TDuration::DurationType::V_MEASURE)
                              x2 = s->measure()->abbox().right() - _spatium;
                        }

                  if (system2 != system1)
                        x1  = x2 - 2 * _spatium;
                  y   = pt.y();
                  ss1 = system2->staff(staffStart);
                  ss2 = system2->staff(lastStaff);
                  y1  = ss1->y() - 2 * _spatium + y;
                  y2  = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;
                  p.drawLine(QLineF(x1, y1, x2, y1).translated(system2->page()->pos()));
                  p.drawLine(QLineF(x1, y2, x2, y2).translated(system2->page()->pos()));
                  s = ns;
                  }
            //
            // draw vertical end line
            //
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));
            }
      p.setMatrixEnabled(false);
      if ((_score->layoutMode() != LayoutMode::LINE) && !r1.isEmpty()) {
            p.setClipRegion(r1);  // only background
            if (bgPixmap == 0 || bgPixmap->isNull())
                  p.fillRect(r, _bgColor);
            else
                  p.drawTiledPixmap(r, *bgPixmap, r.topLeft() - QPoint(_matrix.m31(), _matrix.m32()));
            }
      p.restore();
      }

//---------------------------------------------------------
//   zoomStep: zoom in or out by some number of steps
//---------------------------------------------------------

void ScoreView::zoomStep(qreal step, const QPoint& pos)
      {
      qreal _mag = mag();

      _mag *= qPow(1.1, step);

      zoom(_mag, QPointF(pos));
      }

//---------------------------------------------------------
//   zoom: zoom to some absolute zoom level
//---------------------------------------------------------

void ScoreView::zoom(qreal _mag, const QPointF& pos)
      {
      QPointF p1 = imatrix.map(pos);

      if (_mag > 16.0)
            _mag = 16.0;
      else if (_mag < 0.05)
            _mag = 0.05;

      mscore->setMag(_mag);
      setMag(_mag);
      _magIdx = MagIdx::MAG_FREE;

      QPointF p2 = imatrix.map(pos);
      QPointF p3 = p2 - p1;

      double m = _mag;

      int dx    = lrint(p3.x() * m);
      int dy    = lrint(p3.y() * m);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit viewRectChanged();
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      update();
      }

//---------------------------------------------------------
//   gestureEvent
//---------------------------------------------------------

bool ScoreView::gestureEvent(QGestureEvent *event)
      {
      if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
            // Zoom in/out when receiving a pinch gesture
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);

            static qreal magStart = 1.0;

            if (pinch->state() == Qt::GestureStarted) {
                  magStart = mag();
                  }
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
                  qreal value = pinch->property("scaleFactor").toReal();
                  zoom(magStart*value, pinch->startCenterPoint());
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
      QPoint stepsScrolled = event->angleDelta();

      int dx = 0, dy = 0, n = 0;
      qreal nReal = 0.0;

      if (!pixelsScrolled.isNull()) {
            dx = pixelsScrolled.x();
            dy = pixelsScrolled.y();
            nReal = static_cast<qreal>(dx) / PIXELSSTEPSFACTOR;
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

      if (event->modifiers() & Qt::ControlModifier) {
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

//-----------------------------------------------------------------------------
//   constraints
// (ws: too restrictive; it should be possible at least to move the right edge
//    of the score to center of the screen (or top, left, bottom).
//    I do this often before zooming in.
//    I believe there should be no limitation at all. Its irritating that dragging
//    the canvas sometimes work and sometimes not bc. of some arbitrary limitation
//    which is not immediate obvious.)
//-----------------------------------------------------------------------------

void ScoreView::constraintCanvas (int* /*dxx*/, int* /*dyy*/)
      {
#if 0
      const qreal margin = 50.0; //move to preferences?
      int dx = *dxx;
      int dy = *dyy;
      QRectF rect = QRectF(0, 0, width(), height());

      Page* firstPage = score()->pages().front();
      Page* lastPage = score()->pages().back();

      if (firstPage && lastPage) {
            QPointF offsetPt(xoffset(), yoffset());
            QRectF firstPageRect(firstPage->pos().x() * mag(),
                                      firstPage->pos().y() * mag(),
                                      firstPage->width() * mag(),
                                      firstPage->height() * mag());
            QRectF lastPageRect(lastPage->pos().x() * mag(),
                                         lastPage->pos().y() * mag(),
                                         lastPage->width() * mag(),
                                         lastPage->height() * mag());
            QRectF pagesRect = firstPageRect.unite(lastPageRect).translated(offsetPt);
            pagesRect.adjust(-margin, -margin, margin, margin);
            QRectF toPagesRect = pagesRect.translated(dx, dy);

            // move right
            if (dx > 0) {
                  if (toPagesRect.right() > rect.right() && toPagesRect.left() > rect.left()) {
                        if(pagesRect.width() <= rect.width()) {
                              dx = rect.right() - pagesRect.right();
                              }
                        else {
                              dx = rect.left() - pagesRect.left();
                              }
                        }
                  }
            else { // move left, dx < 0
                  if (toPagesRect.left() < rect.left() && toPagesRect.right() < rect.right()) {
                        if (pagesRect.width() <= rect.width()) {
                              dx = rect.left() - pagesRect.left();
                              }
                        else {
                              dx = rect.right() - pagesRect.right();
                              }
                        }
                  }

            // move down
            if (dy > 0) {
                  if (toPagesRect.bottom() > rect.bottom() && toPagesRect.top() > rect.top()) {
                        if (pagesRect.height() <= rect.height()) {
                              dy = rect.bottom() - pagesRect.bottom();
                              }
                        else {
                              dy = rect.top() - pagesRect.top();
                              }
                        }
                  }
            else { // move up, dy < 0
                  if (toPagesRect.top() < rect.top() && toPagesRect.bottom() < rect.bottom()) {
                        if (pagesRect.height() <= rect.height()) {
                              dy = rect.top() - pagesRect.top();
                              }
                        else {
                              dy = rect.bottom() - pagesRect.bottom();
                              }
                        }
                  }
            }
      *dxx = dx;
      *dyy = dy;
#endif
      }

//---------------------------------------------------------
//   drawDebugInfo
//---------------------------------------------------------

static void drawDebugInfo(QPainter& p, const Element* _e)
      {
      const Element* e = _e;
#if 0
      if (e->type() == Element::Type::NOTE) {
            e = e->parent();
            const ChordRest* cr = static_cast<const ChordRest*>(e);
            p.setPen(Qt::red);
            p.setBrush(Qt::NoBrush);
            QRectF bb = cr->bbox();
            qreal x1, y1, x2, y2;
            bb.getCoords(&x1, &y1, &x2, &y2);

            QPointF pos(e->pagePos());
            p.translate(pos);
            Space sp = cr->space();
            QRectF r;
            r.setCoords(-sp.lw(), y1, sp.rw(), y2);
            p.drawRect(r);
            p.translate(-pos);
            return;
            }
#endif
      //
      //  draw bounding box rectangle for all
      //  selected Elements
      //
      QPointF pos(e->pagePos());
      p.translate(pos);
      p.setBrush(Qt::NoBrush);

      p.setPen(QPen(Qt::red, 0.0));
      // p.drawPath(e->shape());
      p.drawRect(e->bbox());

      p.setPen(QPen(Qt::red, 0.0));
      qreal w = 5.0 / p.matrix().m11();
      qreal h = w;
      qreal x = 0; // e->bbox().x();
      qreal y = 0; // e->bbox().y();
      p.drawLine(QLineF(x-w, y-h, x+w, y+h));
      p.drawLine(QLineF(x+w, y-h, x-w, y+h));

      p.translate(-pos);
      if (e->parent()) {
            const Element* ee = e->parent();
            if (e->type() == Element::Type::NOTE)
                  ee = static_cast<const Note*>(e)->chord()->segment();
            else if (e->type() == Element::Type::CLEF)
                  ee = static_cast<const Clef*>(e)->segment();

            p.setPen(QPen(Qt::green, 0.0));

            p.drawRect(ee->pageBoundingRect());

            if (ee->type() == Element::Type::SEGMENT) {
                  QPointF pt = ee->pagePos();
                  p.setPen(QPen(Qt::blue, 0.0));
                  p.drawLine(QLineF(pt.x()-w, pt.y()-h, pt.x()+w, pt.y()+h));
                  p.drawLine(QLineF(pt.x()+w, pt.y()-h, pt.x()-w, pt.y()+h));
                  }
            }
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void ScoreView::drawElements(QPainter& painter, const QList<Element*>& el)
      {
      for (const Element* e : el) {
            e->itemDiscovered = 0;
            if (!e->visible() && (score()->printing() || !score()->showInvisible()))
                  continue;
            QPointF pos(e->pagePos());
            painter.translate(pos);
            e->draw(&painter);
            painter.translate(-pos);
            if (MScore::debugMode && e->selected())
                  drawDebugInfo(painter, e);
            }
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void ScoreView::setMag(qreal nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m13(), _matrix.m21(),
         nmag, _matrix.m23(), _matrix.dx()*deltamag, _matrix.dy()*deltamag, _matrix.m33());
      imatrix = _matrix.inverted();
      emit scaleChanged(nmag * score()->spatium());
      if (grips) {
            qreal w = 8.0 / nmag;
            qreal h = 8.0 / nmag;
            QRectF r(-w*.5, -h*.5, w, h);
            for (int i = 0; i < grips; ++i) {
                  QPointF p(grip[i].center());
                  grip[i] = r.translated(p);
                  }
            }
      }

//---------------------------------------------------------
//   setMagIdx
//---------------------------------------------------------

void ScoreView::setMag(MagIdx idx, double mag)
      {
      _magIdx = idx;
      setMag(mag);
      update();
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
//   setFocusRect
//---------------------------------------------------------

void ScoreView::setFocusRect()
      {
      if (mscore->splitScreen()) {
            if (!focusFrame) {
                  focusFrame = new QFocusFrame;
                  QPalette p(focusFrame->palette());
                  p.setColor(QPalette::WindowText, MScore::selectColor[0]);
                  focusFrame->setPalette(p);
                  }
            focusFrame->setWidget(static_cast<QWidget*>(this));
            focusFrame->show();
            }
      else {
            if (focusFrame)
                  focusFrame->setWidget(0);
            }
      }

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void ScoreView::editCopy()
      {
      if (editObject && editObject->isText()) {
            //
            // store selection as plain text
            //
            Text* text = static_cast<Text*>(editObject);
            QString s = text->selectedText();
            if (!s.isEmpty())
                  QApplication::clipboard()->setText(s, QClipboard::Clipboard);
            }
      }

//---------------------------------------------------------
//   normalCopy
//---------------------------------------------------------

void ScoreView::normalCopy()
      {
      if (!_score->selection().canCopy()) {
            qDebug("cannot copy selection: intersects a tuplet");
            QMessageBox::information(0, "MuseScore",
                  tr("Please select the complete tuplet and retry the copy operation"),
                  QMessageBox::Ok, QMessageBox::NoButton);
            return;
            }
      QString mimeType = _score->selection().mimeType();
      if (!mimeType.isEmpty()) {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData(mimeType, _score->selection().mimeData());
            if (MScore::debugMode)
                  qDebug("cmd copy: <%s>", mimeData->data(mimeType).data());
            QApplication::clipboard()->setMimeData(mimeData);
            }
      }

//---------------------------------------------------------
//   normalCut
//---------------------------------------------------------

void ScoreView::normalCut()
      {
      if (!_score->selection().canCopy()) {
            qDebug("cannot copy selection: intersects a tuplet");
            QMessageBox::information(0, "MuseScore",
                  tr("Please select the complete tuplet and retry the cut operation"),
                  QMessageBox::Ok, QMessageBox::NoButton);
            return;
            }
      _score->startCmd();
      normalCopy();
      _score->cmdDeleteSelection();
      _score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   editPaste
//---------------------------------------------------------

void ScoreView::editPaste()
      {
      if (editObject->isText()) {
            if (editObject->type() == Element::Type::LYRICS)
                  static_cast<Lyrics*>(editObject)->paste(this);
            else
                  static_cast<Text*>(editObject)->paste();
            }
      }

//---------------------------------------------------------
//   normalPaste
//---------------------------------------------------------

void ScoreView::normalPaste()
      {
      _score->startCmd();

      const QMimeData* ms = QApplication::clipboard()->mimeData();
      PasteStatus status = _score->cmdPaste(ms,this);
      switch (status) {
            case PasteStatus::NO_DEST:
                  errorMessage->showMessage(tr("no destination to paste"), "pasteDestination");
                  break;
            case PasteStatus::DEST_TUPLET:
                  errorMessage->showMessage(tr("cannot paste into tuplet"), "pasteTuplet");
                  break;
            case PasteStatus::TUPLET_CROSSES_BAR:
                  errorMessage->showMessage(tr("tuplet cannot cross bars"), "tupletCrossBar");
                  _score->undo()->current()->unwind();
                  break;
            default:
                  ;
           }

      _score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   cmdGotoElement
//---------------------------------------------------------

void ScoreView::cmdGotoElement(Element* e)
      {
      if (e) {
            if (e->type() == Element::Type::NOTE)
                  score()->setPlayNote(true);
            score()->select(e, SelectType::SINGLE, 0);
            moveCursor();
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void ScoreView::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
      if (MScore::debugMode)
            qDebug("ScoreView::cmd <%s>", qPrintable(cmd));

      if (cmd == "escape")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "note-input" || cmd == "copy" || cmd == "paste"
         || cmd == "cut" || cmd == "fotomode") {
            sm->postEvent(new CommandEvent(cmd));
            }
      else if (cmd == "lyrics") {
            Lyrics* lyrics = _score->addLyrics();
            if (lyrics) {
                  _score->setLayoutAll(true);
                  startEdit(lyrics);
                  return;     // no endCmd()
                  }
            }
      else if (cmd == "figured-bass") {
            FiguredBass* fb = _score->addFiguredBass();
            if (fb) {
                  _score->setLayoutAll(true);
                  startEdit(fb);
                  return;     // no endCmd()
                  }
            }
      else if (cmd == "mag")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "add-slur")
            cmdAddSlur();
      else if (cmd == "add-hairpin")
            cmdAddHairpin(false);
      else if (cmd == "add-hairpin-reverse")
            cmdAddHairpin(true);
      else if (cmd == "add-noteline")
            cmdAddNoteLine();
      else if (cmd == "note-c")
            cmdAddPitch(0, false);
      else if (cmd == "note-d")
            cmdAddPitch(1, false);
      else if (cmd == "note-e")
            cmdAddPitch(2, false);
      else if (cmd == "note-f")
            cmdAddPitch(3, false);
      else if (cmd == "note-g")
            cmdAddPitch(4, false);
      else if (cmd == "note-a")
            cmdAddPitch(5, false);
      else if (cmd == "note-b")
            cmdAddPitch(6, false);
      else if (cmd == "chord-c")
            cmdAddPitch(0, true);
      else if (cmd == "chord-d")
            cmdAddPitch(1, true);
      else if (cmd == "chord-e")
            cmdAddPitch(2, true);
      else if (cmd == "chord-f")
            cmdAddPitch(3, true);
      else if (cmd == "chord-g")
            cmdAddPitch(4, true);
      else if (cmd == "chord-a")
            cmdAddPitch(5, true);
      else if (cmd == "chord-b")
            cmdAddPitch(6, true);
      else if (cmd == "insert-c")
            cmdInsertNote(0);
      else if (cmd == "insert-d")
            cmdInsertNote(1);
      else if (cmd == "insert-e")
            cmdInsertNote(2);
      else if (cmd == "insert-f")
            cmdInsertNote(3);
      else if (cmd == "insert-g")
            cmdInsertNote(4);
      else if (cmd == "insert-a")
            cmdInsertNote(5);
      else if (cmd == "insert-b")
            cmdInsertNote(6);
      else if (cmd == "chord-text")
            cmdAddChordName();

      else if (cmd == "title-text")
            cmdAddText(TEXT::TITLE);
      else if (cmd == "subtitle-text")
            cmdAddText(TEXT::SUBTITLE);
      else if (cmd == "composer-text")
            cmdAddText(TEXT::COMPOSER);
      else if (cmd == "poet-text")
            cmdAddText(TEXT::POET);
      else if (cmd == "system-text")
            cmdAddText(TEXT::SYSTEM);
      else if (cmd == "staff-text")
            cmdAddText(TEXT::STAFF);
      else if (cmd == "rehearsalmark-text")
            cmdAddText(TEXT::REHEARSAL_MARK);

      else if (cmd == "edit-element") {
            Element* e = _score->selection().element();
            if (e) {
                  _score->setLayoutAll(false);
                  startEdit(e);
                  }
            }
      else if (cmd == "play") {
            if (seq && seq->canStart())
                  sm->postEvent(new CommandEvent(cmd));
            else
                  getAction("play")->setChecked(false);
            }
      else if (cmd == "find")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "page-prev")
            pagePrev();
      else if (cmd == "page-next")
            pageNext();
      else if (cmd == "page-top")
            pageTop();
      else if (cmd == "page-end")
            pageEnd();
      else if (cmd == "select-next-chord"
         || cmd == "select-prev-chord"
         || cmd == "select-next-measure"
         || cmd == "select-prev-measure"
         || cmd == "select-begin-line"
         || cmd == "select-end-line"
         || cmd == "select-begin-score"
         || cmd == "select-end-score"
         || cmd == "select-staff-above"
         || cmd == "select-staff-below") {
            Element* el = _score->selectMove(cmd);
            if (el)
                  adjustCanvasPosition(el, false);
            updateAll();
            }
      else if (cmd == "next-chord"
         || cmd == "prev-chord"
         || cmd == "next-track"
         || cmd == "prev-track"
         || cmd == "next-measure"
         || cmd == "prev-measure") {
            Element* el = score()->selection().element();
            if (el && (el->isText())) {
                  score()->startCmd();
                  if (cmd == "prev-chord")
                        el->undoChangeProperty(P_ID::USER_OFF, el->userOff() - QPointF (MScore::nudgeStep * el->spatium(), 0.0));
                  else if (cmd == "next-chord")
                        el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF (MScore::nudgeStep * el->spatium(), 0.0));
                  else if (cmd == "prev-measure")
                        el->undoChangeProperty(P_ID::USER_OFF, el->userOff() - QPointF (MScore::nudgeStep10 * el->spatium(), 0.0));
                  else if (cmd == "next-measure")
                        el->undoChangeProperty(P_ID::USER_OFF, el->userOff() + QPointF (MScore::nudgeStep10 * el->spatium(), 0.0));
                  score()->endCmd();
                  }
            else {
                  Element* el = _score->move(cmd);
                  if (el)
                        adjustCanvasPosition(el, false);
                  updateAll();
                  }
            }
      else if (cmd == "pitch-up-diatonic")
            score()->upDown(true, UpDownMode::DIATONIC);
      else if (cmd == "pitch-down-diatonic")
            score()->upDown(false, UpDownMode::DIATONIC);
      else if (cmd == "move-up") {
            Element* el = score()->selection().element();
            if (el && el->type() == Element::Type::NOTE) {
                  Note* note = static_cast<Note*>(el);
                  score()->moveUp(note->chord());
                  }
            }
      else if (cmd == "move-down") {
            Element* el = score()->selection().element();
            if (el && el->type() == Element::Type::NOTE) {
                  Note* note = static_cast<Note*>(el);
                  score()->moveDown(note->chord());
                  }
            }
      else if (cmd == "up-chord") {
            Element* el = score()->selection().element();
            if (el && (el->type() == Element::Type::NOTE || el->type() == Element::Type::REST))
                  cmdGotoElement(score()->upAlt(el));
            }
      else if (cmd == "down-chord") {
            Element* el = score()->selection().element();
            if (el && (el->type() == Element::Type::NOTE || el->type() == Element::Type::REST))
                  cmdGotoElement(score()->downAlt(el));
            }
      else if (cmd == "top-chord" ) {
            Element* el = score()->selection().element();
            if (el && el->type() == Element::Type::NOTE)
                  cmdGotoElement(score()->upAltCtrl(static_cast<Note*>(el)));
            }
      else if (cmd == "bottom-chord") {
            Element* el = score()->selection().element();
            if (el && el->type() == Element::Type::NOTE)
                  cmdGotoElement(score()->downAltCtrl(static_cast<Note*>(el)));
            }
      else if (cmd == "rest" || cmd == "rest-TAB")
            cmdEnterRest();
      else if (cmd == "rest-1")
            cmdEnterRest(TDuration(TDuration::DurationType::V_WHOLE));
      else if (cmd == "rest-2")
            cmdEnterRest(TDuration(TDuration::DurationType::V_HALF));
      else if (cmd == "rest-4")
            cmdEnterRest(TDuration(TDuration::DurationType::V_QUARTER));
      else if (cmd == "rest-8")
            cmdEnterRest(TDuration(TDuration::DurationType::V_EIGHT));
      else if (cmd.startsWith("interval")) {
            int n = cmd.mid(8).toInt();
            QList<Note*> nl = _score->selection().noteList();
            if (!nl.isEmpty()) {
                  if (!noteEntryMode())
                        sm->postEvent(new CommandEvent("note-input"));
                  _score->cmdAddInterval(n, nl);
                  }
            }
      else if (cmd == "tie") {
            _score->cmdAddTie();
            moveCursor();
            }
      else if (cmd == "duplet")
            cmdTuplet(2);
      else if (cmd == "triplet")
            cmdTuplet(3);
      else if (cmd == "quadruplet")
            cmdTuplet(4);
      else if (cmd == "quintuplet")
            cmdTuplet(5);
      else if (cmd == "sextuplet")
            cmdTuplet(6);
      else if (cmd == "septuplet")
            cmdTuplet(7);
      else if (cmd == "octuplet")
            cmdTuplet(8);
      else if (cmd == "nonuplet")
            cmdTuplet(9);
      else if (cmd == "tuplet-dialog") {
            _score->startCmd();
            Tuplet* tuplet = mscore->tupletDialog();
            if (tuplet)
                  cmdCreateTuplet(_score->getSelectedChordRest(), tuplet);
            _score->endCmd();
            moveCursor();
            }
      else if (cmd == "repeat-sel")
            cmdRepeatSelection();
      else if (cmd == "voice-1")
            changeVoice(0);
      else if (cmd == "voice-2")
            changeVoice(1);
      else if (cmd == "voice-3")
            changeVoice(2);
      else if (cmd == "voice-4")
            changeVoice(3);
      else if (cmd == "enh-up")
            cmdChangeEnharmonic(true);
      else if (cmd == "enh-down")
            cmdChangeEnharmonic(false);
      else if (cmd == "revision") {
            Score* s = _score;
            if (s->parentScore())
                  s = s->parentScore();
            s->createRevision();
            }
      else if (cmd == "append-measure")
            cmdAppendMeasures(1, Element::Type::MEASURE);
      else if (cmd == "insert-measure")
          cmdInsertMeasures(1, Element::Type::MEASURE);
      else if (cmd == "insert-hbox")
          cmdInsertMeasures(1, Element::Type::HBOX);
      else if (cmd == "insert-vbox")
          cmdInsertMeasures(1, Element::Type::VBOX);
      else if (cmd == "append-hbox") {
          MeasureBase* mb = appendMeasure(Element::Type::HBOX);
            _score->select(mb, SelectType::SINGLE, 0);
            }
      else if (cmd == "append-vbox") {
          MeasureBase* mb = appendMeasure(Element::Type::VBOX);
            _score->select(mb, SelectType::SINGLE, 0);
            }
      else if (cmd == "insert-textframe")
            cmdInsertMeasure(Element::Type::TBOX);
      else if (cmd == "append-textframe") {
            MeasureBase* mb = appendMeasure(Element::Type::TBOX);
            if (mb) {
                  TBox* tf = static_cast<TBox*>(mb);
                  Text* text = 0;
                  foreach(Element* e, *tf->el()) {
                        if (e->type() == Element::Type::TEXT) {
                              text = static_cast<Text*>(e);
                              break;
                              }
                        }
                  if (text) {
                        _score->select(text, SelectType::SINGLE, 0);
                        startEdit(text);
                        }
                  }
            }
      else if (cmd == "insert-fretframe")
            cmdInsertMeasure(Element::Type::FBOX);
      else if (cmd == "move-left")
            cmdMoveCR(true);
      else if (cmd == "move-right")
            cmdMoveCR(false);
      else if (cmd == "reset") {
            if (editMode()) {
                  editObject->reset();
                  updateGrips();
                  _score->end();
                  }
            else {
                  _score->startCmd();
                  foreach(Element* e, _score->selection().elements())
                        e->reset();
                  _score->endCmd();
                  }
            _score->setLayoutAll(true);
            }
#ifdef OMR
      else if (cmd == "show-omr") {
            if (_score->omr())
                  showOmr(!_score->showOmr());
            }
#endif
      else if (cmd == "split-measure") {
            Element* e = _score->selection().element();
            if (!(e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST))) {
                  QMessageBox::warning(0, "MuseScore",
                     tr("No chord/rest selected:\n"
                     "please select a chord/rest and try again"));
                  }
            else {
                  if (e->type() == Element::Type::NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if (cr->segment()->splitsTuplet()) {
                        QMessageBox::warning(0, "MuseScore",
                           tr("Cannot split measure here:\n"
                           "cannot split tuplet"));
                        }
                  else
                        _score->cmdSplitMeasure(cr);
                  }
            }
      else if (cmd == "join-measure") {
            Measure* m1;
            Measure* m2;
            if (!_score->selection().measureRange(&m1, &m2) || m1 == m2) {
                  QMessageBox::warning(0, "MuseScore",
                     tr("No measures selected:\n"
                     "please select a range of measures to join and try again"));
                  }
            else {
                  _score->cmdJoinMeasure(m1, m2);
                  }
            }
      else if (cmd == "delete") {
            _score->cmdDeleteSelection();
            }
      else if (cmd == "full-measure-rest") {
            _score->cmdFullMeasureRest();
            }
      else if (cmd == "next-lyric" || cmd == "prev-lyric") {
            editCmd(cmd);
            }
      else if (cmd == "toggle-visible") {
            _score->startCmd();
            foreach(Element* e, _score->selection().elements())
                  _score->undo(new ChangeProperty(e, P_ID::VISIBLE, !e->getProperty(P_ID::VISIBLE).toBool()));
            _score->endCmd();
            mscore->endCmd();
            }
      else if (cmd == "set-visible") {
            _score->startCmd();
            foreach(Element* e, _score->selection().elements())
                  _score->undo(new ChangeProperty(e, P_ID::VISIBLE, true));
            _score->endCmd();
            mscore->endCmd();
            }
      else if (cmd == "unset-visible") {
            _score->startCmd();
            foreach(Element* e, _score->selection().elements())
                  _score->undo(new ChangeProperty(e, P_ID::VISIBLE, false));
            _score->endCmd();
            mscore->endCmd();
            }

      // STATE_HARMONY_FIGBASS_EDIT actions

      else if (cmd == "advance-longa") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division << 4);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division << 4);
            }
      else if (cmd == "advance-breve") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division << 3);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division << 3);
            }
      else if (cmd == "advance-1") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division << 2);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division << 2);
            }
      else if (cmd == "advance-2") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division << 1);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division << 1);
            }
      else if (cmd == "advance-4") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division);
            }
      else if (cmd == "advance-8") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division >> 1);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division >> 1);
            }
      else if (cmd == "advance-16") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division >> 2);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division >> 2);
            }
      else if (cmd == "advance-32") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division >> 3);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division >> 3);
            }
      else if (cmd == "advance-64") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTicksTab(MScore::division >> 4);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTicksTab(MScore::division >> 4);
            }
      else if (cmd == "prev-measure-TEXT") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTab(true);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTab(true, true);
            }
      else if (cmd == "next-measure-TEXT") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyTab(false);
            else if (editObject->type() == Element::Type::FIGURED_BASS)
                  figuredBassTab(true,false);
            }
      else if (cmd == "prev-beat-TEXT") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyBeatsTab(false, true);
            }
      else if (cmd == "next-beat-TEXT") {
            if (editObject->type() == Element::Type::HARMONY)
                  harmonyBeatsTab(false,false);
            }

      // STATE_NOTE_ENTRY_TAB actions

      else if(cmd == "string-above") {
            int   strg = _score->inputState().string();
            if(strg > 0) {
                  _score->inputState().setString(strg-1);
                  moveCursor();
                  }
            }
      else if(cmd == "string-below") {
            InputState& is          = _score->inputState();
            int         maxStrg     = _score->staff(is.track() / VOICES)->lines() - 1;
            int         strg        = is.string();
            if(strg < maxStrg) {
                  is.setString(strg+1);
                  moveCursor();
                  }
            }
      else if(cmd == "fret-0")
            cmdAddFret(0);
      else if(cmd == "fret-1")
            cmdAddFret(1);
      else if(cmd == "fret-2")
            cmdAddFret(2);
      else if(cmd == "fret-3")
            cmdAddFret(3);
      else if(cmd == "fret-4")
            cmdAddFret(4);
      else if(cmd == "fret-5")
            cmdAddFret(5);
      else if(cmd == "fret-6")
            cmdAddFret(6);
      else if(cmd == "fret-7")
            cmdAddFret(7);
      else if(cmd == "fret-8")
            cmdAddFret(8);
      else if(cmd == "fret-9")
            cmdAddFret(9);
      else if(cmd == "fret-10")
            cmdAddFret(10);
      else if(cmd == "fret-11")
            cmdAddFret(11);
      else if(cmd == "fret-12")
            cmdAddFret(12);
      else if(cmd == "fret-13")
            cmdAddFret(13);
      else if(cmd == "fret-14")
            cmdAddFret(14);
      else
            _score->cmd(a);
      if (_score->processMidiInput())
            mscore->endCmd();
      }

//---------------------------------------------------------
//   showOmr
//---------------------------------------------------------

void ScoreView::showOmr(bool flag)
      {
      _score->setShowOmr(flag);
      ScoreTab* t = mscore->getTab1();
      if (t->view() != this)
            t = mscore->getTab2();
      if (t->view() == this)
            t->setCurrent(t->currentIndex());
      else
            qDebug("view not found");
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void ScoreView::startNoteEntry()
      {
      InputState& is = _score->inputState();

      is.setSegment(0);
      Note* note  = 0;
      Element* el = _score->selection().activeCR() ? _score->selection().activeCR() : _score->selection().element();
      if (el == 0 || (el->type() != Element::Type::CHORD && el->type() != Element::Type::REST && el->type() != Element::Type::NOTE)) {
            // if no note/rest is selected, start with voice 0
            int track = is.track() == -1 ? 0 : (is.track() / VOICES) * VOICES;
            el = _score->searchNote(0, track);
            Q_ASSERT(el);
            }
      if (el->type() == Element::Type::CHORD) {
            Chord* c = static_cast<Chord*>(el);
            note = c->selectedNote();
            if (note == 0)
                  note = c->upNote();
            el    = note;
            }
      TDuration d(is.duration());
      if (!d.isValid() || d.isZero() || d.type() == TDuration::DurationType::V_MEASURE)
            is.setDuration(TDuration(TDuration::DurationType::V_QUARTER));

      _score->select(el, SelectType::SINGLE, 0);
      is.update(el);
      is.setRest(false);
      is.setNoteEntryMode(true);

      getAction("pad-rest")->setChecked(false);
      setMouseTracking(true);
      shadowNote->setVisible(true);
      dragElement = 0;
      _score->setUpdateAll();
      _score->end();

      Staff* staff = _score->staff(is.track() / VOICES);
      switch (staff->staffType()->group()) {
            case StaffGroup::STANDARD:
                  break;
            case StaffGroup::TAB: {
                  int strg = 0;                 // assume topmost string as current string
                  // if entering note entry with a note selected and the note has a string
                  // set InputState::_string to note visual string
                  if (el->type() == Element::Type::NOTE) {
                        strg = (static_cast<Note*>(el))->string();
                        strg = staff->staffType()->physStringToVisual(strg);
                        }
                  is.setString(strg);
                  break;
                  }
            case StaffGroup::PERCUSSION:
                  break;
            }

      // set cursor after setting the stafftype-dependent state
      moveCursor();
      setCursorOn(true);
      }

//---------------------------------------------------------
//   endNoteEntry
//---------------------------------------------------------

void ScoreView::endNoteEntry()
      {
      InputState& is = _score->inputState();
      is.setNoteEntryMode(false);
      if (is.slur()) {
            const QList<SpannerSegment*>& el = is.slur()->spannerSegments();
            if (!el.isEmpty())
                  el.front()->setSelected(false);
            is.setSlur(nullptr);
            }
      setMouseTracking(false);
      shadowNote->setVisible(false);
      setCursorOn(false);
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   contextPopup
//---------------------------------------------------------

void ScoreView::contextPopup(QContextMenuEvent* ev)
      {
      QPoint gp = ev->globalPos();

      data.startMove = toLogical(ev->pos());
      Element* e = elementNear(data.startMove);
      if (e) {
            if (!e->selected()) {
                  // bool control = (ev->modifiers() & Qt::ControlModifier) ? true : false;
                  // _score->select(e, control ? SelectType::ADD : SelectType::SINGLE, 0);
                  curElement = e;
//TODO?                  select(ev);
                  }
            Element::Type type = e->type();
            if (seq)
                  seq->stopNotes();       // stop now because we dont get a mouseRelease event
            if (type == Element::Type::MEASURE)
                  measurePopup(gp, static_cast<Measure*>(e));
            else {
                  objectPopup(gp, e);
                  }
            }
      else {
            QMenu* popup = mscore->genCreateMenu();
            _score->setLayoutAll(true);
            _score->end();
            popup->popup(gp);
            }
      }

//---------------------------------------------------------
//   dragScoreView
//---------------------------------------------------------

void ScoreView::dragScoreView(QMouseEvent* ev)
      {
      QPoint d = ev->pos() - _matrix.map(data.startMove).toPoint();
      int dx   = d.x();
      int dy   = d.y();

      if (dx == 0 && dy == 0)
            return;

      constraintCanvas(&dx, &dy);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      // scroll schedules an update which is probably too small
      // hack around:
      if (dx > 0)
            update(-10, 0, dx + 50, height());
      else if (dx < 0)
            update(width() - 50 + dx, 0, width() + 10, height());
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      emit viewRectChanged();
      }

//---------------------------------------------------------
//   dragNoteEntry
//    mouse move event in note entry mode
//---------------------------------------------------------

void ScoreView::dragNoteEntry(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      QRectF r(shadowNote->canvasBoundingRect());
      setShadowNote(p);
      r |= shadowNote->canvasBoundingRect();
      update(toPhysical(r).adjusted(-2, -2, 2, 2));
      }

//---------------------------------------------------------
//   noteEntryButton
//    mouse button press in note entry mode
//---------------------------------------------------------

void ScoreView::noteEntryButton(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->startCmd();
      _score->putNote(p, ev->modifiers() & Qt::ShiftModifier);
      _score->endCmd();
      mscore->endCmd();
      ChordRest* cr = _score->inputState().cr();
      if (cr)
            adjustCanvasPosition(cr, false);
      }

//---------------------------------------------------------
//   select
//---------------------------------------------------------

void ScoreView::select(QMouseEvent* ev)
      {
      Qt::KeyboardModifiers keyState = ev->modifiers();
      if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
            cloneElement(curElement);
            return;
            }
      Element::Type type = curElement->type();
      int dragStaffIdx = 0;
      if (type == Element::Type::MEASURE) {
            System* dragSystem = (System*)(curElement->parent());
            dragStaffIdx  = getStaff(dragSystem, data.startMove);
            }
      if ((ev->type() == QEvent::MouseButtonRelease) && ((!curElement->selected() || addSelect)))
            return;
      // As findSelectableElement may return a measure
      // when clicked "a little bit" above or below it, getStaff
      // may not find the staff and return -1, which would cause
      // select() to crash
      if (dragStaffIdx >= 0) {
            SelectType st = SelectType::SINGLE;
            if (keyState == Qt::NoModifier)
                  st = SelectType::SINGLE;
            else if (keyState & Qt::ShiftModifier)
                  st = SelectType::RANGE;
            else if (keyState & Qt::ControlModifier) {
                  if (curElement->selected() && (ev->type() == QEvent::MouseButtonPress)) {
                        // do not deselect on ButtonPress, only on ButtonRelease
                        addSelect = false;
                        return;
                        }
                  addSelect = true;
                  st = SelectType::ADD;
                  }
            _score->select(curElement, st, dragStaffIdx);
            if (curElement && curElement->type() == Element::Type::NOTE) {
                  Note* note = static_cast<Note*>(curElement);
                  int pitch = note->ppitch();
                  mscore->play(note, pitch);
                  }
            }
      else
            curElement = 0;
      _score->setUpdateAll(true);   //DEBUG
      mscore->endCmd();
      }

//---------------------------------------------------------
//   mousePress
//    return true if element is clicked
//---------------------------------------------------------

bool ScoreView::mousePress(QMouseEvent* ev)
      {
      startMoveI = ev->pos();
      data.startMove  = imatrix.map(QPointF(startMoveI));
      curElement = elementNear(data.startMove);

      if (curElement && curElement->type() == Element::Type::MEASURE) {
            System* dragSystem = (System*)(curElement->parent());
            int dragStaffIdx  = getStaff(dragSystem, data.startMove);
            if (dragStaffIdx < 0)
                  curElement = 0;
            }
      return curElement != 0;
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void ScoreView::mouseReleaseEvent(QMouseEvent* event)
      {
      if (seq)
            seq->stopNoteTimer();
      QWidget::mouseReleaseEvent(event);
      }

//---------------------------------------------------------
//   onEditPasteTransition
//---------------------------------------------------------

void ScoreView::onEditPasteTransition(QMouseEvent* ev)
      {
      data.startMove = imatrix.map(QPointF(ev->pos()));
      Element* e = elementNear(data.startMove);
      if (e == editObject) {
            if (editObject->mousePress(data.startMove, ev)) {
                  _score->addRefresh(editObject->canvasBoundingRect());
                  _score->end();
                  }
            }
      }

//---------------------------------------------------------
//   editScoreViewDragTransition
//    Check for mouse click outside of editObject.
//---------------------------------------------------------

bool ScoreView::editScoreViewDragTransition(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      Element* e = elementNear(p);

      if (e == 0 || e->type() == Element::Type::MEASURE) {
            data.startMove   = p;
//TODOxxx            dragElement = e;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   editSelectTransition
//    Check for mouse click outside of editObject.
//---------------------------------------------------------

bool ScoreView::editSelectTransition(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      Element* e = elementNear(p);

      if (e != editObject) {
            data.startMove   = p;
//TODOxxx            dragElement = e;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   doDragLasso
//---------------------------------------------------------

void ScoreView::doDragLasso(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->addRefresh(lasso->canvasBoundingRect());
      QRectF r;
      r.setCoords(data.startMove.x(), data.startMove.y(), p.x(), p.y());
      lasso->setRect(r.normalized());
      QRectF _lassoRect(lasso->rect());
      r = _matrix.mapRect(_lassoRect);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);
      _score->addRefresh(lasso->canvasBoundingRect());
      _score->lassoSelect(lasso->rect());
      _score->end();
      }

//---------------------------------------------------------
//   endLasso
//---------------------------------------------------------

void ScoreView::endLasso()
      {
      _score->addRefresh(lasso->canvasBoundingRect());
      lasso->setRect(QRectF());
      _score->lassoSelectEnd();
      _score->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void ScoreView::deselectAll()
      {
      _score->deselectAll();
      _score->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   noteEntryMode
//---------------------------------------------------------

bool ScoreView::noteEntryMode() const
      {
      return sm->configuration().contains(states[NOTE_ENTRY]);
      }

//---------------------------------------------------------
//   editMode
//---------------------------------------------------------

bool ScoreView::editMode() const
      {
      return sm->configuration().contains(states[EDIT]);
      }

//---------------------------------------------------------
//   fotoMode
//---------------------------------------------------------

bool ScoreView::fotoMode() const
      {
      return sm->configuration().contains(states[FOTOMODE]);
      }

//---------------------------------------------------------
//   editInputTransition
//---------------------------------------------------------

void ScoreView::editInputTransition(QInputMethodEvent* ie)
      {
      if (editObject->edit(this, curGrip, 0, 0, ie->commitString())) {
            if (editObject->isText())
                  mscore->textTools()->updateTools();
            updateGrips();
            _score->update();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ScoreView::setDropTarget(const Element* el)
      {
      if (dropTarget != el) {
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  _score->addRefresh(dropTarget->canvasBoundingRect());
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  _score->addRefresh(dropTarget->canvasBoundingRect());
                  }
            }
      if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      }

//---------------------------------------------------------
//   setDropRectangle
//---------------------------------------------------------

void ScoreView::setDropRectangle(const QRectF& r)
      {
      if (dropRectangle.isValid())
            _score->addRefresh(dropRectangle);
      dropRectangle = r;
      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->canvasBoundingRect());
            dropTarget = 0;
            }
      else if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      _score->addRefresh(r);
      }

//---------------------------------------------------------
//   setDropAnchor
//---------------------------------------------------------

void ScoreView::setDropAnchor(const QLineF& l)
      {
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
/*      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->canvasBoundingRect());
            dropTarget = 0;
            }
      */
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      dropAnchor = l;
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal ScoreView::mag() const
      {
      return _matrix.m11();
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void ScoreView::setOffset(qreal x, qreal y)
      {
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), x, y, _matrix.m33());
      imatrix = _matrix.inverted();
      emit viewRectChanged();
      emit offsetChanged(x, y);
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal ScoreView::xoffset() const
      {
      return _matrix.dx();
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal ScoreView::yoffset() const
      {
      return _matrix.dy();
      }

//---------------------------------------------------------
//   fsize
//---------------------------------------------------------

QSizeF ScoreView::fsize() const
      {
      QSize s = size();
      return QSizeF(s.width() * imatrix.m11(), s.height() * imatrix.m22());
      }

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void ScoreView::pageNext()
      {
      if (score()->pages().empty())
            return;
      if (score()->layoutMode() == LayoutMode::LINE) {
            qreal x = xoffset() - width() * .8;
            MeasureBase* lm = score()->last();
            qreal lx = (lm->pos().x() + lm->width()) * mag() - width() * .8;
            if (x < -lx)
                  x = -lx;
            setOffset(x, yoffset());
            }
      else {
            Page* page = score()->pages().back();
            qreal x    = xoffset() - (page->width() + 25.0) * mag();
            qreal lx   = 10.0 - page->pos().x() * mag();

            if (x < lx)
                  x = lx;
            setOffset(x, 10.0);
            }
      update();
      }

//---------------------------------------------------------
//   pagePrev
//---------------------------------------------------------

void ScoreView::pagePrev()
      {
      if (score()->pages().empty())
            return;
      if (score()->layoutMode() == LayoutMode::LINE) {
            qreal x = xoffset() + width() * .8;
            if (x > 0.0)
                  x = 0;
            setOffset(x, yoffset());
            }
      else {
            Page* page = score()->pages().front();
            qreal x    = xoffset() + (page->width() + 25.0) * mag();
            if (x > 10.0)
                  x = 10.0;
            setOffset(x, 10.0);
            }
      update();
      }

//---------------------------------------------------------
//   pageTop
//---------------------------------------------------------

void ScoreView::pageTop()
      {
      if (score()->layoutMode() == LayoutMode::LINE)
            setOffset(0.0, yoffset());
      else
            setOffset(10.0, 10.0);
      update();
      }

//---------------------------------------------------------
//   pageEnd
//---------------------------------------------------------

void ScoreView::pageEnd()
      {
      if (score()->pages().empty())
            return;
      if (score()->layoutMode() == LayoutMode::LINE) {
            MeasureBase* lm = score()->last();
            qreal lx = (lm->canvasPos().x() + lm->width()) * mag();
            lx -= width() * .8;
            setOffset(-lx, yoffset());
            }
      else {
            Page* lastPage = score()->pages().back();
            QPointF p(lastPage->pos());
            setOffset(25.0 - p.x() * mag(), 25.0);
            }
      update();
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void ScoreView::adjustCanvasPosition(const Element* el, bool playBack)
      {
      if (score()->layoutMode() == LayoutMode::LINE) {
            qreal xo;  // new x offset
            qreal curPosR = _cursor->rect().right();
            qreal curPosL = _cursor->rect().left();
            qreal curPosMagR = curPosR * mag() + xoffset();
            qreal curPosMagL = curPosL * mag() + xoffset();

            if (curPosMagR >= width() * 0.90) {   // leaves 10% margin to the right
                  xo = xoffset() - curPosMagL + width() * 0.05;
                  setOffset(xo, yoffset());
                  update();
                  }
            else if (curPosMagL < width() * 0.05) {  // leaves 5% margin to the left
                  xo = -curPosL*mag() + width() * 0.05;
                  setOffset(xo, yoffset());
                  update();
                  }
            return;
            }

      const MeasureBase* m;
      if (!el)
            return;
      else if (el->type() == Element::Type::NOTE)
            m = static_cast<const Note*>(el)->chord()->measure();
      else if (el->type() == Element::Type::REST)
            m = static_cast<const Rest*>(el)->measure();
      else if (el->type() == Element::Type::CHORD)
            m = static_cast<const Chord*>(el)->measure();
      else if (el->type() == Element::Type::SEGMENT)
            m = static_cast<const Segment*>(el)->measure();
      else if (el->type() == Element::Type::LYRICS)
            m = static_cast<const Lyrics*>(el)->measure();
      else if ( (el->type() == Element::Type::HARMONY || el->type() == Element::Type::FIGURED_BASS)
         && el->parent()->type() == Element::Type::SEGMENT)
            m = static_cast<const Segment*>(el->parent())->measure();
      else if (el->type() == Element::Type::HARMONY && el->parent()->type() == Element::Type::FRET_DIAGRAM
         && el->parent()->parent()->type() == Element::Type::SEGMENT)
            m = static_cast<const Segment*>(el->parent()->parent())->measure();
      else if (el->type() == Element::Type::MEASURE || el->type() == Element::Type::VBOX)
            m = static_cast<const MeasureBase*>(el);
      else
            return;

      System* sys = m->system();
      if (!sys)
            return;

      QPointF p(el->canvasPos());
      QRectF r(imatrix.mapRect(geometry()));
      QRectF mRect(m->canvasBoundingRect());
      QRectF sysRect(sys->canvasBoundingRect());

      // only try to track measure if not during playback
      if (!playBack)
            sysRect = mRect;

      double _spatium    = score()->spatium();
      const qreal border = _spatium * 3;
      QRectF showRect = QRectF(mRect.x(), sysRect.y(), mRect.width(), sysRect.height())
                        .adjusted(-border, -border, border, border);

      // canvas is not as wide as measure, track note instead
      if (r.width() < showRect.width()) {
            showRect.setX(p.x());
            showRect.setWidth(el->width());
            }

      // canvas is not as tall as system
      if (r.height() < showRect.height()) {
            if (sys->staves()->size() == 1 || !playBack) {
                  // track note if single staff
                  showRect.setY(p.y());
                  showRect.setHeight(el->height());
                  }
            else {
                  // let user control height
//                   showRect.setY(r.y());
//                   showRect.setHeight(1);
                  }
            }

      if (r.contains(showRect))
            return;

      qreal x   = - xoffset() / mag();
      qreal y   = - yoffset() / mag();

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left())
            x = showRect.left() - border;
      else if (showRect.left() > r.right())
            x = showRect.right() - width() / mag() + border;
      else if (r.width() >= showRect.width() && showRect.right() > r.right())
            x = showRect.left() - border;
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom())
            y = showRect.top() - border;
      else if (showRect.top() > r.bottom())
            y = showRect.bottom() - height() / mag() + border;
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom())
            y = showRect.top() - border;

      // align to page borders if extends beyond
      Page* page = sys->page();
      if (x < page->x() || r.width() >= page->width())
            x = page->x();
      else if (r.width() < page->width() && r.width() + x > page->width() + page->x())
            x = (page->width() + page->x()) - r.width();
      if (y < page->y() || r.height() >= page->height())
            y = page->y();
      else if (r.height() < page->height() && r.height() + y > page->height())
            y = (page->height() + page->y()) - r.height();

      // hack: don't update if we haven't changed the offset
      if (oldX == x && oldY == y)
            return;

      setOffset(-x * mag(), -y * mag());
      update();
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest()
      {
      const InputState& is = _score->inputState();
      if (is.track() == -1 || is.segment() == 0)          // invalid state
            return;
      cmdEnterRest(is.duration());
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest(const TDuration& d)
      {
qDebug("cmdEnterRest %s", qPrintable(d.name()));
      if (!noteEntryMode())
            sm->postEvent(new CommandEvent("note-input"));
      _score->cmdEnterRest(d);
#if 0
      expandVoice();
      if (_is.cr() == 0) {
            qDebug("cannot enter rest here");
            return;
            }

      int track = _is.track;
      Segment* seg  = setNoteRest(_is.cr(), track, -1, d.fraction(), 0, AUTO);
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr)
            nextInputPos(cr, false);
      _is.rest = false;  // continue with normal note entry
#endif
      }

//---------------------------------------------------------
//   mscoreState
//---------------------------------------------------------

ScoreState ScoreView::mscoreState() const
      {
      if (sm->configuration().contains(states[NOTE_ENTRY])) {
            const InputState is = _score->inputState();
            Staff* staff = _score->staff(is.track() / VOICES);
            switch( staff->staffType()->group()) {
                  case StaffGroup::STANDARD:
                        return STATE_NOTE_ENTRY_PITCHED;
                  case StaffGroup::TAB:
                        return STATE_NOTE_ENTRY_TAB;
                  case StaffGroup::PERCUSSION:
                        return STATE_NOTE_ENTRY_DRUM;
                  }
            }
      if (sm->configuration().contains(states[EDIT]) || sm->configuration().contains(states[DRAG_EDIT])) {
            if (editObject && (editObject->type() == Element::Type::LYRICS))
                  return STATE_LYRICS_EDIT;
            else if (editObject &&
                  ( (editObject->type() == Element::Type::HARMONY) || editObject->type() == Element::Type::FIGURED_BASS) )
                  return STATE_HARMONY_FIGBASS_EDIT;
            else if (editObject && editObject->isText())
                  return STATE_TEXT_EDIT;
            return STATE_EDIT;
            }
      if (sm->configuration().contains(states[FOTOMODE]))
            return STATE_FOTO;
      if (sm->configuration().contains(states[PLAY]))
            return STATE_PLAY;
      return STATE_NORMAL;
      }

//---------------------------------------------------------
//   enterState
//    for debugging
//---------------------------------------------------------

void ScoreView::enterState()
      {
      mscore->changeState(mscoreState());
      if (MScore::debugMode)
            qDebug("%p enterState <%s>", this, qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   exitState
//    for debugging
//---------------------------------------------------------

void ScoreView::exitState()
      {
      if (MScore::debugMode)
            qDebug("%p exitState <%s>", this, qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool ScoreView::event(QEvent* event)
      {
      if (event->type() == QEvent::KeyPress && editObject) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
                  if(editObject->isText()) {
                        return true;
                        }
                  bool rv = true;
                  if (ke->key() == Qt::Key_Tab) {
                        curGrip += 1;
                        if (curGrip >= grips) {
                              curGrip = 0;
                              rv = false;
                              }
                        updateGrips();
                        _score->end();
                        if (curGrip)
                              return true;
                        }
                  else if (ke->key() == Qt::Key_Backtab) {
                        curGrip -= 1;
                        if (curGrip < 0) {
                              curGrip = grips -1;
                              rv = false;
                              }
                        }
                  updateGrips();
                  _score->end();
                  if (rv)
                        return true;
                  }
            }
      else if (event->type() == CloneDrag) {
            Element* e = static_cast<CloneEvent*>(event)->element();
            cloneElement(e);
            }
      else if (event->type() == QEvent::Gesture) {
            return gestureEvent(static_cast<QGestureEvent*>(event));
            }
      return QWidget::event(event);
      }

//---------------------------------------------------------
//   startUndoRedo
//---------------------------------------------------------

void ScoreView::startUndoRedo()
      {
      // exit edit mode
      _score->setLayoutAll(false);
      if (sm->configuration().contains(states[EDIT]))
            sm->postEvent(new CommandEvent("escape"));
      }

//---------------------------------------------------------
//   cmdAddSlur
//    'S' typed on keyboard
//---------------------------------------------------------

void ScoreView::cmdAddSlur()
      {
      InputState& is = _score->inputState();
      if (noteEntryMode() && is.slur()) {
            const QList<SpannerSegment*>& el = is.slur()->spannerSegments();
            if (!el.isEmpty())
                  el.front()->setSelected(false);
            is.setSlur(nullptr);
            return;
            }
      if (_score->selection().isRange()) {
            _score->startCmd();
            int startTrack = _score->selection().staffStart() * VOICES;
            int endTrack   = _score->selection().staffEnd() * VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  QList<Note*> nl = _score->selection().noteList(track);
                  Note* firstNote = 0;
                  Note* lastNote  = 0;
                  foreach(Note* n, nl) {
                        if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                              firstNote = n;
                        if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                              lastNote = n;
                        }
                  if (firstNote) {
                        if (firstNote == lastNote)
                              lastNote = 0;
                        ChordRest* cr1 = firstNote->chord();
                        ChordRest* cr2 = lastNote ? lastNote->chord() : nextChordRest(cr1);
                        if (cr2 == 0)
                              cr2 = cr1;
                        Slur* slur = new Slur(_score);
                        slur->setTick(cr1->tick());
                        slur->setTick2(cr2->tick());
                        slur->setTrack(cr1->track());
                        slur->setTrack2(cr2->track());
                        slur->setParent(0);
                        _score->undoAddElement(slur);
                        }
                  }
            _score->endCmd();
            mscore->endCmd();
            }
      else {
            QList<Note*> nl = _score->selection().noteList();
            Note* firstNote = 0;
            Note* lastNote  = 0;
            foreach(Note* n, nl) {
                  if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                        firstNote = n;
                  if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                        lastNote = n;
                  }
            if (!firstNote)
                  return;
            if (firstNote == lastNote)
                  lastNote = 0;
            cmdAddSlur(firstNote, lastNote);
            }
      }

//---------------------------------------------------------
//   cmdAddNoteLine
//---------------------------------------------------------

void ScoreView::cmdAddNoteLine()
      {
      Note* firstNote = 0;
      Note* lastNote  = 0;
      if (_score->selection().isRange()) {
            int startTrack = _score->selection().staffStart() * VOICES;
            int endTrack   = _score->selection().staffEnd() * VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  QList<Note*> nl = _score->selection().noteList(track);
                  foreach(Note* n, nl) {
                        if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                              firstNote = n;
                        if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                              lastNote = n;
                        }
                  }
            }
      else {
            QList<Note*> nl = _score->selection().noteList();
            foreach(Note* n, nl) {
                  if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                        firstNote = n;
                  if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                        lastNote = n;
                  }
            }
      if (!firstNote || !lastNote) {
            qDebug("addNoteLine: no note %p %p", firstNote, lastNote);
            return;
            }
      TextLine* tl = new TextLine(_score);
      tl->setParent(firstNote);
      tl->setStartElement(firstNote);
      tl->setEndElement(lastNote);
      tl->setDiagonal(true);
      tl->setAnchor(Spanner::Anchor::NOTE);
      tl->setTick(firstNote->chord()->tick());
      _score->startCmd();
      _score->undoAddElement(tl);
      _score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   cmdAddSlur
//---------------------------------------------------------

void ScoreView::cmdAddSlur(Note* firstNote, Note* lastNote)
      {
      _score->startCmd();
      ChordRest* cr1 = firstNote->chord();
      ChordRest* cr2 = lastNote ? lastNote->chord() : nextChordRest(cr1);

      if (cr2 == 0)
            cr2 = cr1;

      Slur* slur = new Slur(_score);
      slur->setTick(cr1->tick());
      slur->setTick2(cr2->tick());
      slur->setTrack(cr1->track());
      if (cr2->staff()->part() == cr1->staff()->part() && !cr2->staff()->isLinked(cr1->staff()))
            slur->setTrack2(cr2->track());
      else
            slur->setTrack2(cr1->track());

      slur->setStartElement(cr1);
      slur->setEndElement(cr2);

      _score->undoAddElement(slur);
      slur->layout();

      _score->endCmd();
      _score->startCmd();

      if (cr1 == cr2) {
            SlurSegment* ss = slur->frontSegment();
            ss->setSlurOffset(3, QPointF(3.0, 0.0));
            }
      const QList<SpannerSegment*>& el = slur->spannerSegments();
      if (noteEntryMode()) {
            _score->inputState().setSlur(slur);
            if (!el.isEmpty())
                  el.front()->setSelected(true);
            else
                  qDebug("addSlur: no segment");
            _score->endCmd();
            }
      else {
            //
            // start slur in edit mode if lastNote is not given
            //
            if (editObject && editObject->isText()) {
                  _score->endCmd();
                  return;
                  }
            if ((lastNote == 0) && !el.isEmpty()) {
                  editObject = el.front();
                  sm->postEvent(new CommandEvent("edit"));  // calls startCmd()
                  }
            else
                  _score->endCmd();
            }
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    '<' typed on keyboard
//---------------------------------------------------------

void ScoreView::cmdAddHairpin(bool decrescendo)
      {
      ChordRest* cr1;
      ChordRest* cr2;
      _score->getSelectedChordRest2(&cr1, &cr2);
      if (!cr1)
            return;
      if (cr2 == 0)
            cr2 = nextChordRest(cr1);
      if (cr2 == 0)
            return;

      _score->startCmd();
      Hairpin* pin = new Hairpin(_score);
      pin->setHairpinType(decrescendo ? Hairpin::Type::DECRESCENDO : Hairpin::Type::CRESCENDO);
      pin->setTrack(cr1->track());
      pin->setTick(cr1->segment()->tick());
      pin->setTick2(cr2->segment()->tick());
      _score->undoAddElement(pin);
      pin->layout();
      _score->endCmd();
      _score->startCmd();

      const QList<SpannerSegment*>& el = pin->spannerSegments();

      if (noteEntryMode()) {
            _score->endCmd();
            }
      else {
            if (!el.isEmpty()) {
                  editObject = el.front();
                  sm->postEvent(new CommandEvent("edit"));  // calls startCmd()
                  }
            else
                  _score->endCmd();
            }
      }

//---------------------------------------------------------
//   cmdChangeEnharmonic
//---------------------------------------------------------

void ScoreView::cmdChangeEnharmonic(bool up)
      {
      _score->startCmd();
      for (Note* n : _score->selection().noteList()) {
            Staff* staff = n->staff();
            if (staff->part()->instr()->useDrumset())
                  continue;
            if (staff->isTabStaff()) {
                  int string = n->line() + (up ? 1 : -1);
                  int fret   = staff->part()->instr()->stringData()->fret(n->pitch(), string);
                  if (fret != -1) {
                        score()->undoChangeProperty(n, P_ID::FRET, fret);
                        score()->undoChangeProperty(n, P_ID::STRING, string);
                        }
                  }
            else {
                  static const int tab[36] = {
                        26, 14,  2,  // 60  B#   C   Dbb
                        21, 21,  9,  // 61  C#   C#  Db
                        28, 16,  4,  // 62  C##  D   Ebb
                        23, 23, 11,  // 63  D#   D#  Eb
                        30, 18,  6,  // 64  D##  E   Fb
                        25, 13,  1,  // 65  E#   F   Gbb
                        20, 20,  8,  // 66  F#   F#  Gb
                        27, 15,  3,  // 67  F##  G   Abb
                        22, 22, 10,  // 68  G#   G#  Ab
                        29, 17,  5,  // 69  G##  A   Bbb
                        24, 24, 12,  // 70  A#   A#  Bb
                        31, 19,  7,  // 71  A##  B   Cb
                        };
                  int tpc  = n->tpc();
                  int line = n->line();
                  int i;
                  for (i = 0; i < 36; ++i) {
                        if (tab[i] == tpc) {
                              int k = i % 3;
                              i-= k;
                              if ((k < 2) && tab[i] == tab[i+1])
                                    ++k;
                              if (k < 2) {
                                    ++k;
                                    tpc = tab[i + k];
                                    ++line;
                                    }
                              else {
                                    if (tab[i + 2] != tab[i + 1])
                                          --line;
                                    --k;
                                    if (tab[i + 1] != tab[i])
                                          --line;
                                    --k;
                                    tpc = tab[i];
                                    }
                              break;
                              }
                        }
                  if (i == 36)
                        qDebug("tpc %d not found", tpc);
                  else
                        n->undoSetTpc(tpc);
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   cloneElement
//---------------------------------------------------------

void ScoreView::cloneElement(Element* e)
      {
      if (!e->isMovable() && e->type() != Element::Type::SPACER && e->type() != Element::Type::VBOX)
            return;
      if(e->type() == Element::Type::NOTE || e->type() == Element::Type::MEASURE)
            return;
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      if (e->isSegment())
            e = static_cast<SpannerSegment*>(e)->spanner();
      mimeData->setData(mimeSymbolFormat, e->mimeData(QPointF()));
      drag->setMimeData(mimeData);
      drag->setPixmap(QPixmap());
      drag->start(Qt::CopyAction);
      }

//---------------------------------------------------------
//   changeEditElement
//---------------------------------------------------------

void ScoreView::changeEditElement(Element* e)
      {
      int grip = curGrip;
      endEdit();
      startEdit(e, grip);
      }

//---------------------------------------------------------
//   setCursorVisible
//---------------------------------------------------------

void ScoreView::setCursorVisible(bool v)
      {
      _cursor->setVisible(v);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n, ChordRest* cr)
      {
      if (cr->durationType() < TDuration(TDuration::DurationType::V_128TH) && cr->durationType() != TDuration(TDuration::DurationType::V_MEASURE)) {
            mscore->noteTooShortForTupletDialog();
            return;
            }

      Fraction f(cr->duration());
      int tick    = cr->tick();
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator()*2) {
            ratio /= 2;
            fr    /= 2;
            }

      Tuplet* tuplet = new Tuplet(_score);
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setDuration(f);
      TDuration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);

      cmdCreateTuplet(cr, tuplet);
      }

void ScoreView::cmdCreateTuplet( ChordRest* cr, Tuplet* tuplet)
      {
      _score->cmdCreateTuplet(cr, tuplet);

      const QList<DurationElement*>& cl = tuplet->elements();
      int ne = cl.size();
      DurationElement* el = 0;
      if (ne && cl[0]->type() == Element::Type::REST)
            el  = cl[0];
      else if (ne > 1)
            el = cl[1];
      if (el) {
            _score->select(el, SelectType::SINGLE, 0);
            if (!noteEntryMode()) {
                  sm->postEvent(new CommandEvent("note-input"));
                  qApp->processEvents();
                  }
            _score->inputState().setDuration(tuplet->baseLen());
            mscore->updateInputState(_score);
            }
      }

//---------------------------------------------------------
//   changeVoice
//---------------------------------------------------------

void ScoreView::changeVoice(int voice)
      {
      InputState* is = &score()->inputState();
      int track = (is->track() / VOICES) * VOICES + voice;
      is->setTrack(track);

      if (is->noteEntryMode()) {
            if (is->segment()) { // can be null for eg repeatMeasure
                  is->setSegment(is->segment()->measure()->first(Segment::Type::ChordRest));
                  score()->setUpdateAll(true);
                  score()->end();
                  mscore->setPos(is->segment()->tick());
                  }
            }
      else {
            score()->startCmd();
            QList<Element*> el;
            foreach(Element* e, score()->selection().elements()) {
                  if (e->type() == Element::Type::NOTE) {
                        Note* note = static_cast<Note*>(e);
                        Chord* chord = note->chord();
                        if (chord->voice() != voice) {
                              int notes = note->chord()->notes().size();
                              if (notes > 1) {
                                    //
                                    // TODO: check destination voice content
                                    //
                                    Note* newNote   = new Note(*note);
                                    Chord* newChord = new Chord(score());
                                    newNote->setSelected(false);
                                    el.append(newNote);
                                    int track = chord->staffIdx() * VOICES + voice;
                                    newChord->setTrack(track);
                                    newChord->setDurationType(chord->durationType());
                                    newChord->setDuration(chord->duration());
                                    newChord->setParent(chord->parent());
                                    newChord->add(newNote);
                                    score()->undoRemoveElement(note);
                                    score()->undoAddElement(newChord);
                                    }
                              else if (notes == 1 && voice && chord->voice()) {
                                    Chord* newChord = new Chord(*chord);
                                    int track = chord->staffIdx() * VOICES + voice;
                                    newChord->setTrack(track);
                                    newChord->setParent(chord->parent());
                                    score()->undoRemoveElement(chord);
                                    score()->undoAddElement(newChord);
                                    }
                              }
                        }
                  }
            score()->selection().clear();
            foreach(Element* e, el)
                  score()->select(e, SelectType::ADD, -1);
            score()->setLayoutAll(true);
            score()->endCmd();
            }
      mscore->updateInputState(score());
      }

//---------------------------------------------------------
//   harmonyEndEdit
//---------------------------------------------------------

void ScoreView::harmonyEndEdit()
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);

      if (harmony->isEmpty())
            _score->undoRemoveElement(harmony);
      }

//---------------------------------------------------------
//   modifyElement
//---------------------------------------------------------

void ScoreView::modifyElement(Element* el)
      {
      if (el == 0) {
            qDebug("modifyElement: el==0");
            return;
            }
      Score* cs = el->score();
      if (!cs->selection().isSingle()) {
            qDebug("modifyElement: cs->selection().state() != SelState::SINGLE");
            delete el;
            return;
            }
      Element* e = cs->selection().element();
      Chord* chord;
      if (e->type() == Element::Type::CHORD)
            chord = static_cast<Chord*>(e);
      else if (e->type() == Element::Type::NOTE)
            chord = static_cast<Note*>(e)->chord();
      else {
            qDebug("modifyElement: no note/Chord selected:");
            e->dump();
            delete el;
            return;
            }
      switch (el->type()) {
            case Element::Type::ARTICULATION:
                  chord->add(static_cast<Articulation*>(el));
                  break;
            default:
                  qDebug("modifyElement: %s not ARTICULATION", el->name());
                  delete el;
                  return;
            }
      cs->setLayoutAll(true);
      }

//---------------------------------------------------------
//   harmonyTab
//---------------------------------------------------------

void ScoreView::harmonyTab(bool back)
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);
      if (!harmony->parent() || harmony->parent()->type() != Element::Type::SEGMENT){
            qDebug("harmonyTab: no segment parent");
            return;
            }
      int track        = harmony->track();
      Segment* segment = static_cast<Segment*>(harmony->parent());
      if (!segment) {
            qDebug("harmonyTicksTab: no segment");
            return;
            }

      // moving to next/prev measure

      Measure* measure = segment->measure();
      if (measure) {
            if (back)
                  measure = measure->prevMeasure();
            else
                  measure = measure->nextMeasure();
            }
      if (!measure) {
            qDebug("harmonyTab: no prev/next measure");
            return;
            }

      segment = measure->findSegment(Segment::Type::ChordRest, measure->tick());
      if (!segment) {
            qDebug("harmonyTab: no ChordRest segment as measure");
            return;
            }

      endEdit();

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == Element::Type::HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, -1);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   harmonyBeatsTab
//    manages [;:], moving forward or back to the next beat
//    and Space/Shift-Space, to stop at next note, rest, harmony or beat.
//---------------------------------------------------------

void ScoreView::harmonyBeatsTab(bool noterest, bool back)
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);
      int track         = harmony->track();
      Segment* segment = static_cast<Segment*>(harmony->parent());
      if (!segment) {
            qDebug("harmonyBeatsTab: no segment");
            return;
            }
      Measure* measure = segment->measure();
      int tick = segment->tick();

      if (back && tick == measure->tick()) {
            // previous bar, if any
            measure = measure->prevMeasure();
            if (!measure) {
                  qDebug("harmonyBeatsTab: no previous measure");
                  return;
                  }
            }

      Fraction f = measure->len();
      int ticksPerBeat = f.ticks() / ((f.numerator()>3 && (f.numerator()%3)==0 && f.denominator()>4) ? f.numerator()/3 : f.numerator());
      int tickInBar = tick - measure->tick();
      int newTick = measure->tick() + ((tickInBar+(back?-1:ticksPerBeat))/ticksPerBeat)*ticksPerBeat;

      // look for next/prev beat, note, rest or chord
      for (;;) {
            segment = back ? segment->prev1(Segment::Type::ChordRest) : segment->next1(Segment::Type::ChordRest);

            if (!segment || (back ? (segment->tick() < newTick) : (segment->tick() > newTick))) {
                  // no segment or moved past the beat - create new segment
                  if (!back && newTick >= measure->tick() + f.ticks()) {
                        // next bar, if any
                        measure = measure->nextMeasure();
                        if (!measure) {
                              qDebug("harmonyBeatsTab: no next measure");
                              return;
                              }
                        }
                  segment = new Segment(measure, Segment::Type::ChordRest, newTick);
                  if (!segment) {
                        qDebug("harmonyBeatsTab: no prev segment");
                        return;
                        }
                  _score->undoAddElement(segment);
                  break;
                  }

            if (segment->tick() == newTick)
                  break;

            if (noterest) {
                  int minTrack = (track / VOICES ) * VOICES;
                  int maxTrack = minTrack + (VOICES-1);
                  if (segment->findAnnotationOrElement(Element::Type::HARMONY, minTrack, maxTrack))
                        break;
                  }
            }

      endEdit();

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == Element::Type::HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, -1);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   harmonyTicksTab
//    manages [Ctrl] [1]-[9], moving forward the given number of ticks
//---------------------------------------------------------

void ScoreView::harmonyTicksTab(int ticks)
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);
      int track         = harmony->track();
      Segment* segment = static_cast<Segment*>(harmony->parent());
      if (!segment) {
            qDebug("harmonyTicksTab: no segment");
            return;
            }
      Measure* measure = segment->measure();

      int newTick   = segment->tick() + ticks;

      // find the measure containing the target tick
      while (newTick >= measure->tick() + measure->ticks()) {
            measure = measure->nextMeasure();
            if (!measure) {
                  qDebug("harmonyTicksTab: no next measure");
                  return;
                  }
            }

      // look for a segment at this tick; if none, create one
      while(segment && segment->tick() < newTick)
            segment = segment->next1(Segment::Type::ChordRest);
      if (!segment || segment->tick() > newTick) {      // no ChordRest segment at this tick
            segment = new Segment(measure, Segment::Type::ChordRest, newTick);
            if (!segment) {
                  qDebug("harmonyTicksTab: no next segment");
                  return;
                  }
            _score->undoAddElement(segment);
            }

      endEdit();

      _score->startCmd();

      // search for next chord name
      harmony = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == Element::Type::HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  harmony = h;
                  break;
                  }
            }

      if (!harmony) {
            harmony = new Harmony(_score);
            harmony->setTrack(track);
            harmony->setParent(segment);
            _score->undoAddElement(harmony);
            }

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony, -1);
      mscore->changeState(mscoreState());

      adjustCanvasPosition(harmony, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n)
      {
      _score->startCmd();
      if (noteEntryMode()) {
            _score->expandVoice();
            ChordRest* cr = _score->inputState().cr();
            if (cr) {
                  _score->changeCRlen(cr, _score->inputState().duration());
                  cmdTuplet(n, cr);
                  }
            }
      else {
            QSet<ChordRest*> set;
            foreach(Element* e, _score->selection().elements()) {
                  if (e->type() == Element::Type::NOTE) {
                        Note* note = static_cast<Note*>(e);
                        if(note->noteType() != NoteType::NORMAL) { //no tuplet on grace notes
                              _score->endCmd();
                              return;
                              }
                        e = note->chord();
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if(!set.contains(cr)) {
                              cmdTuplet(n, cr);
                              set.insert(cr);
                              }
                        }
                  }
            }
      _score->endCmd();
      moveCursor();     // do this after endCmd to make sure segment has been laid out
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void ScoreView::midiNoteReceived(int pitch, bool chord)
      {
      MidiInputEvent ev;
      ev.pitch = pitch;
      ev.chord = chord;

qDebug("midiNoteReceived %d chord %d", pitch, chord);
      score()->enqueueMidiEvent(ev);
      if (!score()->undo()->active())
            cmd(0);
      }

//---------------------------------------------------------
//   cmdInsertNote
//---------------------------------------------------------

void ScoreView::cmdInsertNote(int note)
      {
      qDebug("not implemented: cmdInsertNote %d", note);
      }

//---------------------------------------------------------
//   cmdAddPitch
///   insert note or add note to chord
//    c d e f g a b entered:
//---------------------------------------------------------

void ScoreView::cmdAddPitch(int note, bool addFlag)
      {
      InputState& is = _score->inputState();
      if (is.track() == -1)          // invalid state
            return;
      if (is.segment() == 0) {
            qDebug("cannot enter notes here (no chord rest at current position)");
            return;
            }
      Drumset* ds = is.drumset();
      int octave = 4;
      if (ds) {
            char note1 = "CDEFGAB"[note];
            int pitch = -1;
            int voice = 0;
            for (int i = 0; i < 127; ++i) {
                  if (!ds->isValid(i))
                        continue;
                  if (ds->shortcut(i) && (ds->shortcut(i) == note1)) {
                        pitch = i;
                        voice = ds->voice(i);
                        break;
                        }
                  }
            if (pitch == -1) {
                  qDebug("  shortcut %c not defined in drumset", note1);
                  return;
                  }
            is.setDrumNote(pitch);
            is.setTrack(is.track() / VOICES + voice);
            octave = pitch / 12;
            }
      else {
            // if adding notes, add above the upNote of the current chord
            Element* el = score()->selection().element();
            if (addFlag && el && el->type() == Element::Type::NOTE) {
                  Chord* chord = static_cast<Note*>(el)->chord();
                  Note* n = chord->upNote();
                  octave = n->epitch() / 12;
                  if (note < n->epitch() / (octave * 7))
                        octave++;
                  }
            else {
                  int curPitch = -1;
                  if (is.segment()) {
                        Staff* staff = score()->staff(is.track() / VOICES);
                        Segment* seg = is.segment()->prev1(Segment::Type::ChordRest | Segment::Type::Clef);
                        while(seg) {
                              if (seg->segmentType() == Segment::Type::ChordRest) {
                                    Element* p = seg->element(is.track());
                                    if (p && p->type() == Element::Type::CHORD) {
                                          Chord* ch = static_cast<Chord*>(p);
                                          curPitch = ch->downNote()->epitch();
                                          break;
                                          }
                                    }
                              else if (seg->segmentType() == Segment::Type::Clef) {
                                    Element* p = seg->element( (is.track() / VOICES) * VOICES); // clef on voice 1
                                    if(p && p->type() == Element::Type::CLEF) {
                                          Clef* clef = static_cast<Clef*>(p);
                                          // check if it's an actual key change or just a courtesy
                                          ClefType ctb = staff->clef(clef->tick() - 1);
                                          if (ctb != clef->clefType() || clef->tick() == 0) {
                                                curPitch = line2pitch(4, clef->clefType(), Key::C); // C 72 for treble clef
                                                break;
                                                }
                                          }
                                    }
                              seg = seg->prev1(Segment::Type::ChordRest | Segment::Type::Clef);
                              }
                        octave = curPitch / 12;
                        }

                  static const int tab[] = { 0, 2, 4, 5, 7, 9, 11 };
                  int delta = octave * 12 + tab[note] - curPitch;
                  if (delta > 6)
                        --octave;
                  else if (delta < -6)
                        ++octave;
                  }
            }

      if (!noteEntryMode()) {
            sm->postEvent(new CommandEvent("note-input"));
            qApp->processEvents();
            }
      _score->cmdAddPitch(octave * 7 + note, addFlag);
      adjustCanvasPosition(is.cr(), false);
      }

//---------------------------------------------------------
//   cmdAddFret
///   insert note with given fret on current string
//---------------------------------------------------------

void ScoreView::cmdAddFret(int fret)
      {
      if (mscoreState() != STATE_NOTE_ENTRY_TAB) // only acceptable in TAB note entry
            return;
      InputState& is = _score->inputState();
      if (is.track() == -1)                     // invalid state
            return;
      if (is.segment() == 0 /*|| is.cr() == 0*/) {
            qDebug("cannot enter notes here (no chord rest at current position)");
            return;
            }

      _score->startCmd();
      Position pos;
      pos.segment   = is.segment();
/* frets are always put at cursor position, never at selected note position
      if(addFlag)
            {
            Element* el = score()->selection().element();
            if (el && el->type() == Element::NOTE) {
                 ChordRest* cr = static_cast<ChordRest*>(((Note*)el)->chord());
                 if (cr) pos.segment = cr->segment();
                 }
            }
*/
      pos.staffIdx  = is.track() / VOICES;
      pos.line      = is.string();
      pos.fret      = fret;

      score()->putNote(pos, false);
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdAddChordName
//---------------------------------------------------------

void ScoreView::cmdAddChordName()
      {
      if (!_score->checkHasMeasures())
            return;
      ChordRest* cr = _score->getSelectedChordRest();
      if (!cr)
            return;
      _score->startCmd();
      Harmony* harmony = new Harmony(_score);
      harmony->setTrack(cr->track());
      harmony->setParent(cr->segment());
      _score->undoAddElement(harmony);

      _score->select(harmony, SelectType::SINGLE, 0);
      startEdit(harmony);
      _score->setLayoutAll(true);
      _score->update();
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void ScoreView::cmdAddText(TEXT type)
      {
      if (!_score->checkHasMeasures())
            return;
      Text* s = 0;
      _score->startCmd();
      switch(type) {
            case TEXT::TITLE:
            case TEXT::SUBTITLE:
            case TEXT::COMPOSER:
            case TEXT::POET:
                  {
                  MeasureBase* measure = _score->first();
                  if (measure->type() != Element::Type::VBOX)
                        measure = _score->insertMeasure(Element::Type::VBOX, measure);
                  s = new Text(_score);
                  switch(type) {
                        case TEXT::TITLE:    s->setTextStyleType(TextStyleType::TITLE);    break;
                        case TEXT::SUBTITLE: s->setTextStyleType(TextStyleType::SUBTITLE); break;
                        case TEXT::COMPOSER: s->setTextStyleType(TextStyleType::COMPOSER); break;
                        case TEXT::POET:     s->setTextStyleType(TextStyleType::POET);     break;
                        default: /* can't happen, but need to keep compiler happy */ break;
                        }
                  s->setParent(measure);
                  adjustCanvasPosition(measure, false);
                  }
                  break;

            case TEXT::REHEARSAL_MARK:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new RehearsalMark(_score);
                  s->setTrack(0);
                  s->setParent(cr->segment());
                  }
                  break;
            case TEXT::STAFF:
            case TEXT::SYSTEM:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new StaffText(_score);
                  if (type == TEXT::SYSTEM) {
                        s->setTrack(0);
                        s->setTextStyleType(TextStyleType::SYSTEM);
                        }
                  else {
                        s->setTrack(cr->track());
                        s->setTextStyleType(TextStyleType::STAFF);
                        }
                  s->setParent(cr->segment());
                  }
                  break;
            }

      if (s) {
            _score->undoAddElement(s);
            _score->setLayoutAll(true);
            _score->select(s, SelectType::SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            }
      else
            _score->endCmd();
      }

//---------------------------------------------------------
//   cmdAppendMeasures
///   Append \a n measures.
///
///   Keyboard callback, called from pulldown menu.
//
//    - called from pulldown menu
//---------------------------------------------------------

void ScoreView::cmdAppendMeasures(int n, Element::Type type)
      {
      _score->startCmd();
      appendMeasures(n, type);
      _score->endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::appendMeasure(Element::Type type)
      {
      _score->startCmd();
      MeasureBase* mb = _score->insertMeasure(type, 0);
      _score->endCmd();
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void ScoreView::appendMeasures(int n, Element::Type type)
      {
      if (_score->noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      for (int i = 0; i < n; ++i)
            _score->insertMeasure(type, 0);
      }

//---------------------------------------------------------
//   checkSelectionStateForInsertMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::checkSelectionStateForInsertMeasure()
      {
      MeasureBase* mb = 0;
      if (_score->selection().isRange()) {
            mb = _score->selection().startSegment()->measure();
            return mb;
            }

      mb = _score->selection().findMeasure();
      if (mb)
            return mb;

      Element* e = _score->selection().element();
      if (e) {
            if (e->type() == Element::Type::VBOX || e->type() == Element::Type::TBOX)
                  return static_cast<MeasureBase*>(e);
            }
      QMessageBox::warning(0, "MuseScore",
            tr("No measure selected:\n" "Please select a measure and try again"));
      return 0;
      }

//---------------------------------------------------------
//   cmdInsertMeasures
//---------------------------------------------------------

void ScoreView::cmdInsertMeasures(int n, Element::Type type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
      for (int i = 0; i < n; ++i)
            _score->insertMeasure(type, mb);

      // measure may be part of mm rest:
      if (!_score->styleB(StyleIdx::createMultiMeasureRests) && type == Element::Type::MEASURE)
            _score->select(mb, SelectType::SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdInsertMeasure
//---------------------------------------------------------

void ScoreView::cmdInsertMeasure(Element::Type type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
      mb = _score->insertMeasure(type, mb);
      if (mb->type() == Element::Type::TBOX) {
            TBox* tbox = static_cast<TBox*>(mb);
            Text* s = tbox->getText();
            _score->select(s, SelectType::SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            return;
            }
      if (mb)
           _score->select(mb, SelectType::SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void ScoreView::cmdRepeatSelection()
      {
      const Selection& selection = _score->selection();

      if (noteEntryMode() && selection.isSingle()) {
            Element* el = _score->selection().element();
            if (el && el->type() == Element::Type::NOTE) {
                  if (!_score->inputState().endOfScore()) {
                        _score->startCmd();
                        bool addTo = false;
                        Chord* c = static_cast<Note*>(el)->chord();
                        for (Note* note : c->notes()) {
                              NoteVal nval = note->noteVal();
                              _score->addPitch(nval, addTo);
                              addTo = true;
                              }
                        _score->endCmd();
                        mscore->endCmd();
                        }
                  }
            return;
            }
      if (!selection.isRange()) {
            qDebug("wrong selection type");
            return;
            }

      QString mimeType = selection.mimeType();
      if (mimeType.isEmpty()) {
            qDebug("mime type is empty");
            return;
            }
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, selection.mimeData());
      if (MScore::debugMode)
            qDebug("cmdRepeatSelection: <%s>", mimeData->data(mimeType).data());
      QApplication::clipboard()->setMimeData(mimeData);

      QByteArray data(mimeData->data(mimeType));
      XmlReader xml(data);
      xml.setPasteMode(true);

      int dStaff = selection.staffStart();
      Segment* endSegment = selection.endSegment();

      if (endSegment && endSegment->segmentType() != Segment::Type::ChordRest)
            endSegment = endSegment->next1(Segment::Type::ChordRest);
      if (endSegment && endSegment->element(dStaff * VOICES)) {
            Element* e = endSegment->element(dStaff * VOICES);
            if (e) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  _score->startCmd();
                  _score->pasteStaff(xml, cr->segment(), cr->staffIdx());
                  _score->setLayoutAll(true);
                  _score->endCmd();
                  }
            else
                  qDebug("ScoreView::cmdRepeatSelection: cannot paste: %p <%s>", e, e ? e->name() : "");
            }
      else {
            qDebug("cmdRepeatSelection: cannot paste: endSegment: %p dStaff %d", endSegment, dStaff);
            }
      }

//---------------------------------------------------------
//   selectMeasure
//---------------------------------------------------------

void ScoreView::selectMeasure(int n)
      {
      int i = 0;
      for (Measure* measure = _score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (++i < n)
                  continue;
            _score->selection().setState(SelState::RANGE);
            _score->selection().setStartSegment(measure->first());
            _score->selection().setEndSegment(measure->last());
            _score->selection().setStaffStart(0);
            _score->selection().setStaffEnd(_score->nstaves());
            _score->selection().updateSelectedElements();
            _score->selection().setState(SelState::RANGE);
            _score->addRefresh(measure->canvasBoundingRect());
            adjustCanvasPosition(measure, true);
            _score->setUpdateAll(true);
            _score->end();
            break;
            }
      }

//---------------------------------------------------------
//   search
//---------------------------------------------------------

void ScoreView::search(const QString& s)
      {
      bool ok;

      int n = s.toInt(&ok);
      if (ok && n >= 0)
            searchMeasure(n);
      else {
            if (s.size() >= 2 && s[0].toLower() == 'p' && s[1].isNumber()) {
                  n = s.mid(1).toInt(&ok);
                  if (ok && n >= 0)
                        searchPage(n);
                  }
            else {
                  //search rehearsal marks
                  for (Segment* seg = score()->firstSegment(); seg; seg = seg->next1(Segment::Type::ChordRest)) {
                        for (Element* e : seg->annotations()){
                              if (e->type() == Element::Type::REHEARSAL_MARK) {
                                    RehearsalMark* rm = static_cast<RehearsalMark*>(e);
                                    QString rms = rm->text();
                                    if (rms.contains(s)) {
                                          gotoMeasure(seg->measure());
                                          break;
                                          }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   searchPage
//---------------------------------------------------------

void ScoreView::searchPage(int n)
      {
      n -= score()->pageNumberOffset();
      if (n <= 0)
            n = 1;
      n--;
      if (n >= _score->npages())
            n = _score->npages() - 1;
      const Page* page = _score->pages()[n];
      foreach (System* s, *page->systems()) {
            if (s->firstMeasure()) {
                  gotoMeasure(s->firstMeasure());
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   searchMeasure
//---------------------------------------------------------

void ScoreView::searchMeasure(int n)
      {
      if (n <= 0)
            return;
      --n;
      int i = 0;
      Measure* measure;
      for (measure = _score->firstMeasureMM(); measure; measure = measure->nextMeasureMM()) {
            int nn = _score->styleB(StyleIdx::createMultiMeasureRests) && measure->isMMRest()
               ? measure->mmRestCount() : 1;
            if (n >= i && n < (i+nn))
                  break;
            i += nn;
            }
      if (!measure)
            measure = score()->lastMeasureMM();
      gotoMeasure(measure);
      }

//---------------------------------------------------------
//   gotoMeasure
//---------------------------------------------------------

void ScoreView::gotoMeasure(Measure* measure)
      {
      adjustCanvasPosition(measure, true);
      int tracks = _score->nstaves() * VOICES;
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->segmentType() != Segment::Type::ChordRest)
                  continue;
            int track;
            for (track = 0; track < tracks; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                  if (cr) {
                        Element* e;
                        if (cr->type() == Element::Type::CHORD)
                              e =  static_cast<Chord*>(cr)->upNote();
                        else //REST
                              e = cr;

                        _score->select(e, SelectType::SINGLE, 0);
                        break;
                        }
                  }
            if (track != tracks)
                  break;
            }
      _score->setUpdateAll(true);
      _score->end();
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void ScoreView::layoutChanged()
      {
      if (mscore->navigator())
            mscore->navigator()->layoutChanged();
      }

//---------------------------------------------------------
//   ScoreView::figuredBassEndEdit
//    derived from harmonyEndEdit()
//    remove the FB if empty
//---------------------------------------------------------

void ScoreView::figuredBassEndEdit()
      {
      FiguredBass* fb = static_cast<FiguredBass*>(editObject);

      if (fb->isEmpty())
            _score->undoRemoveElement(fb);
      }

//---------------------------------------------------------
//   ScoreView::figuredBassTab
//    derived from harmonyTab() (for Harmony)
//    manages [Space] / [Shift][Space] keys, moving editing to FB of next/prev ChordRest
//    and [Tab] / [Shift][Tab] keys, moving to FB of next/prev measure
//---------------------------------------------------------

void ScoreView::figuredBassTab(bool bMeas, bool bBack)
      {
      FiguredBass* fb   = (FiguredBass*)editObject;
      Segment* nextSegm;
      Segment* segm     = fb->segment();
      int track         = fb->track();

      if (!segm) {
            qDebug("figuredBassTab: no segment");
            return;
            }

      // if moving to next/prev measure

      if (bMeas) {
            Measure* meas = segm->measure();
            if (meas) {
                  if (bBack)
                        meas = meas->prevMeasure();
                  else
                        meas = meas->nextMeasure();
                  }
            if (!meas) {
                  qDebug("figuredBassTab: no prev/next measure");
                  return;
                  }
            // find initial ChordRest segment
            nextSegm = meas->findSegment(Segment::Type::ChordRest, meas->tick());
            if (!nextSegm) {
                  qDebug("figuredBassTab: no ChordRest segment at measure");
                  return;
                  }
            }

      // if moving to next/prev chord segment

      else {
            // search next chord segment in same staff
            nextSegm = bBack ? segm->prev1(Segment::Type::ChordRest) : segm->next1(Segment::Type::ChordRest);
            int minTrack = (track / VOICES ) * VOICES;
            int maxTrack = minTrack + (VOICES-1);

            while (nextSegm) {                   // look for a ChordRest in the compatible track range
                  if(nextSegm->findAnnotationOrElement(Element::Type::FIGURED_BASS, minTrack, maxTrack))
                        break;
                  nextSegm = bBack ? nextSegm->prev1(Segment::Type::ChordRest) : nextSegm->next1(Segment::Type::ChordRest);
                  }

            if (!nextSegm) {
                  qDebug("figuredBassTab: no prev/next segment");
                  return;
                  }
            }

      endEdit();

      _score->startCmd();
      bool bNew;
      // add a (new) FB element, using chord duration as default duration
      FiguredBass * fbNew = FiguredBass::addFiguredBassToSegment(nextSegm, track, 0, &bNew);
      if (bNew)
            _score->undoAddElement(fbNew);
      _score->select(fbNew, SelectType::SINGLE, 0);
      startEdit(fbNew, -1);
      mscore->changeState(mscoreState());
      adjustCanvasPosition(fbNew, false);
      ((FiguredBass*)editObject)->moveCursorToEnd();
      _score->setLayoutAll(true);
//      _score->update();                         // used by lyricsTab() but not by harmonyTab(): needed or not?
      }

//---------------------------------------------------------
//   figuredBassTicksTab
//    manages [Ctrl] [1]-[9], extending current FB of the given number of ticks
//---------------------------------------------------------

void ScoreView::figuredBassTicksTab(int ticks)
      {
      FiguredBass* fb   = (FiguredBass*)editObject;
      int track         = fb->track();
      Segment* segm     = fb->segment();
      if (!segm) {
            qDebug("figuredBassTicksTab: no segment");
            return;
            }
      Measure* measure = segm->measure();

      int nextSegTick   = segm->tick() + ticks;

      // find the measure containing the target tick
      while (nextSegTick >= measure->tick() + measure->ticks()) {
            measure = measure->nextMeasure();
            if (!measure) {
                  qDebug("figuredBassTicksTab: no next measure");
                  return;
                  }
            }

      // look for a segment at this tick; if none, create one
      Segment * nextSegm = segm;
      while (nextSegm && nextSegm->tick() < nextSegTick)
            nextSegm = nextSegm->next1(Segment::Type::ChordRest);
      if (!nextSegm || nextSegm->tick() > nextSegTick) {      // no ChordRest segm at this tick
            nextSegm = new Segment(measure, Segment::Type::ChordRest, nextSegTick);
            if (!nextSegm) {
                  qDebug("figuredBassTicksTab: no next segment");
                  return;
                  }
            _score->undoAddElement(nextSegm);
            }

      endEdit();

      _score->startCmd();
      bool bNew;
      FiguredBass * fbNew = FiguredBass::addFiguredBassToSegment(nextSegm, track, ticks, &bNew);
      if (bNew)
            _score->undoAddElement(fbNew);
      _score->select(fbNew, SelectType::SINGLE, 0);
      startEdit(fbNew, -1);
      mscore->changeState(mscoreState());
      adjustCanvasPosition(fbNew, false);
      ((FiguredBass*)editObject)->moveCursorToEnd();
      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      if (!e1->selectable())
            return false;
      if (e1->z() == e2->z()) {
            if (e1->type() == e2->type()) {
                  if (e1->type() == Element::Type::NOTEDOT) {
                        const NoteDot* n1 = static_cast<const NoteDot*>(e1);
                        const NoteDot* n2 = static_cast<const NoteDot*>(e2);
                        if (n1->note()->hidden())
                              return n2;
                        else
                              return n1;
                        }
                  else if (e1->type() == Element::Type::NOTE) {
                        const Note* n1 = static_cast<const Note*>(e1);
                        const Note* n2 = static_cast<const Note*>(e2);
                        if (n1->hidden())
                              return n2;
                        else
                              return n1;
                        }
                  }
            }
      return e1->z() < e2->z();
      }

//---------------------------------------------------------
//   elementNear
//---------------------------------------------------------

Element* ScoreView::elementNear(QPointF p)
      {
      Page* page = point2page(p);
      if (!page) {
            // qDebug("  no page");
            return 0;
            }

      p -= page->pos();
      double w  = (preferences.proximity * .5) / matrix().m11();
      QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

      QList<Element*> el = page->items(r);
      QList<Element*> ll;
      foreach (Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->selectable() || e->type() == Element::Type::PAGE)
                  continue;
            if (e->contains(p))
                  ll.append(e);
            }
      int n = ll.size();
      if ((n == 0) || ((n == 1) && (ll[0]->type() == Element::Type::MEASURE))) {
            //
            // if no relevant element hit, look nearby
            //
            foreach (Element* e, el) {
                  if (e->type() == Element::Type::PAGE || !e->selectable())
                        continue;
                  if (e->intersects(r))
                        ll.append(e);
                  }
            }
      if (ll.empty()) {
            // qDebug("  nothing found");
            return 0;
            }
      qSort(ll.begin(), ll.end(), elementLower);

#if 0
      qDebug("elementNear");
      foreach(const Element* e, ll)
            qDebug("  %s selected %d z %d", e->name(), e->selected(), e->z());
#endif
      Element* e = ll.at(0);
      return e;
      }

//---------------------------------------------------------
//   posChanged
//---------------------------------------------------------

void ScoreView::posChanged(POS pos, unsigned /*tick*/)
      {
      switch (pos) {
            case POS::CURRENT:
                  moveCursor();
                  break;
            case POS::LEFT:
                  _curLoopIn->move(_score->pos(POS::LEFT));
                  break;
            case POS::RIGHT:
                  _curLoopOut->move(_score->pos(POS::RIGHT));
                  break;
            }
      }

//---------------------------------------------------------
//   loopToggled
//---------------------------------------------------------

void ScoreView::loopToggled(bool val)
      {
      if (_score->lastMeasure() == 0)
            return;
      if (_score->pos(POS::LEFT) == 0 && _score->pos(POS::RIGHT) == 0)
            _score->setPos(POS::RIGHT, _score->lastMeasure()->endTick()-1);
      _curLoopIn->move(_score->loopInTick());
      _curLoopOut->move(_score->loopOutTick());
      _curLoopIn->setVisible(val);
      _curLoopOut->setVisible(val);
      update();
      }

//---------------------------------------------------------
//   cmdMoveCR
//---------------------------------------------------------

void ScoreView::cmdMoveCR(bool left)
      {
      Element* e = _score->getSelectedElement();
      if (e && (e->type() == Element::Type::NOTE || e->type() == Element::Type::REST)) {
            if (e->type() == Element::Type::NOTE)
                  e = e->parent();
            QList<ChordRest*> crl;
            if (e->links()) {
                  for (Element* cr : *e->links())
                        crl.append(static_cast<ChordRest*>(cr));
                  }
            else
                  crl.append(static_cast<ChordRest*>(e));

            for (ChordRest* cr1 : crl) {
                  ChordRest* cr2 = left ? prevChordRest(cr1) : nextChordRest(cr1);
                  if (cr2) {
                        _score->startCmd();
                        _score->undo(new SwapCR(cr1, cr2));
                        _score->endCmd();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   updateContinuousPanel
//   slot triggered when moving around the score to keep the panel visible
//---------------------------------------------------------

void ScoreView::updateContinuousPanel()
      {
      if (_score->layoutMode() == LayoutMode::LINE)
            update();
      }
}
