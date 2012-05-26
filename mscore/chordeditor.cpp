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

//---------------------------------------------------------
//   ChordEditor
//---------------------------------------------------------

ChordEditor::ChordEditor(Chord* c, QWidget* parent)
   : QDialog(parent)
      {
      _chord = c;
      setWindowTitle(QString("MuseScore: Chord Articulation"));

      QGridLayout* layout = new QGridLayout;
      setLayout(layout);
      layout->setSpacing(0);

      double xmag = .1;
      pianoroll   = new ChordView;
      pianoroll->scale(xmag, 1.0);
      layout->addWidget(pianoroll, 0, 0);
      pianoroll->setChord(_chord);
      pianoroll->setEvenGrid(false);      //TODO
      QDialogButtonBox* bb = new QDialogButtonBox(
         QDialogButtonBox::Ok | QDialogButtonBox::Cancel
         );
      layout->addWidget(bb, 1, 0);
      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));

      resize(800, 400);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ChordEditor::accept()
      {

      QDialog::accept();
      }

