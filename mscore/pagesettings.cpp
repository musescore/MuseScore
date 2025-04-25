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

#include "musescore.h"
#include "navigator.h"
#include "pagesettings.h"

#include "libmscore/excerpt.h"
#include "libmscore/mscore.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/style.h"

namespace Ms {

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

PageSettings::PageSettings(QWidget* parent)
   : AbstractDialog(parent)
      {
      clonedScore = 0;
      setObjectName("PageSettings");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      NScrollArea* sa = new NScrollArea;
      preview = new Navigator(sa, this);
//      preview->setPreviewOnly(true);

      static_cast<QVBoxLayout*>(previewGroup->layout())->insertWidget(0, sa);

      mmUnit = true;      // should be made a global configuration item
      _changeFlag = false;

      if (mmUnit)
            mmButton->setChecked(true);
      else
            inchButton->setChecked(true);

      MuseScore::restoreGeometry(this);

      for (int i = 0; i < QPageSize::LastPageSize; ++i)
            pageGroup->addItem(QPageSize::name(QPageSize::PageSizeId(i)), i);

      connect(mmButton,             SIGNAL(clicked()),            SLOT(mmClicked()));
      connect(inchButton,           SIGNAL(clicked()),            SLOT(inchClicked()));
      connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      connect(buttonApplyToAllParts,SIGNAL(clicked()),            SLOT(applyToAllParts()));
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
      delete clonedScore;
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

void PageSettings::setScore(Score* s)
      {
      cs = s;
      delete clonedScore;
      clonedScore = s->clone();
      // HACK: clone doesn't actually copy style completely for older scores;
      // instead it replaces any style settings that were at the older defaults with the current defaults
      // this is not desired here, but might be in other places that Score::clone() is used
      // so instead we simply re-copy the style here
      int defaultsVersion = s->style().defaultStyleVersion();
      clonedScore->style().setDefaultStyleVersion(defaultsVersion);
      clonedScore->style() = s->style();

      clonedScore->setLayoutMode(LayoutMode::PAGE);

      clonedScore->doLayout();
      preview->setScore(clonedScore);
      buttonApplyToAllParts->setEnabled(!cs->isMaster());
      updateValues();
      updatePreview();
      _changeFlag = false;
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

      QString suffix;
      double singleStepSize;
      double singleStepScale;
      if (mm) {
            suffix = tr("mm");
            singleStepSize = 1.0;
            singleStepScale = 0.05;
            }
      else {
            suffix = tr("in", "abbreviation for inch");
            singleStepSize = 0.05;
            singleStepScale = 0.002;
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
      updatePreview();
      }

void PageSettings::on_resetPageStyleButton_clicked()
      {
      preview->score()->style().resetStyles(preview->score(), pageStyles());
      pageOffsetEntry->setValue(1);

      updatePreview();
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      preview->score()->style().set(Sid::pageTwosided, flag);
      updateValues();
      updatePreview();
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void PageSettings::buttonBoxClicked(QAbstractButton* button)
      {
      switch (buttonBox->buttonRole(button)) {
            case QDialogButtonBox::ApplyRole:
                  PageSettings::apply();
                  break;
            case QDialogButtonBox::AcceptRole:
                  PageSettings::apply();
                  // fall through
            case QDialogButtonBox::RejectRole:
                  close();
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PageSettings::apply()
      {
      if (!_changeFlag)
            return;
      cs->startCmd();
      applyToScore(cs);
      cs->endCmd();
      mscore->endCmd();
      _changeFlag = false;
      }

//---------------------------------------------------------
//   applyToScore
//---------------------------------------------------------

void PageSettings::applyToScore(Score* s)
      {
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
      s->undoChangePageNumberOffset(pageOffsetEntry->value() - 1);
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void PageSettings::applyToAllParts()
      {
      if (!_changeFlag)
            return;
      cs->startCmd();
      for (Excerpt*& e : cs->excerpts())
            applyToScore(e->partScore());
      cs->endCmd();
      _changeFlag = false;
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
            updatePreview();
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
      updatePreview();
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

      updatePreview();
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
      updatePreview();
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageOddBottomMargin, val);
      updatePreview();
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageEvenTopMargin, val);
      updatePreview();
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
      updatePreview();
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
      updatePreview();
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->style().set(Sid::pageEvenBottomMargin, val);
      updatePreview();
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
      updatePreview();
      }

//---------------------------------------------------------
//   pageOffsetChanged
//---------------------------------------------------------

void PageSettings::pageOffsetChanged(int val)
      {
      preview->score()->setPageNumberOffset(val-1);
      updatePreview();
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
      preview->score()->style().set(Sid::pageHeight, val);
      preview->score()->style().set(Sid::pageWidth, val2);

      updatePreview();
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
      updatePreview();
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview()
      {
      _changeFlag = true;
      preview->score()->doLayout();
      preview->layoutChanged();
      }
}

