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
#include "drumroll.h"
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
      setWindowTitle(QString("MuseScore"));
//      setIconSize(QSize(preferences.iconWidth, preferences.iconHeight));

      QWidget* mainWidget = new QWidget;
      QGridLayout* layout = new QGridLayout;
      mainWidget->setLayout(layout);
      layout->setSpacing(0);

      QToolBar* tb = addToolBar(tr("toolbar1"));
      tb->addAction(getAction("undo"));
      tb->addAction(getAction("redo"));
      tb->addSeparator();
#ifdef HAS_MIDI
      tb->addAction(getAction("midi-on"));
#endif
      QAction* a = getAction("follow");
      a->setCheckable(true);
      a->setChecked(preferences.followSong);

      tb->addAction(a);

      tb->addSeparator();
      tb->addAction(getAction("rewind"));
      tb->addAction(getAction("play"));
      tb->addSeparator();

      //-------------
      tb = addToolBar(tr("toolbar2"));
      layout->addWidget(tb, 1, 0, 1, 2);

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
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void DrumrollEditor::setStaff(Staff* st)
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
            veloType->setCurrentIndex(int(MScore::OFFSET_VAL));
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

void DrumrollEditor::selectionChanged()
      {
      updateSelection();
//      _score->blockSignals(true);
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  _score->select(note, SELECT_SINGLE, 0);
            }
      else if (items.size() == 0) {
            _score->select(0, SELECT_SINGLE, 0);
            }
      else {
            _score->select(0, SELECT_SINGLE, 0);
            foreach(QGraphicsItem* item, items) {
                  Note* note = static_cast<Note*>(item->data(0).value<void*>());
                  if (note)
                        _score->select(note, SELECT_ADD, 0);
                  }
            }
      _score->setUpdateAll();
      _score->end();
//      _score->blockSignals(false);
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void DrumrollEditor::changeSelection(int)
      {
//      gv->scene()->blockSignals(true);
      gv->scene()->clearSelection();
      QList<QGraphicsItem*> il = gv->scene()->items();
      foreach(QGraphicsItem* item, il) {
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  item->setSelected(note->selected());
            }
//      gv->scene()->blockSignals(false);
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
      if ((note == 0) || (MScore::ValueType(val) == note->veloType()))
            return;

      _score->undo()->beginMacro();
      _score->undo(new ChangeVelocity(note, MScore::ValueType(val), note->veloOffset()));
      _score->undo()->endMacro(_score->undo()->current()->childCount() == 0);
      updateVelocity(note);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void DrumrollEditor::updateVelocity(Note* note)
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

void DrumrollEditor::keyPressed(int pitch)
      {
      seq->startNote(staff->part()->instr()->channel(0).channel, pitch, 80, 0, 0.0);
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

void DrumrollEditor::heartBeat(Seq* seq)
      {
      unsigned t = seq->getCurTick();
      if (locator[0].tick() != t) {
            locator[0].setTick(t);
            gv->moveLocator(0);
            ruler->update();
            if (preferences.followSong)
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
      score()->endCmd();
      }
}

