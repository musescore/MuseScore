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

enum class PaperType { // 4 paper size types for the typesList combo box
      NOTYPE = -1,
      Common,
      Metric,
      Imperial,
      Other
      };

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------
struct PageUnit; // instead of #include "libmscore/style.h"

class PageSettings : public AbstractDialog, private Ui::PageSettingsBase {
      Q_OBJECT

      Score* cs;
      Score* clonedScore;
      Navigator* preview;
      double sp20_mm;
      double sp20_in;
      double sp20_p;
      double sp20_dd;
      double sp20_c;

//      std::unique_ptr<Score> clonedScoreForNavigator;

      virtual void hideEvent(QHideEvent*);
      void updateWidgets(bool onlyUnits = false);
      void updateDecimals(Score *score, PageUnit *unit, int decimals);
      void updateMaximum(double max);
      void updateWidthHeight(const QRectF & rect);
      void updatePreview();
      void blockSignals(bool);
      void applyToScore(Score* s, bool runCmd = true);
      void lrMargins(double val, bool isLeft, bool isOdd, QDoubleSpinBox* spinOne);
      void widthHeightChanged(double w, double h, bool byType);
      double marginMinMax(double val, double max, QDoubleSpinBox* spinner);
      PaperType getPaperType(int id);

   private slots:
      void typeChanged(int idx, bool isSignal = true);
      void sizeChanged();
      void unitsChanged();

      void setToDefault();
      void applyToAllParts();
      void okCancel(QAbstractButton*);
      void done(int val);

      void twosidedToggled(bool b);
      void orientationToggled(bool);
      void otmChanged(double val);
      void obmChanged(double val);
      void olmChanged(double val);
      void ormChanged(double val);
      void etmChanged(double val);
      void ebmChanged(double val);
      void elmChanged(double val);
      void ermChanged(double val);
      void spatiumChanged(double val);
      void widthChanged(double val);
      void heightChanged(double val);
      void pageOffsetChanged(int val);

   protected:
      virtual void retranslate() { retranslateUi(this); }

   public:
      PageSettings(QWidget* parent = 0);
      ~PageSettings();
      void setScore(Score*);
      };
} // namespace Ms
#endif

