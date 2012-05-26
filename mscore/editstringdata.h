//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editstringdata.h 3775 2010-12-17 23:55:35Z miwarre $
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

//#include <QDialog>
#include "ui_editstringdata.h"
/*
namespace Ui {
    class EditStringData;
}
*/
class EditStringData : public QDialog, private Ui::EditStringDataBase {
      Q_OBJECT

      int *             _frets;
      bool              _modified;
      QList<int> *      _strings;         // pointer to original string list
      QList<int>        _stringsLoc;      // local working copy of string list

   public:
      EditStringData(QWidget *parent, QList<int> * strings, int * frets);
      ~EditStringData();

   protected:
//      void changeEvent(QEvent *e);
      QString midiCodeToStr(int midiCode);

//private:
//    Ui::EditStringData *ui;

private slots:
      void accept();
      void deleteStringClicked();
      void editStringClicked();
      void newStringClicked();
};

#endif // EDITSTRINGDATA_H
