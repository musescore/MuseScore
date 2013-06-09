//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editstyle.h 5403 2012-03-03 00:01:53Z miwarre $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __EDITSTYLE_H__
#define __EDITSTYLE_H__

#include "ui_editstyle.h"
#include "globals.h"
#include "libmscore/style.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase {
      Q_OBJECT

      Score* cs;
      MStyle lstyle;    // local copy of style

      QButtonGroup* stemGroups[VOICES];

      void getValues();
      void setValues();

      void apply();

   private slots:
      void selectChordDescriptionFile();
      void setChordStyle(bool);
      void buttonClicked(QAbstractButton*);
      void editTextClicked(int id);

      void on_comboFBFont_currentIndexChanged(int index);

public:
      enum { PAGE_NOTE = 6 };
      EditStyle(Score*, QWidget*);
      void setPage(int no);
      };


} // namespace Ms
#endif


