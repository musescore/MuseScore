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
#include "measuressettingsmodel.h"

#include "engraving/dom/score.h"
#include "engraving/dom/page.h"

#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationselection.h"

#include "translation.h"

using namespace mu::propertiespanel;
using namespace mu::notation;
using namespace muse::actions;
using namespace mu::engraving;

MeasuresSettingsModel::MeasuresSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                             IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setSectionType(PropertiesPanelSectionType::SECTION_MEASURES);
    setTitle(muse::qtrc("propertiespanel", "Measures"));
}

void MeasuresSettingsModel::loadProperties()
{
    updateAllSystemsAreLocked();
    updateAllPagesAreLocked();
    updateScoreIsInPageView();
    updateIsMakeIntoSystemAvailable();
    updateIsMakeIntoPageAvailable();
    updateSystemCount();
    updatePageCount();
}

bool MeasuresSettingsModel::shouldUpdateOnEmptyPropertyAndStyleIdSets() const
{
    return true;
}

void MeasuresSettingsModel::onNotationChanged(const engraving::PropertyIdSet&, const engraving::StyleIdSet&)
{
    loadProperties();
}

void MeasuresSettingsModel::moveMeasureDownPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToNextPage();
}

void MeasuresSettingsModel::moveMeasureUpPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToPrevPage();
}

QString MeasuresSettingsModel::shortcutMoveMeasureUpPage() const
{
    return shortcutsForActionCode("move-measure-to-prev-page");
}

QString MeasuresSettingsModel::shortcutMoveMeasureDownPage() const
{
    return shortcutsForActionCode("move-measure-to-next-page");
}

QString MeasuresSettingsModel::shortcutMakeIntoPage() const
{
    return shortcutsForActionCode("make-into-page");
}

void MeasuresSettingsModel::makeIntoPage()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->makeIntoPage();
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

void MeasuresSettingsModel::moveMeasureUpSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToPrevSystem();
}

QString MeasuresSettingsModel::shortcutMoveMeasureUpSystem() const
{
    return shortcutsForActionCode("move-measure-to-prev-system");
}

void MeasuresSettingsModel::moveMeasureDownSystem()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->moveMeasureToNextSystem();
}

QString MeasuresSettingsModel::shortcutMoveMeasureDownSystem() const
{
    return shortcutsForActionCode("move-measure-to-next-system");
}

void MeasuresSettingsModel::toggleSystemLock()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->toggleSystemLock();
}

QString MeasuresSettingsModel::shortcutToggleSystemLock() const
{
    return shortcutsForActionCode("toggle-system-lock");
}

void MeasuresSettingsModel::togglePageLock()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->interaction()->togglePageLock();
}

QString MeasuresSettingsModel::shortcutTogglePageLock() const
{
    return shortcutsForActionCode("toggle-page-lock");
}

bool MeasuresSettingsModel::allSystemsAreLocked() const
{
    return m_allSystemsAreLocked;
}

bool MeasuresSettingsModel::allPagesAreLocked() const
{
    return m_allPagesAreLocked;
}

void MeasuresSettingsModel::updateAllSystemsAreLocked()
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

void MeasuresSettingsModel::updateAllPagesAreLocked()
{
    if (isEmpty()) {
        return;
    }

    std::vector<Page*> pages = selection()->selectedPages();

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

bool MeasuresSettingsModel::scoreIsInPageView() const
{
    return m_scoreIsInPageView;
}

bool MeasuresSettingsModel::isMakeIntoSystemAvailable() const
{
    return m_isMakeIntoSystemAvailable;
}

bool MeasuresSettingsModel::isMakeIntoPageAvailable() const
{
    return m_isMakeIntoPageAvailable;
}

int MeasuresSettingsModel::systemCount() const
{
    return static_cast<int>(m_systemCount);
}

int MeasuresSettingsModel::pageCount() const
{
    return static_cast<int>(m_pageCount);
}

void MeasuresSettingsModel::updateScoreIsInPageView()
{
    bool isInPageView = currentNotation()->viewMode() != LayoutMode::LINE;

    if (m_scoreIsInPageView != isInPageView) {
        m_scoreIsInPageView = isInPageView;
        emit scoreIsInPageViewChanged(m_scoreIsInPageView);
    }
}

void MeasuresSettingsModel::updateIsMakeIntoSystemAvailable()
{
    if (isEmpty()) {
        return;
    }

    const MeasureBase* startMB = selection()->startMeasureBase();
    const MeasureBase* endMB = selection()->endMeasureBase();
    if (!startMB || !endMB) {
        return;
    }

    bool available = true;
    if (startMB->isStartOfSystemLock() && endMB->isEndOfSystemLock() && startMB->systemLock() == endMB->systemLock()) {
        available = false;
    }

    if (m_isMakeIntoSystemAvailable != available) {
        m_isMakeIntoSystemAvailable = available;
        emit isMakeIntoSystemAvailableChanged(m_isMakeIntoSystemAvailable);
    }
}

void MeasuresSettingsModel::updateIsMakeIntoPageAvailable()
{
    bool available = !isEmpty();

    if (m_isMakeIntoPageAvailable != available) {
        m_isMakeIntoPageAvailable = available;
        emit isMakeIntoPageAvailableChanged(m_isMakeIntoPageAvailable);
    }
}

void MeasuresSettingsModel::updateSystemCount()
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

void MeasuresSettingsModel::updatePageCount()
{
    if (isEmpty()) {
        return;
    }

    size_t count = selection()->selectedPages().size();
    if (count != m_pageCount) {
        m_pageCount = count;
        emit pageCountChanged(static_cast<int>(count));
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
