//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "config.h"
#include "chordeditor.h"
#include "piano.h"
#include "chordview.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "voiceselector.h"
#include "libmscore/note.h"
#include "musescore.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"
#include "seq.h"
#include "preferences.h"
#include "seq.h"
#include "libmscore/chord.h"
#include "libmscore/noteevent.h"

//---------------------------------------------------------
//   ChordEditor
//---------------------------------------------------------

ChordEditor::ChordEditor(Note* c, QWidget* parent)
   : QDialog(parent)
      {
      _note = c;
      setWindowTitle(QString("MuseScore: Chord Articulation"));

      // save original events
      int notes = _chord->notes().size();
      for (int i = 0; i < notes; ++i)
            events.append(_chord->notes()[i]->playEvents());

      QGridLayout* layout = new QGridLayout;
      setLayout(layout);
      layout->setSpacing(0);

      double xmag = .1;
      chordView   = new ChordView;
      chordView->scale(xmag, 1.0);
      layout->addWidget(chordView, 0, 0);
      chordView->setChord(_chord);
      chordView->setEvenGrid(false);      //TODO
      bb = new QDialogButtonBox(
         QDialogButtonBox::Reset | QDialogButtonBox::Ok | QDialogButtonBox::Cancel
         );
      layout->addWidget(bb, 1, 0);
      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      connect(bb, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));
      resize(800, 400);
      dirty = false;
      }

//---------------------------------------------------------
//   clicked - Reset
//---------------------------------------------------------

void ChordEditor::clicked(QAbstractButton* b)
      {
      if (bb->standardButton(b) == QDialogButtonBox::Reset) {
            int notes = _chord->notes().size();
            for (int i = 0; i < notes; ++i) {
                  Note* note = _chord->notes()[i];
                  note->setPlayEvents(events[i]);
                  }
            chordView->setChord(_chord);
            }
      }

//---------------------------------------------------------
//   ChordEditor
//---------------------------------------------------------

ChordEditor::~ChordEditor()
      {
      delete[] events;
      }

//---------------------------------------------------------
//   clicked - Reset
//---------------------------------------------------------

void ChordEditor::clicked(QAbstractButton* b)
      {
      if (bb->standardButton(b) == QDialogButtonBox::Reset) {
            Chord* chord = _note->chord();
            for (int i = 0; i < notes; ++i) {
                  Note* note = chord->notes()[i];
                  note->setPlayEvents(events[i]);
                  }
            pianoroll->setChord(chord);
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ChordEditor::accept()
      {
      QList<NoteEventList> nevents;

      if (chordView->dirty() || dirty) {
            int notes = _chord->notes().size();
            for (int i = 0; i < notes; ++i)
                  nevents.append(_chord->notes()[i]->playEvents());
            for (int i = 0; i < notes; ++i)
                  _chord->notes()[i]->setPlayEvents(events[i]);
            _chord->score()->undo(new ChangeEventList(_chord, nevents, chordView->dirty()));
            }
      QDialog::accept();
      }

//---------------------------------------------------------
//   reject
//---------------------------------------------------------

void ChordEditor::reject()
      {
      int notes = _chord->notes().size();
      for (int i = 0; i < notes; ++i) {
            Note* note = _chord->notes()[i];
            note->setPlayEvents(events[i]);
            }
      QDialog::reject();
      }

