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
#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"

using namespace mu::notation;

NotationAutomation::NotationAutomation(IGetScore* getScore,
                                       muse::async::Channel<muse::RectF> notationChanged)
    : m_getScore(getScore), m_notationChanged(notationChanged)
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

    if (enabled) { // TODO: Placeholder - need to init this somewhere...
        if (score() && score()->masterScore()) {
            score()->masterScore()->initAutomation();
        } else {
            ASSERT_X("No score for automation...")
        }
    }

    m_isAutomationModeEnabled = enabled;
    m_automationModeEnabledChanged.notify();
    m_automationLinesDataChanged.notify(); // TODO: Delete once above placeholder has been removed...
}

muse::async::Notification NotationAutomation::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

QVariant NotationAutomation::automationLinesData() const
{
    QVariantList automationLinesData;

    IF_ASSERT_FAILED(automation()) {
        return automationLinesData;
    }

    for (const System* system : score()->systems()) {
        const QVariantList linesData = linesDataForSystem(system);
        if (!linesData.empty()) {
            automationLinesData << linesData;
        }
    }

    return automationLinesData;
}

QVariantList NotationAutomation::linesDataForSystem(const System* system) const
{
    QVariantList lines;

    const int systemStartTick = system->first()->tick().ticks();
    const int systemEndTick = system->last()->endTick().ticks();

    staff_idx_t staffIdx = system->firstVisibleStaff();
    while (staffIdx != muse::nidx) {
        const Staff* staff = score()->staff(staffIdx);
        const SysStaff* sysStaff = system->staff(staffIdx);
        IF_ASSERT_FAILED(staff && sysStaff) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }
        if (!staff->isPrimaryStaff()) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        QVariantList pointsOnLine;

        const mu::engraving::AutomationCurveKey key { mu::engraving::AutomationType::Dynamics, staff->id(), std::nullopt };
        for (auto point : automation()->curve(key)) {
            const int tick = point.first;
            if (tick < systemStartTick || tick > systemEndTick) {
                continue;
            }

            const mu::engraving::AutomationPoint& autoPoint = point.second;

            QVariantMap pointData;

            // TODO: xFactor is a placeholder - it assumes time is linear in a staff - which is not the case. We should
            // instead base this on segment/timetick positions...
            const double xFactor = static_cast<double>(tick - systemStartTick) / (systemEndTick - systemStartTick);
            pointData["x"] = xFactor;

            pointData["y"] = autoPoint.inValue;
            pointsOnLine << pointData;
        }

        if (pointsOnLine.isEmpty()) {
            staffIdx = system->nextVisibleStaff(staffIdx);
            continue;
        }

        const muse::RectF staffRect = sysStaff->bbox().translated(system->canvasPos());

        QVariantMap lineData;
        lineData["x"] = staffRect.x();
        lineData["y"] = staffRect.y();
        lineData["width"] = staffRect.width();
        lineData["height"] = staffRect.height();
        lineData["points"] = pointsOnLine;

        lines << lineData;

        staffIdx = system->nextVisibleStaff(staffIdx);
    }

    return lines;
}

muse::async::Notification NotationAutomation::automationLinesDataChanged() const
{
    return m_automationLinesDataChanged;
}

mu::engraving::Score* NotationAutomation::score() const
{
    return m_getScore ? m_getScore->score() : nullptr;
}

mu::engraving::IAutomation* NotationAutomation::automation() const
{
    return score() ? score()->automation() : nullptr;
}
