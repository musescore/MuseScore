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

SystemPageLayoutSettingsModel::SystemPageLayoutSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                             IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setSectionType(PropertiesPanelSectionType::SECTION_SYSTEM_PAGE_LAYOUT);
    setTitle(muse::qtrc("propertiespanel", "System & page layout"));
}

void SystemPageLayoutSettingsModel::loadProperties()
{
    updateAllSystemsAreLocked();
    updateAllPagesAreLocked();
    updateScoreIsInPageView();

    const QSet<BoxType> boxTypes = boxTypesInSelection();
    updateIsBoxOnlySelection(boxTypes);
    updateIsSystemTabAvailable(boxTypes);
    updateIsMakeIntoSystemAvailable(boxTypes);
    updateIsMakeIntoPageAvailable(boxTypes);

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

bool SystemPageLayoutSettingsModel::isMakeIntoSystemAvailable() const
{
    return m_isMakeIntoSystemAvailable;
}

bool SystemPageLayoutSettingsModel::isMakeIntoPageAvailable() const
{
    return m_isMakeIntoPageAvailable;
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

void SystemPageLayoutSettingsModel::updateIsMakeIntoSystemAvailable(const QSet<BoxType>& boxTypes)
{
    if (isEmpty()) {
        return;
    }

    const MeasureBase* startMB = selection()->startMeasureBase();
    const MeasureBase* endMB = selection()->endMeasureBase();
    if (!startMB || !endMB) {
        return;
    }

    const bool selectionHasMeasures = boxTypes.contains(BoxType::Measure);
    const bool systemAlreadyExists = startMB->isStartOfSystemLock() && endMB->isEndOfSystemLock()
                                     && startMB->systemLock() == endMB->systemLock();

    const bool available = selectionHasMeasures && !systemAlreadyExists;

    if (m_isMakeIntoSystemAvailable != available) {
        m_isMakeIntoSystemAvailable = available;
        emit isMakeIntoSystemAvailableChanged(m_isMakeIntoSystemAvailable);
    }
}

void SystemPageLayoutSettingsModel::updateIsMakeIntoPageAvailable(const QSet<BoxType>& boxTypes)
{
    const bool available = boxTypes.contains(BoxType::Measure);

    if (m_isMakeIntoPageAvailable != available) {
        m_isMakeIntoPageAvailable = available;
        emit isMakeIntoPageAvailableChanged(m_isMakeIntoPageAvailable);
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

    for (const MeasureBase* mb : selection()->measureBasesInSelection()) {
        if (mb->isMeasure()) {
            boxTypes.insert(BoxType::Measure);
        } else if (mb->isHBox()) {
            boxTypes.insert(BoxType::Horizontal);
        } else if (mb->isVBoxBase()) {
            // Slight hack - for these purposes we'll take "Vertical" to mean VBoxBase...
            boxTypes.insert(BoxType::Vertical);
        }
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
