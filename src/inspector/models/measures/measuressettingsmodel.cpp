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
#include "measuressettingsmodel.h"

#include "dom/score.h"

#include "translation.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace muse::actions;
using namespace mu::engraving;

MeasuresSettingsModel::MeasuresSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setSectionType(InspectorSectionType::SECTION_MEASURES);
    setTitle(muse::qtrc("inspector", "Measures"));
}

void MeasuresSettingsModel::loadProperties()
{
    updateAllSystemsAreLocked();
    updateScoreIsInPageView();
    updateIsMakeIntoSystemAvailable();
}

void MeasuresSettingsModel::onCurrentNotationChanged()
{
    INotationPtr notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->undoStack()->changesChannel().onReceive(this, [this](const ChangesRange&) {
        onNotationChanged({}, {});
    });

    AbstractInspectorModel::onCurrentNotationChanged();
}

void MeasuresSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet&)
{
    loadProperties();
}

bool MeasuresSettingsModel::isEmpty() const
{
    INotationSelectionPtr selection = this->selection();
    return !selection || !selection->isRange();
}

void MeasuresSettingsModel::insertMeasures(int numberOfMeasures, InsertMeasuresTarget target)
{
    ActionData actionData = ActionData::make_arg1(numberOfMeasures);

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

void MeasuresSettingsModel::moveMeasureUp()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToPrevSystem();
}

QString MeasuresSettingsModel::shortcutMoveMeasureUp() const
{
    return shortcutsForActionCode("move-measure-to-prev-system");
}

void MeasuresSettingsModel::moveMeasureDown()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToNextSystem();
}

QString MeasuresSettingsModel::shortcutMoveMeasureDown() const
{
    return shortcutsForActionCode("move-measure-to-next-system");
}

void MeasuresSettingsModel::toggleSystemLock()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->toggleSystemLock();
    updateAllSystemsAreLocked();
}

QString MeasuresSettingsModel::shortcutToggleSystemLock() const
{
    return shortcutsForActionCode("toggle-system-lock");
}

bool MeasuresSettingsModel::allSystemsAreLocked() const
{
    return m_allSystemsAreLocked;
}

void MeasuresSettingsModel::updateAllSystemsAreLocked()
{
    std::vector<System*> systems = currentNotation()->elements()->msScore()->selection().selectedSystems();

    bool allLocked = true;
    for (System* system : systems) {
        if (!system->isLocked()) {
            allLocked = false;
            break;
        }
    }

    if (m_allSystemsAreLocked != allLocked) {
        m_allSystemsAreLocked = allLocked;
        emit allSystemsAreLockedChanged(m_allSystemsAreLocked);
    }
}

bool MeasuresSettingsModel::scoreIsInPageView() const
{
    return m_scoreIsInPageView;
}

bool MeasuresSettingsModel::isMakeIntoSystemAvailable() const
{
    return m_isMakeIntoSystemAvailable;
}

void MeasuresSettingsModel::updateScoreIsInPageView()
{
    const Score* score = currentNotation()->elements()->msScore();
    bool isInPageView = score->layoutMode() == LayoutMode::PAGE;

    if (m_scoreIsInPageView != isInPageView) {
        m_scoreIsInPageView = isInPageView;
        emit scoreIsInPageViewChanged(m_scoreIsInPageView);
    }
}

void MeasuresSettingsModel::updateIsMakeIntoSystemAvailable()
{
    const Selection& selection = currentNotation()->elements()->msScore()->selection();
    const MeasureBase* startMB = selection.startMeasureBase();
    const MeasureBase* endMB = selection.endMeasureBase();

    bool available = true;
    if (startMB->isStartOfSystemLock() && endMB->isEndOfSystemLock() && startMB->systemLock() == endMB->systemLock()) {
        available = false;
    }

    if (m_isMakeIntoSystemAvailable != available) {
        m_isMakeIntoSystemAvailable = available;
        emit isMakeIntoSystemAvailableChanged(m_isMakeIntoSystemAvailable);
    }
}

void MeasuresSettingsModel::makeIntoSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->makeIntoSystem();
}

QString MeasuresSettingsModel::shortcutMakeIntoSystem() const
{
    return shortcutsForActionCode("make-into-system");
}
