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
#include "emptystavesvisiblitysettingsmodel.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/system.h"
#include "engraving/dom/undo.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace mu::engraving;

EmptyStavesVisibilitySettingsModel::EmptyStavesVisibilitySettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_EMPTY_STAVES);
    setTitle(muse::qtrc("inspector", "Empty staves visibility"));
}

bool EmptyStavesVisibilitySettingsModel::isEmpty() const
{
    INotationSelectionPtr selection = this->selection();
    return !selection || !selection->isRange();
}

void EmptyStavesVisibilitySettingsModel::hideEmptyStavesInSelection()
{
    if (isEmpty()) {
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    const INotationSelectionRangePtr range = sel->range();
    const staff_idx_t staffStart = range->startStaffIndex();
    const staff_idx_t staffEnd = range->endStaffIndex();

    beginCommand(muse::TranslatableString("undoableAction", "Hide empty staves in selection"));

    for (System* system : systems) {
        for (MeasureBase* mb = system->firstMeasure(); mb; mb = system->nextMeasure(mb)) {
            Measure* measure = mb->isMeasure() ? toMeasure(mb) : nullptr;
            if (!measure) {
                continue;
            }

            for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                score->undo(new ChangeMStaffHideIfEmpty(measure, staffIdx, AutoOnOff::ON));
            }
        }
    }

    endCommand();
}

void EmptyStavesVisibilitySettingsModel::showAllEmptyStaves()
{
    if (isEmpty()) {
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    beginCommand(muse::TranslatableString("undoableAction", "Show all empty staves"));

    for (System* system : systems) {
        for (MeasureBase* mb = system->firstMeasure(); mb; mb = system->nextMeasure(mb)) {
            Measure* measure = mb->isMeasure() ? toMeasure(mb) : nullptr;
            if (!measure) {
                continue;
            }

            // Apply to all staves in the score, not just selected ones
            for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                score->undo(new ChangeMStaffHideIfEmpty(measure, staffIdx, AutoOnOff::OFF));
            }
        }
    }

    endCommand();
}

void EmptyStavesVisibilitySettingsModel::resetEmptyStavesVisibility()
{
    if (isEmpty()) {
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    beginCommand(muse::TranslatableString("undoableAction", "Reset empty staves visibility"));

    for (System* system : systems) {
        for (MeasureBase* mb = system->firstMeasure(); mb; mb = system->nextMeasure(mb)) {
            Measure* measure = mb->isMeasure() ? toMeasure(mb) : nullptr;
            if (!measure) {
                continue;
            }

            // Apply to all staves in the score, not just selected ones
            for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                score->undo(new ChangeMStaffHideIfEmpty(measure, staffIdx, AutoOnOff::AUTO));
            }
        }
    }

    endCommand();
}
