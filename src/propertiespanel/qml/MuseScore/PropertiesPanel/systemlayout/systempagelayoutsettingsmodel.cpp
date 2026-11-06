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

#include "systempagelayoutsettingsmodel.h"

#include "global/translation.h"

#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"

#include "engraving/dom/score.h"
#include "engraving/dom/page.h"

#include "notationscene/notationcommands.h"

using namespace mu::propertiespanel;
using namespace mu::notation;
using namespace mu::engraving;

static const std::vector<muse::rcommand::Command> SYSTEM_PAGE_COMMANDS = {
    TOGGLE_SYSTEM_LOCK_COMMAND,
    MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND,
    MAKE_INTO_SYSTEM_COMMAND,
    TOGGLE_PAGE_LOCK_COMMAND,
    MOVE_SYSTEM_TO_PREV_PAGE_COMMAND,
    MAKE_INTO_PAGE_COMMAND
};

SystemPageLayoutSettingsModel::SystemPageLayoutSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                             IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setSectionType(PropertiesPanelSectionType::SECTION_SYSTEM_PAGE_LAYOUT);
    setTitle(muse::qtrc("propertiespanel", "System & page layout"));

    for (const muse::rcommand::Command& command : SYSTEM_PAGE_COMMANDS) {
        const bool enabled = commandsState()->commandState(command).enabled;
        m_commandsEnabledMap.emplace(command, enabled);
    }

    commandsState()->commandStateChanged().onReceive(this, [this](const muse::rcommand::Command& command,
                                                                  const muse::rcommand::CommandState& state) {
        auto it = m_commandsEnabledMap.find(command);
        if (it == m_commandsEnabledMap.end() || it->second == state.enabled) {
            return;
        }
        it->second = state.enabled;
        emit commandStatesChanged();
    });
}

void SystemPageLayoutSettingsModel::loadProperties()
{
    updateAllSystemsAreLocked();
    updateAllPagesAreLocked();
    updateScoreIsInPageView();

    const QSet<BoxType> boxTypes = boxTypesInSelection();
    updateIsBoxOnlySelection(boxTypes);
    updateIsSystemTabAvailable(boxTypes);

    updateSystemCount();
    updatePageCount();
}

bool SystemPageLayoutSettingsModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    return true;
}

void SystemPageLayoutSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet&)
{
    loadProperties();
}

void SystemPageLayoutSettingsModel::moveSystemDownPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveSystemToNextPage();
}

void SystemPageLayoutSettingsModel::moveSystemUpPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveSystemToPrevPage();
}

QString SystemPageLayoutSettingsModel::shortcutMoveSystemUpPage() const
{
    return shortcutsForCommand(MOVE_SYSTEM_TO_PREV_PAGE_COMMAND);
}

QString SystemPageLayoutSettingsModel::shortcutMoveSystemDownPage() const
{
    return shortcutsForCommand(MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND);
}

QString SystemPageLayoutSettingsModel::shortcutMakeIntoPage() const
{
    return shortcutsForCommand(MAKE_INTO_PAGE_COMMAND);
}

void SystemPageLayoutSettingsModel::makeIntoPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->makeIntoPage();
}

bool SystemPageLayoutSettingsModel::isEmpty() const
{
    INotationSelectionPtr selection = this->selection();
    if (!selection) {
        return true;
    }
    if (selection->isRange()) {
        return false;
    }
    //! NOTE: element will be null for a list selection. This is fine though since (for
    //! now) the actions in this model are incompatible with a list of elements...
    const EngravingItem* element = selection->element();
    return !element || !element->isBox();
}

void SystemPageLayoutSettingsModel::moveMeasureUpSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToPrevSystem();
}

QString SystemPageLayoutSettingsModel::shortcutMoveMeasureUpSystem() const
{
    return shortcutsForCommand(MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND);
}

void SystemPageLayoutSettingsModel::moveMeasureDownSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToNextSystem();
}

QString SystemPageLayoutSettingsModel::shortcutMoveMeasureDownSystem() const
{
    return shortcutsForCommand(MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND);
}

void SystemPageLayoutSettingsModel::toggleSystemLock()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->toggleSystemLock();
}

QString SystemPageLayoutSettingsModel::shortcutToggleSystemLock() const
{
    return shortcutsForCommand(TOGGLE_SYSTEM_LOCK_COMMAND);
}

void SystemPageLayoutSettingsModel::togglePageLock()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->togglePageLock();
}

QString SystemPageLayoutSettingsModel::shortcutTogglePageLock() const
{
    return shortcutsForCommand(TOGGLE_PAGE_LOCK_COMMAND);
}

bool SystemPageLayoutSettingsModel::allSystemsAreLocked() const
{
    return m_allSystemsAreLocked;
}

bool SystemPageLayoutSettingsModel::allPagesAreLocked() const
{
    return m_allPagesAreLocked;
}

void SystemPageLayoutSettingsModel::updateAllSystemsAreLocked()
{
    if (isEmpty()) {
        return;
    }

    std::vector<System*> systems = selection()->selectedSystems();

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

void SystemPageLayoutSettingsModel::updateAllPagesAreLocked()
{
    if (isEmpty()) {
        return;
    }

    std::vector<Page*> pages = selection()->pagesContainingSelection();

    bool allLocked = true;
    for (Page* page : pages) {
        if (!page->isLocked()) {
            allLocked = false;
            break;
        }
    }

    if (m_allPagesAreLocked != allLocked) {
        m_allPagesAreLocked = allLocked;
        emit allPagesAreLockedChanged(m_allPagesAreLocked);
    }
}

bool SystemPageLayoutSettingsModel::scoreIsInPageView() const
{
    return m_scoreIsInPageView;
}

bool SystemPageLayoutSettingsModel::isBoxOnlySelection() const
{
    return m_isBoxOnlySelection;
}

bool SystemPageLayoutSettingsModel::isSystemTabAvailable() const
{
    return m_isSystemTabAvailable;
}

int SystemPageLayoutSettingsModel::systemCount() const
{
    return static_cast<int>(m_systemCount);
}

int SystemPageLayoutSettingsModel::pageCount() const
{
    return static_cast<int>(m_pageCount);
}

bool SystemPageLayoutSettingsModel::isToggleSystemLockEnabled() const
{
    return muse::value(m_commandsEnabledMap, TOGGLE_SYSTEM_LOCK_COMMAND);
}

bool SystemPageLayoutSettingsModel::isMoveNextPrevSystemEnabled() const
{
    return muse::value(m_commandsEnabledMap, MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND);
}

bool SystemPageLayoutSettingsModel::isMakeIntoSystemEnabled() const
{
    return muse::value(m_commandsEnabledMap, MAKE_INTO_SYSTEM_COMMAND);
}

bool SystemPageLayoutSettingsModel::isTogglePageLockEnabled() const
{
    return muse::value(m_commandsEnabledMap, TOGGLE_PAGE_LOCK_COMMAND);
}

bool SystemPageLayoutSettingsModel::isMoveNextPrevPageEnabled() const
{
    return muse::value(m_commandsEnabledMap, MOVE_SYSTEM_TO_PREV_PAGE_COMMAND);
}

bool SystemPageLayoutSettingsModel::isMakeIntoPageEnabled() const
{
    return muse::value(m_commandsEnabledMap, MAKE_INTO_PAGE_COMMAND);
}

void SystemPageLayoutSettingsModel::updateScoreIsInPageView()
{
    if (!currentNotation()) {
        return;
    }

    bool isInPageView = currentNotation()->viewMode() != LayoutMode::LINE;

    if (m_scoreIsInPageView != isInPageView) {
        m_scoreIsInPageView = isInPageView;
        emit scoreIsInPageViewChanged(m_scoreIsInPageView);
    }
}

void SystemPageLayoutSettingsModel::updateIsBoxOnlySelection(const QSet<BoxType>& boxTypes)
{
    const bool boxOnlySelection = !isEmpty() && !boxTypes.contains(BoxType::Measure);
    if (m_isBoxOnlySelection != boxOnlySelection) {
        m_isBoxOnlySelection = boxOnlySelection;
        emit isBoxOnlySelectionChanged(m_isBoxOnlySelection);
    }
}

void SystemPageLayoutSettingsModel::updateIsSystemTabAvailable(const QSet<BoxType>& boxTypes)
{
    const bool available = boxTypes.contains(BoxType::Measure)
                           || boxTypes.contains(BoxType::Horizontal);

    if (m_isSystemTabAvailable != available) {
        m_isSystemTabAvailable = available;
        emit isSystemTabAvailableChanged(m_isSystemTabAvailable);
    }
}

QSet<BoxType> SystemPageLayoutSettingsModel::boxTypesInSelection() const
{
    QSet<BoxType> boxTypes;

    if (isEmpty()) {
        return boxTypes;
    }

    const auto checkBoxType = [&boxTypes](const EngravingItem* item) {
        if (item->isMeasure()) {
            boxTypes.insert(BoxType::Measure);
        } else if (item->isHBox()) {
            boxTypes.insert(BoxType::Horizontal);
        } else if (item->isVBoxBase()) {
            // Slight hack - for these purposes we'll take "Vertical" to mean VBoxBase...
            boxTypes.insert(BoxType::Vertical);
        }
    };

    if (selection()->isRange()) {
        for (const MeasureBase* mb : selection()->measureBasesInSelection()) {
            checkBoxType(mb);
        }
    } else {
        checkBoxType(selection()->element());
    }

    return boxTypes;
}

void SystemPageLayoutSettingsModel::updateSystemCount()
{
    if (isEmpty()) {
        return;
    }

    size_t count = selection()->selectedSystems().size();
    if (count != m_systemCount) {
        m_systemCount = count;
        emit systemCountChanged(static_cast<int>(count));
    }
}

void SystemPageLayoutSettingsModel::updatePageCount()
{
    if (isEmpty()) {
        return;
    }

    size_t count = selection()->pagesContainingSelection().size();
    if (count != m_pageCount) {
        m_pageCount = count;
        emit pageCountChanged(static_cast<int>(count));
    }
}

void SystemPageLayoutSettingsModel::makeIntoSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->makeIntoSystem();
}

QString SystemPageLayoutSettingsModel::shortcutMakeIntoSystem() const
{
    return shortcutsForCommand(MAKE_INTO_SYSTEM_COMMAND);
}
