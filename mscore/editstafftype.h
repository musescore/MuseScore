//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2014 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EDITSTAFFTYPE_H__
#define __EDITSTAFFTYPE_H__

#include "ui_editstafftype.h"
#include "ui_stafftypetemplates.h"
#include "libmscore/mscore.h"
#include "libmscore/stafftype.h"

namespace Ms {

class ScoreView;
class StaffType;
class Staff;

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

class EditStaffType : public QDialog, private Ui::EditStaffType {
      Q_OBJECT

      Staff* staff;
      StaffType staffType;

      virtual void hideEvent(QHideEvent *);
      void blockSignals(bool block);

      void setFromDlg();

      void tabStemsCompatibility(bool checked);
      void tabMinimShortCompatibility(bool checked);
      void tabStemThroughCompatibility(bool checked);
      QString createUniqueStaffTypeName(StaffGroup group);
      void setValues();

   private slots:
      void nameEdited(const QString&);
      void durFontNameChanged(int idx);
      void fretFontNameChanged(int idx);
      void tabStemThroughToggled(bool checked);
      void tabMinimShortToggled(bool checked);
      void tabStemsToggled(bool checked);
      void updatePreview();

      void savePresets();
      void loadPresets();
      void resetToTemplateClicked();
      void addToTemplatesClicked();
//      void staffGroupChanged(int);

   public:
      EditStaffType(QWidget* parent, Staff*);
      ~EditStaffType() {}
      const StaffType* getStaffType() const { return &staffType; }
      };

//---------------------------------------------------------
//   StaffTypeTemplates
//---------------------------------------------------------

class StaffTypeTemplates : public QDialog, private Ui::StaffTypeTemplates {
      Q_OBJECT

   public:
      StaffTypeTemplates(const StaffType&, QWidget* parent = 0);
      StaffType* staffType() const;
      };

} // namespace Ms
#endif

