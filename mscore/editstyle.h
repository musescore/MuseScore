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
//   StyleWidget
//---------------------------------------------------------

struct StyleWidget {
      StyleIdx idx;
      QWidget* widget;
      };

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase {
      Q_OBJECT

      Score* cs;
      QPushButton* buttonApplyToAllParts;
      MStyle lstyle;    // local copy of style

      QButtonGroup* stemGroups[VOICES];

      QVector<StyleWidget> styleWidgets;

      virtual void hideEvent(QHideEvent*);

      void getValues();
      void setValues();
      void setHeaderText(StyleIdx idx, QTextEdit* te);
      void setFooterText(StyleIdx idx, QTextEdit* te);

      void apply();
      void applyToAllParts();

   private slots:
      void selectChordDescriptionFile();
      void setChordStyle(bool);
      void toggleHeaderOddEven(bool);
      void toggleFooterOddEven(bool);
      void buttonClicked(QAbstractButton*);
      void setSwingParams(bool);
      void systemMinDistanceValueChanged(double);
      void systemMaxDistanceValueChanged(double);
      void resetStyleValue(int i);

      void on_comboFBFont_currentIndexChanged(int index);

public:
      static const int PAGE_NOTE = 6;
      EditStyle(Score*, QWidget*);
      void setPage(int no);
      };


} // namespace Ms
#endif


