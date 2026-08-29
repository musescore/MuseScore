/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "notationautomation.h"

#include "engraving/dom/masterscore.h"
#include "engraving/editing/transaction/transaction.h"

#include "translation.h"
#include "log.h"

using namespace mu::notation;

NotationAutomation::NotationAutomation(INotationUndoStackPtr undoStack)
    : m_undoStack(std::move(undoStack))
{
}

bool NotationAutomation::isAutomationModeEnabled() const
{
    return m_isAutomationModeEnabled;
}

void NotationAutomation::setAutomationModeEnabled(bool enabled)
{
    if (m_isAutomationModeEnabled == enabled) {
        return;
    }
    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

AutomationDataConstPtr NotationAutomation::automationData() const
{
    return m_masterScore ? m_masterScore->automationData() : nullptr;
}

void NotationAutomation::editPoints(const AutomationCurveKey& key, AutomationPointEdits& edits)
{
    IF_ASSERT_FAILED(m_masterScore && m_undoStack) {
        return;
    }

    m_undoStack->transaction(muse::TranslatableString("undoableAction", "Edit automation points"),
                             [&](engraving::Transaction&) {
        m_masterScore->editAutomationPoints(key, edits);
    });
}

void NotationAutomation::setMasterScore(engraving::MasterScore* masterScore)
{
    m_masterScore = masterScore;
}
