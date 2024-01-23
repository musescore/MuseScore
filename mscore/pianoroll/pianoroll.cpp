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
#include "shortcut.h"
#include "config.h"
#include "pianokeyboard.h"
#include "pianoruler.h"
#include "pianolevels.h"
#include "pianolevelschooser.h"
#include "pianoview.h"
#include "musescore.h"
#include "seq.h"
#include "preferences.h"
#include "waveview.h"
#include "notetweakerdialog.h"
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


      QActionGroup* ag = Shortcut::getActionGroupForWidget(MsWidget::PIANO_ROLL_EDITOR);
      ag->setParent(this);
      addActions(ag->actions());
      connect(ag, SIGNAL(triggered(QAction*)), this, SLOT(handleAction(QAction*)));

      noteTweakerDlg = new NoteTweakerDialog(this);


      QWidget* mainWidget = new QWidget;
      QToolBar* tbMain = addToolBar("Toolbar Main");
      if (qApp->layoutDirection() == Qt::LayoutDirection::LeftToRight) {
            tbMain->addAction(getAction("undo"));
            tbMain->addAction(getAction("redo"));
            }
      else {
            tbMain->addAction(getAction("redo"));
            tbMain->addAction(getAction("undo"));
            }
      tbMain->addSeparator();
#ifdef HAS_MIDI
      tbMain->addAction(getAction("midi-on"));
#endif
      tbMain->addSeparator();

      tbMain->addAction(getAction("rewind"));
      tbMain->addAction(getAction("play"));
      tbMain->addSeparator();

      tbMain->addAction(getAction("loop"));
      tbMain->addSeparator();
      tbMain->addAction(getAction("repeat"));
      QAction* followAction = getAction("follow");
      followAction->setChecked(preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG));
      tbMain->addAction(followAction);
      tbMain->addSeparator();
      tbMain->addAction(getAction("metronome"));

      showWave = new QAction(tr("Wave"), tbMain);
      showWave->setToolTip(tr("Show wave display"));
      showWave->setCheckable(true);
      showWave->setChecked(false);
      connect(showWave, SIGNAL(toggled(bool)), SLOT(showWaveView(bool)));
      tbMain->addAction(showWave);

      tbMain->addSeparator();

      partLabel = new QLabel(tr("Part:"));
      tbMain->addWidget(partLabel);

      // --------------------------------------------------
      // toolbars


      //----

      QToolBar* tbTool = addToolBar("Action Buttons");
      QButtonGroup* bngrpActionBns = new QButtonGroup();


      struct ToolIconData
      {
            QString _icon;
            PianoRollEditTool _tool;
            QString _tooltip;
            bool _selected;
            };
      ToolIconData _iconDataTool[] = {
            { QStringLiteral(":/data/icons/preEdit-select.svg"), PianoRollEditTool::SELECT, tr("Select Notes"), true },
            { QStringLiteral(":/data/icons/preEdit-insertNote.svg"), PianoRollEditTool::ADD, tr("Insert Note"), false },
            //{ QStringLiteral(":/data/icons/preEdit-appendChord.svg"), PianoRollEditTool::APPEND_NOTE, tr("Append Note to Chord"), false },
            { QStringLiteral(":/data/icons/preEdit-cutNote.svg"), PianoRollEditTool::CUT, tr("Cut Chord"), false },
            { QStringLiteral(":/data/icons/preEdit-eraseNote.svg"), PianoRollEditTool::ERASE, tr("Erase Note"), false },
            { QStringLiteral(":/data/icons/preEdit-changeLength.svg"), PianoRollEditTool::EVENT_ADJUST, tr("Change Chord Length"), false },
            { QStringLiteral(":/data/icons/preEdit-tie.svg"), PianoRollEditTool::TIE, tr("Toggle Tie"), false },
            { "", PianoRollEditTool::LAST, "", false },
            };

      for (ToolIconData* p = _iconDataTool; p->_tool != PianoRollEditTool::LAST; ++p) {
            QToolButton* bn = new QToolButton();
            QIcon icon;
            icon.addFile(p->_icon, QSize(), QIcon::Normal, QIcon::Off);
            bn->setIcon(icon);
            bn->setCheckable(true);
            bn->setToolTip(p->_tooltip);
            PianoRollEditTool tool = p->_tool;
            connect(bn, &QToolButton::clicked, this, [=]() {this->setEditNoteTool(tool); });

            if (p->_selected)
                  bn->setChecked(true);
            bngrpActionBns->addButton(bn);
            tbTool->addWidget(bn);
            }

      //----

      struct LenIconData
      {
            QString _icon;
            int _measureFrac;  //Note length is 2^n of a measure
            bool _selected;
      };

      LenIconData _iconData[] = {
            { QStringLiteral(":/data/icons/note-longa.svg"), 2, false },
            { QStringLiteral(":/data/icons/note-breve.svg"), 1, false },
            { QStringLiteral(":/data/icons/note-1.svg"), 0, true },
            { QStringLiteral(":/data/icons/note-2.svg"), -1, false },
            { QStringLiteral(":/data/icons/note-4.svg"), -2, false },
            { QStringLiteral(":/data/icons/note-8.svg"), -3, false },
            { QStringLiteral(":/data/icons/note-16.svg"), -4, false },
            { QStringLiteral(":/data/icons/note-32.svg"), -5, false },
            { QStringLiteral(":/data/icons/note-64.svg"), -6, false },
            { QStringLiteral(":/data/icons/note-128.svg"), -7, false },
            { QStringLiteral(":/data/icons/note-256.svg"), -8, false },
            { QStringLiteral(":/data/icons/note-512.svg"), -9, false },
            { QStringLiteral(":/data/icons/note-1024.svg"), -10, false },
            { "", 0, false },
            };

      QToolBar* tbNoteLen = addToolBar("Toolbar Note Length");
      QButtonGroup* bngrpNoteLen = new QButtonGroup();

      for (LenIconData* p = _iconData; !p->_icon.isEmpty(); ++p) {
            QToolButton* bnLen = new QToolButton();
            QIcon icon;
            icon.addFile(p->_icon, QSize(), QIcon::Normal, QIcon::Off);
            bnLen->setIcon(icon);
            bnLen->setCheckable(true);
            int length = p->_measureFrac;
            connect(bnLen, &QToolButton::clicked, this, [=](){this->setEditNoteLength(length);});

            if (p->_selected)
                  bnLen->setChecked(true);
            bngrpNoteLen->addButton(bnLen);
            tbNoteLen->addWidget(bnLen);
            }

      //----

      QToolBar* tbDots = addToolBar("Toolbar Dots");
      QButtonGroup* bngrpNoteDot = new QButtonGroup();

      struct DotIconData
      {
            QString _icon;
            int _len;
            bool _selected;
      };
      DotIconData _iconDotData[] = {
            { QStringLiteral(":/data/icons/note-dot.svg"), 1, false },
            { QStringLiteral(":/data/icons/note-double-dot.svg"), 2, false },
            { QStringLiteral(":/data/icons/note-dot3.svg"), 3, false },
            { QStringLiteral(":/data/icons/note-dot4.svg"), 4, false },
            { "", -1, false },
            };


      for (DotIconData* p = _iconDotData; p->_len != -1; ++p) {
            QToolButton* bn = new QToolButton();
            QIcon icon;
            icon.addFile(p->_icon, QSize(), QIcon::Normal, QIcon::Off);
            bn->setIcon(icon);
            bn->setCheckable(true);
            int length = p->_len;
            connect(bn, &QToolButton::clicked, this, [=](){this->setEditNoteDots(length, bn);});

            if (p->_selected)
                  bn->setChecked(true);
            bngrpNoteDot->addButton(bn);
            tbDots->addWidget(bn);
            }


      //----

      QToolBar* tbVoices = addToolBar("Toolbar Voices");
      QButtonGroup* bngrpVoices = new QButtonGroup();
      //bngrpNoteLen = new QButtonGroup();

      struct VoiceIconData
      {
            QString _icon;
            int _voice;
            QString _tooltip;
            bool _selected;
      };
      VoiceIconData _iconDataVoice[] = {
            { QStringLiteral(":/data/icons/voice-1.svg"), 0, tr("Voice 1"), true },
            { QStringLiteral(":/data/icons/voice-2.svg"), 1, tr("Voice 2"), false },
            { QStringLiteral(":/data/icons/voice-3.svg"), 2, tr("Voice 3"), false },
            { QStringLiteral(":/data/icons/voice-4.svg"), 3, tr("Voice 4"), false },
            { "", -1, "", false },
            };

      for (VoiceIconData* p = _iconDataVoice; p->_voice != -1; ++p) {
            QToolButton* bn = new QToolButton();
            QIcon icon;
            icon.addFile(p->_icon, QSize(), QIcon::Normal, QIcon::Off);
            bn->setIcon(icon);
            bn->setCheckable(true);
            bn->setToolTip(p->_tooltip);
            int voice = p->_voice;
            connect(bn, &QToolButton::clicked, this, [=](){this->setEditNoteVoice(voice);});

            if (p->_selected)
                  bn->setChecked(true);
            bngrpVoices->addButton(bn);
            tbVoices->addWidget(bn);
            }

      // --------------------------------------------------
      // empty area for spacing

      addToolBarBreak();
      QToolBar* tbTweak = addToolBar("Toolbar Tweak");

      tbTweak->addWidget(new QLabel(tr("Cursor:")));
      pos = new Awl::PosLabel;
      pos->setFrameStyle(QFrame::NoFrame | QFrame::Plain);

      tbTweak->addWidget(pos);
      Awl::PitchLabel* pl = new Awl::PitchLabel();
      pl->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      tbTweak->addWidget(pl);

      tbTweak->addSeparator();

      tbTweak->addWidget(new QLabel(tr("Subdiv.:")));
      subdiv = new QSpinBox;
      subdiv->setToolTip(tr("Subdivide the beat this many times"));
      subdiv->setMinimum(0);
      subdiv->setValue(0);
      tbTweak->addWidget(subdiv);

      tbTweak->addWidget(new QLabel(tr("Tuplet:")));
      tuplet = new QSpinBox;
      tuplet->setToolTip(tr("Edit notes aligned to tuplets of this many beats"));
      tuplet->setMinimum(1);
      tuplet->setValue(1);
      tbTweak->addWidget(tuplet);

      tbTweak->addWidget(new QLabel(tr("Stripe Pattern:")));
      barPattern = new QComboBox;
      barPattern->setToolTip(tr("White stripes show the tones of this chord."));
      for (int i = 0; !PianoView::barPatterns[i].name.isEmpty(); ++i) {
            barPattern->addItem(qApp->translate("BarPattern", PianoView::barPatterns[i].name.toUtf8().data()), i);
            }
      tbTweak->addWidget(barPattern);

      tbTweak->addSeparator();
      tbTweak->addWidget(new QLabel(tr("Velocity:")));
      veloType = new QComboBox;
      veloType->addItem(tr("Offset"), int(Note::ValueType::OFFSET_VAL));
      veloType->addItem(tr("User"),   int (Note::ValueType::USER_VAL));
      tbTweak->addWidget(veloType);

      velocity = new QSpinBox;
      velocity->setRange(-1000, 1000);
      velocity->setReadOnly(true);
      tbTweak->addWidget(velocity);

      tbTweak->addWidget(new QLabel(tr("Pitch:")));
      pitch = new Awl::PitchEdit;
      pitch->setReadOnly(true);
      tbTweak->addWidget(pitch);

      tbTweak->addWidget(new QLabel(tr("OnTime:")));
      tbTweak->addWidget((onTime = new QSpinBox));
      onTime->setRange(-2000, 2000);

      tbTweak->addWidget(new QLabel(tr("Len:")));
      tbTweak->addWidget((tickLen = new QSpinBox));
      tickLen->setRange(-2000, 60000);


      // --------------------------------------------------
      // empty area for spacing

      QWidget* topLeftSpacer = new QWidget;
      topLeftSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      topLeftSpacer->setFixedWidth(PIANO_KEYBOARD_WIDTH);
      topLeftSpacer->setFixedHeight(pianoRulerHeight);

      ruler = new PianoRuler;
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(pianoRulerHeight);

      pianoKbd = new PianoKeyboard;
      pianoKbd->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      pianoKbd->setFixedWidth(PIANO_KEYBOARD_WIDTH);

      pianoView = new PianoView;
      pianoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      pianoView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      hsb = new QScrollBar(Qt::Horizontal);
      connect(pianoView->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)),
            SLOT(rangeChanged(int,int)));

      QWidget* noteAreaWidget = new QWidget;

      QGridLayout* noteAreaLayout = new QGridLayout;
      noteAreaLayout->setContentsMargins(0, 0, 0, 0);
      noteAreaLayout->setSpacing(0);
      noteAreaLayout->addWidget(topLeftSpacer, 0, 0, 1, 1);
      noteAreaLayout->addWidget(ruler, 0, 1, 1, 1);
      noteAreaLayout->addWidget(pianoKbd, 1, 0, 1, 1);
      noteAreaLayout->addWidget(pianoView, 1, 1, 1, 1);
      noteAreaLayout->addWidget(hsb, 2, 1, 1, 1);
      noteAreaWidget->setLayout(noteAreaLayout);

      // levels area
      pianoLevelsChooser = new PianoLevelsChooser;
      pianoLevelsChooser->setPianoView(pianoView);
      pianoLevelsChooser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      pianoLevelsChooser->setFixedWidth(PIANO_KEYBOARD_WIDTH);

      pianoLevels = new PianoLevels;
      pianoLevels->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      QWidget* levelsAreaWidget = new QWidget;
      QHBoxLayout* levelsAreaLayout = new QHBoxLayout;
      levelsAreaLayout->setContentsMargins(0, 0, 0, 0);
      levelsAreaLayout->setSpacing(0);
      levelsAreaLayout->addWidget(pianoLevelsChooser);
      levelsAreaLayout->addWidget(pianoLevels);
      levelsAreaWidget->setLayout(levelsAreaLayout);

      // layout
      QSplitter* editAreaSplitter = new QSplitter(Qt::Vertical);
      editAreaSplitter->addWidget(noteAreaWidget);
      editAreaSplitter->addWidget(levelsAreaWidget);
      editAreaSplitter->setFrameShape(QFrame::NoFrame);

      editAreaSplitter->setSizes(QList<int>({300, 100}));

      split = new QSplitter(Qt::Vertical);
      split->setFrameShape(QFrame::NoFrame);

      QGridLayout* layout = new QGridLayout;
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSpacing(0);
      layout->setColumnMinimumWidth(0, PIANO_KEYBOARD_WIDTH);
      layout->addWidget(editAreaSplitter, 1, 0, 1, 1);

      mainWidget->setLayout(layout);
      setCentralWidget(mainWidget);

      connect(pianoView->verticalScrollBar(),   SIGNAL(valueChanged(int)), pianoKbd, SLOT(setYpos(int)));
      connect(pianoView->horizontalScrollBar(), SIGNAL(valueChanged(int)), hsb,      SLOT(setValue(int)));

      connect(pianoView,          SIGNAL(xZoomChanged(qreal)),            ruler,       SLOT(setXZoom(qreal)));
      connect(pianoView,          SIGNAL(xZoomChanged(qreal)),            pianoLevels, SLOT(setXZoom(qreal)));
      connect(pianoView,          SIGNAL(noteHeightChanged(int)),         pianoKbd,    SLOT(setNoteHeight(int)));
      connect(pianoView,          SIGNAL(pitchChanged(int)),              pl,          SLOT(setPitch(int)));
      connect(pianoView,          SIGNAL(pitchChanged(int)),              pianoKbd,    SLOT(setPitch(int)));
      connect(pianoKbd,           SIGNAL(pitchChanged(int)),              pl,          SLOT(setPitch(int)));
      connect(pianoView,          SIGNAL(trackingPosChanged(const Pos&)), pos,         SLOT(setValue(const Pos&)));
      connect(pianoView,          SIGNAL(trackingPosChanged(const Pos&)), ruler,       SLOT(setPos(const Pos&)));
      connect(pianoView,          SIGNAL(trackingPosChanged(const Pos&)), pianoLevels, SLOT(setPos(const Pos&)));
      connect(ruler,              SIGNAL(posChanged(const Pos&)),         pos,         SLOT(setValue(const Pos&)));
      connect(pianoLevels,        SIGNAL(posChanged(const Pos&)),         pos,         SLOT(setValue(const Pos&)));
      connect(tuplet,             SIGNAL(valueChanged(int)),              pianoView,   SLOT(setTuplet(int)));
      connect(tuplet,             SIGNAL(valueChanged(int)),              pianoLevels, SLOT(setTuplet(int)));
      connect(barPattern,         SIGNAL(activated(int)),                 pianoView,   SLOT(setBarPattern(int)));
      connect(subdiv,             SIGNAL(valueChanged(int)),              pianoView,   SLOT(setSubdiv(int)));
      connect(subdiv,             SIGNAL(valueChanged(int)),              pianoLevels, SLOT(setSubdiv(int)));
      connect(pianoLevelsChooser, SIGNAL(levelsIndexChanged(int)),        pianoLevels, SLOT(setLevelsIndex(int)));
      connect(pianoKbd,           SIGNAL(pitchHighlightToggled(int)),     pianoView,   SLOT(togglePitchHighlight(int)));

      connect(hsb,         SIGNAL(valueChanged(int)),                 SLOT(setXpos(int)));
      connect(pianoView->horizontalScrollBar(), SIGNAL(valueChanged(int)),   SLOT(setXpos(int)));

      connect(ruler,              SIGNAL(locatorMoved(int, const Pos&)), SLOT(moveLocator(int, const Pos&)));
      connect(pianoLevels,        SIGNAL(locatorMoved(int, const Pos&)), SLOT(moveLocator(int, const Pos&)));
      connect(veloType,           SIGNAL(activated(int)),                SLOT(veloTypeChanged(int)));
      connect(velocity,           SIGNAL(valueChanged(int)),             SLOT(velocityChanged(int)));
      connect(onTime,             SIGNAL(valueChanged(int)),             SLOT(onTimeChanged(int)));
      connect(tickLen,            SIGNAL(valueChanged(int)),             SLOT(tickLenChanged(int)));
      connect(pianoView,          SIGNAL(selectionChanged()),            SLOT(selectionChanged()));
      connect(pianoView,          SIGNAL(showNoteTweakerRequest()),      SLOT(showNoteTweaker()));
      connect(pianoKbd,           SIGNAL(keyPressed(int)),               SLOT(keyPressed(int)));
      connect(pianoKbd,           SIGNAL(keyReleased(int)),              SLOT(keyReleased(int)));
      connect(pianoLevels,        SIGNAL(noteLevelsChanged()),           SLOT(selectionChanged()));
      connect(noteTweakerDlg,     SIGNAL(notesChanged()),                SLOT(selectionChanged()));
      connect(pianoLevelsChooser, SIGNAL(notesChanged()),                SLOT(selectionChanged()));

      readSettings();

      actions.append(getAction("tie"));
      actions.append(getAction("play"));
      actions.append(getAction("delete"));
      actions.append(getAction("pitch-up"));
      actions.append(getAction("pitch-down"));
      actions.append(getAction("pitch-up-octave"));
      actions.append(getAction("pitch-down-octave"));

//      QMenu* popup = new QMenu(this);
//      popup->setSeparatorsCollapsible(false);
//      QAction* a = popup->addSeparator();
//      popup->addAction(getAction("cut"));
//      popup->addAction(getAction("copy"));
//      popup->addAction(getAction("paste"));
//      popup->addAction(getAction("swap"));
//      popup->addAction(getAction("delete"));

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
//   setEditNoteLength
//---------------------------------------------------------

void PianorollEditor::setEditNoteLength(int len)
      {
            pianoView->setEditNoteLength(Fraction::fromTicks(pow(2, len + 2) * MScore::division));
      }

//---------------------------------------------------------
//   setEditNoteVoice
//---------------------------------------------------------

void PianorollEditor:: setEditNoteVoice(int voice)
      {
            pianoView->setEditNoteVoice(voice);
      }

//---------------------------------------------------------
//   setEditNoteDots
//---------------------------------------------------------

void PianorollEditor::setEditNoteDots(int value, QToolButton* bn)
      {
      if (pianoView->editNoteDots() == value) {
            bn->group()->setExclusive(false);
            bn->setChecked(false);
            bn->group()->setExclusive(true);
            pianoView->setEditNoteDots(0);
            }
      else
            pianoView->setEditNoteDots(value);
      }

//---------------------------------------------------------
//   setEditNoteTool
//---------------------------------------------------------

void PianorollEditor::setEditNoteTool(PianoRollEditTool value)
      {
      pianoView->setEditNoteTool(value);
      }

//---------------------------------------------------------
//   handleAction
//---------------------------------------------------------

void PianorollEditor::handleAction(QAction* a)
      {
      QString cmd(a->data().toString());

      if (cmd == "zoom-in-horiz-pre")
            zoom(1, true);
      else if (cmd == "zoom-out-horiz-pre")
            zoom(-1, true);
      else if (cmd == "zoom-in-vert-pre")
            zoom(1, false);
      else if (cmd == "zoom-out-vert-pre")
            zoom(-1, false);
      }


//---------------------------------------------------------
//   showNoteTweaker
//---------------------------------------------------------

void PianorollEditor::showNoteTweaker()
      {
      noteTweakerDlg->show();
      }

//---------------------------------------------------------
//   focusOnPosition
//---------------------------------------------------------

void PianorollEditor::focusOnPosition(Position* p)
      {
      if (!p || !p->segment)
            return;

      // move view so that view is centered on this element
      pianoView->ensureVisible(p->segment->tick().ticks());
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianorollEditor::setStaff(Staff* st)
      {
      if (staff == st)
            return;

      if (st)
            partLabel->setText(tr("Part: %1").arg(st->partName()));

      if ((st && st->score() != _score) || (!st && _score)) {
            if (_score) {
                  _score->removeViewer(this);
                  disconnect(_score, SIGNAL(posChanged(POS,unsigned)), this, SLOT(posChanged(POS,unsigned)));
                  disconnect(_score, SIGNAL(playlistChanged()), this, SLOT(playlistChanged()));
                  }
            _score = st ? st->score() : nullptr;
            if (_score) {
                  _score->addViewer(this);
                  setLocator(POS::CURRENT, _score->pos(POS::CURRENT).ticks());
                  setLocator(POS::LEFT,    _score->pos(POS::LEFT).ticks());
                  setLocator(POS::RIGHT,   _score->pos(POS::RIGHT).ticks());
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
      else
            setWindowTitle(tr("Piano roll editor"));
      ruler->setScore(_score, locator);
      pianoView->setStaff(staff, locator);
      pianoLevels->setScore(_score, locator);
      pianoLevels->setStaff(staff, locator);
      pianoLevelsChooser->setStaff(staff);
      pianoKbd->setStaff(staff);
      noteTweakerDlg->setStaff(staff);

      updateSelection();
      setEnabled(st);
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
      resize(QSize(800, 600)); // ensure default size if no geometry in settings
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void PianorollEditor::setXpos(int x)
      {
      pianoView->horizontalScrollBar()->setValue(x);
      ruler->setXpos(x);
      pianoLevels->setXpos(x);
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
      QList<PianoItem*> items = pianoView->getSelectedItems();
      bool enabled = false;

      if (items.size() == 1) {
            PianoItem* item = items[0];
            Note* note = item->note();

            pitch->setValue(note->pitch());

            NoteEvent* event = item->getTweakNoteEvent();
            if (event) {
                  onTime->setValue(event->ontime());
                  tickLen->setValue(event->len());
                  }

            updateVelocity(note);
            }

      // if all selected notes don't have the same veloType,
      // velocity field should be disabled
      bool sameVeloType = true;
      if (items.size()) {
            enabled = true;

            Note::ValueType vt = items[0]->note()->veloType();
            for (int i = 1; i < items.size(); i++) {
                  if (items[i]->note()->veloType() != vt) {
                        sameVeloType = false;
                        break;
                        }
                  }
            }

      velocity->setEnabled(enabled && sameVeloType);
      pitch->setEnabled(enabled);
      veloType->setEnabled(enabled);
      onTime->setEnabled(enabled);
      tickLen->setEnabled(enabled);
      pianoLevelsChooser->updateSetboxValue();
      }

//---------------------------------------------------------
//   selectionChanged
//    called if selection in PianoView changed
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      QList<PianoItem*> items = pianoView->getSelectedItems();
      if (items.size() == 1) {
            Note* note = items[0]->note();
            _score->select(note, SelectType::SINGLE, 0);
            }
      else if (items.size() == 0)
            _score->select(0, SelectType::SINGLE, 0);
      else {
            _score->deselectAll();
            for (PianoItem* item : items) {
                  Note* note = item->note();
                  if (!note->selected())
                        _score->select(note, SelectType::ADD, 0);
                  }
            }
      for (MuseScoreView* view : score()->getViewer())
            view->updateAll();

      pianoView->scene()->update();
      pianoLevels->update();
      updateSelection();
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void PianorollEditor::changeSelection(SelState)
      {
      }

//---------------------------------------------------------
//   veloTypeChanged
//---------------------------------------------------------

void PianorollEditor::veloTypeChanged(int val)
      {
      QList<PianoItem*> items = pianoView->getSelectedItems();
      if (!items.size())
            return;

      _score->startCmd();
      for (int i = 0; i < items.size(); i++) {
            PianoItem* item = items[i];
            Note* note = item->note();
            if (Note::ValueType(val) == note->veloType())
                  return;

            int newVelocity = note->veloOffset();
            int dynamicsVel = staff->velocities().val(note->tick());

            // change velocity to equivalent in new metric
            switch (Note::ValueType(val)) {
                  case Note::ValueType::USER_VAL:
                        newVelocity = static_cast<int>(dynamicsVel * (1 + newVelocity / 100.0));
                        break;
                  case Note::ValueType::OFFSET_VAL:
                        newVelocity = static_cast<int>((newVelocity / (qreal)dynamicsVel - 1) * 100);
                        break;
                  }
            
            _score->undo(new ChangeVelocity(note, Note::ValueType(val), newVelocity));
            updateVelocity(note);
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void PianorollEditor::updateVelocity(Note* note)
      {
      Note::ValueType vt = note->veloType();
      veloType->setCurrentIndex(int(vt));
      switch (vt) {
            case Note::ValueType::USER_VAL:
                  velocity->setReadOnly(false);
                  velocity->setSuffix("");
                  break;
            case Note::ValueType::OFFSET_VAL:
                  velocity->setReadOnly(false);
                  velocity->setSuffix("%");
                  break;
            }

      switch (vt) {
            case Note::ValueType::USER_VAL:
                  velocity->setValue(note->veloOffset());
                  break;
            case Note::ValueType::OFFSET_VAL:
                  velocity->setValue(note->veloOffset());
                  break;
            }

      pianoLevels->update();
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void PianorollEditor::velocityChanged(int val)
      {
      QList<PianoItem*> items = pianoView->getSelectedItems();
      if (!items.size())
            return;

      _score->startCmd();
      for (int i = 0; i < items.size(); i++) {
            PianoItem* item = items[i];
            Note* note = item->note();
            Note::ValueType vt = note->veloType();

            if (val == note->veloOffset())
                  return;

            _score->undo(new ChangeVelocity(note, vt, val));
            }
      _score->endCmd();

      pianoLevels->update();
      }

//---------------------------------------------------------
//   keyPressed
//---------------------------------------------------------

void PianorollEditor::keyPressed(int p)
      {
      seq->startNote(staff->part()->instrument()->channel(0)->channel(), p, 80, 0, 0.0);
      }

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void PianorollEditor::keyReleased(int /*p*/)
      {
      seq->stopNotes();
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PianorollEditor::heartBeat(Seq* s)
      {
      unsigned tick = s->getCurTick();
      if (score()->masterScore())
            tick = score()->masterScore()->repeatList().utick2tick(tick);
      if (locator[0].tick() != tick) {
            posChanged(POS::CURRENT, tick);
            if (preferences.getBool(PREF_APP_PLAYBACK_FOLLOWSONG))
                  pianoView->ensureVisible(tick);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianorollEditor::moveLocator(int i, const Pos& p)
      {
      if (locator[i].valid())
            score()->setPos(POS(i), Fraction::fromTicks(p.tick()));
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void PianorollEditor::cmd(QAction* /*a*/)
      {
      //score()->startCmd();
      pianoView->setStaff(staff, locator);
      pianoLevels->setStaff(staff, locator);
      pianoLevelsChooser->setStaff(staff);
      pianoKbd->setStaff(staff);
      //score()->endCmd();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void PianorollEditor::dataChanged(const QRectF&)
      {
      }

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

void PianorollEditor::removeScore()
      {
      _score = nullptr;
      setStaff(nullptr);
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
//   matrix
//---------------------------------------------------------

const QTransform& PianorollEditor::matrix() const
      {
      static QTransform t;
      return t;
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
      if (updateScheduled)
            return;

      QTimer::singleShot(0, this, &PianorollEditor::doUpdate);
      updateScheduled = true;
      }

//---------------------------------------------------------
//   doUpdate
//---------------------------------------------------------

void PianorollEditor::doUpdate()
      {
      updateScheduled = false;

      if (staff && staff->idx() == -1) { // staff removed
            removeScore();
            return;
            }
      pianoView->updateNotes();
      pianoLevels->updateNotes();
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
                  connect(pianoView, SIGNAL(magChanged(double,double)), waveView, SLOT(setMag(double,double)));
                  connect(pianoView, SIGNAL(posChanged(const Pos&)), waveView,   SLOT(setValue(const Pos&)));
                  waveView->setAudio(_score->audio());
                  waveView->setScore(_score, locator);
                  split->addWidget(waveView);
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
      pianoView->moveLocator(int(p));
      if (waveView)
            waveView->moveLocator(int(p));
      ruler->update();
      pianoLevels->update();
      }

//---------------------------------------------------------
//   onTimeChanged
//---------------------------------------------------------

void PianorollEditor::onTimeChanged(int val)
      {
      QList<PianoItem*> items = pianoView->getSelectedItems();
      if (!items.size())
            return;

      _score->startCmd();
      for (int i = 0; i < items.size(); i++) {
            PianoItem* item = items[i];
            Note* note = item->note();
            NoteEvent* event = item->getTweakNoteEvent();
            if (!event || event->ontime() == val)
                  return;

            NoteEvent ne = *event;
            ne.setOntime(val);
            
            _score->undo(new ChangeNoteEvent(note, event, ne));
            }
      _score->endCmd();

      pianoView->updateNotes();
      pianoLevels->updateNotes();
      }

//---------------------------------------------------------
//   tickLenChanged
//---------------------------------------------------------

void PianorollEditor::tickLenChanged(int val)
      {
      QList<PianoItem*> items = pianoView->getSelectedItems();
      if (!items.size())
            return;

      _score->startCmd();
      for (int i = 0; i < items.size(); i++) {
            PianoItem* item = items[i];
            Note* note = item->note();
            NoteEvent* event = item->getTweakNoteEvent();
            if (!event || event->len() == val)
                  return;

            NoteEvent ne = *event;
            ne.setLen(val);
            
            _score->undo(new ChangeNoteEvent(note, event, ne));
            }
      _score->endCmd();

      pianoView->updateNotes();
      pianoLevels->updateNotes();
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void PianorollEditor::zoom(int amount, bool horiz)
      {
      int cx = pianoView->width() / 2;
      int cy = pianoView->height() / 2;

      pianoView->zoomView(amount, horiz, cx, cy);
      }

}
