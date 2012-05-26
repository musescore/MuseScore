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

//---------------------------------------------------------
//   EditStringData
//    To edit the string data (tuning and number of frets) for an instrument
//---------------------------------------------------------

EditStringData::EditStringData(QWidget *parent, QList<int> * strings, int * frets)
   : QDialog(parent)
      {
      setupUi(this);
      _strings = strings;
      // if any string, insert into string list control and select the first one
      if(_strings->size()) {
            int   i, j;
            // insert into local working copy sorting by decreasing MIDI code value
/*            foreach(i, *_strings) {
                  for(j=_stringsLoc.size()-1; j >= 0 && i > _stringsLoc[j]; j--)
                        ;
                  _stringsLoc.insert(j+1, i);
                  }
            // add to string list dlg control
            foreach(i, _stringsLoc)
                  stringList->addItem(midiCodeToStr(i));
            stringList->setCurrentRow(0);
*/
            // insert into local working copy and into string list dlg control
            // IN REVERSED ORDER
            for(i=_strings->size()-1; i >= 0; i--) {
                  j = (*_strings)[i];
                  _stringsLoc.append(j);
                  stringList->addItem(midiCodeToStr(j));
                  }
            stringList->setCurrentRow(0);
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
      connect(stringList,   SIGNAL(doubleClicked(QModelIndex)), SLOT(editStringClicked()));
      _modified = false;
      }

EditStringData::~EditStringData()
{
//    delete ui;
}
/*
void EditStringData::changeEvent(QEvent *e)
{
      QDialog::changeEvent(e);
      switch (e->type()) {
            case QEvent::LanguageChange:
                  retranslateUi(this);
                  break;
            default:
                  break;
      }
}
*/
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
      if (stringList->count() == 0) {
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

      EditPitch* ep = new EditPitch(this, _stringsLoc[i]);
      if ( (newCode=ep->exec()) != -1) {
            // update item value in local string list and item text in dlg list control
            _stringsLoc[i] = newCode;
            QListWidgetItem * item = stringList->item(i);
            item->setText(midiCodeToStr(newCode));
            _modified = true;
            }
      }

//---------------------------------------------------------
//   newStringClicked
//---------------------------------------------------------

void EditStringData::newStringClicked()
      {
      int         i, newCode;

      EditPitch* ep = new EditPitch(this);
      if ( (newCode=ep->exec()) != -1) {
            // find sorted postion for new string tuning value
/*            for(i=_stringsLoc.size()-1; i >= 0 && newCode > _stringsLoc[i]; i--)
                  ;
            // insert in local string list and in dlg list control
            _stringsLoc.insert(i+1, newCode);
            stringList->insertItem(i+1, midiCodeToStr(newCode));
            // select last added item and ensure buttons are active
            stringList->setCurrentRow(i+1);
*/
            // add below selected string o at the end if no selected string
            i = stringList->currentRow();
            if(i < 0)
                  i = stringList->count() - 1;
            // insert in local string list and in dlg list control
            _stringsLoc.insert(i+1, newCode);
            stringList->insertItem(i+1, midiCodeToStr(newCode));
            // select last added item and ensure buttons are active
            stringList->setCurrentRow(i+1);
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

static char g_cNoteName[12][4] =
{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

QString EditStringData::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(g_cNoteName[midiCode % 12]).arg(midiCode / 12 - 1);
      }
