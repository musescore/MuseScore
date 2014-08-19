//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editstringdata.cpp 5392 2012-02-28 11:41:52Z miwarre $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "editstringdata.h"
#include "editpitch.h"

namespace Ms {

//---------------------------------------------------------
//   EditStringData
//    To edit the string data (tuning and number of frets) for an instrument
//---------------------------------------------------------

EditStringData::EditStringData(QWidget *parent, QList<instrString> * strings, int * frets)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      _strings = strings;
      QStringList hdrLabels;
      int         numOfStrings = _strings->size();
      hdrLabels << tr("Open", "string data") << tr("Pitch", "string data");
      stringList->setHorizontalHeaderLabels(hdrLabels);
      stringList->setRowCount(numOfStrings);
      // if any string, insert into string list control and select the first one

      if(numOfStrings > 0) {
            int   i;
            instrString strg;
            // insert into local working copy and into string list dlg control
            // IN REVERSED ORDER
            for(i=0; i < numOfStrings; i++) {
                  strg = (*_strings)[numOfStrings - i - 1];
                  _stringsLoc.append(strg);
                  QTableWidgetItem *newCheck = new QTableWidgetItem();
                  newCheck->setFlags(Qt::ItemFlag(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
                  newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);
                  stringList->setItem(i, 0, newCheck);
                  QTableWidgetItem *newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
                  stringList->setItem(i, 1, newPitch);
                  }
            stringList->setCurrentCell(0, 1);
            }
      // if no string yet, disable buttons acting on individual string
      else {
            editString->setEnabled(false);
            deleteString->setEnabled(false);
            }

      _frets = frets;
      numOfFrets->setValue(*_frets);

      connect(deleteString, SIGNAL(clicked()), SLOT(deleteStringClicked()));
      connect(editString,   SIGNAL(clicked()), SLOT(editStringClicked()));
      connect(newString,    SIGNAL(clicked()), SLOT(newStringClicked()));
      connect(stringList,   SIGNAL(doubleClicked(QModelIndex)),     SLOT(editStringClicked()));
      connect(stringList,   SIGNAL(itemClicked(QTableWidgetItem*)), SLOT(listItemClicked(QTableWidgetItem *)));
      _modified = false;
      }

EditStringData::~EditStringData()
{
}

//---------------------------------------------------------
//   deleteStringClicked
//---------------------------------------------------------

void EditStringData::deleteStringClicked()
      {
      int         i = stringList->currentRow();

      // remove item from local string list and from dlg list control
      _stringsLoc.removeAt(i);
      stringList->model()->removeRow(i);
      // if no more items, disable buttons acting on individual string
      if (stringList->rowCount() == 0) {
            editString->setEnabled(false);
            deleteString->setEnabled(false);
            }
      _modified = true;
      }

//---------------------------------------------------------
//   editStringClicked
//---------------------------------------------------------

void EditStringData::editStringClicked()
      {
      int         i = stringList->currentRow();
      int         newCode;

      EditPitch* ep = new EditPitch(this, _stringsLoc[i].pitch);
      if ( (newCode=ep->exec()) != -1) {
            // update item value in local string list and item text in dlg list control
            _stringsLoc[i].pitch = newCode;
            QTableWidgetItem * item = stringList->item(i, 1);
            item->setText(midiCodeToStr(newCode));
            _modified = true;
            }
      }

//---------------------------------------------------------
//   listItemClicked
//---------------------------------------------------------

void EditStringData::listItemClicked(QTableWidgetItem * item)
      {
      int col = item->column();
      if (col != 0)                 // ignore clicks not on check boxes
            return;
      int row = item->row();

      // flip openness in local string list, then sync dlg list ctrl
      bool open = !_stringsLoc[row].open;
      _stringsLoc[row].open = open;
      stringList->item(row, col)->setCheckState(open ? Qt::Checked : Qt::Unchecked);
      _modified = true;
      }

//---------------------------------------------------------
//   newStringClicked
//---------------------------------------------------------

void EditStringData::newStringClicked()
      {
      int         i, newCode;

      EditPitch* ep = new EditPitch(this);
      if ( (newCode=ep->exec()) != -1) {
            // add below selected string or at the end if no selected string
            i = stringList->currentRow() + 1;
            if(i <= 0)
                  i = stringList->rowCount();
            // insert in local string list and in dlg list control
            instrString strg = {newCode, 0};
            _stringsLoc.insert(i, strg);
            stringList->insertRow(i);
            QTableWidgetItem *newCheck = new QTableWidgetItem();
            newCheck->setFlags(Qt::ItemFlag(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
            newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);
            stringList->setItem(i, 0, newCheck);
            QTableWidgetItem *newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
            stringList->setItem(i, 1, newPitch);
            // select last added item and ensure buttons are active
            stringList->setCurrentCell(i, 1);
            editString->setEnabled(true);
            deleteString->setEnabled(true);
            _modified = true;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStringData::accept()
      {
      // store data back into original variables
      // string tunings are copied in reversed order (from lowest to highest)
      if(_modified) {
            _strings->clear();
            for(int i=_stringsLoc.size()-1; i >= 0; i--)
                  _strings->append(_stringsLoc[i]);
            }
      if(*_frets != numOfFrets->value()) {
            *_frets = numOfFrets->value();
            _modified = true;
            }

      if(_modified)
            QDialog::accept();
      else
            QDialog::reject();            // if no data change, no need to trigger changes downward the caller chain
      }

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------

static const char* g_cNoteName[] = {
      QT_TRANSLATE_NOOP("editstringdata", "C"),
      QT_TRANSLATE_NOOP("editstringdata", "C#"),
      QT_TRANSLATE_NOOP("editstringdata", "D"),
      QT_TRANSLATE_NOOP("editstringdata", "Eb"),
      QT_TRANSLATE_NOOP("editstringdata", "E"),
      QT_TRANSLATE_NOOP("editstringdata", "F"),
      QT_TRANSLATE_NOOP("editstringdata", "F#"),
      QT_TRANSLATE_NOOP("editstringdata", "G"),
      QT_TRANSLATE_NOOP("editstringdata", "Ab"),
      QT_TRANSLATE_NOOP("editstringdata", "A"),
      QT_TRANSLATE_NOOP("editstringdata", "Bb"),
      QT_TRANSLATE_NOOP("editstringdata", "B")
      };

QString EditStringData::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(qApp->translate("editstringdata", g_cNoteName[midiCode % 12])).arg(midiCode / 12 - 1);
      }
}
