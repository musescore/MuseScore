//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __EDITSTAFFTYPE_H__
#define __EDITSTAFFTYPE_H__

#include "ui_stafftype.h"
#include "libmscore/mscore.h"

namespace Ms {

// uncomment to use Navigator for tabulature preview; comment to use ScoreView
//#define _USE_NAVIGATOR_PREVIEW_

class Navigator;
class ScoreView;
class StaffType;
class StaffTypePercussion;
class StaffTypeTablature;
class Staff;

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

class EditStaffType : public QDialog, private Ui::EditStaffType {
      Q_OBJECT

      Staff* staff;
      QList<StaffType*> staffTypes;
      bool modified;
#ifdef _USE_NAVIGATOR_PREVIEW_
      Navigator* tabPreview;
#else
      ScoreView* tabPreview;
#endif

      void blockPercPreviewSignals(bool block);
      void blockTabPreviewSignals(bool block);
      void saveCurrent(QListWidgetItem*);
      void setDlgFromPerc(const StaffTypePercussion *st);
      void setPercFromDlg(StaffTypePercussion* st);
      void setDlgFromTab(const StaffTypeTablature* stt);
      void setTabFromDlg(StaffTypeTablature* stt);
      void tabStemsCompatibility(bool checked);
      void tabMinimShortCompatibility(bool checked);
      void tabStemThroughCompatibility(bool checked);
      QString createUniqueStaffTypeName(StaffGroup group);

   private slots:
      void typeChanged(QListWidgetItem*, QListWidgetItem*);
      void createNewType();
      void nameEdited(const QString&);
      void presetTablatureChanged(int idx);
      void presetPercChanged(int idx);
      void durFontNameChanged(int idx);
      void fretFontNameChanged(int idx);
      void on_pushFullConfig_clicked();
      void on_pushQuickConfig_clicked();
      void tabStemThroughToggled(bool checked);
      void tabMinimShortToggled(bool checked);
      void tabStemsToggled(bool checked);
      void updatePreviews();
      void updatePercPreview();
      void updateTabPreview();

public slots:
      virtual void accept();

   public:
      EditStaffType(QWidget* parent, Staff*);
      ~EditStaffType();
      bool isModified() const                 { return modified;   }
      QList<StaffType*> getStaffTypes() const { return staffTypes; }
      };


} // namespace Ms
#endif

