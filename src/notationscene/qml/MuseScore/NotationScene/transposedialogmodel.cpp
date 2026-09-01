/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "transposedialogmodel.h"

using namespace mu::notation;

TransposeDialogModel::TransposeDialogModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void TransposeDialogModel::load()
{
    INotationInteractionPtr interaction = context()->currentNotation()->interaction();
    INotationSelectionPtr selection = interaction->selection();

    if (selection->isNone()) {
        interaction->select(SelectionTarget::All);
        m_allSelected = true;
    }

    // TRANSPOSE_TO_KEY and "transpose keys" is only possible if selection state is SelState::RANGE
    bool rangeSelection = selection->isRange();

    m_enableTransposeKeys = rangeSelection;
    emit enableTransposeKeysChanged();

    m_enableTransposeToKey = rangeSelection;
    emit enableTransposeToKeyChanged();

    if (!rangeSelection && m_mode == TransposeMode::TO_KEY) {
        setMode(static_cast<int>(TransposeMode::BY_INTERVAL));
    }

    const std::vector<EngravingItem*>& elements = selection->elements();
    bool hasChordNames = std::any_of(elements.cbegin(), elements.cend(), [](const EngravingItem* item) {
        return item->isHarmony() || item->isFretDiagram();
    });

    m_enableTransposeChordNames = hasChordNames;
    emit enableTransposeChordNamesChanged();

    restorePreviousState();
}

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

int TransposeDialogModel::direction() const { return static_cast<int>(m_direction); }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TransposeDialogModel::apply()
{
    TransposeOptions options;
    options.mode = m_mode;
    options.direction = m_direction;
    options.key = Key(m_keyIndex - 7);
    options.interval = m_mode == TransposeMode::DIATONICALLY ? m_degreeIndex + 1 : m_intervalIndex;
    options.needTransposeKeys = m_mode == TransposeMode::DIATONICALLY ? m_keepDegreeAlterations : m_transposeKeys;
    options.needTransposeChordNames = m_transposeChordNames;
    options.needTransposeDoubleSharpsFlats = m_useDoubleSharpsFlats;

    saveState();

    INotationInteractionPtr interaction = context()->currentNotation()->interaction();
    interaction->transpose(options);

    if (m_allSelected) {
        interaction->clearSelection();
    }
}

//---------------------------------------------------------
//   property getters
//---------------------------------------------------------

int TransposeDialogModel::mode() const { return static_cast<int>(m_mode); }
int TransposeDialogModel::keyIndex() const { return m_keyIndex; }
int TransposeDialogModel::intervalIndex() const { return m_intervalIndex; }
int TransposeDialogModel::degreeIndex() const { return m_degreeIndex; }
bool TransposeDialogModel::keepDegreeAlterations() const { return m_keepDegreeAlterations; }
bool TransposeDialogModel::transposeKeys() const { return m_transposeKeys; }
bool TransposeDialogModel::transposeChordNames() const { return m_transposeChordNames; }
bool TransposeDialogModel::useDoubleSharpsFlats() const { return m_useDoubleSharpsFlats; }

bool TransposeDialogModel::enableTransposeToKey() const { return m_enableTransposeToKey; }
bool TransposeDialogModel::enableTransposeKeys() const { return m_enableTransposeKeys; }
bool TransposeDialogModel::enableTransposeChordNames() const { return m_enableTransposeChordNames; }

//---------------------------------------------------------
//   property setters
//---------------------------------------------------------

void TransposeDialogModel::setMode(int mode)
{
    TransposeMode m = static_cast<TransposeMode>(mode);
    if (m_mode == m) {
        return;
    }
    m_mode = m;
    emit modeChanged();

    if (m_mode != TransposeMode::TO_KEY && m_direction == TransposeDirection::CLOSEST) {
        setDirection(static_cast<int>(TransposeDirection::UP));
    }
}

void TransposeDialogModel::setKeyIndex(int index)
{
    if (m_keyIndex == index) {
        return;
    }
    m_keyIndex = index;
    emit keyIndexChanged();
}

void TransposeDialogModel::setIntervalIndex(int index)
{
    if (m_intervalIndex == index) {
        return;
    }
    m_intervalIndex = index;
    emit intervalIndexChanged();
}

void TransposeDialogModel::setDegreeIndex(int index)
{
    if (m_degreeIndex == index) {
        return;
    }
    m_degreeIndex = index;
    emit degreeIndexChanged();
}

void TransposeDialogModel::setDirection(int direction)
{
    TransposeDirection d = static_cast<TransposeDirection>(direction);
    if (m_direction == d) {
        return;
    }
    m_direction = d;
    emit directionChanged();
}

void TransposeDialogModel::setKeepDegreeAlterations(bool value)
{
    if (m_keepDegreeAlterations == value) {
        return;
    }
    m_keepDegreeAlterations = value;
    emit keepDegreeAlterationsChanged();
}

void TransposeDialogModel::setTransposeKeys(bool value)
{
    if (m_transposeKeys == value) {
        return;
    }
    m_transposeKeys = value;
    emit transposeKeysChanged();
}

void TransposeDialogModel::setTransposeChordNames(bool value)
{
    if (m_transposeChordNames == value) {
        return;
    }
    m_transposeChordNames = value;
    emit transposeChordNamesChanged();
}

void TransposeDialogModel::setUseDoubleSharpsFlats(bool value)
{
    if (m_useDoubleSharpsFlats == value) {
        return;
    }
    m_useDoubleSharpsFlats = value;
    emit useDoubleSharpsFlatsChanged();
}

//---------------------------------------------------------
//   previousState / restorePreviousState / saveState
//---------------------------------------------------------

TransposeDialogState& TransposeDialogModel::previousState()
{
    static TransposeDialogState state;
    return state;
}

void TransposeDialogModel::restorePreviousState()
{
    const TransposeDialogState& state = previousState();

    TransposeMode restoredMode = state.chromaticChecked
                                ? (state.transposeByKeyChecked
                                    && m_enableTransposeToKey ? TransposeMode::TO_KEY : TransposeMode::BY_INTERVAL)
                                : TransposeMode::DIATONICALLY;
    setMode(static_cast<int>(restoredMode));

    setKeyIndex(state.keyListIdx);
    setIntervalIndex(state.chromaticIntervalIdx);
    setDegreeIndex(state.diatonicIntervalIdx);

    setDirection(static_cast<int>(state.direction));

    setTransposeKeys(state.needTransposeKeysChecked);
    setKeepDegreeAlterations(state.keepDegreeAlterationsChecked);
    setTransposeChordNames(state.needTransposeChordNamesChecked);
    setUseDoubleSharpsFlats(state.needTransposeDoubleSharpsFlatsIdx == 1);
}

void TransposeDialogModel::saveState()
{
    TransposeDialogState& state = previousState();

    state.chromaticChecked = m_mode != TransposeMode::DIATONICALLY;
    state.transposeByKeyChecked = m_mode == TransposeMode::TO_KEY;
    state.keyListIdx = m_keyIndex;
    state.chromaticIntervalIdx = m_intervalIndex;
    state.diatonicIntervalIdx = m_degreeIndex;
    state.direction = m_direction;
    state.keepDegreeAlterationsChecked = m_keepDegreeAlterations;
    state.needTransposeKeysChecked = m_transposeKeys;
    state.needTransposeChordNamesChecked = m_transposeChordNames;
    state.needTransposeDoubleSharpsFlatsIdx = m_useDoubleSharpsFlats ? 1 : 0;
}
