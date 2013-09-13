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

#include "config.h"
#include "pianoroll.h"
#include "piano.h"
#include "ruler.h"
#include "pianoview.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "awl/pitchlabel.h"
#include "awl/pitchedit.h"
#include "awl/poslabel.h"
#include "musescore.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "seq.h"
#include "preferences.h"
#include "seq.h"
#include "waveview.h"

namespace Ms {

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(QWidget* parent)
   : QMainWindow(parent)
      {
      setWindowTitle(QString("MuseScore"));

      waveView = 0;
      _score = 0;
      staff  = 0;

      QWidget* mainWidget = new QWidget;

      QToolBar* tb = addToolBar(tr("toolbar1"));
      tb->addAction(getAction("undo"));
      tb->addAction(getAction("redo"));
      tb->addSeparator();
#ifdef HAS_MIDI
      tb->addAction(getAction("midi-on"));
#endif
      tb->addSeparator();

      tb->addAction(getAction("rewind"));
      tb->addAction(getAction("play"));
      tb->addSeparator();

      QAction* a = getAction("follow");
      a->setCheckable(true);
      a->setChecked(preferences.followSong);
      tb->addAction(a);
      tb->addSeparator();

      showWave = new QAction(tr("Wave"), tb);
      showWave->setToolTip(tr("show wave display"));
      showWave->setCheckable(true);
      showWave->setChecked(false);
      connect(showWave, SIGNAL(toggled(bool)), SLOT(showWaveView(bool)));
      tb->addAction(showWave);

      //-------------
      tb = addToolBar(tr("toolbar2"));
      static const char* sl3[] = { "voice-1", "voice-2", "voice-3", "voice-4" };
      QActionGroup* voiceGroup = new QActionGroup(this);
      for (const char* s : sl3) {
            QAction* a = getAction(s);
            a->setCheckable(true);
            voiceGroup->addAction(a);
            tb->addAction(getAction(s));
            }

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Cursor:")));
      pos = new Awl::PosLabel;
      tb->addWidget(pos);
      Awl::PitchLabel* pl = new Awl::PitchLabel();
      tb->addWidget(pl);

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Velocity:")));
      veloType = new QComboBox;
      veloType->addItem(tr("offset"), MScore::OFFSET_VAL);
      veloType->addItem(tr("user"),   MScore::USER_VAL);
      tb->addWidget(veloType);

      velocity = new QSpinBox;
      velocity->setRange(-1, 127);
      velocity->setSpecialValueText("--");
      velocity->setReadOnly(true);
      tb->addWidget(velocity);

      tb->addWidget(new QLabel(tr("Pitch:")));
      pitch = new Awl::PitchEdit;
      pitch->setReadOnly(true);
      tb->addWidget(pitch);

      //-------------
      qreal xmag = .1;
      ruler = new Ruler;
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);

      ruler->setMag(xmag, 1.0);

      Piano* piano = new Piano;
      piano->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      piano->setFixedWidth(pianoWidth);

      gv  = new PianoView;
      gv->scale(xmag, 1.0);
      gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      gv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      hsb = new QScrollBar(Qt::Horizontal);
      connect(gv->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)),
         SLOT(rangeChanged(int,int)));

      // layout
      QHBoxLayout* hbox = new QHBoxLayout;
      hbox->setSpacing(0);
      hbox->addWidget(piano);
      hbox->addWidget(gv);

      split = new QSplitter(Qt::Vertical);
      split->setFrameShape(QFrame::NoFrame);

      QWidget* split1 = new QWidget;      // piano - pianoview
      split1->setLayout(hbox);
      split->addWidget(split1);

      QGridLayout* layout = new QGridLayout;
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSpacing(0);
      layout->setColumnMinimumWidth(0, pianoWidth + 5);
      layout->addWidget(tb,    0, 0, 1, 2);
      layout->addWidget(ruler, 1, 1);
      layout->addWidget(split, 2, 0, 1, 2);
      layout->addWidget(hsb,   3, 1);

      mainWidget->setLayout(layout);
      setCentralWidget(mainWidget);

      connect(gv->verticalScrollBar(), SIGNAL(valueChanged(int)), piano, SLOT(setYpos(int)));

      connect(gv,          SIGNAL(magChanged(double,double)),  ruler, SLOT(setMag(double,double)));
      connect(gv,          SIGNAL(magChanged(double,double)),  piano, SLOT(setMag(double,double)));
      connect(gv,          SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(gv,          SIGNAL(pitchChanged(int)),          piano, SLOT(setPitch(int)));
      connect(piano,       SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(gv,          SIGNAL(posChanged(const Pos&)),     pos,   SLOT(setValue(const Pos&)));
      connect(gv,          SIGNAL(posChanged(const Pos&)),     ruler, SLOT(setPos(const Pos&)));
      connect(ruler,       SIGNAL(posChanged(const Pos&)),     pos,   SLOT(setValue(const Pos&)));

      connect(hsb,         SIGNAL(valueChanged(int)),  SLOT(setXpos(int)));
      connect(gv,          SIGNAL(xposChanged(int)),   SLOT(setXpos(int)));
      connect(gv->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(setXpos(int)));

      connect(ruler,       SIGNAL(locatorMoved(int)),  SLOT(moveLocator(int)));
      connect(veloType,    SIGNAL(activated(int)),     SLOT(veloTypeChanged(int)));
      connect(velocity,    SIGNAL(valueChanged(int)),  SLOT(velocityChanged(int)));
      connect(gv->scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
      connect(piano,       SIGNAL(keyPressed(int)),    SLOT(keyPressed(int)));
      connect(piano,       SIGNAL(keyReleased(int)),   SLOT(keyReleased(int)));
      resize(800, 400);

      QActionGroup* ag = new QActionGroup(this);
      a = new QAction(this);
      a->setData("delete");
      a->setShortcut(Qt::Key_Delete);
      ag->addAction(a);
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));
      setXpos(0);
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void PianorollEditor::setXpos(int x)
      {
      gv->horizontalScrollBar()->setValue(x);
      ruler->setXpos(x);
      if (waveView && showWave->isChecked())
            waveView->setXpos(x);
      }

//---------------------------------------------------------
//   rangeChanged
//---------------------------------------------------------

void PianorollEditor::rangeChanged(int min, int max)
      {
      hsb->setRange(min, max);
      }

//---------------------------------------------------------
//   ~PianorollEditor
//---------------------------------------------------------

PianorollEditor::~PianorollEditor()
      {
      if (_score)
            _score->removeViewer(this);
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianorollEditor::setStaff(Staff* st)
      {
      staff = st;
      _score = staff->score();
      setWindowTitle(QString(tr("MuseScore: <%1> Staff: %2")).arg(_score->name()).arg(st->idx()));
      TempoMap* tl = _score->tempomap();
      TimeSigMap*  sl = _score->sigmap();
      for (int i = 0; i < 3; ++i)
            locator[i].setContext(tl, sl);

      locator[0].setTick(480 * 5 + 240);  // some random test values
      locator[1].setTick(480 * 3 + 240);
      locator[2].setTick(480 * 12 + 240);

      gv->setStaff(staff, locator);
      ruler->setScore(_score, locator);
      if (waveView)
            waveView->setScore(_score, locator);
      pos->setContext(tl, sl);
      updateSelection();
      showWave->setEnabled(_score->audio() != 0);
      }

//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void PianorollEditor::updateSelection()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            PianoItem* item = static_cast<PianoItem*>(items[0]);
            if (item->type() == PianoItemType) {
                  Note* note = item->note();
                  pitch->setEnabled(true);
                  pitch->setValue(note->pitch());
                  veloType->setEnabled(true);
                  velocity->setEnabled(true);
                  updateVelocity(note);
                  }
            }
      else if (items.size() == 0) {
            velocity->setValue(0);
            velocity->setEnabled(false);
            pitch->setValue(0);
            pitch->setEnabled(false);
            veloType->setEnabled(false);
            veloType->setCurrentIndex(int(MScore::USER_VAL));
            }
      else {
            velocity->setEnabled(true);
            velocity->setValue(0);
            velocity->setReadOnly(false);
            pitch->setEnabled(true);
            pitch->setDeltaMode(true);
            pitch->setValue(0);
            veloType->setEnabled(true);
            veloType->setCurrentIndex(int(MScore::OFFSET_VAL));
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      updateSelection();
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            if (item->type() == PianoItemType) {
                  Note* note = static_cast<PianoItem*>(item)->note();
                  _score->select(note, SELECT_SINGLE, 0);
                  }
            }
      else if (items.size() == 0) {
            _score->select(0, SELECT_SINGLE, 0);
            }
      else {
            _score->deselectAll();
            foreach(QGraphicsItem* item, items) {
                  if (item->type() == PianoItemType) {
                        Note* note = static_cast<PianoItem*>(item)->note();
                        _score->select(note, SELECT_ADD, 0);
                        }
                  }
            }
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void PianorollEditor::changeSelection(int)
      {
      gv->scene()->blockSignals(true);
      gv->scene()->clearSelection();
      QList<QGraphicsItem*> il = gv->scene()->items();
      foreach(QGraphicsItem* item, il) {
            if (item->type() == PianoItemType) {
                  Note* note = static_cast<PianoItem*>(item)->note();
                  item->setSelected(note->selected());
                  }
            }
      gv->scene()->blockSignals(false);
      }

//---------------------------------------------------------
//   veloTypeChanged
//---------------------------------------------------------

void PianorollEditor::veloTypeChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      if (item->type() != PianoItemType)
            return;
      Note* note = static_cast<PianoItem*>(item)->note();
      if (MScore::ValueType(val) == note->veloType())
            return;

      _score->undo()->beginMacro();
      _score->undo(new ChangeVelocity(note, MScore::ValueType(val), note->veloOffset()));
      _score->undo()->endMacro(_score->undo()->current()->childCount() == 0);
      updateVelocity(note);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void PianorollEditor::updateVelocity(Note* note)
      {
      MScore::ValueType vt = note->veloType();
      if (vt != MScore::ValueType(veloType->currentIndex())) {
            veloType->setCurrentIndex(int(vt));
            switch(vt) {
                  case MScore::USER_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("");
                        velocity->setRange(0, 127);
                        break;
                  case MScore::OFFSET_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("%");
                        velocity->setRange(-200, 200);
                        break;
                  }
            }
      switch(vt) {
            case MScore::USER_VAL:
                  // TODO velocity->setValue(note->velocity());
                  break;
            case MScore::OFFSET_VAL:
                  velocity->setValue(note->veloOffset());
                  break;
            }
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void PianorollEditor::velocityChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      if (item->type() != PianoItemType)
            return;
      Note* note = static_cast<PianoItem*>(item)->note();
      MScore::ValueType vt = note->veloType();

      if (vt == MScore::OFFSET_VAL)
            return;

      _score->undo()->beginMacro();
      _score->undo(new ChangeVelocity(note, vt, val));
      _score->undo()->endMacro(_score->undo()->current()->childCount() == 0);
      }

//---------------------------------------------------------
//   keyPressed
//---------------------------------------------------------

void PianorollEditor::keyPressed(int pitch)
      {
      seq->startNote(staff->part()->instr()->channel(0).channel, pitch, 80, 0, 0.0);
      }

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void PianorollEditor::keyReleased(int /*pitch*/)
      {
      seq->stopNotes();
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PianorollEditor::heartBeat(Seq* seq)
      {
      unsigned t = seq->getCurTick();
      if (locator[0].tick() != t) {
            locator[0].setTick(t);
            gv->moveLocator(0);
            if (waveView)
                  waveView->moveLocator(0);
            ruler->update();
            if (preferences.followSong)
                  gv->ensureVisible(t);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianorollEditor::moveLocator(int i)
      {
      if (locator[i].valid()) {
            seq->seek(locator[i].tick());
            gv->moveLocator(i);
            if (waveView)
                  waveView->moveLocator(i);
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void PianorollEditor::cmd(QAction* a)
      {
      score()->startCmd();
      if (a->data() == "delete") {
            QList<QGraphicsItem*> items = gv->items();
            foreach(QGraphicsItem* item, items) {
                  if (item->type() == PianoItemType) {
                        Note* note = static_cast<PianoItem*>(item)->note();
                        score()->deleteItem(note);
                        }
                  }
            }

      gv->setStaff(staff, locator);
      score()->endCmd();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void PianorollEditor::dataChanged(const QRectF&)
      {
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void PianorollEditor::moveCursor()
      {
      }

//---------------------------------------------------------
//   updateLoopCursor
//---------------------------------------------------------

void PianorollEditor::updateLoopCursors()
      {
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void PianorollEditor::adjustCanvasPosition(const Element*, bool)
      {
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PianorollEditor::setScore(Score* s)
      {
      if (_score)
            _score->removeViewer(this);
      _score = s;
      _score->addViewer(this);
      }

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

void PianorollEditor::removeScore()
      {
      _score = 0;
      }

//---------------------------------------------------------
//   changeEditElement
//---------------------------------------------------------

void PianorollEditor::changeEditElement(Element*)
      {
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

QCursor PianorollEditor::cursor() const
      {
      return QCursor();
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

void PianorollEditor::setCursor(const QCursor&)
      {
      }

//---------------------------------------------------------
//   gripCount
//---------------------------------------------------------

int PianorollEditor::gripCount() const
      {
      return 0;
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

const QRectF& PianorollEditor::getGrip(int) const
      {
      static QRectF r;
      return r;
      }

//---------------------------------------------------------
//   matrix
//---------------------------------------------------------

const QTransform& PianorollEditor::matrix() const
      {
      static QTransform t;
      return t;
      }

//---------------------------------------------------------
//   setDropRectangle
//---------------------------------------------------------

void PianorollEditor::setDropRectangle(const QRectF&)
      {
      }

//---------------------------------------------------------
//   cmdAddSlur
//---------------------------------------------------------

void PianorollEditor::cmdAddSlur(Note*, Note*)
      {
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void PianorollEditor::startEdit()
      {
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void PianorollEditor::startEdit(Element*, int)
      {
      }

//---------------------------------------------------------
//   elementNear
//---------------------------------------------------------

Element* PianorollEditor::elementNear(QPointF)
      {
      return 0;
      }

//---------------------------------------------------------
//   updateAll
//---------------------------------------------------------

void PianorollEditor::updateAll()
      {
      gv->update();
      }

//---------------------------------------------------------
//   showWavView
//---------------------------------------------------------

void PianorollEditor::showWaveView(bool val)
      {
      if (val) {
            if (waveView == 0) {
                  waveView = new WaveView;
                  connect(gv, SIGNAL(magChanged(double,double)), waveView, SLOT(setMag(double,double)));
                  connect(gv, SIGNAL(posChanged(const Pos&)), waveView,   SLOT(setValue(const Pos&)));
                  waveView->setAudio(_score->audio());
                  waveView->setScore(_score, locator);
                  split->addWidget(waveView);
                  waveView->setMag(ruler->xmag(), 1.0);
                  waveView->setXpos(ruler->xpos());
                  }
            waveView->setVisible(true);
            }
      else {
            if (waveView)
                  waveView->setVisible(false);
            }
      }
}

