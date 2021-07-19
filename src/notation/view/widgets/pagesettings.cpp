/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "pagesettings.h"

#include <QPageSize>

#include "engraving/style/pagestyle.h"

#include "widgetstatestore.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"
#include "libmscore/excerpt.h"

using namespace mu::notation;
using namespace Ms;

PageSettings::PageSettings(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("PageSettings");
    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    mmUnit = true;        // should be made a global configuration item
    _changeFlag = false;

    if (mmUnit) {
        mmButton->setChecked(true);
    } else {
        inchButton->setChecked(true);
    }

    WidgetStateStore::restoreGeometry(this);

    for (int i = 0; i < QPageSize::LastPageSize; ++i) {
        pageGroup->addItem(QPageSize::name(QPageSize::PageSizeId(i)), i);
    }

    connect(mmButton,             SIGNAL(clicked()),            SLOT(mmClicked()));
    connect(inchButton,           SIGNAL(clicked()),            SLOT(inchClicked()));
    connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
    connect(buttonApplyToAllParts, SIGNAL(clicked()),            SLOT(applyToAllParts()));
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

PageSettings::PageSettings(const PageSettings& other)
    : QDialog(other.parentWidget())
{
}

void PageSettings::showEvent(QShowEvent* event)
{
    globalContext()->currentNotation()->undoStack()->prepareChanges();
    updateValues();
    QWidget::showEvent(event);
}

void PageSettings::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

void PageSettings::blockSignals(bool block)
{
    for (auto w : { oddPageTopMargin, oddPageBottomMargin, oddPageLeftMargin, oddPageRightMargin,
                    evenPageTopMargin, evenPageBottomMargin, evenPageLeftMargin, evenPageRightMargin, spatiumEntry,
                    pageWidth, pageHeight }) {
        w->blockSignals(block);
    }
    pageOffsetEntry->blockSignals(block);
    pageGroup->blockSignals(block);
}

void PageSettings::setMarginsMax(double pw)
{
    oddPageLeftMargin->setMaximum(pw);
    oddPageRightMargin->setMaximum(pw);
    evenPageLeftMargin->setMaximum(pw);
    evenPageRightMargin->setMaximum(pw);
}

Score* PageSettings::score() const
{
    return globalContext()->currentNotation()->elements()->msScore();
}

double PageSettings::styleValueDouble(StyleId styleId) const
{
    return globalContext()->currentNotation()->style()->styleValue(styleId).toDouble();
}

bool PageSettings::styleValueBool(StyleId styleId) const
{
    return globalContext()->currentNotation()->style()->styleValue(styleId).toBool();
}

void PageSettings::setStyleValue(StyleId styleId, const QVariant& newValue) const
{
    globalContext()->currentNotation()->style()->setStyleValue(styleId, newValue);
}

//---------------------------------------------------------
//   updateValues
//    set gui values from style
//---------------------------------------------------------

void PageSettings::updateValues()
{
    bool mm = mmButton->isChecked();

    blockSignals(true);

    const char* suffix;
    double singleStepSize;
    double singleStepScale;
    if (mm) {
        suffix = "mm";
        singleStepSize = 1.0;
        singleStepScale = 0.2;
    } else {
        suffix = "in";
        singleStepSize = 0.05;
        singleStepScale = 0.005;
    }
    for (auto w : { oddPageTopMargin, oddPageBottomMargin, oddPageLeftMargin, oddPageRightMargin, evenPageTopMargin,
                    evenPageBottomMargin, evenPageLeftMargin, evenPageRightMargin, spatiumEntry, pageWidth, pageHeight }) {
        w->setSuffix(suffix);
        w->setSingleStep(singleStepSize);
    }
    spatiumEntry->setSingleStep(singleStepScale);

    double f = mm ? INCH : 1.0;

    double w = styleValueDouble(Sid::pageWidth);
    double h = styleValueDouble(Sid::pageHeight);
    setMarginsMax(w * f);

    double pw = styleValueDouble(Sid::pagePrintableWidth);
    pageGroup->setCurrentIndex(int(QPageSize::id(QSizeF(w, h), QPageSize::Inch, QPageSize::FuzzyOrientationMatch)));

    oddPageTopMargin->setValue(styleValueDouble(Sid::pageOddTopMargin) * f);
    oddPageBottomMargin->setValue(styleValueDouble(Sid::pageOddBottomMargin) * f);
    oddPageLeftMargin->setValue(styleValueDouble(Sid::pageOddLeftMargin) * f);
    oddPageRightMargin->setValue((w - pw - styleValueDouble(Sid::pageOddLeftMargin)) * f);

    evenPageTopMargin->setValue(styleValueDouble(Sid::pageEvenTopMargin) * f);
    evenPageBottomMargin->setValue(styleValueDouble(Sid::pageEvenBottomMargin) * f);
    evenPageLeftMargin->setValue(styleValueDouble(Sid::pageEvenLeftMargin) * f);
    evenPageRightMargin->setValue((w - pw - styleValueDouble(Sid::pageEvenLeftMargin)) * f);
    pageHeight->setValue(h * f);
    pageWidth->setValue(w * f);

    double f1 = mm ? 1.0 / DPMM : 1.0 / DPI;
    spatiumEntry->setValue(score()->spatium() * f1);

    bool _twosided = styleValueBool(Sid::pageTwosided);
    evenPageTopMargin->setEnabled(_twosided);
    evenPageBottomMargin->setEnabled(_twosided);
    evenPageLeftMargin->setEnabled(_twosided);
    evenPageRightMargin->setEnabled(_twosided);

    if (twosided->isChecked()) {
        evenPageRightMargin->setValue(oddPageLeftMargin->value());
        evenPageLeftMargin->setValue(oddPageRightMargin->value());
    } else {
        evenPageRightMargin->setValue(oddPageRightMargin->value());
        evenPageLeftMargin->setValue(oddPageLeftMargin->value());
    }

    landscapeButton->setChecked(styleValueDouble(Sid::pageWidth) > styleValueDouble(Sid::pageHeight));
    portraitButton->setChecked(styleValueDouble(Sid::pageWidth) <= styleValueDouble(Sid::pageHeight));

    twosided->setChecked(_twosided);

    pageOffsetEntry->setValue(score()->pageNumberOffset() + 1);

    blockSignals(false);
    _changeFlag = true;
}

void PageSettings::inchClicked()
{
    mmUnit = false;
    updateValues();
}

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
    qreal w = styleValueDouble(Sid::pageWidth);
    qreal h = styleValueDouble(Sid::pageHeight);

    setStyleValue(Sid::pageWidth, h);
    setStyleValue(Sid::pageHeight, w);

    double f = mmUnit ? 1.0 / INCH : 1.0;
    setStyleValue(Sid::pagePrintableWidth, h - (oddPageLeftMargin->value() + oddPageRightMargin->value()) * f);
    updateValues();
}

void PageSettings::on_resetPageStyleButton_clicked()
{
    for (auto styleId : pageStyles()) {
        globalContext()->currentNotation()->style()->resetStyleValue(styleId);
    }

    updateValues();
}

void PageSettings::twosidedToggled(bool flag)
{
    setStyleValue(Sid::pageTwosided, flag);
    updateValues();
}

void PageSettings::buttonBoxClicked(QAbstractButton* button)
{
    switch (buttonBox->buttonRole(button)) {
    case QDialogButtonBox::AcceptRole:
        accept();
        break;
    case QDialogButtonBox::RejectRole:
        reject();
        break;
    default:
        break;
    }
}

void PageSettings::accept()
{
    globalContext()->currentNotation()->undoStack()->commitChanges();
    QDialog::accept();
}

void PageSettings::reject()
{
    globalContext()->currentNotation()->undoStack()->rollbackChanges();
    QDialog::reject();
}

void PageSettings::applyToScore(Score* s)
{
    double f  = mmUnit ? 1.0 / INCH : 1.0;
    double f1 = mmUnit ? DPMM : DPI;

    s->undoChangeStyleVal(Sid::pageWidth, pageWidth->value() * f);
    s->undoChangeStyleVal(Sid::pageHeight, pageHeight->value() * f);
    s->undoChangeStyleVal(Sid::pagePrintableWidth, (pageWidth->value() - oddPageLeftMargin->value() - oddPageRightMargin->value()) * f);
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

void PageSettings::applyToAllParts()
{
    if (!_changeFlag) {
        return;
    }
    for (Excerpt* e : score()->excerpts()) {
        applyToScore(e->partScore());
    }
    _changeFlag = false;
}

void PageSettings::pageFormatSelected(int size)
{
    if (size >= 0) {
        int id = pageGroup->currentData().toInt();
        QSizeF sz = QPageSize::size(QPageSize::PageSizeId(id), QPageSize::Inch);

        setStyleValue(Sid::pageWidth, sz.width());
        setStyleValue(Sid::pageHeight, sz.height());

        double f  = mmUnit ? 1.0 / INCH : 1.0;
        setStyleValue(Sid::pagePrintableWidth, sz.width() - (oddPageLeftMargin->value() + oddPageRightMargin->value()) * f);
        updateValues();
    }
}

void PageSettings::otmChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }
    setStyleValue(Sid::pageOddTopMargin, val);
}

void PageSettings::olmChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    if (twosided->isChecked()) {
        evenPageRightMargin->blockSignals(true);
        evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
        evenPageRightMargin->blockSignals(false);
    } else {
        evenPageLeftMargin->blockSignals(true);
        evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
        evenPageLeftMargin->blockSignals(false);
    }

    setStyleValue(Sid::pageOddLeftMargin, val);
    setStyleValue(Sid::pagePrintableWidth, styleValueDouble(Sid::pageWidth) - styleValueDouble(Sid::pageEvenLeftMargin) - val);
}

void PageSettings::ormChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    if (twosided->isChecked()) {
        evenPageLeftMargin->blockSignals(true);
        evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
        setStyleValue(Sid::pageEvenLeftMargin, val);
        evenPageLeftMargin->blockSignals(false);
    } else {
        evenPageRightMargin->blockSignals(true);
        evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
        evenPageRightMargin->blockSignals(false);
    }
    setStyleValue(Sid::pagePrintableWidth, styleValueDouble(Sid::pageWidth) - styleValueDouble(Sid::pageOddLeftMargin) - val);
}

void PageSettings::obmChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    setStyleValue(Sid::pageOddBottomMargin, val);
}

void PageSettings::etmChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    setStyleValue(Sid::pageEvenTopMargin, val);
}

void PageSettings::elmChanged(double val)
{
    double f  = mmUnit ? 1.0 / INCH : 1.0;
    val *= f;

    if (twosided->isChecked()) {
        oddPageRightMargin->blockSignals(true);
        oddPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
        oddPageRightMargin->blockSignals(false);
    }

    setStyleValue(Sid::pageEvenLeftMargin, val);
    setStyleValue(Sid::pagePrintableWidth, styleValueDouble(Sid::pageWidth) - evenPageRightMargin->value() * f - val);
}

void PageSettings::ermChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    if (twosided->isChecked()) {
        oddPageLeftMargin->blockSignals(true);
        oddPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
        oddPageLeftMargin->blockSignals(false);
    }

    setStyleValue(Sid::pagePrintableWidth, styleValueDouble(Sid::pageWidth) - styleValueDouble(Sid::pageEvenLeftMargin) - val);
    setStyleValue(Sid::pageOddLeftMargin, val);
}

void PageSettings::ebmChanged(double val)
{
    if (mmUnit) {
        val /= INCH;
    }

    setStyleValue(Sid::pageEvenBottomMargin, val);
}

void PageSettings::spatiumChanged(double val)
{
    val *= mmUnit ? DPMM : DPI;
    double oldVal = score()->spatium();
    setStyleValue(Sid::spatium, val);
    score()->spatiumChanged(oldVal, val);
}

void PageSettings::pageOffsetChanged(int val)
{
    // TODO: Cancel does not work when page offset is changed?
    score()->setPageNumberOffset(val - 1);
}

void PageSettings::pageHeightChanged(double val)
{
    double val2 = pageWidth->value();
    if (mmUnit) {
        val /= INCH;
        val2 /= INCH;
    }
    pageGroup->setCurrentIndex(0);        // custom

    setStyleValue(Sid::pageHeight, val);
    setStyleValue(Sid::pageWidth, val2);
}

void PageSettings::pageWidthChanged(double val)
{
    setMarginsMax(val);

    double f    = mmUnit ? 1.0 / INCH : 1.0;
    double val2 = pageHeight->value() * f;
    val        *= f;
    pageGroup->setCurrentIndex(0);        // custom

    setStyleValue(Sid::pageWidth, val);
    setStyleValue(Sid::pageHeight, val2);
    setStyleValue(Sid::pagePrintableWidth, val - (oddPageLeftMargin->value() + evenPageLeftMargin->value()) * f);
}
