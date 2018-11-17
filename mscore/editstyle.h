//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
#include "libmscore/mscore.h"
#include "libmscore/style.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   StyleWidget
//---------------------------------------------------------

struct StyleWidget {
      Sid idx;
      bool showPercent;
      QObject* widget;
      QToolButton* reset;
      };

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase {
      Q_OBJECT

      Score* cs;
      QPushButton* buttonApplyToAllParts;
      QButtonGroup* stemGroups[VOICES];
      QVector<StyleWidget> styleWidgets;
      QButtonGroup* keySigNatGroup;
      QButtonGroup* clefTypeGroup;

      virtual void hideEvent(QHideEvent*);
      QVariant getValue(Sid idx);
      void setValues();

      void applyToAllParts();
      const StyleWidget& styleWidget(Sid) const;

   private slots:
      void selectChordDescriptionFile();
      void setChordStyle(bool);
      void toggleHeaderOddEven(bool);
      void toggleFooterOddEven(bool);
      void buttonClicked(QAbstractButton*);
      void setSwingParams(bool);
      void lyricsDashMinLengthValueChanged(double);
      void lyricsDashMaxLengthValueChanged(double);
      void systemMinDistanceValueChanged(double);
      void systemMaxDistanceValueChanged(double);
      void resetStyleValue(int);
      void valueChanged(int);
      void textStyleChanged(int);
      void resetTextStyle(Pid);
      void textStyleValueChanged(Pid, QVariant);
      void on_comboFBFont_currentIndexChanged(int index);
      void on_buttonTogglePagelist_clicked();

public:
      static const int PAGE_NOTE = 6;
      EditStyle(Score*, QWidget*);
      void setPage(int no);
      };


} // namespace Ms
#endif
