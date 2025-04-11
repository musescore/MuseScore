/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QKeyEvent>
#include <QPageSize>

#include "engraving/dom/page.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/excerpt.h"
#include "engraving/style/pagestyle.h"

#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace muse::ui;
using namespace mu::engraving;

PageSettings::PageSettings(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
{
    setObjectName("PageSettings");
    setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    mmUnit = configuration()->metricUnit();
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

    connect(mmButton,        &QRadioButton::clicked, this, &PageSettings::mmClicked);
    connect(inchButton,      &QRadioButton::clicked, this, &PageSettings::inchClicked);
    connect(buttonBox,       &QDialogButtonBox::clicked, this, &PageSettings::buttonBoxClicked);
    connect(buttonApplyToAllParts, &QPushButton::clicked, this, &PageSettings::applyToAllParts);
    connect(portraitButton,  &QRadioButton::clicked, this, &PageSettings::orientationClicked);
    connect(landscapeButton, &QRadioButton::clicked, this, &PageSettings::orientationClicked);
    connect(twosided,        &QCheckBox::toggled,    this, &PageSettings::twosidedToggled);

    connect(pageHeight,           &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::pageHeightChanged);
    connect(pageWidth,            &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::pageWidthChanged);
    connect(oddPageTopMargin,     &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::otmChanged);
    connect(oddPageBottomMargin,  &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::obmChanged);
    connect(oddPageLeftMargin,    &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::olmChanged);
    connect(oddPageRightMargin,   &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::ormChanged);
    connect(evenPageTopMargin,    &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::etmChanged);
    connect(evenPageBottomMargin, &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::ebmChanged);
    connect(evenPageRightMargin,  &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::ermChanged);
    connect(evenPageLeftMargin,   &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::elmChanged);
    connect(spatiumEntry,         &QDoubleSpinBox::valueChanged,
            this,                 &PageSettings::spatiumChanged);

    connect(pageGroup,            &QComboBox::activated,
            this, &PageSettings::pageFormatSelected);
    connect(pageOffsetEntry,      &QSpinBox::valueChanged,
            this, &PageSettings::pageOffsetChanged);
}

void PageSettings::showEvent(QShowEvent* event)
{
    globalContext()->currentNotation()->undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Edit page settings"));
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

void PageSettings::setStyleValue(StyleId styleId, const PropertyValue& newValue) const
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

    QString suffix;
    double singleStepSize;
    double singleStepScale;
    if (mm) {
        suffix = muse::qtrc("global", "mm");
        singleStepSize = 1.0;
        singleStepScale = 0.05;
    } else {
        suffix = muse::qtrc("global", "in", /*disambiguation*/ "abbreviation of inch");
        singleStepSize = 0.05;
        singleStepScale = 0.002;
    }
    for (auto w : { oddPageTopMargin, oddPageBottomMargin, oddPageLeftMargin, oddPageRightMargin, evenPageTopMargin,
                    evenPageBottomMargin, evenPageLeftMargin, evenPageRightMargin, spatiumEntry, pageWidth, pageHeight }) {
        w->setSuffix(suffix);
        w->setSingleStep(singleStepSize);
    }
    spatiumEntry->setSingleStep(singleStepScale);

    double f = mm ? mu::engraving::INCH : 1.0;

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
    spatiumEntry->setValue(score()->style().spatium() * f1);

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
    configuration()->setMetricUnit(false);
    updateValues();
}

void PageSettings::mmClicked()
{
    mmUnit = true;
    configuration()->setMetricUnit(true);
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

    pageOffsetEntry->setValue(1);

    updateValues();
}

void PageSettings::twosidedToggled(bool flag)
{
    setStyleValue(Sid::pageTwosided, flag);
    updateValues();
}

void PageSettings::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        return;
    }
    QDialog::keyPressEvent(event);
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
    globalContext()->currentNotation()->style()->styleChanged().notify();
    QDialog::accept();
}

void PageSettings::reject()
{
    globalContext()->currentNotation()->undoStack()->rollbackChanges();
    globalContext()->currentNotation()->style()->styleChanged().notify();
    QDialog::reject();
}

void PageSettings::applyToScore(Score* s)
{
    double f  = mmUnit ? 1.0 / INCH : 1.0;
    double f1 = mmUnit ? DPMM : DPI;

    std::unordered_map<Sid, PropertyValue> values;
    values.emplace(Sid::pageWidth, pageWidth->value() * f);
    values.emplace(Sid::pageHeight, pageHeight->value() * f);
    values.emplace(Sid::pagePrintableWidth, (pageWidth->value() - oddPageLeftMargin->value() - oddPageRightMargin->value()) * f);
    values.emplace(Sid::pageEvenTopMargin, evenPageTopMargin->value() * f);
    values.emplace(Sid::pageEvenBottomMargin, evenPageBottomMargin->value() * f);
    values.emplace(Sid::pageEvenLeftMargin, evenPageLeftMargin->value() * f);
    values.emplace(Sid::pageOddTopMargin, oddPageTopMargin->value() * f);
    values.emplace(Sid::pageOddBottomMargin, oddPageBottomMargin->value() * f);
    values.emplace(Sid::pageOddLeftMargin, oddPageLeftMargin->value() * f);
    values.emplace(Sid::pageTwosided, twosided->isChecked());
    values.emplace(Sid::spatium, spatiumEntry->value() * f1);

    s->undoChangeStyleValues(std::move(values));
    s->undoChangePageNumberOffset(pageOffsetEntry->value() - 1);
}

void PageSettings::applyToAllParts()
{
    if (!_changeFlag) {
        return;
    }
    for (Excerpt* e : score()->masterScore()->excerpts()) {
        applyToScore(e->excerptScore());
    }
    _changeFlag = false;
}

void PageSettings::pageFormatSelected(int size)
{
    if (size >= 0) {
        bool landscape = landscapeButton->isChecked();
        int id = pageGroup->currentData().toInt();
        QSizeF sz = QPageSize::size(QPageSize::PageSizeId(id), QPageSize::Inch);

        setStyleValue(Sid::pageWidth, landscape ? sz.height() : sz.width());
        setStyleValue(Sid::pageHeight, landscape ? sz.width() : sz.height());

        double f  = mmUnit ? 1.0 / INCH : 1.0;
        setStyleValue(Sid::pagePrintableWidth,
                      (landscape ? sz.height() : sz.width()) - (oddPageLeftMargin->value() + oddPageRightMargin->value()) * f);
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
    setStyleValue(Sid::spatium, val); // this will also call Score::spatiumChanged()
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
