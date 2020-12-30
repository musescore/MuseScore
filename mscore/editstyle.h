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

class QScrollArea;

namespace Ms {

class Score;
class EditStyle;

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
//   EditStylePage
///   This is a type for a pointer to any QWidget that is a member of EditStyle.
///   It's used to create static references to the pointers to pages.
//---------------------------------------------------------

typedef QWidget* EditStyle::* EditStylePage;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase {
      Q_OBJECT

      Score* cs = nullptr;
      QPushButton* buttonApplyToAllParts = nullptr;
      QVector<StyleWidget> styleWidgets;
      QScrollArea* scrollArea = nullptr;
      bool isTooBig = false;
      bool hasShown = false;
      bool needResetStyle = false;

      virtual void showEvent(QShowEvent*);
      virtual void hideEvent(QHideEvent*);
      QVariant getValue(Sid idx);
      void setValues();

      void resetStyle(Score* score);
      void applyToAllParts();
      const StyleWidget& styleWidget(Sid) const;

      void adjustPagesStackSize(int currentPageIndex);

      static EditStylePage pageForElement(Element*);

   private slots:
      void selectChordDescriptionFile();
      void setChordStyle(bool);
      void enableStyleWidget(const Sid idx, bool enable);
      void enableVerticalSpreadClicked(bool);
      void disableVerticalSpreadClicked(bool);
      void toggleHeaderOddEven(bool);
      void toggleFooterOddEven(bool);
      void buttonClicked(QAbstractButton*);
      void setSwingParams(bool);
      void concertPitchToggled(bool);
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
      void on_resetStylesButton_clicked();
      void editUserStyleName();
      void endEditUserStyleName();
      void resetUserStyleName();

   public:
      EditStyle(Score*, QWidget*);
      void setPage(int no);
      void setScore(Score* s) { cs = s; }

      void gotoElement(Element* e);
      void gotoHeaderFooterPage();
      static bool elementHasPage(Element* e);
      
   public slots:
      void accept();
      void reject();
      };


} // namespace Ms
#endif
