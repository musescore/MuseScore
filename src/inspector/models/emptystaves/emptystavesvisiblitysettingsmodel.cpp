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
#include "engraving/rendering/score/systemlayout.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace mu::engraving;
using mu::engraving::rendering::score::SystemLayout;

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

bool EmptyStavesVisibilitySettingsModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    return true;
}

void EmptyStavesVisibilitySettingsModel::loadProperties()
{
    updateCanHideEmptyStavesInSelection();
    updateCanShowAllEmptyStaves();
    updateCanResetEmptyStavesVisibility();
}

void EmptyStavesVisibilitySettingsModel::onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&)
{
    loadProperties();
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

    // avoid `beginCommand` because it sets `m_shouldUpdateOnScoreChange`,
    // expecting that we'll update the model ourselves, but we prefer to rely
    // on the automatic updates, so that we follow the same path when a change
    // occurs from inside this model or outside this model
    undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Hide empty staves"));

    for (System* system : systems) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            score->cmdSetHideStaffIfEmptyOverride(staffIdx, system, AutoOnOff::ON);
        }
    }

    undoStack()->commitChanges();
}

void EmptyStavesVisibilitySettingsModel::showAllEmptyStaves()
{
    if (isEmpty()) {
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    // avoid `beginCommand` because it sets `m_shouldUpdateOnScoreChange`
    undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Show empty staves"));

    for (System* system : systems) {
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            score->cmdSetHideStaffIfEmptyOverride(staffIdx, system, engraving::AutoOnOff::OFF);
        }
    }

    undoStack()->commitChanges();
}

void EmptyStavesVisibilitySettingsModel::resetEmptyStavesVisibility()
{
    if (isEmpty()) {
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    // avoid `beginCommand` because it sets `m_shouldUpdateOnScoreChange`
    undoStack()->prepareChanges(muse::TranslatableString("undoableAction", "Reset empty staves visibility"));

    for (System* system : systems) {
        // Apply to all staves in the score, not just selected ones
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            score->cmdSetHideStaffIfEmptyOverride(staffIdx, system, engraving::AutoOnOff::AUTO);
        }
    }

    undoStack()->commitChanges();
}

void EmptyStavesVisibilitySettingsModel::updateCanHideEmptyStavesInSelection()
{
    auto set = [this] (bool can) {
        if (m_canHideEmptyStavesInSelection == can) {
            return;
        }
        m_canHideEmptyStavesInSelection = can;
        emit canHideEmptyStavesInSelectionChanged();
    };

    if (isEmpty()) {
        set(false);
        return;
    }

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    const INotationSelectionRangePtr range = sel->range();
    const staff_idx_t staffStart = range->startStaffIndex();
    const staff_idx_t staffEnd = range->endStaffIndex();

    for (const System* system : systems) {
        for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            if (system->staff(staffIdx)->show()
                && SystemLayout::canChangeSysStaffVisibility(system, staffIdx)) {
                set(true);
                return;
            }
        }
    }

    set(false);
}

void EmptyStavesVisibilitySettingsModel::updateCanShowAllEmptyStaves()
{
    auto set = [this] (bool can) {
        if (m_canShowAllEmptyStaves == can) {
            return;
        }
        m_canShowAllEmptyStaves = can;
        emit canShowAllEmptyStavesChanged();
    };

    if (isEmpty()) {
        set(false);
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    for (const System* system : systems) {
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            if (!system->staff(staffIdx)->show()
                && SystemLayout::canChangeSysStaffVisibility(system, staffIdx)) {
                set(true);
                return;
            }
        }
    }

    set(false);
}

void EmptyStavesVisibilitySettingsModel::updateCanResetEmptyStavesVisibility()
{
    auto set = [this] (bool can) {
        if (m_canResetEmptyStavesVisibility == can) {
            return;
        }
        m_canResetEmptyStavesVisibility = can;
        emit canResetEmptyStavesVisibilityChanged();
    };

    if (isEmpty()) {
        set(false);
        return;
    }

    Score* score = currentNotation()->elements()->msScore();

    const INotationSelectionPtr sel = selection();
    const std::vector<System*> systems = sel->selectedSystems();

    for (const System* system : systems) {
        for (const MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                if (toMeasure(mb)->hideStaffIfEmpty(staffIdx) != engraving::AutoOnOff::AUTO) {
                    set(true);
                    return;
                }
            }
        }
    }

    set(false);
}
