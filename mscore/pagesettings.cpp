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

#include "globals.h"
#include "pagesettings.h"
#include "libmscore/page.h"
#include "libmscore/style.h"
#include "libmscore/score.h"
#include "navigator.h"
#include "libmscore/mscore.h"
#include "libmscore/excerpt.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

PageSettings::PageSettings(QWidget* parent)
   : AbstractDialog(parent)
      {
      setObjectName("PageSettings");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      NScrollArea* sa = new NScrollArea;
      preview = new Navigator(sa, this);
      preview->setPreviewOnly(true);

      static_cast<QVBoxLayout*>(previewGroup->layout())->insertWidget(0, sa);

      mmUnit = true;      // should be made a global configuration item

      if (mmUnit)
            mmButton->setChecked(true);
      else
            inchButton->setChecked(true);

      MuseScore::restoreGeometry(this);

      for (int i = 0; i < QPageSize::LastPageSize; ++i)
            pageGroup->addItem(QPageSize::name(QPageSize::PageSizeId(i)), i);

      connect(mmButton,             SIGNAL(clicked()),            SLOT(mmClicked()));
      connect(inchButton,           SIGNAL(clicked()),            SLOT(inchClicked()));
      connect(buttonApply,          SIGNAL(clicked()),            SLOT(apply()));
      connect(buttonApplyToAllParts,SIGNAL(clicked()),            SLOT(applyToAllParts()));
      connect(buttonOk,             SIGNAL(clicked()),            SLOT(ok()));
      connect(portraitButton,       SIGNAL(clicked()),            SLOT(orientationClicked()));
      connect(landscapeButton,      SIGNAL(clicked()),            SLOT(orientationClicked()));
      connect(twosided,             SIGNAL(toggled(bool)),        SLOT(twosidedToggled(bool)));
      connect(pageHeight,           SIGNAL(valueChanged(double)), SLOT(pageHeightChanged(double)));
      connect(pageWidth,            SIGNAL(valueChanged(double)), SLOT(pageWidthChanged(double)));
      connect(oddPageTopMargin,     SIGNAL(valueChanged(double)), SLOT(otmChanged(double)));
      connect(oddPageBottomMargin,  SIGNAL(valueChanged(double)), SLOT(obmChanged(double)));
      connect(oddPageLeftMargin,    SIGNAL(valueChanged(double)), SLOT(olmChanged(double)));
      connect(oddPageRightMargin,   SIGNAL(valueChanged(double)), SLOT(ormChanged(double)));
      connect(evenPageTopMargin,    SIGNAL(valueChanged(double)), SLOT(etmChanged(double)));
      connect(evenPageBottomMargin, SIGNAL(valueChanged(double)), SLOT(ebmChanged(double)));
      connect(evenPageRightMargin,  SIGNAL(valueChanged(double)), SLOT(ermChanged(double)));
      connect(evenPageLeftMargin,   SIGNAL(valueChanged(double)), SLOT(elmChanged(double)));
      connect(pageGroup,            SIGNAL(activated(int)),       SLOT(pageFormatSelected(int)));
      connect(spatiumEntry,         SIGNAL(valueChanged(double)), SLOT(spatiumChanged(double)));
      connect(pageOffsetEntry,      SIGNAL(valueChanged(int)),    SLOT(pageOffsetChanged(int)));
      }

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

PageSettings::~PageSettings()
      {
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PageSettings::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PageSettings::setScore(MasterScore* s)
      {
      cs  = s;
      MasterScore* sl = s->clone();
      preview->setScore(sl);
      buttonApplyToAllParts->setEnabled(!s->isMaster());
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   blockSignals
//---------------------------------------------------------

void PageSettings::blockSignals(bool block)
      {
      for (auto w : { oddPageTopMargin, oddPageBottomMargin, oddPageLeftMargin, oddPageRightMargin,
         evenPageTopMargin, evenPageBottomMargin, evenPageLeftMargin, evenPageRightMargin, spatiumEntry,
         pageWidth, pageHeight } )
            {
            w->blockSignals(block);
            }
      pageOffsetEntry->blockSignals(block);
      pageGroup->blockSignals(block);
      }

//---------------------------------------------------------
//   setMarginsMax
//---------------------------------------------------------

void PageSettings::setMarginsMax(double pw)
      {
      oddPageLeftMargin->setMaximum(pw);
      oddPageRightMargin->setMaximum(pw);
      evenPageLeftMargin->setMaximum(pw);
      evenPageRightMargin->setMaximum(pw);
      }

//---------------------------------------------------------
//   updateValues
//    set gui values from style
//---------------------------------------------------------

void PageSettings::updateValues()
      {
      Score* score = preview->score();
      bool mm = mmButton->isChecked();

      blockSignals(true);

      const char* suffix;
      double singleStepSize;
      double singleStepScale;
      if (mm) {
            suffix = "mm";
            singleStepSize = 1.0;
            singleStepScale = 0.2;
            }
      else {
            suffix = "in";
            singleStepSize = 0.05;
            singleStepScale = 0.005;
            }
      for (auto w : { oddPageTopMargin, oddPageBottomMargin, oddPageLeftMargin, oddPageRightMargin, evenPageTopMargin,
         evenPageBottomMargin, evenPageLeftMargin, evenPageRightMargin, spatiumEntry, pageWidth, pageHeight } )
            {
            w->setSuffix(suffix);
            w->setSingleStep(singleStepSize);
            }
      spatiumEntry->setSingleStep(singleStepScale);

      double f = mm ? INCH : 1.0;

      double w = score->styleD(Sid::pageWidth);
      double h = score->styleD(Sid::pageHeight);
      setMarginsMax(w * f);

      double pw = score->styleD(Sid::pagePrintableWidth);
      pageGroup->setCurrentIndex(int(QPageSize::id(QSizeF(w, h), QPageSize::Inch, QPageSize::FuzzyOrientationMatch)));

      oddPageTopMargin->setValue(score->styleD(Sid::pageOddTopMargin) * f);
      oddPageBottomMargin->setValue(score->styleD(Sid::pageOddBottomMargin) * f);
      oddPageLeftMargin->setValue(score->styleD(Sid::pageOddLeftMargin) * f);
      oddPageRightMargin->setValue((w - pw - score->styleD(Sid::pageOddLeftMargin)) * f);

      evenPageTopMargin->setValue(score->styleD(Sid::pageEvenTopMargin) * f);
      evenPageBottomMargin->setValue(score->styleD(Sid::pageEvenBottomMargin) * f);
      evenPageLeftMargin->setValue(score->styleD(Sid::pageEvenLeftMargin) * f);
      evenPageRightMargin->setValue((w - pw - score->styleD(Sid::pageEvenLeftMargin)) * f);
      pageHeight->setValue(h * f);
      pageWidth->setValue(w * f);

      double f1 = mm ? 1.0/DPMM : 1.0/DPI;
      spatiumEntry->setValue(score->spatium() * f1);


      bool _twosided = score->styleB(Sid::pageTwosided);
      evenPageTopMargin->setEnabled(_twosided);
      evenPageBottomMargin->setEnabled(_twosided);
      evenPageLeftMargin->setEnabled(_twosided);
      evenPageRightMargin->setEnabled(_twosided);

      if (twosided->isChecked()) {
            evenPageRightMargin->setValue(oddPageLeftMargin->value());
            evenPageLeftMargin->setValue(oddPageRightMargin->value());
            }
      else {
            evenPageRightMargin->setValue(oddPageRightMargin->value());
            evenPageLeftMargin->setValue(oddPageLeftMargin->value());
            }

      landscapeButton->setChecked(score->styleD(Sid::pageWidth) > score->styleD(Sid::pageHeight));
      portraitButton->setChecked(score->styleD(Sid::pageWidth) <= score->styleD(Sid::pageHeight));

      twosided->setChecked(_twosided);

      pageOffsetEntry->setValue(score->pageNumberOffset() + 1);

      blockSignals(false);
      }

//---------------------------------------------------------
//   inchClicked
//---------------------------------------------------------

void PageSettings::inchClicked()
      {
      mmUnit = false;
      updateValues();
      }

//---------------------------------------------------------
//   mmClicked
//---------------------------------------------------------

void PageSettings::mmClicked()
      {
      mmUnit = true;
      updateValues();
      }

//---------------------------------------------------------
//   orientationClicked
//    swap width/height
//---------------------------------------------------------

void PageSettings::orientationClicked()
      {
      qreal w = preview->score()->styleD(Sid::pageWidth);
      qreal h = preview->score()->styleD(Sid::pageHeight);

      preview->score()->style().set(Sid::pageWidth, h);
      preview->score()->style().set(Sid::pageHeight, w);

      double f = mmUnit ? 1.0/INCH : 1.0;
      preview->score()->style().set(Sid::pagePrintableWidth, h - (oddPageLeftMargin->value() + oddPageRightMargin->value()) * f);
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      preview->score()->style().set(Sid::pageTwosided, flag);
      updateValues();
      updatePreview(1);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PageSettings::apply()
      {
      applyToScore(cs);
      mscore->endCmd();
      }

//---------------------------------------------------------
//   applyToScore
//---------------------------------------------------------

void PageSettings::applyToScore(Score* s)
      {
      s->startCmd();
      double f  = mmUnit ? 1.0/INCH : 1.0;
      double f1 = mmUnit ? DPMM : DPI;

      s->undoChangeStyleVal(Sid::pageWidth, pageWidth->value() * f);
      s->undoChangeStyleVal(Sid::pageHeight, pageHeight->value() * f);
      s->undoChangeStyleVal(Sid::pagePrintableWidth, (pageWidth->value() - oddPageLeftMargin->value() - oddPageRightMargin->value())  * f);
      s->undoChangeStyleVal(Sid::pageEvenTopMargin, evenPageTopMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageEvenBottomMargin, evenPageBottomMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageEvenLeftMargin, evenPageLeftMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageOddTopMargin, oddPageTopMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageOddBottomMargin, oddPageBottomMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageOddLeftMargin, oddPageLeftMargin->value() * f);
      s->undoChangeStyleVal(Sid::pageTwosided, twosided->isChecked());
      s->undoChangeStyleVal(Sid::spatium, spatiumEntry->value() * f1);

      s->endCmd();
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void PageSettings::applyToAllParts()
      {
      for (Excerpt* e : cs->excerpts())
            applyToScore(e->partScore());
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void PageSettings::ok()
      {
      apply();
      done(0);
      }

//---------------------------------------------------------
//   done
//---------------------------------------------------------

void PageSettings::done(int val)
      {
      cs->setLayoutAll();     // HACK
      QDialog::done(val);
      }

//---------------------------------------------------------
//   pageFormatSelected
//---------------------------------------------------------

void PageSettings::pageFormatSelected(int size)
      {
      if (size >= 0) {
            Score* s  = preview->score();
            int id    = pageGroup->currentData().toInt();
            QSizeF sz = QPageSize::size(QPageSize::PageSizeId(id), QPageSize::Inch);

            s->style().set(Sid::pageWidth, sz.width());
            s->style().set(Sid::pageHeight, sz.height());

            double f  = mmUnit ? 1.0/INCH : 1.0;
            s->style().set(Sid::pagePrintableWidth, sz.width() - (oddPageLeftMargin->value() + oddPageRightMargin->value())  * f);
            updateValues();
            updatePreview(0);
            }
      }

//---------------------------------------------------------
//   otmChanged
//---------------------------------------------------------

void PageSettings::otmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageOddTopMargin, val);
      updatePreview(1);
      }

//---------------------------------------------------------
//   olmChanged
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      if (twosided->isChecked()) {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }
      else {
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageLeftMargin->blockSignals(false);
            }
      Score* s = preview->score();
      s->style().set(Sid::pageOddLeftMargin, val);
      s->style().set(Sid::pagePrintableWidth, s->styleD(Sid::pageWidth) - s->styleD(Sid::pageEvenLeftMargin) - val);

      updatePreview(0);
      }

//---------------------------------------------------------
//   ormChanged
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      Score* s = preview->score();
      if (twosided->isChecked()) {
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            s->style().set(Sid::pageEvenLeftMargin, val);
            evenPageLeftMargin->blockSignals(false);
            }
      else {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }
      s->style().set(Sid::pagePrintableWidth, s->styleD(Sid::pageWidth) - s->styleD(Sid::pageOddLeftMargin) - val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageOddBottomMargin, val);
      updatePreview(1);
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageEvenTopMargin, val);
      updatePreview(1);
      }

//---------------------------------------------------------
//   elmChanged
//---------------------------------------------------------

void PageSettings::elmChanged(double val)
      {
      double f  = mmUnit ? 1.0/INCH : 1.0;
      val *= f;

      if (twosided->isChecked()) {
            oddPageRightMargin->blockSignals(true);
            oddPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            oddPageRightMargin->blockSignals(false);
            }
      Score* s = preview->score();
      s->style().set(Sid::pageEvenLeftMargin, val);
      s->style().set(Sid::pagePrintableWidth, s->styleD(Sid::pageWidth) - evenPageRightMargin->value() * f - val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   ermChanged
//---------------------------------------------------------

void PageSettings::ermChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      if (twosided->isChecked()) {
            oddPageLeftMargin->blockSignals(true);
            oddPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            oddPageLeftMargin->blockSignals(false);
            }
      Score* s = preview->score();
      s->style().set(Sid::pagePrintableWidth, s->styleD(Sid::pageWidth) - s->styleD(Sid::pageEvenLeftMargin) - val);
      s->style().set(Sid::pageOddLeftMargin, val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageEvenBottomMargin, val);
      updatePreview(1);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void PageSettings::spatiumChanged(double val)
      {
      val *= mmUnit ? DPMM : DPI;
      double oldVal = preview->score()->spatium();
      preview->score()->setSpatium(val);
      preview->score()->spatiumChanged(oldVal, val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   pageOffsetChanged
//---------------------------------------------------------

void PageSettings::pageOffsetChanged(int val)
      {
      preview->score()->setPageNumberOffset(val-1);
      updatePreview(0);
      }

//---------------------------------------------------------
//   pageHeightChanged
//---------------------------------------------------------

void PageSettings::pageHeightChanged(double val)
      {
      double val2 = pageWidth->value();
      if (mmUnit) {
            val /= INCH;
            val2 /= INCH;
            }
      pageGroup->setCurrentIndex(0);      // custom
      preview->score()->style().set(Sid::pageWidth, val);
      preview->score()->style().set(Sid::pageHeight, val2);

      updatePreview(1);
      }

//---------------------------------------------------------
//   pageWidthChanged
//---------------------------------------------------------

void PageSettings::pageWidthChanged(double val)
      {
      setMarginsMax(val);

      double f    = mmUnit ? 1.0/INCH : 1.0;
      double val2 = pageHeight->value() * f;
      val        *= f;
      pageGroup->setCurrentIndex(0);      // custom
      preview->score()->style().set(Sid::pageWidth, val);
      preview->score()->style().set(Sid::pageHeight, val2);
      preview->score()->style().set(Sid::pagePrintableWidth, val - (oddPageLeftMargin->value() + evenPageLeftMargin->value()) * f);
      updatePreview(0);
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview(int val)
      {
      updateValues();
      switch(val) {
            case 0:
                  preview->score()->doLayout();
                  break;
            case 1:
                  preview->score()->doLayout();
                  break;
            }
      preview->layoutChanged();
      }
}

