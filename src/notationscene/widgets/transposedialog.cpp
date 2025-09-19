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

#include "transposedialog.h"

#include "async/notifylist.h"

#include "notation/internal/mscoreerrorscontroller.h"
#include "ui/view/widgetstatestore.h"

using namespace mu::notation;
using namespace muse::ui;

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
    : QDialog(parent), muse::Injectable(muse::iocCtxForQWidget(this))
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

    const std::vector<EngravingItem*>& elements = selection()->elements();
    bool hasChordNames = std::any_of(elements.cbegin(), elements.cend(), [](const EngravingItem* item) {
        return item->isHarmony() || item->isFretDiagram();
    });
    setEnableTransposeChordNames(hasChordNames);

    restorePreviousState();

    connect(this, &TransposeDialog::accepted, this, &TransposeDialog::apply);

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

//---------------------------------------------------------
//   TransposeDialog slots
//---------------------------------------------------------

void TransposeDialog::transposeByKeyToggled(bool val)
{
    if (val) {
        transposeByInterval->setChecked(false);
    } else {
        if (!transposeByInterval->isChecked()) {
            transposeByKey->setChecked(true);
        }
    }
}

void TransposeDialog::transposeByIntervalToggled(bool val)
{
    if (val) {
        transposeByKey->setChecked(false);
    } else {
        if (!transposeByKey->isChecked()) {
            transposeByInterval->setChecked(true);
        }
    }
}

void TransposeDialog::chromaticBoxToggled(bool val)
{
    if (val) {
        diatonicBox->setChecked(false);
    } else {
        if (!diatonicBox->isChecked()) {
            chromaticBox->setChecked(true);
        }
    }
}

void TransposeDialog::diatonicBoxToggled(bool val)
{
    if (val) {
        chromaticBox->setChecked(false);
    } else {
        if (!chromaticBox->isChecked()) {
            diatonicBox->setChecked(true);
        }
    }
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
//   showEvent
//---------------------------------------------------------

void TransposeDialog::showEvent(QShowEvent* event)
{
    WidgetStateStore::restoreGeometry(this);
    QDialog::showEvent(event);
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

    saveState();

    interaction()->transpose(options);

    MScoreErrorsController(iocContext()).checkAndShowMScoreError();

    if (m_allSelected) {
        interaction()->clearSelection();
    }
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

bool TransposeDialog::useDoubleSharpsFlats() const
{
    return accidentalOptions->currentIndex() == 1;
}

TransposeDialogState& TransposeDialog::previousState()
{
    static TransposeDialogState options;
    return options;
}

void TransposeDialog::restorePreviousState()
{
    const TransposeDialogState& state = previousState();

    chromaticBox->setChecked(state.chromaticChecked);
    diatonicBox->setChecked(!state.chromaticChecked);

    transposeByKey->setChecked(state.transposeByKeyChecked);
    transposeByInterval->setChecked(!state.transposeByKeyChecked);

    keyList->setCurrentIndex(state.keyListIdx);
    closestKey->setChecked(state.closestKeyChecked);
    upKey->setChecked(state.upKeyChecked);
    downKey->setChecked(!state.closestKeyChecked && !state.upKeyChecked);

    intervalList->setCurrentIndex(state.chromaticIntervalIdx);
    upInterval->setChecked(state.upIntervalChecked);
    downInterval->setChecked(!state.upIntervalChecked);

    transposeKeys->setChecked(state.needTransposeKeysChecked);

    degreeList->setCurrentIndex(state.diatonicIntervalIdx);
    upDiatonic->setChecked(state.upDiatonicChecked);
    downDiatonic->setChecked(!state.upDiatonicChecked);
    keepDegreeAlterations->setChecked(state.keepDegreeAlterationsChecked);

    transposeChordNames->setChecked(state.needTransposeChordNamesChecked);
    accidentalOptions->setCurrentIndex(state.needTransposeDoubleSharpsFlatsIdx);
}

void TransposeDialog::saveState()
{
    TransposeDialogState& state = previousState();

    state.chromaticChecked = chromaticBox->isChecked();
    state.transposeByKeyChecked = transposeByKey->isChecked();
    state.closestKeyChecked = closestKey->isChecked();
    state.upKeyChecked = upKey->isChecked();
    state.keyListIdx = keyList->currentIndex();
    state.upIntervalChecked = upInterval->isChecked();
    state.chromaticIntervalIdx = intervalList->currentIndex();
    state.upDiatonicChecked = upDiatonic->isChecked();
    state.diatonicIntervalIdx = degreeList->currentIndex();
    state.keepDegreeAlterationsChecked = keepDegreeAlterations->isChecked();
    state.needTransposeKeysChecked = transposeKeys->isChecked();
    state.needTransposeChordNamesChecked = transposeChordNames->isChecked();
    state.needTransposeDoubleSharpsFlatsIdx = accidentalOptions->currentIndex();
}
