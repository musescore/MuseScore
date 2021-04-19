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

#include "transposedialog.h"

#include "framework/global/widgetstatestore.h"

using namespace mu::notation;

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("TransposeDialog");
    setupUi(this);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(transposeByKey,      &QGroupBox::clicked, this, &TransposeDialog::transposeByKeyToggled);
    connect(transposeByInterval, &QGroupBox::clicked, this, &TransposeDialog::transposeByIntervalToggled);
    connect(chromaticBox,        &QGroupBox::clicked, this, &TransposeDialog::chromaticBoxToggled);
    connect(diatonicBox,         &QGroupBox::clicked, this, &TransposeDialog::diatonicBoxToggled);

    if (selection()->isNone()) {
        interaction()->selectAll();
        m_allSelected = true;
    }

    // TRANSPOSE_TO_KEY and "transpose keys" is only possible if selection state is SelState::RANGE
    bool rangeSelection = selection()->isRange();
    setEnableTransposeKeys(rangeSelection);
    setEnableTransposeToKey(rangeSelection);
    setEnableTransposeChordNames(rangeSelection);
    setKey(firstPitchedStaffKey());

    connect(this, &TransposeDialog::accepted, this, &TransposeDialog::apply);

    WidgetStateStore::restoreGeometry(this);
}

TransposeDialog::TransposeDialog(const TransposeDialog& dialog)
    : TransposeDialog(dialog.parentWidget())
{
}

//---------------------------------------------------------
//   TransposeDialog slots
//---------------------------------------------------------

void TransposeDialog::transposeByKeyToggled(bool val)
{
    transposeByInterval->setChecked(!val);
}

void TransposeDialog::transposeByIntervalToggled(bool val)
{
    transposeByKey->setChecked(!val);
}

void TransposeDialog::chromaticBoxToggled(bool val)
{
    diatonicBox->setChecked(!val);
}

void TransposeDialog::diatonicBoxToggled(bool val)
{
    chromaticBox->setChecked(!val);
}

//---------------------------------------------------------
//   mode
//---------------------------------------------------------

TransposeMode TransposeDialog::mode() const
{
    return chromaticBox->isChecked()
           ? (transposeByKey->isChecked() ? TransposeMode::TO_KEY : TransposeMode::BY_INTERVAL)
           : TransposeMode::DIATONICALLY;
}

//---------------------------------------------------------
//   enableTransposeByKey
//---------------------------------------------------------

void TransposeDialog::setEnableTransposeToKey(bool val)
{
    transposeByKey->setEnabled(val);
    transposeByInterval->setChecked(!val);
    transposeByKey->setChecked(val);
}

//---------------------------------------------------------
//   enableTransposeChordNames
//---------------------------------------------------------

void TransposeDialog::setEnableTransposeChordNames(bool val)
{
    transposeChordNames->setEnabled(val);
    transposeChordNames->setChecked(val);
}

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

TransposeDirection TransposeDialog::direction() const
{
    switch (mode()) {
    case TransposeMode::TO_KEY:
        if (closestKey->isChecked()) {
            return TransposeDirection::CLOSEST;
        }
        return upKey->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    case TransposeMode::BY_INTERVAL:
        return upInterval->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    case TransposeMode::DIATONICALLY:
        return upDiatonic->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
    case TransposeMode::UNKNOWN:
        return TransposeDirection::UNKNOWN;
    }
    return TransposeDirection::UP;
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TransposeDialog::hideEvent(QHideEvent* event)
{
    WidgetStateStore::saveGeometry(this);
    QWidget::hideEvent(event);
}

INotationPtr TransposeDialog::notation() const
{
    return context()->currentNotation();
}

INotationInteractionPtr TransposeDialog::interaction() const
{
    return notation()->interaction();
}

INotationSelectionPtr TransposeDialog::selection() const
{
    return interaction()->selection();
}

void TransposeDialog::apply()
{
    TransposeOptions options;

    options.mode = mode();
    options.direction = direction();
    options.key = transposeKey();
    options.interval = transposeInterval();
    options.needTransposeKeys = getTransposeKeys();
    options.needTransposeChordNames = getTransposeChordNames();
    options.needTransposeDoubleSharpsFlats = useDoubleSharpsFlats();

    interaction()->transpose(options);

    if (m_allSelected) {
        interaction()->clearSelection();
    }
}

Key TransposeDialog::firstPitchedStaffKey() const
{
    int startStaffIdx = 0;
    int endStaffIdx   = 0;
    Fraction startTick = Fraction(0, 1);
    INotationSelectionRangePtr range = selection()->range();

    if (selection()->isRange()) {
        startStaffIdx = range->startStaffIndex();
        endStaffIdx = range->endStaffIndex();
        startTick = range->startTick();
    }

    Key key = Key::C;

    for (const Part* part : notation()->parts()->partList()) {
        for (const Staff* staff : *part->staves()) {
            if (staff->idx() < startStaffIdx || staff->idx() > endStaffIdx) {
                continue;
            }

            if (staff->isPitchedStaff(startTick)) {
                key = staff->key(startTick);
                bool concertPitchEnabled = notation()->style()->styleValue(StyleId::concertPitch).toBool();

                if (!concertPitchEnabled) {
                    int diff = staff->part()->instrument(startTick)->transpose().chromatic;

                    if (diff) {
                        key = Ms::transposeKey(key, diff, staff->part()->preferSharpFlat());
                    }
                }

                break;
            }
        }
    }

    return key;
}

void TransposeDialog::setEnableTransposeKeys(bool val)
{
    transposeKeys->setEnabled(val);
}

bool TransposeDialog::getTransposeKeys() const
{
    return chromaticBox->isChecked()
           ? transposeKeys->isChecked()
           : keepDegreeAlterations->isChecked();
}

bool TransposeDialog::getTransposeChordNames() const
{
    return transposeChordNames->isChecked();
}

Key TransposeDialog::transposeKey() const
{
    return Key(keyList->currentIndex() - 7);
}

int TransposeDialog::transposeInterval() const
{
    return chromaticBox->isChecked()
           ? intervalList->currentIndex()
           : degreeList->currentIndex() + 1;
}

void TransposeDialog::setKey(Key k)
{
    keyList->setCurrentIndex(int(k) + 7);
}

bool TransposeDialog::useDoubleSharpsFlats() const
{
    return accidentalOptions->currentIndex() == 1;
}
