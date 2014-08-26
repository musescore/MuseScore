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
      QHBoxLayout* layout = new QHBoxLayout;
      w->setLayout(layout);

      QVBoxLayout* layout1 = new QVBoxLayout;
      QToolButton* tb = new QToolButton;
      tb->setText(tr("Edit Drumset"));
      layout1->addWidget(tb);
      layout1->addStretch();
      layout->addLayout(layout1);

      drumPalette = new Palette;
      drumPalette->setName(tr("Drums"));
      drumPalette->setMag(0.8);
      drumPalette->setSelectable(true);
      drumPalette->setGrid(28, 60);
      PaletteScrollArea* sa = new PaletteScrollArea(drumPalette);
      layout->addWidget(sa);

      setWidget(w);
//      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

      w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();
      connect(tb, SIGNAL(clicked()), SLOT(editDrumset()));
      void boxClicked(int);
      connect(drumPalette, SIGNAL(boxClicked(int)), SLOT(drumNoteSelected(int)));
      }

//---------------------------------------------------------
//   updateDrumset
//---------------------------------------------------------

void DrumTools::updateDrumset()
      {
      drumPalette->clear();
      if (drumset == 0)
            return;
      int drumInstruments = 0;
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (drumset->isValid(pitch))
                  ++drumInstruments;
            }
      int i = 0;
      double _spatium = gscore->spatium();
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (!drumset->isValid(pitch))
                  continue;
            bool up;
            int line      = drumset->line(pitch);
            NoteHead::Group noteHead  = drumset->noteHead(pitch);
            int voice     = drumset->voice(pitch);
            MScore::Direction dir = drumset->stemDirection(pitch);
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
            note->setPitch(pitch);
            note->setTpcFromPitch();
            note->setLine(line);
            note->setPos(0.0, _spatium * .5 * line);
            note->setHeadGroup(noteHead);
            chord->add(note);
            Stem* stem = new Stem(gscore);
            stem->setLen((up ? -3.0 : 3.0) * _spatium);
            chord->setStem(stem);
            stem->setPos(chord->stemPos());
            int sc = drumset->shortcut(pitch);
            QString shortcut;
            if (sc)
                  shortcut = QChar(sc);
            drumPalette->append(chord, qApp->translate("drumset", drumset->name(pitch).toUtf8().constData()), shortcut);
            ++i;
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void DrumTools::setDrumset(Score* s, Staff* st, Drumset* ds)
      {
      if (s == _score && staff == st && drumset == ds)
            return;
      _score  = s;
      staff   = st;
      drumset = ds;
      updateDrumset();
      }

//---------------------------------------------------------
//   editDrumset
//---------------------------------------------------------

void DrumTools::editDrumset()
      {
      EditDrumset eds(drumset, this);
      eds.exec();
      }

//---------------------------------------------------------
//   drumNoteSelected
//---------------------------------------------------------

void DrumTools::drumNoteSelected(int val)
      {
      Element* element = drumPalette->element(val);
      if(element && element->type() == Element::Type::CHORD) {
            Chord* ch        = static_cast<Chord*>(element);
            Note* note       = ch->downNote();
            int ticks        = MScore::defaultPlayDuration;
            int pitch        = note->pitch();
            seq->startNote(staff->part()->instr()->channel(0).channel, pitch, 80, ticks, 0.0);

            _score->inputState().setTrack(element->track());
            _score->inputState().setDrumNote(pitch);

            getAction("voice-1")->setChecked(element->voice() == 0);
            getAction("voice-2")->setChecked(element->voice() == 1);
            getAction("voice-3")->setChecked(element->voice() == 2);
            getAction("voice-4")->setChecked(element->voice() == 3);
            }
      }
}

