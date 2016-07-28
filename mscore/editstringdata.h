//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef EDITSTRINGDATA_H
#define EDITSTRINGDATA_H

#include "ui_editstringdata.h"
#include "libmscore/stringdata.h"

namespace Ms {

//---------------------------------------------------------
//   EditStringData
//---------------------------------------------------------

class EditStringData : public QDialog, private Ui::EditStringDataBase {
      Q_OBJECT

      int*              _frets;
      bool              _modified;
      QList<instrString>* _strings;         // pointer to original string list
      QList<instrString>  _stringsLoc;      // local working copy of string list

      virtual void hideEvent(QHideEvent*);

   public:
      EditStringData(QWidget *parent, QList<instrString> * strings, int * frets);
      ~EditStringData();

   protected:
      QString midiCodeToStr(int midiCode);

   private slots:
      void accept();
      void deleteStringClicked();
      void editStringClicked();
      void listItemClicked(QTableWidgetItem * item);
      void newStringClicked();
      };

}

#endif // EDITSTRINGDATA_H
