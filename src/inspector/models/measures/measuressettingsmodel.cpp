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
    INotationSelectionPtr selection = this->selection();
    return !selection || !selection->isRange();
}

void MeasuresSettingsModel::insertMeasures(int numberOfMeasures, InsertMeasuresTarget target)
{
    actions::ActionData actionData = actions::ActionData::make_arg1(numberOfMeasures);

    switch (target) {
    case InsertMeasuresTarget::AfterSelection:
        dispatcher()->dispatch("insert-measures-after-selection", actionData);
        break;
    case InsertMeasuresTarget::BeforeSelection:
        dispatcher()->dispatch("insert-measures", actionData);
        break;
    case InsertMeasuresTarget::AtStartOfScore:
        dispatcher()->dispatch("insert-measures-at-start-of-score", actionData);
        break;
    case InsertMeasuresTarget::AtEndOfScore:
        dispatcher()->dispatch("append-measures", actionData);
        break;
    }
}

void MeasuresSettingsModel::deleteSelectedMeasures()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->removeSelectedMeasures();
}
