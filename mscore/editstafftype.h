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

#include "ui_stafftype.h"
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
      ScoreView* preview = 0;

      void blockPercPreviewSignals(bool block);
      void blockTabPreviewSignals(bool block);

      void setPercFromDlg();
      void setTabFromDlg();

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
      void updatePreviews();
      void updatePercPreview();
      void updateTabPreview();

      void savePresets();
      void loadPresets();
      void loadFromTemplateClicked();
      void addToTemplatesClicked();
      void staffGroupChanged(int);

public slots:
      virtual void accept();

   public:
      EditStaffType(QWidget* parent, Staff*);
      ~EditStaffType() {}
      const StaffType* getStaffType() const { return &staffType; }
      };


} // namespace Ms
#endif

