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
#include "preferences.h"
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
      clonedScore = 0;
      setObjectName("PageSettings");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      NScrollArea* sa = new NScrollArea;
      preview = new Navigator(sa, this);
//      preview->setPreviewOnly(true);

      static_cast<QVBoxLayout*>(previewGroup->layout())->insertWidget(0, sa);

      MuseScore::restoreGeometry(this);

      typesList->addItem("Common");
      typesList->addItem("Metric");
      typesList->addItem("Imperial");
      typesList->addItem("Other");

      for (int i = 0; i <= QPageSize::Cicero; ++i)
            unitsList->addItem(QString("%1 (%2)").arg(pageUnits[i].name())
                                                 .arg(pageUnits[i].suffix()));

      // The dialog box rounds the spatium to 3 decimal places.
      // These member variables are the default value ofSPATIUM20 converted to
      // each unit and rounded to 3 decimals. These variables are used to avoid
      // rounding errors by forcing the unrounded default value of SPATIUM20.
      double spPoints = SPATIUM20 / DPI_F; // SPATIUM20 in points == 5pt
      sp20_mm = rint(SPATIUM20 / DPMM * 1000) / 1000; // millimeters
      sp20_in = rint(SPATIUM20 / DPI  * 1000) / 1000; // inches
      sp20_p  = rint(spPoints  / PICA * 1000) / 1000; // picas
      spPoints /= DIDOT;                   // SPATIUM20 in didot
      sp20_dd = rint(spPoints         * 1000) / 1000; // didot
      sp20_c  = rint(spPoints  / PICA * 1000) / 1000; // cicero

      connect(buttonBox,        SIGNAL(clicked(QAbstractButton*)), SLOT(okCancel(QAbstractButton*)));
      connect(resetToDefault,   SIGNAL(clicked()), SLOT(setToDefault()));
      connect(applyToParts,     SIGNAL(clicked()), SLOT(applyToAllParts()));
      connect(twosided,         SIGNAL(toggled(bool)), SLOT(twosidedToggled(bool)));
      connect(portrait,         SIGNAL(toggled(bool)), SLOT(orientationToggled(bool)));
      connect(landscape,        SIGNAL(toggled(bool)), SLOT(orientationToggled(bool)));
      connect(pageOffset,       SIGNAL(valueChanged(int)),    SLOT(pageOffsetChanged(int)));
      connect(pageWidth,        SIGNAL(valueChanged(double)), SLOT(widthChanged(double)));
      connect(pageHeight,       SIGNAL(valueChanged(double)), SLOT(heightChanged(double)));
      connect(staffSpace,       SIGNAL(valueChanged(double)), SLOT(spatiumChanged(double)));
      connect(oddTopMargin,     SIGNAL(valueChanged(double)), SLOT(otmChanged(double)));
      connect(oddBottomMargin,  SIGNAL(valueChanged(double)), SLOT(obmChanged(double)));
      connect(oddLeftMargin,    SIGNAL(valueChanged(double)), SLOT(olmChanged(double)));
      connect(oddRightMargin,   SIGNAL(valueChanged(double)), SLOT(ormChanged(double)));
      connect(evenTopMargin,    SIGNAL(valueChanged(double)), SLOT(etmChanged(double)));
      connect(evenBottomMargin, SIGNAL(valueChanged(double)), SLOT(ebmChanged(double)));
      connect(evenRightMargin,  SIGNAL(valueChanged(double)), SLOT(ermChanged(double)));
      connect(evenLeftMargin,   SIGNAL(valueChanged(double)), SLOT(elmChanged(double)));
      connect(typesList,        SIGNAL(currentIndexChanged(int)), SLOT(typeChanged(int)));
      connect(sizesList,        SIGNAL(currentIndexChanged(int)), SLOT(sizeChanged()));
      connect(unitsList,        SIGNAL(currentIndexChanged(int)), SLOT(unitsChanged()));
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
      clonedScore->setLayoutMode(LayoutMode::PAGE);

      clonedScore->doLayout();
      preview->setScore(clonedScore);
      applyToParts->setEnabled(!cs->isMaster());
      updateWidgets();
      updatePreview();
      }

//---------------------------------------------------------
//   blockSignals - helper for updateWidgets
//---------------------------------------------------------

void PageSettings::blockSignals(bool block)
      {
      for (auto w : { oddTopMargin, oddBottomMargin, oddLeftMargin, oddRightMargin,
                     evenTopMargin,evenBottomMargin,evenLeftMargin,evenRightMargin,
                     staffSpace } )
            {
            w->blockSignals(block);
            }
      twosided->blockSignals(block);
      typesList->blockSignals(block);
      sizesList->blockSignals(block);
      unitsList->blockSignals(block);
      portrait->blockSignals(block);
      landscape->blockSignals(block);
      pageOffset->blockSignals(block);
      }

//---------------------------------------------------------
//   getPaperType()
//---------------------------------------------------------

PaperType PageSettings::getPaperType(int id)
      {
      for (auto i = MScore::sizesCommon.begin(); i != MScore::sizesCommon.end(); ++i) {
            if (*i == id)
                  return PaperType::Common;
            }
      if (MScore::sizesMetric.find(id) != MScore::sizesMetric.end())
            return PaperType::Metric;
      else if (MScore::sizesImperial.find(id) != MScore::sizesImperial.end())
            return PaperType::Imperial;
      else if (MScore::sizesOther.find(id) != MScore::sizesOther.end())
            return PaperType::Other;
      else
            return PaperType::NOTYPE; // it's a Custom page size
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview()
      {
      preview->score()->doLayout();
      preview->layoutChanged();
      }

//---------------------------------------------------------
//   updateWidgets
//    set widget values from preview->score()
//---------------------------------------------------------

void PageSettings::updateWidgets(bool onlyUnits)
      {
      blockSignals(true);
      Score* score = preview->score();
      MPageLayout& odd  = score->style().pageOdd();
      MPageLayout& even = score->style().pageEven();
      bool isGlobal = preferences.getBool(PREF_APP_PAGE_UNITS_GLOBAL);
      int idxUnit;

      if (isGlobal) { // .setUnits() must precede the setting of widget values
            idxUnit = preferences.getInt(PREF_APP_PAGE_UNITS_VALUE);
            odd.setUnits(QPageLayout::Unit(idxUnit));  // handles conversions
            even.setUnits(QPageLayout::Unit(idxUnit)); // for widget display.
            }
      else
            idxUnit = int(odd.units());

      unitsList->setCurrentIndex(idxUnit);
      unitsGroup->setVisible(!isGlobal);

      if (!onlyUnits) { // if only units are changing, these widgets are unaffected
            PaperType type;
            QPageSize::PageSizeId psid = score->style().pageSize().id();
            if (psid == QPageSize::Custom) {
                  type = PaperType::Common;        // gotta choose one, Custom is any type
                  if (odd.width() > odd.height())
                        landscape->setChecked(true);
                  else
                        portrait ->setChecked(true);
                }
            else {
                  type = getPaperType(int(psid));
                  if (type == PaperType::NOTYPE) { // fallback, in case a psid becomes invalid
                        type = PaperType::Common;
                        psid = QPageSize::Custom;
                        }
                  if (odd.orientation() == QPageLayout::Portrait)
                        portrait ->setChecked(true);
                  else
                        landscape->setChecked(true);
                  }
            typesList->setCurrentIndex(int(type)); // typesList blocks signals - cleaner that way
            typeChanged(int(type), false);         // typeChanged() loads sizesList
            sizesList->setCurrentIndex(sizesList->findData(int(psid)));

            bool is2 = score->styleB(Sid::pageTwosided);
            twosided->setChecked(is2);
            for (auto w : { evenTopMargin, evenBottomMargin, evenLeftMargin, evenRightMargin })
                  w->setEnabled(is2);

            pageOffset->setValue(score->pageNumberOffset() + 1);
            }

      PageUnit unit = pageUnits[idxUnit];
      int decimals = unit.decimals();
      bool isLessPrecise = (decimals < pageWidth->decimals());
      if (!isLessPrecise) // if decimals is increasing or maintaining, update now
            updateDecimals(score, &unit, decimals);

      double max = unit.max(); // same for maximum
      bool isSmaller = (max < pageWidth->maximum());
      if (!isSmaller)
            updateMaximum(max);

      // Set the margin and width/height values, all QDoubleSpinBox widgets
      QMarginsF marg = odd.margins();
      oddTopMargin->setValue(marg.top());
      oddBottomMargin->setValue(marg.bottom());
      oddLeftMargin->setValue(marg.left());
      oddRightMargin->setValue(marg.right());

      marg = even.margins();
      evenTopMargin->setValue(marg.top());
      evenBottomMargin->setValue(marg.bottom());
      evenLeftMargin->setValue(marg.left());
      evenRightMargin->setValue(marg.right());

      updateWidthHeight(odd.fullRect()); // blocks signals for the width and height widgets

      if (isLessPrecise) // if decimals is decreasing, update last
          updateDecimals(score, &unit, decimals);

      if (!isSmaller)    // if maximum is decreasing, update last
            updateMaximum(max);

      blockSignals(false);
      }

//---------------------------------------------------------
//   updateDecimals
//   - updates margin and width/height widget decimal property (and others)
//   - decimals can truncate, causing numerical errors
//   - called only by updateWidgets()
//---------------------------------------------------------
void PageSettings::updateDecimals(Score* score, PageUnit *unit, int decimals)
      {
      double step = unit->step();
      const char* suffix = unit->suffix();

      for (auto w : { oddTopMargin, oddBottomMargin, oddLeftMargin, oddRightMargin,
                     evenTopMargin,evenBottomMargin,evenLeftMargin,evenRightMargin,
                     pageWidth, pageHeight } )
            {
            w->setDecimals(decimals);
            w->setSuffix(suffix);
            w->setSingleStep(step);
            }
      staffSpace->setSingleStep(unit->stepSpatium());
      staffSpace->setSuffix(suffix);
      staffSpace->setValue(score->spatium() / unit->paintFactor());
      }

//---------------------------------------------------------
//   updateMaximum
//   - updates max page width and height values
//   - max values can truncate, causing numerical errors
//   - max values prevent int overflows in pageFullWidth/Height styles
//   - called only by updateWidgets()
//---------------------------------------------------------
void PageSettings::updateMaximum(double max)
      {
      pageWidth->setMaximum(max);
      pageHeight->setMaximum(max);
      }

//---------------------------------------------------------
//   updateWidthHeight - updates Width and Height widgets
//---------------------------------------------------------

void PageSettings::updateWidthHeight(const QRectF& rect)
      {
      pageWidth ->blockSignals(true);
      pageHeight->blockSignals(true);
      pageWidth ->setValue(rect.width());
      pageHeight->setValue(rect.height());
      pageWidth ->blockSignals(false);
      pageHeight->blockSignals(false);
      }

//---------------------------------------------------------
//   typeChanged - populates sizesList based on typesList
//         isSignal == false when called by updateWidgets
//---------------------------------------------------------

void PageSettings::typeChanged(int idx, bool isSignal)
      {
      std::set<int>* sizes;
      switch (PaperType(idx)) {
            case PaperType::Common:
                  sizes = 0;
                  break;
            case PaperType::Metric:
                  sizes = &MScore::sizesMetric;
                  break;
            case PaperType::Imperial:
                  sizes = &MScore::sizesImperial;
                  break;
            case PaperType::Other:
                  sizes = &MScore::sizesOther;
                  break;
            default:
                  Q_ASSERT_X(false, "PageSettings::typeChanged", "PaperType not set!");
                  return;
            }
      sizesList->clear(); // Custom is at the top of every list
      sizesList->addItem(QPageSize::name(QPageSize::Custom), int(QPageSize::Custom));
      if (sizes) {
            for (auto i = sizes->begin(); i != sizes->end(); ++i)
                  sizesList->addItem(QPageSize::name(QPageSize::PageSizeId(*i)), *i);
            }
      else { // Common is a vector
            for (auto i = MScore::sizesCommon.begin(); i != MScore::sizesCommon.end(); ++i)
                  sizesList->addItem(QPageSize::name(QPageSize::PageSizeId(*i)), *i);
            }

      // Find the current page size's index in the new list of sizes, or use
      // Custom (= 0). Even if it's a preset size, if it doesn't exist in this
      // type's list, then the list must display Custom. But if the currentIndex
      // is Custom, then maybe it's a preset size in this new type's list.
      if (isSignal) {
            int id = sizesList->currentData().toInt(); // current PageSizeId as int
            if (QPageSize::PageSizeId(id) != QPageSize::Custom)
                  sizesList->setCurrentIndex(qMax(0, sizesList->findData(id)));
            else // try to find the page size by the width and height
                  widthHeightChanged(pageWidth->value(), pageHeight->value(), true);
            }
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void PageSettings::sizeChanged()
      {
      QPageSize::PageSizeId psid = QPageSize::PageSizeId(sizesList->currentData().toInt());
      Score* score = preview->score();
      MStyle& style = score->style();
      QPageSize::Unit unit = QPageSize::Unit(style.pageOdd().units());
      QPageSize qps;

      bool isPreset = (psid != QPageSize::Custom);
      if (isPreset)
            qps = QPageSize(psid);
      else
            qps = QPageSize(QSizeF(pageWidth->value(), pageHeight->value()),
                            unit,
                            QPageSize::name(psid),
                            QPageSize::ExactMatch);

      style.setPageSize(qps);
      style.pageOdd ().setPageSize(qps);
      style.pageEven().setPageSize(qps);

      if (isPreset) { // switching to Custom doesn't change the actual page size
            QRectF rect = qps.rect(unit);
            updateWidthHeight(rect);
            updatePreview();
            }
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      preview->score()->style().set(Sid::pageTwosided, flag);

      for (auto w : { evenTopMargin, evenBottomMargin, evenLeftMargin, evenRightMargin })
            w->setEnabled(flag);

      evenLeftMargin ->blockSignals(true);
      evenRightMargin->blockSignals(true);
      if (flag) {
            evenLeftMargin ->setValue(oddRightMargin ->value());
            evenRightMargin->setValue(oddLeftMargin->value());
            }
      else {
            evenLeftMargin ->setValue(oddLeftMargin ->value());
            evenRightMargin->setValue(oddRightMargin->value());
            }
      evenLeftMargin ->blockSignals(false);
      evenRightMargin->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   widthChanged, heightChanged, and widthHeightChanged
//    manually changing width/height
//---------------------------------------------------------

void PageSettings::widthChanged(double val)
      {
      widthHeightChanged(val, pageHeight->value(), false);
      }
void PageSettings::heightChanged(double val)
      {
      widthHeightChanged(pageWidth->value(), val, false);
      }
void PageSettings::widthHeightChanged(double w, double h, bool byType)
      {
      // QPageSize::FuzzyMatch is +/-3pt. Now that there is a pageSize XML
      // tag, there is no need for fuzzy matches except reading older scores.
      // Using exact matches here is cleaner for custom sizes. It's a hassle
      // to check both orientations for a match, but the results are clean.
      // QPageSize rounds inches and millimeters to 2 decimals, and points to
      // whole integers. Exact match is only that exact, which is good. For
      // example: using inches, entering 8.5 x 10.83 selects Quarto page size.
      MStyle& style = preview->score()->style();
      QSizeF size = QSizeF(w, h);
      // Is the new page size custom or preset? MuseScore uses only a subset of
      // the QPageSize::PageSizeId enum. Extra validation via getPaperType().
      PaperType type = PaperType::NOTYPE;
      QPageSize::Unit unit = QPageSize::Unit(unitsList->currentIndex());
      QPageSize::PageSizeId psid = QPageSize::id(size, unit, QPageSize::ExactMatch);
      if (psid != QPageSize::Custom) {
            type = getPaperType(int(psid));
            if (type == PaperType::NOTYPE)
                  psid = QPageSize::Custom;
            else if (!portrait->isChecked()) {
                  portrait->blockSignals(true);
                  portrait->setChecked(true);
                  portrait->blockSignals(false);
                  }
            }
      else {
            size.transpose(); // try to match it in Landscape orientation
            psid = QPageSize::id(size, unit, QPageSize::ExactMatch);
            if (psid != QPageSize::Custom) {
                  type = getPaperType(int(psid));
                  if (type == PaperType::NOTYPE)
                        psid = QPageSize::Custom;
                  else if (!landscape->isChecked()) {
                        landscape->blockSignals(true);
                        landscape->setChecked(true);
                        landscape->blockSignals(false);
                        }
                  }
            if (psid == QPageSize::Custom)
                  size.transpose(); // revert to Portrait for use below
            }

      int idx = sizesList->findData(int(psid));
      if (idx < 0) { // psid is a preset in another type's list.
            if (byType) { // indicates that typesList.currentIndex must not change
                  psid = QPageSize::Custom;
                  size = QSizeF(w, h); // reset it, in case it was transposed
            }
            else
                  typesList->setCurrentIndex(int(type)); // populates sizesList

            idx = sizesList->findData(int(psid));
            }

      // Set the list index, if it has changed
      if (idx != sizesList->currentIndex()) {
            sizesList->blockSignals(true);
            sizesList->setCurrentIndex(idx);
            sizesList->blockSignals(false);
            }

      // Update the style
      QPageSize qps;
      if (psid != QPageSize::Custom)
            qps = QPageSize(psid);
      else // Custom always portrait until orientation is stored in the .mscx file
            qps = QPageSize(size, unit, QPageSize::name(psid), QPageSize::ExactMatch);

      style.setPageSize(qps);
      style.pageOdd ().setPageSize(qps);
      style.pageEven().setPageSize(qps);

      updatePreview();
      }

//---------------------------------------------------------
//   orientationToggled
//    a pair of radio buttons that toggle the page orientation
//    it swaps width and height values, but not margins.
//---------------------------------------------------------
void PageSettings::orientationToggled(bool)
      {
      MStyle& style = preview->score()->style();
      if (style.pageSize().id() != QPageSize::Custom) {
            // Until orientation is stored in the .mscx file, custom page sizes
            // are always portrait, and toggling these radio buttons only flips
            // the width and height. Orientation is meaningless for custom sizes,
            // as it has always been.  It's based only on width > height, and it
            // only exists inside this dialog.
            QPageLayout::Orientation orient = portrait->isChecked()
                                            ? QPageLayout::Portrait
                                            : QPageLayout::Landscape;
            style.pageOdd ().setOrientation(orient);
            style.pageEven().setOrientation(orient);
            }
      updateWidthHeight(style.pageOdd().fullRect());
      updatePreview();
      }

//// Margins /////////////////////////////////////////////
// this code could be much more compact if the signals provided a "this",
// the element, the spin box. There might be a way to do that in Qt with events.
//---------------------------------------------------------
//   marginMinMax - helper for all 8 margins' signals, handles out of range error
//---------------------------------------------------------
double PageSettings::marginMinMax(double val, double max, QDoubleSpinBox* spinner)
      {
      double minMax = val < 0 ? 0 : max;
      spinner->blockSignals(true);
      spinner->setValue(minMax);
      spinner->blockSignals(false);
      return minMax;
      }
//// top and bottom first, they have no odd/even synchronzation
//---------------------------------------------------------
//   otmChanged - odd top margin
//---------------------------------------------------------

void PageSettings::otmChanged(double val)
      {
      MPageLayout& layout = preview->score()->style().pageOdd();
      if (!layout.setTopMargin(val)) // only error is out of range
            layout.setTopMargin(marginMinMax(val, layout.maximumMargins().top(), oddTopMargin));
      updatePreview();
      }

//---------------------------------------------------------
//   obmChanged - odd bottom margin
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      MPageLayout& layout = preview->score()->style().pageOdd();
      if (!layout.setBottomMargin(val)) // only error is out of range
            layout.setTopMargin(marginMinMax(val, layout.maximumMargins().bottom(), oddBottomMargin));
      updatePreview();
}

//---------------------------------------------------------
//   etmChanged - even top margin
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      MPageLayout& layout = preview->score()->style().pageEven();
      if (!layout.setTopMargin(val)) // only error is out of range
            layout.setTopMargin(marginMinMax(val, layout.maximumMargins().top(), evenTopMargin));
      updatePreview();
      }

//---------------------------------------------------------
//   ebmChanged - even bottom margin
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      MPageLayout& layout = preview->score()->style().pageEven();
      if (!layout.setBottomMargin(val)) // only error is out of range
            layout.setTopMargin(marginMinMax(val, layout.maximumMargins().bottom(), evenBottomMargin));
      updatePreview();
      }

//// Left and right margins synchronize across odd/even pages
//---------------------------------------------------------
//   lrMargins - helper for the left/right margin valueChanged signals
//---------------------------------------------------------
void PageSettings::lrMargins(double val, bool isLeft, bool isOdd, QDoubleSpinBox* spinOne)
      {
      MStyle& style = preview->score()->style();
      MPageLayout& one   = isOdd ? style.pageOdd()  : style.pageEven();
      MPageLayout& other = isOdd ? style.pageEven() : style.pageOdd();
      QDoubleSpinBox* spinOther;

      bool b = isLeft ? one.setLeftMargin(val) : one.setRightMargin(val);
      if (!b) {                    // val exceeds the max
            double marg = isLeft ? one.maximumMargins().left()
                                 : one.maximumMargins().right();
            val = marginMinMax(val, marg, spinOne);
            if (isLeft)
                  one.setLeftMargin(val);
            else
                  one.setRightMargin(val);
            }

      if (isLeft) {
            other.setRightMargin(val);
            if (twosided->isChecked())
                  spinOther = isOdd ? evenRightMargin : oddRightMargin;
            else
                  spinOther = evenLeftMargin; // even left margin is disabled
            }
      else {
            other.setLeftMargin(val);
            if (twosided->isChecked())
                  spinOther = isOdd ? evenLeftMargin  : oddLeftMargin;
            else
                  spinOther = evenRightMargin; // even right margin is disabled
            }
      spinOther->blockSignals(true);
      spinOther->setValue(val);
      spinOther->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   olmChanged - odd left margin
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      lrMargins(val, true, true, oddLeftMargin);
      }

//---------------------------------------------------------
//   ormChanged - odd right margin
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      lrMargins(val, false, true, oddRightMargin);
      }

//---------------------------------------------------------
//   elmChanged - even left margin
//---------------------------------------------------------

void PageSettings::elmChanged(double val)
      {
      lrMargins(val, true, false, evenLeftMargin);
      }

//---------------------------------------------------------
//   ermChanged - even right margin
//---------------------------------------------------------

void PageSettings::ermChanged(double val)
      {
      lrMargins(val, false, false, evenRightMargin);
      }
//// end margins ////
//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void PageSettings::spatiumChanged(double val)
      {
      Score* score  = preview->score();
      double oldVal = score->spatium();
      double newVal;
      QPageLayout::Unit unit = score->style().pageOdd().units();

      if ((unit == QPageLayout::Millimeter && val == sp20_mm) ||
          (unit == QPageLayout::Inch       && val == sp20_in) ||
          (unit == QPageLayout::Pica       && val == sp20_p)  ||
          (unit == QPageLayout::Didot      && val == sp20_dd) ||
          (unit == QPageLayout::Cicero     && val == sp20_c))
            newVal = SPATIUM20; // force the default value
      else
            newVal = val * pageUnits[int(unit)].paintFactor();

      score->setSpatium(newVal);
      score->spatiumChanged(oldVal, newVal);
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
//   unitsChanged
//---------------------------------------------------------

void PageSettings::unitsChanged()
      {
      QPageLayout::Unit u = QPageLayout::Unit(unitsList->currentIndex());
      MStyle& style = preview->score()->style();
      style.pageOdd().setUnits(u);
      style.pageEven().setUnits(u);
      updateWidgets(true);
      }

///////////////////// Buttons /////////////////////////////
//---------------------------------------------------------
//   setToDefault
//---------------------------------------------------------

void PageSettings::setToDefault()
      {
      MStyle& style = preview->score()->style();
      MStyle& def   = MScore::defaultStyle();
      style.setPageOdd(def.pageOdd());
      style.setPageEven(def.pageEven());
      style.setPageSize(def.pageSize());
      updateWidgets();
      updatePreview();
      }

//---------------------------------------------------------
//   applyToAllParts
//---------------------------------------------------------

void PageSettings::applyToAllParts()
      {
      cs->startCmd();
      for (Excerpt* e : cs->excerpts())
            applyToScore(e->partScore(), false);
      cs->endCmd();
      }

//---------------------------------------------------------
//   applyToScore
//---------------------------------------------------------

void PageSettings::applyToScore(Score* score, bool runCmd)
      {
      Score* prev = preview->score();
      QPageSize& psize = prev->style().pageSize();
      MPageLayout& odd = prev->style().pageOdd();
      MPageLayout& even = prev->style().pageEven();

      // Has anything changed? This prevents extra undo commands on the stack
      // when the user clicks Apply or OK with no changes.
      if (score->styleB(Sid::pageTwosided) == twosided->isChecked() &&
          score->pageNumberOffset()        == prev->pageNumberOffset() &&
          score->spatium()                 == prev->spatium() &&
          score->style().pageSize().id()   == prev->style().pageSize().id())
            {
            MPageLayout& sOdd  = score->style().pageOdd();
            MPageLayout& sEven = score->style().pageEven();
            if (sOdd.units()             == odd.units() &&
                sOdd.orientation()       == odd.orientation() &&
                sOdd.fullRect().width()  == odd.fullRect().width() &&
                sOdd.fullRect().height() == odd.fullRect().height() &&
                sOdd.margins().left()    == odd.margins().left() &&
                sOdd.margins().right()   == odd.margins().right() &&
                sOdd.margins().top()     == odd.margins().top() &&
                sOdd.margins().bottom()  == odd.margins().bottom() &&
                sEven.margins().top()    == even.margins().top() &&
                sEven.margins().bottom() == even.margins().bottom())
                  {
                  return; // nothing has changed
                  }
            }

      if (runCmd)
            score->startCmd();
      score->undoChangePageNumberOffset(pageOffset->value() - 1); // why isn't this a style?
      score->undoChangePageSettings(psize, odd, even);
      score->undoChangeStyleVal(Sid::spatium,      prev->spatium());
      score->undoChangeStyleVal(Sid::pageTwosided, twosided->isChecked());
      if (runCmd)
            score->endCmd();
      }

//---------------------------------------------------------
//   okCancel
//---------------------------------------------------------

void PageSettings::okCancel(QAbstractButton* b)
      {
      switch (buttonBox->standardButton(b)) {
            case QDialogButtonBox::Apply:
                  applyToScore(cs);
                  break;
            case QDialogButtonBox::Ok:
                  applyToScore(cs);
                  done(0);
                  break;
            case QDialogButtonBox::Cancel:
                  done(0);
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   done
//---------------------------------------------------------

void PageSettings::done(int val)
      {
      cs->setLayoutAll();     // HACK
      QDialog::done(val);
      }
}
