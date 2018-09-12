//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PAGESETTINGS_H__
#define __PAGESETTINGS_H__

#include "ui_pagesettings.h"
#include "abstractdialog.h"

namespace Ms {

class MasterScore;
class Score;
class Navigator;

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

class PageSettings : public AbstractDialog, private Ui::PageSettingsBase {
      Q_OBJECT

      Navigator* preview;
      bool mmUnit;
      MasterScore* cs;

      virtual void hideEvent(QHideEvent*);
      void updateValues();
      void updatePreview(int);
      void blockSignals(bool);
      void applyToScore(Score*);
      void setMarginsMax(double);

   private slots:
      void mmClicked();
      void inchClicked();
      void pageFormatSelected(int);

      void apply();
      void applyToAllParts();
      void ok();
      void done(int val);

      void twosidedToggled(bool);
      void otmChanged(double val);
      void obmChanged(double val);
      void olmChanged(double val);
      void ormChanged(double val);
      void etmChanged(double val);
      void ebmChanged(double val);
      void elmChanged(double val);
      void ermChanged(double val);
      void spatiumChanged(double val);
      void pageHeightChanged(double);
      void pageWidthChanged(double);
      void pageOffsetChanged(int val);
      void orientationClicked();

   protected:
      virtual void retranslate() { retranslateUi(this); }

   public:
      PageSettings(QWidget* parent = 0);
      ~PageSettings();
      void setScore(Score*);
      };


} // namespace Ms
#endif

