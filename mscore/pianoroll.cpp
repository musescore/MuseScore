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

#include "pianoroll.h"
#include "config.h"
#include "piano.h"
#include "ruler.h"
#include "pianoview.h"
#include "musescore.h"
#include "seq.h"
#include "preferences.h"
#include "waveview.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/repeatlist.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "awl/pitchlabel.h"
#include "awl/pitchedit.h"
#include "awl/poslabel.h"


namespace Ms {

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(QWidget* parent)
   : QMainWindow(parent)
      {
      setObjectName("Pianoroll");
      setWindowTitle(QString("MuseScore"));

      waveView = 0;
      _score   = 0;
      staff    = 0;

      QWidget* mainWidget = new QWidget;
      QToolBar* tb = addToolBar(tr("Toolbar 1"));
      if (qApp->layoutDirection() == Qt::LayoutDirection::LeftToRight) {
            tb->addAction(getAction("undo"));
            tb->addAction(getAction("redo"));
            }
      else {
            tb->addAction(getAction("redo"));
            tb->addAction(getAction("undo"));
            }
      tb->addSeparator();
#ifdef HAS_MIDI
      tb->addAction(getAction("midi-on"));
#endif
      tb->addSeparator();

      tb->addAction(getAction("rewind"));
      tb->addAction(getAction("play"));
      tb->addSeparator();

      tb->addAction(getAction("loop"));
      tb->addSeparator();
      tb->addAction(getAction("repeat"));
      QAction* followAction = getAction("follow");
      followAction->setChecked(preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG));
      tb->addAction(followAction);
      tb->addSeparator();
      tb->addAction(getAction("metronome"));

      showWave = new QAction(tr("Wave"), tb);
      showWave->setToolTip(tr("Show wave display"));
      showWave->setCheckable(true);
      showWave->setChecked(false);
      connect(showWave, SIGNAL(toggled(bool)), SLOT(showWaveView(bool)));
      tb->addAction(showWave);

      //-------------
      tb = addToolBar(tr("Toolbar 2"));
      for (int i = 0; i < VOICES; ++i) {
            QToolButton* b = new QToolButton(this);
            b->setToolButtonStyle(Qt::ToolButtonTextOnly);
            QPalette p(b->palette());
            p.setColor(QPalette::Base, MScore::selectColor[i]);
            b->setPalette(p);
            QAction* a = getAction(voiceActions[i]);
            b->setDefaultAction(a);
            tb->addWidget(b);
            }

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Cursor:")));
      pos = new Awl::PosLabel;
      pos->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

      tb->addWidget(pos);
      Awl::PitchLabel* pl = new Awl::PitchLabel();
      pl->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      tb->addWidget(pl);

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Velocity:")));
      veloType = new QComboBox;
      veloType->addItem(tr("Offset"), int(Note::ValueType::OFFSET_VAL));
      veloType->addItem(tr("User"),   int (Note::ValueType::USER_VAL));
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

      tb->addWidget(new QLabel(tr("OnTime:")));
      tb->addWidget((onTime = new QSpinBox));
      onTime->setRange(-2000, 2000);

      tb->addWidget(new QLabel(tr("Len:")));
      tb->addWidget((tickLen = new QSpinBox));
      tickLen->setRange(-2000, 2000);

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

      connect(ruler,       SIGNAL(locatorMoved(int, const Pos&)), SLOT(moveLocator(int, const Pos&)));
      connect(veloType,    SIGNAL(activated(int)),     SLOT(veloTypeChanged(int)));
      connect(velocity,    SIGNAL(valueChanged(int)),  SLOT(velocityChanged(int)));
      connect(onTime,      SIGNAL(valueChanged(int)),  SLOT(onTimeChanged(int)));
      connect(tickLen,     SIGNAL(valueChanged(int)),  SLOT(tickLenChanged(int)));
      connect(gv->scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
      connect(piano,       SIGNAL(keyPressed(int)),    SLOT(keyPressed(int)));
      connect(piano,       SIGNAL(keyReleased(int)),   SLOT(keyReleased(int)));

      readSettings();

      actions.append(getAction("delete"));
      actions.append(getAction("pitch-up"));
      actions.append(getAction("pitch-down"));
      actions.append(getAction("pitch-up-octave"));
      actions.append(getAction("pitch-down-octave"));
      addActions(actions);
      for (auto* action : actions)
            connect(action, &QAction::triggered, this, [this, action](bool){ cmd(action); });

      setXpos(0);
      }

//---------------------------------------------------------
//   ~PianorollEditor
//---------------------------------------------------------

PianorollEditor::~PianorollEditor()
      {
      if (_score)
            _score->removeViewer(this);
      for (auto* action : actions)
            action->disconnect(this);
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianorollEditor::setStaff(Staff* st)
      {
      if ((st && st->score() != _score) || (!st && _score)) {
            if (_score) {
                  _score->removeViewer(this);
                  disconnect(_score, SIGNAL(posChanged(POS,unsigned)), this, SLOT(posChanged(POS,unsigned)));
                  disconnect(_score, SIGNAL(playlistChanged()), this, SLOT(playlistChanged()));
                  }
            _score = st ? st->score() : nullptr;
            if (_score) {
                  _score->addViewer(this);
                  setLocator(POS::CURRENT, _score->pos(POS::CURRENT));
                  setLocator(POS::LEFT,    _score->pos(POS::LEFT));
                  setLocator(POS::RIGHT,   _score->pos(POS::RIGHT));
                  connect(_score, SIGNAL(posChanged(POS,unsigned)), SLOT(posChanged(POS,unsigned)));
                  connect(_score, SIGNAL(playlistChanged()), SLOT(playlistChanged()));
                  }
            }
      staff = st;
      if (staff) {
            setWindowTitle(tr("<%1> Staff: %2").arg(_score->masterScore()->fileInfo()->completeBaseName()).arg(st->idx()));
            TempoMap* tl = _score->tempomap();
            TimeSigMap*  sl = _score->sigmap();
            for (int i = 0; i < 3; ++i)
                  locator[i].setContext(tl, sl);
            pos->setContext(tl, sl);
            showWave->setEnabled(_score->audio() != 0);
            }
      ruler->setScore(_score, locator);
      gv->setStaff(staff, locator);
      updateSelection();
      setEnabled(st != nullptr);
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void PianorollEditor::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void PianorollEditor::readSettings()
      {
      resize(QSize(800, 600)); //ensure default size if no geometry in settings
      MuseScore::restoreGeometry(this);
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
//   updateSelection
//---------------------------------------------------------

void PianorollEditor::updateSelection()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            PianoItem* item = static_cast<PianoItem*>(items[0]);
            if (item->type() == PianoItemType) {
                  Note* note = item->note();
                  NoteEvent* event = item->event();
                  pitch->setEnabled(true);
                  pitch->setValue(note->pitch());
                  onTime->setValue(event->ontime());
                  tickLen->setValue(event->len());
                  updateVelocity(note);
                  }
            }
      bool b = items.size() != 0;
      velocity->setEnabled(b);
      pitch->setEnabled(b);
      veloType->setEnabled(b);
      onTime->setEnabled(b);
      tickLen->setEnabled(b);
      }

//---------------------------------------------------------
//   selectionChanged
//    called if selection in PianoView changed
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            if (item->type() == PianoItemType) {
                  Note* note = static_cast<PianoItem*>(item)->note();
                  _score->select(note, SelectType::SINGLE, 0);
                  }
            }
      else if (items.size() == 0)
            _score->select(0, SelectType::SINGLE, 0);
      else {
            _score->deselectAll();
            for (QGraphicsItem* item : items) {
                  if (item->type() == PianoItemType) {
                        Note* note = static_cast<PianoItem*>(item)->note();
                        if (!note->selected())
                              _score->select(note, SelectType::ADD, 0);
                        }
                  }
            }
      for (MuseScoreView* view : score()->getViewer())
            view->updateAll();

      gv->scene()->blockSignals(true);
      for (QGraphicsItem* item : gv->scene()->items())
            if (item->type() == PianoItemType)
                item->setSelected(static_cast<PianoItem*>(item)->note()->selected());
      gv->scene()->blockSignals(false);

      gv->scene()->update();
      updateSelection();
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void PianorollEditor::changeSelection(SelState)
      {
      gv->scene()->blockSignals(true);
      gv->scene()->clearSelection();
      QList<QGraphicsItem*> il = gv->scene()->items();
      for (QGraphicsItem* item : il) {
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
      if (Note::ValueType(val) == note->veloType())
            return;

      _score->undoStack()->beginMacro();
      _score->undo(new ChangeVelocity(note, Note::ValueType(val), note->veloOffset()));
      _score->undoStack()->endMacro(_score->undoStack()->current()->childCount() == 0);
      updateVelocity(note);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void PianorollEditor::updateVelocity(Note* note)
      {
      Note::ValueType vt = note->veloType();
      if (vt != Note::ValueType(veloType->currentIndex())) {
            veloType->setCurrentIndex(int(vt));
            switch(vt) {
                  case Note::ValueType::USER_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("");
                        velocity->setRange(0, 127);
                        break;
                  case Note::ValueType::OFFSET_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("%");
                        velocity->setRange(-200, 200);
                        break;
                  }
            }
      switch(vt) {
            case Note::ValueType::USER_VAL:
                  // TODO velocity->setValue(note->velocity());
                  break;
            case Note::ValueType::OFFSET_VAL:
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
      Note::ValueType vt = note->veloType();

      if (vt == Note::ValueType::OFFSET_VAL)
            return;

      _score->undoStack()->beginMacro();
      _score->undo(new ChangeVelocity(note, vt, val));
      _score->undoStack()->endMacro(_score->undoStack()->current()->childCount() == 0);
      }

//---------------------------------------------------------
//   keyPressed
//---------------------------------------------------------

void PianorollEditor::keyPressed(int p)
      {
      seq->startNote(staff->part()->instrument()->channel(0)->channel, p, 80, 0, 0.0);
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

void PianorollEditor::heartBeat(Seq* s)
      {
      unsigned tick = s->getCurTick();
      if (score()->repeatList())
            tick = score()->repeatList()->utick2tick(tick);
      if (locator[0].tick() != tick) {
            posChanged(POS::CURRENT, tick);
            if (preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG))
                  gv->ensureVisible(tick);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianorollEditor::moveLocator(int i, const Pos& p)
      {
      if (locator[i].valid())
            score()->setPos(POS(i), p.tick());
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void PianorollEditor::cmd(QAction* /*a*/)
      {
      //score()->startCmd();
      gv->setStaff(staff, locator);
      //score()->endCmd();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void PianorollEditor::dataChanged(const QRectF&)
      {
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void PianorollEditor::adjustCanvasPosition(const Element*, bool)
      {
      }

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

void PianorollEditor::removeScore()
      {
      setStaff(0);
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

void PianorollEditor::startEdit(Element*, Grip)
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
//      startTimer(0);    // delayed update
//      gv->updateNotes();
//      gv->update();
      }

void PianorollEditor::playlistChanged()
      {
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

//---------------------------------------------------------
//   posChanged
//    position in score has changed
//---------------------------------------------------------

void PianorollEditor::posChanged(POS p, unsigned tick)
      {
      if (locator[int(p)].tick() == unsigned(tick))
            return;
      setLocator(p, tick);
      gv->moveLocator(int(p));
      if (waveView)
            waveView->moveLocator(int(p));
      ruler->update();
      }

//---------------------------------------------------------
//   onTimeChanged
//---------------------------------------------------------

void PianorollEditor::onTimeChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      if (item->type() != PianoItemType)
            return;
      PianoItem* pi = static_cast<PianoItem*>(item);
      Note* note       = pi->note();
      NoteEvent* event = pi->event();
      if (event->ontime() == val)
            return;

      NoteEvent ne = *event;
      ne.setOntime(val);
      _score->startCmd();
      _score->undo(new ChangeNoteEvent(note, event, ne));
      _score->endCmd();
      }

//---------------------------------------------------------
//   tickLenChanged
//---------------------------------------------------------

void PianorollEditor::tickLenChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      if (item->type() != PianoItemType)
            return;
      PianoItem* pi = static_cast<PianoItem*>(item);
      Note* note       = pi->note();
      NoteEvent* event = pi->event();
      if (event->len() == val)
            return;

      NoteEvent ne = *event;
      ne.setLen(val);
      _score->startCmd();
      _score->undo(new ChangeNoteEvent(note, event, ne));
      _score->endCmd();
      }

}
