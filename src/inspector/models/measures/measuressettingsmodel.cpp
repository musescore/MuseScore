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
#include "measuressettingsmodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;

MeasuresSettingsModel::MeasuresSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_MEASURES);
    setTitle(qtrc("inspector", "Measure"));
}

bool MeasuresSettingsModel::isEmpty() const
{
    return !currentNotation() || !currentNotation()->interaction()->selection()->isRange();
}

void MeasuresSettingsModel::insertMeasures(int numberOfMeasures, InsertMeasuresTarget target)
{
    if (!currentNotation() || numberOfMeasures < 1) {
        return;
    }

    auto interaction = currentNotation()->interaction();
    auto selection = interaction->selection();
    if (!selection->isRange()) {
        return;
    }

    int beforeIndex = 0;
    switch (target) {
    case InsertMeasuresTarget::AfterSelection:
        beforeIndex = selection->range()->endMeasureIndex() + 1;
        break;
    case InsertMeasuresTarget::BeforeSelection:
        beforeIndex = selection->range()->startMeasureIndex();
        break;
    case InsertMeasuresTarget::AtStartOfScore:
        beforeIndex = INotationInteraction::ADD_BOXES_AT_START_OF_SCORE;
        break;
    case InsertMeasuresTarget::AtEndOfScore:
        beforeIndex = INotationInteraction::ADD_BOXES_AT_END_OF_SCORE;
        break;
    }

    interaction->addBoxes(notation::BoxType::Measure, numberOfMeasures, beforeIndex);
}

void MeasuresSettingsModel::deleteSelectedMeasures()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->removeSelectedMeasures();
}
