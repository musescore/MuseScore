//=============================================================================
//  MuseScore
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

#include "drumtools.h"
#include "musescore.h"
#include "palette.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/drumset.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "seq.h"
#include "editdrumset.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/stem.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   DrumTools
//---------------------------------------------------------

DrumTools::DrumTools(QWidget* parent)
   : QDockWidget(parent)
      {
      drumset = 0;
      _score  = 0;
      setObjectName("drum-tools");
      setWindowTitle(tr("Drum Tools"));
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));

      QWidget* w = new QWidget(this);
      w->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
      w->setMaximumHeight(100);
      QHBoxLayout* layout = new QHBoxLayout;
      w->setLayout(layout);

      QVBoxLayout* layout1 = new QVBoxLayout;
      layout1->setSpacing(6);
      pitchName = new QLabel;
      pitchName->setAlignment(Qt::AlignCenter);
      pitchName->setWordWrap(true);
      pitchName->setContentsMargins(25, 10, 25, 10);
      layout1->addWidget(pitchName);
      QHBoxLayout* buttonLayout = new QHBoxLayout;
      buttonLayout->setContentsMargins(25, 10, 25, 10);
      QToolButton* tb = new QToolButton;
      tb->setText(tr("Edit Drumset"));
      tb->setMinimumWidth(100);
      tb->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
      buttonLayout->addWidget(tb);
      layout1->addLayout(buttonLayout);
      layout->addLayout(layout1);

      drumPalette = new Palette;
      drumPalette->setName(tr("Drums"));
      drumPalette->setMag(0.8);
      drumPalette->setSelectable(true);
      drumPalette->setGrid(28, 60);
      PaletteScrollArea* sa = new PaletteScrollArea(drumPalette);
      sa->setFocusPolicy(Qt::NoFocus);
      layout->addWidget(sa);

      setWidget(w);

      w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();
      connect(tb, SIGNAL(clicked()), SLOT(editDrumset()));
      void boxClicked(int);
      connect(drumPalette, SIGNAL(boxClicked(int)), SLOT(drumNoteSelected(int)));

      drumPalette->setContextMenuPolicy(Qt::PreventContextMenu);
      }

//---------------------------------------------------------
//   updateDrumset
//---------------------------------------------------------

void DrumTools::updateDrumset(const Drumset* ds)
      {
      drumPalette->clear();
      drumset = ds;
      if (!drumset)
            return;
      double _spatium = gscore->spatium();
      const auto d = ds->drumsByIndex();
      for (int n = 0; n < DRUM_INSTRUMENTS; ++n) {
            DrumInstrument di = d[n];
            if (!drumset->isValid(di.pitch))
                  continue;
            bool up;
            int line      = di.line;
            NoteHead::Group noteHead  = di.notehead;
            int voice     = di.voice;
            MScore::Direction dir = di.stemDirection;
            if (dir == MScore::Direction::UP)
                  up = true;
            else if (dir == MScore::Direction::DOWN)
                  up = false;
            else
                  up = line > 4;

            Chord* chord = new Chord(gscore);
            chord->setDurationType(TDuration::DurationType::V_QUARTER);
            chord->setStemDirection(dir);
            chord->setUp(up);
            chord->setTrack(voice);
            Note* note = new Note(gscore);
            note->setParent(chord);
            note->setTrack(voice);
            note->setPitch(di.pitch);
            note->setTpcFromPitch();
            note->setLine(line);
            note->setPos(0.0, _spatium * .5 * line);
            note->setHeadGroup(noteHead);
            SymId noteheadSym = SymId::noteheadBlack;
            if (noteHead == NoteHead::Group::HEAD_CUSTOM)
                  noteheadSym = drumset->noteHeads(di.pitch, NoteHead::Type::HEAD_QUARTER);
            else
                  noteheadSym = note->noteHead(true, di.notehead, NoteHead::Type::HEAD_QUARTER);

            note->setCachedNoteheadSym(noteheadSym); // we use the cached notehead so we don't recompute it at each layout
            chord->add(note);
            Stem* stem = new Stem(gscore);
            stem->setLen((up ? -3.0 : 3.0) * _spatium);
            chord->add(stem);
            stem->setPos(chord->stemPos());
            int sc = di.shortcut;
            QString shortcut;
            if (sc)
                  shortcut = QChar(sc);
            drumPalette->append(chord, qApp->translate("drumset", drumset->name(di.pitch).toUtf8().data()), shortcut);
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void DrumTools::setDrumset(Score* s, Staff* st, const Drumset* ds)
      {
      if (s == _score && staff == st && drumset == ds)
            return;
      _score  = s;
      staff   = st;
      //drumset = ds;
      updateDrumset(ds);
      }

//---------------------------------------------------------
//   editDrumset
//---------------------------------------------------------

void DrumTools::editDrumset()
      {
      EditDrumset eds(drumset, this);
      if (eds.exec()) {
            _score->startCmd();
            _score->undo(new ChangeDrumset(staff->part()->instrument(), eds.drumset()));
            mscore->updateDrumTools(eds.drumset());
            if (_score->undo()->active()) {
                  _score->setLayoutAll(true);
                  _score->endCmd();
                  }
            }
      }

//---------------------------------------------------------
//   drumNoteSelected
//---------------------------------------------------------

void DrumTools::drumNoteSelected(int val)
      {
      Element* element = drumPalette->element(val);
      if (element && element->type() == Element::Type::CHORD) {
            Chord* ch        = static_cast<Chord*>(element);
            Note* note       = ch->downNote();
            int ticks        = MScore::defaultPlayDuration;
            int pitch        = note->pitch();
            seq->startNote(staff->part()->instrument()->channel(0)->channel, pitch, 80, ticks, 0.0);

            int track = (_score->inputState().track() / VOICES) * VOICES + element->track();
            _score->inputState().setTrack(track);
            _score->inputState().setDrumNote(pitch);

            getAction("voice-1")->setChecked(element->voice() == 0);
            getAction("voice-2")->setChecked(element->voice() == 1);
            getAction("voice-3")->setChecked(element->voice() == 2);
            getAction("voice-4")->setChecked(element->voice() == 3);

            pitchName->setText(drumPalette->getCellName(val));
            }
      }

int DrumTools::selectedDrumNote()
      {
      int idx = drumPalette->getSelectedIdx();
      if (idx < 0)
            return -1;
      Element* element = drumPalette->element(idx);
      if (element && element->type() == Element::Type::CHORD) {
            Chord* ch  = static_cast<Chord*>(element);
            Note* note = ch->downNote();
            pitchName->setText(drumPalette->getCellName(idx));
            return note->pitch();
            }
      else {
            return -1;
            }
      }

}

