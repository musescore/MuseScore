//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editpitch.cpp 3775 2010-12-17 23:55:35Z miwarre $
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

#include "editpitch.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   EditPitch
//    To select a MIDI pitch code using human-readable note names
//---------------------------------------------------------

EditPitch::EditPitch(QWidget *parent)
   : QDialog(parent)
      {
      setObjectName("EditPitchNew");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      tableWidget->setCurrentCell(tableWidget->rowCount()-1-5, 0);                    // select centre C by default
      MuseScore::restoreGeometry(this);
      }

EditPitch::EditPitch(QWidget *parent, int midiCode)
   : QDialog(parent)
      {
      setObjectName("EditPitchEdit");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      tableWidget->setCurrentCell(tableWidget->rowCount()-1-(midiCode/12), midiCode%12);
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditPitch::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }

void EditPitch::accept()
      {
      on_tableWidget_cellDoubleClicked(tableWidget->currentRow(), tableWidget->currentColumn());
      }

void EditPitch::on_tableWidget_cellDoubleClicked(int row, int col)
      {
      // topmost row contains notes for 10-th MIDI octave (numbered as '9')
      int pitch = (tableWidget->rowCount()-1 - row) * 12 + col;
      done((pitch > 127)? 127: pitch);
      }
}

