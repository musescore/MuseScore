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

#ifndef __CHORDEDITOR_H__
#define __CHORDEDITOR_H__

#include "libmscore/noteevent.h"

class ChordView;
class Chord;

//---------------------------------------------------------
//   ChordEditor
//---------------------------------------------------------

class ChordEditor : public QDialog {
      Q_OBJECT

      ChordView* chordView;
      Chord* _chord;
      QDialogButtonBox* bb;
      QList<NoteEventList> events;
      bool dirty;

   private slots:
      void clicked(QAbstractButton*);
      void accept();
      void reject();

   public slots:

   public:
      ChordEditor(Note*, QWidget* parent = 0);
      ~ChordEditor();
      };

#endif


