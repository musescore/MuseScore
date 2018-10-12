//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "drumroll.h"
#include "config.h"
#include "piano.h"
#include "ruler.h"
#include "drumview.h"
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

namespace Ms {

//---------------------------------------------------------
//   DrumrollEditor
//---------------------------------------------------------

DrumrollEditor::DrumrollEditor(QWidget* parent)
   : QMainWindow(parent)
      {
      setObjectName("Drumroll");
      setWindowTitle(QString("MuseScore"));
//      setIconSize(QSize(preferences.iconWidth, preferences.iconHeight));

      QWidget* mainWidget = new QWidget;
      QGridLayout* layout = new QGridLayout;
      mainWidget->setLayout(layout);
      layout->setSpacing(0);

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
      QAction* a = getAction("follow");
      a->setCheckable(true);
      a->setChecked(preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG));

      tb->addAction(a);

      tb->addSeparator();
      tb->addAction(getAction("rewind"));
      tb->addAction(getAction("play"));
      tb->addSeparator();

      //-------------
      tb = addToolBar(tr("Toolbar 3"));
      layout->addWidget(tb, 1, 0, 1, 2);

      for (int i = 0; i < VOICES; ++i) {
            QToolButton* b = new QToolButton(this);
            b->setToolButtonStyle(Qt::ToolButtonTextOnly);
            QPalette p(b->palette());
            p.setColor(QPalette::Base, MScore::selectColor[i]);
            b->setPalette(p);
            QAction* aa = getAction(voiceActions[i]);
            b->setDefaultAction(aa);
            tb->addWidget(b);
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
      veloType->addItem(tr("offset"), int(Note::ValueType::OFFSET_VAL));
      veloType->addItem(tr("user"),   int(Note::ValueType::USER_VAL));
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

      double xmag = .1;
      gv  = new DrumView;
      gv->scale(xmag, 1.0);
      layout->addWidget(gv, 3, 1);

      ruler = new Ruler;
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);
      ruler->setMag(xmag, 1.0);

      layout->addWidget(ruler, 2, 1);

      Piano* piano = new Piano;
      piano->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      piano->setFixedWidth(pianoWidth);
      layout->addWidget(piano, 3, 0);

      setCentralWidget(mainWidget);

      connect(gv->verticalScrollBar(), SIGNAL(valueChanged(int)), piano, SLOT(setYpos(int)));
      connect(gv->horizontalScrollBar(), SIGNAL(valueChanged(int)), ruler, SLOT(setXpos(int)));
      connect(gv,          SIGNAL(xposChanged(int)),           ruler, SLOT(setXpos(int)));
      connect(gv,          SIGNAL(magChanged(double,double)),  ruler, SLOT(setMag(double,double)));
      connect(gv,          SIGNAL(magChanged(double,double)),  piano, SLOT(setMag(double,double)));
      connect(gv,          SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(gv,          SIGNAL(pitchChanged(int)),          piano, SLOT(setPitch(int)));
      connect(piano,       SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(gv,          SIGNAL(posChanged(const Pos&)),     pos,   SLOT(setValue(const Pos&)));
      connect(gv,          SIGNAL(posChanged(const Pos&)),     ruler, SLOT(setPos(const Pos&)));
      connect(ruler,       SIGNAL(posChanged(const Pos&)),     pos,   SLOT(setValue(const Pos&)));
      connect(ruler,       SIGNAL(locatorMoved(int)),                 SLOT(moveLocator(int)));
      connect(veloType,    SIGNAL(activated(int)),                    SLOT(veloTypeChanged(int)));
      connect(velocity,    SIGNAL(valueChanged(int)),                 SLOT(velocityChanged(int)));
      connect(gv->scene(), SIGNAL(selectionChanged()),                SLOT(selectionChanged()));
      connect(piano,       SIGNAL(keyPressed(int)),                   SLOT(keyPressed(int)));
      connect(piano,       SIGNAL(keyReleased(int)),                  SLOT(keyReleased(int)));
      resize(800, 400);

      QActionGroup* ag = new QActionGroup(this);
      a = new QAction(this);
      a->setData("delete");
      a->setShortcut(Qt::Key_Delete);
      ag->addAction(a);
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), SLOT(cmd(QAction*)));

      readSettings();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void DrumrollEditor::writeSettings()
      {
      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void DrumrollEditor::readSettings()
      {
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void DrumrollEditor::setStaff(Staff* st)
      {
      staff = st;
      _score = staff->score();
      setWindowTitle(tr("<%1> Staff: %2").arg(_score->masterScore()->fileInfo()->completeBaseName()).arg(st->idx()));
      TempoMap* tl = _score->tempomap();
      TimeSigMap*  sl = _score->sigmap();
      for (int i = 0; i < 3; ++i)
            locator[i].setContext(tl, sl);

      locator[0].setTick(480 * 5 + 240);  // some random test values
      locator[1].setTick(480 * 3 + 240);
      locator[2].setTick(480 * 12 + 240);

      gv->setStaff(staff, locator);
      ruler->setScore(_score, locator);
      pos->setContext(tl, sl);
      updateSelection();
      }

//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void DrumrollEditor::updateSelection()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            pitch->setEnabled(true);
            pitch->setValue(note->pitch());
            veloType->setEnabled(true);
            velocity->setEnabled(true);
            updateVelocity(note);
            }
      else if (items.size() == 0) {
            velocity->setValue(0);
            velocity->setEnabled(false);
            pitch->setValue(0);
            pitch->setEnabled(false);
            veloType->setEnabled(false);
            veloType->setCurrentIndex(int(Note::ValueType::OFFSET_VAL));
            }
      else {
            velocity->setEnabled(true);
            velocity->setValue(0);
            velocity->setReadOnly(false);
            pitch->setEnabled(true);
            pitch->setDeltaMode(true);
            pitch->setValue(0);
            veloType->setEnabled(true);
            veloType->setCurrentIndex(int(Note::ValueType::OFFSET_VAL));
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void DrumrollEditor::selectionChanged()
      {
      updateSelection();
//      _score->blockSignals(true);
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  _score->select(note, SelectType::SINGLE, 0);
            }
      else if (items.size() == 0) {
            _score->select(0, SelectType::SINGLE, 0);
            }
      else {
            _score->select(0, SelectType::SINGLE, 0);
            foreach(QGraphicsItem* item, items) {
                  Note* note = static_cast<Note*>(item->data(0).value<void*>());
                  if (note)
                        _score->select(note, SelectType::ADD, 0);
                  }
            }
      _score->setUpdateAll();
      _score->update();
//      _score->blockSignals(false);
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void DrumrollEditor::changeSelection(SelState)
      {
      gv->scene()->blockSignals(true);
      gv->scene()->clearSelection();
      QList<QGraphicsItem*> il = gv->scene()->items();
      foreach(QGraphicsItem* item, il) {
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  item->setSelected(note->selected());
            }
      gv->scene()->blockSignals(false);
      }

//---------------------------------------------------------
//   veloTypeChanged
//---------------------------------------------------------

void DrumrollEditor::veloTypeChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      Note* note = (Note*)item->data(0).value<void*>();
      if ((note == 0) || (Note::ValueType(val) == note->veloType()))
            return;

      _score->undoStack()->beginMacro();
      _score->undo(new ChangeVelocity(note, Note::ValueType(val), note->veloOffset()));
      _score->undoStack()->endMacro(_score->undoStack()->current()->childCount() == 0);
      updateVelocity(note);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void DrumrollEditor::updateVelocity(Note* note)
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
      velocity->setValue(note->veloOffset());
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void DrumrollEditor::velocityChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      Note* note = (Note*)item->data(0).value<void*>();
      if (note == 0)
            return;
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

void DrumrollEditor::keyPressed(int p)
      {
      seq->startNote(staff->part()->instrument()->channel(0)->channel, p, 80, 0, 0.0);
      }

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void DrumrollEditor::keyReleased(int /*pitch*/)
      {
      seq->stopNotes();
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void DrumrollEditor::heartBeat(Seq* s)
      {
      unsigned t = s->getCurTick();
      if (locator[0].tick() != t) {
            locator[0].setTick(t);
            gv->moveLocator(0);
            ruler->update();
            if (preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG))
                  gv->ensureVisible(t);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void DrumrollEditor::moveLocator(int i)
      {
      if (locator[i].valid()) {
            seq->seek(locator[i].tick());
            gv->moveLocator(i);
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void DrumrollEditor::cmd(QAction* a)
      {
      score()->startCmd();
      if (a->data() == "delete") {
            QList<QGraphicsItem*> items = gv->items();
            foreach(QGraphicsItem* item, items) {
                  Note* note = static_cast<Note*>(item->data(0).value<void*>());
                  if (note) {
                        score()->deleteItem(note);
                        }
                  }
            }

      gv->setStaff(staff, locator);
      }
}

