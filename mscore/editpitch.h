//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef EDITPITCH_H
#define EDITPITCH_H

#include "ui_editpitch.h"

namespace Ms {

//---------------------------------------------------------
//   EditPitch
//---------------------------------------------------------

class EditPitch : public QDialog, private Ui::EditPitchBase {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);

   private slots:
      void on_tableWidget_cellDoubleClicked(int row, int column);
      void accept();
      void reject() { done(-1); }                             // return an invalid pitch MIDI code

   public:
      EditPitch(QWidget *parent);
      EditPitch(QWidget * parent, int midiCode);
      ~EditPitch() {}
      };

}

#endif // EDITPITCH_H
