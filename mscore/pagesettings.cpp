//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pagesettings.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

      connect(mmButton,             SIGNAL(clicked()),            SLOT(mmClicked()));
      connect(inchButton,           SIGNAL(clicked()),            SLOT(inchClicked()));
      connect(buttonApply,          SIGNAL(clicked()),            SLOT(apply()));
      connect(buttonApplyToAllParts,SIGNAL(clicked()),            SLOT(applyToAllParts()));
      connect(buttonOk,             SIGNAL(clicked()),            SLOT(ok()));
      connect(portraitButton,       SIGNAL(clicked()),            SLOT(portraitClicked()));
      connect(landscapeButton,      SIGNAL(clicked()),            SLOT(landscapeClicked()));
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

      const PageFormat* pf = s->pageFormat();
      pageGroup->clear();
      int index = 0;
      const PaperSize* ps = pf->paperSize();
      for (int i = 0; paperSizes[i].name; ++i) {
            if (ps == &paperSizes[i])
                  index = i;
            pageGroup->addItem(qApp->translate("paperSizes", paperSizes[i].name));
            }

      pageGroup->setCurrentIndex(index);
      buttonApplyToAllParts->setEnabled(!s->isMaster());
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   blockSignals
//---------------------------------------------------------

void PageSettings::blockSignals(bool block)
      {
      oddPageTopMargin->blockSignals(block);
      oddPageBottomMargin->blockSignals(block);
      oddPageLeftMargin->blockSignals(block);
      oddPageRightMargin->blockSignals(block);
      evenPageTopMargin->blockSignals(block);
      evenPageBottomMargin->blockSignals(block);
      evenPageLeftMargin->blockSignals(block);
      evenPageRightMargin->blockSignals(block);
      spatiumEntry->blockSignals(block);
      pageWidth->blockSignals(block);
      pageHeight->blockSignals(block);
      pageOffsetEntry->blockSignals(block);
      pageGroup->blockSignals(block);
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PageSettings::updateValues()
      {
      Score* sc = preview->score();
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
      oddPageTopMargin->setSuffix(suffix);
      oddPageTopMargin->setSingleStep(singleStepSize);
      oddPageBottomMargin->setSuffix(suffix);
      oddPageBottomMargin->setSingleStep(singleStepSize);
      oddPageLeftMargin->setSuffix(suffix);
      oddPageLeftMargin->setSingleStep(singleStepSize);
      oddPageRightMargin->setSuffix(suffix);
      oddPageRightMargin->setSingleStep(singleStepSize);
      evenPageTopMargin->setSuffix(suffix);
      evenPageTopMargin->setSingleStep(singleStepSize);
      evenPageBottomMargin->setSuffix(suffix);
      evenPageBottomMargin->setSingleStep(singleStepSize);
      evenPageLeftMargin->setSuffix(suffix);
      evenPageLeftMargin->setSingleStep(singleStepSize);
      evenPageRightMargin->setSuffix(suffix);
      evenPageRightMargin->setSingleStep(singleStepSize);
      spatiumEntry->setSuffix(suffix);
      spatiumEntry->setSingleStep(singleStepScale);
      pageWidth->setSuffix(suffix);
      pageWidth->setSingleStep(singleStepSize);
      pageHeight->setSuffix(suffix);
      pageHeight->setSingleStep(singleStepSize);

      const PageFormat* pf = sc->pageFormat();
      int index = 0;
      const PaperSize* ps = pf->paperSize();
      for (int i = 0; true; ++i) {
            if (ps == &paperSizes[i]) {
                  index = i;
                  break;
                  }
            }
      pageGroup->setCurrentIndex(index);
//      QString s;

      //qreal printableWidthValue = pf->printableWidth();
      qreal widthValue = pf->size().width();
      if (mm) {
            oddPageTopMargin->setValue(pf->oddTopMargin() * INCH);
            oddPageBottomMargin->setValue(pf->oddBottomMargin() * INCH);
            oddPageLeftMargin->setValue(pf->oddLeftMargin() * INCH);
            oddPageRightMargin->setValue(pf->oddRightMargin() * INCH);

            evenPageTopMargin->setValue(pf->evenTopMargin() * INCH);
            evenPageBottomMargin->setValue(pf->evenBottomMargin() * INCH);
            evenPageLeftMargin->setValue(pf->evenLeftMargin() * INCH);
            evenPageRightMargin->setValue(pf->evenRightMargin() * INCH);

            spatiumEntry->setValue(sc->spatium()/DPMM);
            pageHeight->setValue(pf->size().height() * INCH);
            widthValue          *= INCH;
            }
      else {
            oddPageTopMargin->setValue(pf->oddTopMargin());
            oddPageBottomMargin->setValue(pf->oddBottomMargin());
            oddPageLeftMargin->setValue(pf->oddLeftMargin());
            oddPageRightMargin->setValue(pf->oddRightMargin());

            evenPageTopMargin->setValue(pf->evenTopMargin());
            evenPageBottomMargin->setValue(pf->evenBottomMargin());
            evenPageLeftMargin->setValue(pf->evenLeftMargin());
            evenPageRightMargin->setValue(pf->evenRightMargin());

            spatiumEntry->setValue(sc->spatium()/DPI);
            pageHeight->setValue(pf->size().height());
            }
      pageWidth->setValue(widthValue);

      oddPageLeftMargin->setMaximum(widthValue);
      oddPageRightMargin->setMaximum(widthValue);
      evenPageLeftMargin->setMaximum(widthValue);
      evenPageRightMargin->setMaximum(widthValue);

      evenPageTopMargin->setEnabled(pf->twosided());
      evenPageBottomMargin->setEnabled(pf->twosided());
      evenPageLeftMargin->setEnabled(pf->twosided());
      evenPageRightMargin->setEnabled(pf->twosided());

      if (twosided->isChecked()) {
            evenPageRightMargin->setValue(oddPageLeftMargin->value());
            evenPageLeftMargin->setValue(oddPageRightMargin->value());
            }
      else {
            evenPageRightMargin->setValue(oddPageRightMargin->value());
            evenPageLeftMargin->setValue(oddPageLeftMargin->value());
            }

      landscapeButton->setChecked(pf->width() > pf->height());
      portraitButton->setChecked(pf->width() <= pf->height());

      twosided->setChecked(pf->twosided());

      pageOffsetEntry->setValue(sc->pageNumberOffset() + 1);

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
//   portraitClicked
//---------------------------------------------------------

void PageSettings::portraitClicked()
      {
      PageFormat pf;
      double f  = mmUnit ? 1.0/INCH : 1.0;
      pf.setPrintableWidth(pf.width() - (oddPageLeftMargin->value() + oddPageRightMargin->value())  * f);
      preview->score()->setPageFormat(pf);
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   landscapeClicked
//---------------------------------------------------------

void PageSettings::landscapeClicked()
      {
      PageFormat pf;
      pf.setSize(QSizeF(pf.height(), pf.width()));
      double f  = mmUnit ? 1.0/INCH : 1.0;
      pf.setPrintableWidth(pf.width() - (oddPageLeftMargin->value() + oddPageRightMargin->value())  * f);
      preview->score()->setPageFormat(pf);
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setTwosided(flag);
      preview->score()->setPageFormat(pf);
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
      double f  = mmUnit ? 1.0/INCH : 1.0;
      double f1 = mmUnit ? DPMM : DPI;

      PageFormat pf;

      pf.setSize(QSizeF(pageWidth->value(), pageHeight->value()) * f);
      pf.setPrintableWidth((pageWidth->value() - oddPageLeftMargin->value() - oddPageRightMargin->value())  * f);
      pf.setEvenTopMargin(evenPageTopMargin->value() * f);
      pf.setEvenBottomMargin(evenPageBottomMargin->value() * f);
      pf.setEvenLeftMargin(evenPageLeftMargin->value() * f);
      pf.setOddTopMargin(oddPageTopMargin->value() * f);
      pf.setOddBottomMargin(oddPageBottomMargin->value() * f);
      pf.setOddLeftMargin(oddPageLeftMargin->value() * f);
      pf.setTwosided(twosided->isChecked());

      double sp = spatiumEntry->value() * f1;

      s->startCmd();
      s->undoChangePageFormat(&pf, sp, pageOffsetEntry->value()-1);
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
      if (size > 0) {
            PageFormat pf;
            pf.copy(*preview->score()->pageFormat());
            pf.setSize(&paperSizes[size]);
            double f  = mmUnit ? 1.0/INCH : 1.0;
            pf.setPrintableWidth(pf.width() - (oddPageLeftMargin->value() + oddPageRightMargin->value())  * f);
            preview->score()->setPageFormat(pf);
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
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setOddTopMargin(val);
      preview->score()->setPageFormat(pf);
      updatePreview(1);
      }

//---------------------------------------------------------
//   olmChanged
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      if(twosided->isChecked()) {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }
      else{
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageLeftMargin->blockSignals(false);
            }
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setPrintableWidth(pf.width() - pf.oddRightMargin() - val);
      pf.setOddLeftMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(0);
      }

//---------------------------------------------------------
//   ormChanged
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());

      if (twosided->isChecked()) {
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            pf.setEvenLeftMargin(val);
            evenPageLeftMargin->blockSignals(false);
            }
      else {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }

      pf.setPrintableWidth(pf.width() - pf.oddLeftMargin() - val);

      preview->score()->setPageFormat(pf);
      updatePreview(0);
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setOddBottomMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(1);
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setEvenTopMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(1);
      }

//---------------------------------------------------------
//   elmChanged
//---------------------------------------------------------

void PageSettings::elmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      if(twosided->isChecked()) {
            oddPageRightMargin->blockSignals(true);
            oddPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            oddPageRightMargin->blockSignals(false);
            }
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setPrintableWidth(pf.width() - pf.evenRightMargin() - val);
      pf.setEvenLeftMargin(val);
      preview->score()->setPageFormat(pf);

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

      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setPrintableWidth(pf.width() - pf.evenLeftMargin() - val);
      pf.setOddLeftMargin(val);
      preview->score()->setPageFormat(pf);
      updatePreview(0);
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf;
      pf.copy(*preview->score()->pageFormat());
      pf.setEvenBottomMargin(val);
      preview->score()->setPageFormat(pf);
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
      PageFormat f;
      f.copy(*preview->score()->pageFormat());
      f.setSize(QSizeF(val2, val));
      preview->score()->setPageFormat(f);

      updatePreview(1);
      }

//---------------------------------------------------------
//   pageWidthChanged
//---------------------------------------------------------

void PageSettings::pageWidthChanged(double val)
      {
      double val2 = pageHeight->value();

      oddPageLeftMargin->setMaximum(val);
      oddPageRightMargin->setMaximum(val);
      evenPageLeftMargin->setMaximum(val);
      evenPageRightMargin->setMaximum(val);

      if (mmUnit) {
            val /= INCH;
            val2 /= INCH;
            }
      pageGroup->setCurrentIndex(0);      // custom
      PageFormat f;
      f.copy(*preview->score()->pageFormat());
      f.setSize(QSizeF(val, val2));
      preview->score()->setPageFormat(f);

      updatePreview(0);
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview(int val)
      {
//      updateValues();
      switch(val) {
            case 0:
                  preview->score()->doLayout();
                  break;
            case 1:
//TODO-ws                  preview->score()->doLayoutPages();
                  break;
            }
      preview->layoutChanged();
      }
}

